/**
 * Copyright (C) 2015-2018 Think Silicon S.A. (https://think-silicon.com/)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public v3
 * License as published by the Free Software Foundation;
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 */

/**
 *  @file       FixSampler.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Fix sampler members in glsl struct
 *
 */

#include "FixSampler.h"

#include "utils/Twine.h"
#include "utils/Patch.h"
#include "utils/glLogger.h"

#include <set>
#include <vector>
#include <array>
#include <utility>
#include <sstream>
#include <ostream>
#include <tuple>
#include <algorithm>
#include <iostream>

using namespace glslang;

namespace {

class GLType;

struct GLMember {
    GLMember(const char* N, Location L, bool Sampler,
        size_t ASize, const GLType* Ty = nullptr) :
        Name(N), Loc(L), isSampler(Sampler), ArraySize(ASize), Type(Ty) {}

    StringRef Name;
    Location Loc;
    bool isSampler = false;
    size_t ArraySize = 0; // Only one dimension array is supported.
    const GLType* Type = nullptr;
    GLType* Parent = nullptr;
};

class GLType {
public:
    using const_iterator = std::vector<GLMember>::const_iterator;

    GLType() : Name() {}

    // Create a type for sampler.
    GLType(const char *N, int D) : Name(N), SamplerDim(D) {}

    GLType(const char* N) : Name(N) {}

    std::string GetName() const {
        return Name.str();
    }

    void Add(const GLMember& M) {
        Members.push_back(M);
        Members.back().Parent = this;
    }

    // Returns true if it is a sampler,
    // or all its members are sampler.
    bool isSampler() const {
        for (const auto& M : Members) {
            if (!M.isSampler) {
                if (!M.Type || !M.Type->isSampler()) {
                    return false;
                }
            }
        }
        return true;
    }

    const_iterator begin() const { return Members.begin(); }
    const_iterator end() const { return Members.end(); }

    const GLMember& operator[](size_t Index) const {
        return Members[Index];
    }

    const GLMember& front() const { return Members.front(); }
    const GLMember& back() const { return Members.back(); }
    size_t size() const { return Members.size(); }
    bool isEmpty() const { return Members.empty(); }

private:
    StringRef Name;
    int SamplerDim = 0;
    std::vector<GLMember> Members;
};

using GLTypeMap = std::map<std::string, GLType>;

GLType* CreateSamplerType(enum TSamplerDim Dim, GLTypeMap& GM) {
    static const char * const DimName[] = {
        "",              //EsdNone,
        "sampler1D",     //Esd1D,
        "sampler2D",     //Esd2D,
        "sampler3D",     //Esd3D,
        "samplerCube",   //EsdCube,
        "sampler2DRect", //EsdRect,
        "samplerBuffer", //EsdBuffer,
        "samplerInput",  //EsdSubpass,
        ""               //EsdNumDims
    };
    std::string SN = std::string(DimName[Dim]);
    auto It = GM.find(SN);
    if (It == GM.end()) {
        GM.insert(std::make_pair(SN, GLType(DimName[Dim], (int)Dim)));
    }
    return &GM[SN];
}

GLMember WalkAccess(const TType* TT, Location Loc, GLTypeMap& GM) {
    FUN_ENTRY(GL_LOG_TRACE);

    bool isSampler = TT->getBasicType() == EbtSampler;
    const auto* ST = TT->getStruct();
    size_t Array = 0;

    if (TT->isArray()) {
        const auto* AS = TT->getArraySizes();
        Array = AS->getDimSize(0); // One dimension array assumed.
    }

    if (!ST) {
        GLType *GT = nullptr;
        if (isSampler) {
            GT = CreateSamplerType(TT->getSampler().dim, GM);
        }
        return GLMember(TT->getFieldName().c_str(), Loc, isSampler, Array, GT);
    }

    std::string SN {TT->getTypeName().c_str()}; // struct name
    auto It = GM.find(SN);
    if (It == GM.end()) {
        GM.insert(std::make_pair(SN, GLType(TT->getTypeName().c_str())));
        auto& GT = GM[SN];
        for (const auto& TL : *ST) {
            GT.Add(WalkAccess(TL.type, Location(TL.loc.line, TL.loc.column), GM));
        }
    }
    const auto& GT = GM[SN];
    return GLMember(TT->getFieldName().c_str(), Loc, isSampler, Array, &GT);
}


std::string GetQualifier(const TQualifier& qualifier) {
    FUN_ENTRY(GL_LOG_TRACE);

    std::ostringstream OS;

    if (qualifier.hasLayout()) {
        TQualifier noXfbBuffer = qualifier;
        noXfbBuffer.layoutXfbBuffer = TQualifier::layoutXfbBufferEnd;
        if (noXfbBuffer.hasLayout()) {
            OS << "layout(";
            if (qualifier.hasBinding()) {
                OS << "binding=" << std::to_string(qualifier.layoutBinding);
            }
            OS << ")";
        }
    }
    OS << " " << GetStorageQualifierString(qualifier.storage);
    return OS.str();
}

// Identification of a uniform sampler variable.
// It is identified by the location of the block definition,
// together with the location of the type definition.
struct SamplerKey {
    SamplerKey() = default;
    SamplerKey(Location B, Location T) :
        Block(B), Type(T) {}

    bool operator<(const SamplerKey& K) const {
        return std::tie(Block.Line, Block.Column, Type.Line, Type.Column) <
         std::tie(K.Block.Line, K.Block.Column, K.Type.Line, K.Type.Column);
    }

    Location Block;
    Location Type;
};

std::string MakeVarName(const SamplerKey &Key) {
    FUN_ENTRY(GL_LOG_TRACE);

    std::ostringstream OS;
    OS << "_SAM" << Key.Type.Line << Key.Type.Column
        << Key.Block.Line << Key.Block.Column;
    return OS.str();
}

// A symbol that references a sampler.
class SamplerVar {
public:
    static constexpr size_t AccessMax = 8;
    using AccessChain = std::array<size_t, AccessMax>;
    using const_iterator = AccessChain::const_iterator;

    SamplerVar() = default;
    SamplerVar(const SamplerKey& K,
        const GLType *Ty, size_t I,
        const TQualifier* Q, const GLType* BT) :
        Key(K), Type(Ty), Index(I), Qual(Q), BlockType(BT) {}

    bool SetIndices(const std::vector<size_t>& I) {
        assert(I.size() <= AccessMax);
        if (I.size() > AccessMax) {
            return true;
        }
        std::copy(I.begin(), I.end(), Indices.begin());
        ChainSize = I.size();
        return false; // false means success
    }

    std::string GetName() const {
        return MakeVarName(Key);
    }

    std::string QualifierStr() const {
        return GetQualifier(*Qual);
    }

    std::string SamplerType() const {
        return (*Type)[Index].Type->GetName();
    }

    const GLMember& SamplerMember() const {
        return (*Type)[Index];
    }

    const_iterator begin() const { return Indices.begin(); }
    const_iterator end() const { return Indices.begin() + ChainSize; }

    // If the original block is a pure sampler type
    bool isSampler() const {
        assert(BlockType);
        return BlockType->isSampler();
    }

private:
    // Set the key to get a fully qualified name for this var
    SamplerKey Key;

    // This is the enclosing struct type of the sampler
    const GLType* Type = nullptr;
    // The index of this sampler member in Type
    size_t Index = 0;
    const TQualifier* Qual = nullptr;
    // The type of the block
    const GLType* BlockType = nullptr;

    size_t ChainSize = 0;
    AccessChain Indices;
};

// This class has 3 roles:
// 1. Replace symbols that reference to sampler
// 2. Delete a sampler member, or a sampler struct
// 3. Add a new uniform definition of block
class SamplerPatch : public Expr {
public:
    enum PatchKind {
        InvalidKind,
        SymbolKind,  // Replace a symbol
        BlockKind,   // Add a block
        RemoveKind   // Remove a segment
    };

    SamplerPatch() :
        Expr(), Var{}, Kind(InvalidKind), Indices{} {}
    SamplerPatch(PatchKind K) :
        Expr(), Var{}, Kind(K), Indices{} {}
    SamplerPatch(const SamplerVar *V, PatchKind K) :
        Expr(), Var(V), Kind(K), Indices{} {}
    virtual ~SamplerPatch() = default;

    std::string Lit() const override;

    bool SetIndices(const std::vector<size_t>& I) {
        assert(I.size() <= AccessMax);
        if (I.size() > AccessMax) {
            return true;
        }
        std::copy(I.begin(), I.end(), Indices.begin());
        ChainSize = I.size();
        return false; // false means success
    }

private:
    static constexpr size_t AccessMax = 8;
    using AccessChain = std::array<size_t, AccessMax>;
    using const_iterator = AccessChain::const_iterator;

    const_iterator begin() const { return Indices.begin(); }
    const_iterator end() const { return Indices.begin() + ChainSize; }

    const SamplerVar *Var;
    PatchKind Kind;

    size_t ChainSize = 0;
    AccessChain Indices;
};

std::string SamplerPatch::Lit() const {
    assert(Kind != InvalidKind);

    // This is a deletion
    if (Kind == RemoveKind) {
        return std::string();
    }

    std::string VName = Var->GetName();
    std::ostringstream OS;

    // Replace a reference
    if (Kind == SymbolKind) {
        OS << VName;
        for (auto I : *this) {
            OS << "[" << I << "]";
        }
        return OS.str();
    }

    // Generate uniform interface block definition.
    OS << Var->QualifierStr() << " " << Var->SamplerType()
        << " " << VName;
    // Add array declr.
    for (auto I : *Var) {
        OS << "[" << I << "]";
    }
    OS << ";\n";
    return OS.str();
}


using SamplerMap = std::map<SamplerKey, SamplerVar>;

SamplerVar& CreateVariable(SamplerMap& Map,
        const SamplerKey& Key,
        const GLType* Ty,
        size_t Index,
        const TQualifier *Qual,
        const GLType* BTy) {
    FUN_ENTRY(GL_LOG_TRACE);

    auto It = Map.find(Key);
    if (It == Map.end()) {
        // Create symbol name for uniform variable
        Map.insert({Key, SamplerVar(Key, Ty, Index, Qual, BTy)});
    }
    return Map[Key];
}

const GLType* FindSamplerType(const GLTypeMap& GM, const TType *TT) {
    FUN_ENTRY(GL_LOG_TRACE);
    const auto It = GM.find(std::string(TT->getTypeName().c_str()));
    assert(It != GM.end());
    return &It->second;
}

} // anon namespace


std::string
FixSamplers(const AccessChainList& samplers, std::string& source)
{
    FUN_ENTRY(GL_LOG_DEBUG);
    // The source is at version 400.
    // Comment out sampler members.
    // Create new uniform for each sampler member.
    // Update uses.

    // Does not support:
    // 1. Array inside declr of struct.
    // 2. Nested array either in struct or in interface block.

    // Collect all user defined types that are referenced by interface block
    GLTypeMap GM;
    for (const auto& AC : samplers) {
        const auto& Block = AC.back();
        const auto& Members = *Block.typeloc.type->getStruct();
        // The glslang type of Block has only one member.
        assert(Members.size() == 1);
        WalkAccess(Members[0].type, Location{}, GM);
    }

    SamplerMap Vars;
    std::vector<SamplerPatch> Patches {};

    for (const auto& AC : samplers) {
        const auto& Block = AC.back();
        const auto* Sampler = &AC.front();

        // Populate Access and Member indices, needed by Patch
        // and Var, respectively.
        std::vector<size_t> AccessIndices;
        std::vector<size_t> ArraySizes;
        for (const auto& A : AC) {
            const auto& TL = A.typeloc;
            if (TL.isArray) {
                AccessIndices.push_back(TL.index);
                const auto* Ty = TL.type;
                assert(Ty->isArray());
                // Support only one-dimension array.
                ArraySizes.push_back(Ty->getArraySizes()->getDimSize(0));
            }
        }

        std::reverse(std::begin(AccessIndices), std::end(AccessIndices));
        std::reverse(std::begin(ArraySizes), std::end(ArraySizes));

        // The Sampler may be an array of samplers. If this is the case,
        // we need to find its enclosing struct.
        while (!Sampler->typeloc.type->isStruct()) {
            assert(Sampler->typeloc.isArray);
            ++Sampler;
        }

        SamplerKey VK (Location(Block.typeloc.line, Block.typeloc.column),
            Location(Sampler->typeloc.line, Sampler->typeloc.column));

        // Then get the sampler type by which we can create a new uniform variable for E.
        // Note: This is the container of the sampler member.
        const GLType* GT = FindSamplerType(GM, Sampler->typeloc.type);
        assert(GT);

        const auto& BMembers = *Block.typeloc.type->getStruct();
        const GLType* BGT = FindSamplerType(GM, BMembers[0].type);
        assert(BGT);

        // Generate a uniq var for E.
        auto& V = CreateVariable(Vars, VK, GT,
            Sampler->typeloc.index,
            &Block.typeloc.type->getQualifier(),
            BGT);
        if (V.SetIndices(ArraySizes)) {
            return std::string();
        }

        // Create an expression for symbol reference
        SamplerPatch E(&V, SamplerPatch::SymbolKind);
        E.SetLocation(Location(Block.line, Block.column), Location(Sampler->line, Sampler->column));
        const auto& GMember = V.SamplerMember();
        assert(GMember.isSampler);
        E.End.Column += GMember.Name.length();
        // Check if this member is an array
        if (GMember.ArraySize) {
            E.End.Column += 2 + std::to_string(GMember.ArraySize).length(); // [X]
        }

        if (E.SetIndices(AccessIndices)) {
            return std::string();
        }

        // This expression is done
        Patches.push_back(E);
    }

    Text Txt {source};
    std::set<Location> RemovedBlocks;
    // Now just iterate Vars to create uniform sampler with type.
    // The results are added to Patches.
    for (const auto& KV : Vars) {
        // first is SamplerKey
        // second is SamplerVar
        const auto& K = KV.first;
        const auto& V = KV.second;

        // Find end of the block declr.
        Location TE = Txt.FindLocation("};", K.Block, true);
        TE.Column += 2;

        // Create uniform variables for sampler
        SamplerPatch EU (&V, SamplerPatch::BlockKind);
        EU.SetLocation(Location(TE.Line + 1, 1), Location(TE.Line + 1, 1));
        Patches.push_back(EU);

        // If the block is a pure sampler, remove the block.
        Location TB = Txt.FindLocation("layout(", K.Block, false);
        if (RemovedBlocks.count(TB) == 0) {
            if (V.isSampler()) {
                SamplerPatch E (SamplerPatch::RemoveKind);
                E.SetLocation(TB, TE);
                Patches.push_back(E);
                RemovedBlocks.insert(TB);
            }
        }
    }

    // If a struct consists of sampler members only, the whole struct
    // shall be removed.
    // If a struct contains some but not all sampler members, the
    // sampler members shall be removed.
    for (const auto& KV : GM) {
        const auto& GT = KV.second;
        if (GT.isEmpty()) {
            // This is a sampler type
            continue;
        }
        if (GT.isSampler()) {
            Location Hook = GT.front().Loc;
            Location TB = Txt.FindLocation("struct ", Hook, false);
            if (Hook != GT.back().Loc) {
                Hook = GT.back().Loc;
            }
            Location TE = Txt.FindLocation("};", Hook, true);
            TE.Column += 2;
            SamplerPatch E (SamplerPatch::RemoveKind);
            E.SetLocation(TB, TE);
            Patches.push_back(E);
        } else {
            int I = 0;
            for (const auto& M : GT) {
                if (M.isSampler) {
                    Location TB = Txt.FindLocation(I > 0 ? ";" : "{", M.Loc, false);
                    ++TB.Column;
                    Location TE = Txt.FindLocation(";", M.Loc, true);
                    ++TE.Column;
                    SamplerPatch E (SamplerPatch::RemoveKind);
                    E.SetLocation(TB, TE);
                    Patches.push_back(E);
                }
                ++I;
            }
        }
    }

    // And apply patchs to all expressions,
    // including newly added uniform definition,
    // and masked sampler members.
    // Note that the patches must be sorted,
    // becasue they are operated in lexical sequence.
    std::sort(Patches.begin(), Patches.end());
    Patch<decltype(Patches.cbegin())> Pat(Txt);

    return Pat.Apply(Patches.cbegin(), Patches.cend());
}
