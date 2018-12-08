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
 *  @file       Patch.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      Apply patches to plain text
 *
 */
#ifndef _PATCH_H_
#define _PATCH_H_

#include "Twine.h"
#include "utils/glLogger.h"

#include <string>
#include <vector>
#include <tuple>
#include <numeric>

// A Location defines Line and Column, which all start from 1
struct Location {
    Location() {}
    Location(size_t L, size_t C) : Line(L), Column(C) {}

    bool operator<(const Location& L) const {
        return std::tie(Line, Column) < std::tie(L.Line, L.Column);
    }

    bool operator==(const Location& L) const {
        return Line == L.Line && Column == L.Column;
    }

    bool operator!=(const Location& L) const {
        return !(*this == L);
    }

    size_t Line = 1;
    size_t Column = 1;
};

std::ostream &operator<<(std::ostream &OS, const Location& L);

// An Expr is an expression that within [Begin, End),
// i.e. a half-close-half-open region.
// It is used together with template class Patch.
// User should derive from Expr and apply it as the
// template argument for Patch.
struct Expr {
    Expr() : Begin{}, End{} {}
    virtual ~Expr() {}

    // Override this function to get different expression
    virtual std::string Lit() const { return std::string(); }

    void SetLocation(Location B, Location E) {
        Begin = B;
        End = E;
    }

    bool operator<(const Expr& L) const {
        return std::tie(Begin.Line, Begin.Column, End.Line, End.Column) <
            std::tie(L.Begin.Line, L.Begin.Column, L.End.Line, L.End.Column);
    }

    Location Begin;
    Location End;
};

// Note: Text is a stateless class once constructed.
class Text {
public:
    Text(const char *S) : Src(S) { init(); }
    Text(const std::string& S) : Src(S) { init(); }

    Location FindLocation(const char *Needle, Location From, bool Fwd) const;
    StringRef Segment(Location From, Location To) const;
    Location location(size_t index) const;

    size_t index(Location Curr) const {
        return Curr.Column - 1 +
            std::accumulate(Width.begin(),
                Width.begin() + Curr.Line - 1,
                size_t{});
    }

    StringRef src() const { return Src; }

private:
    void init();

    std::vector<size_t> Width;
    const StringRef Src;
};

template <typename T, typename SFINAN = void>
class Patch {};

// Note: Patch is a stateless class.
template <typename T>
class Patch<T, typename std::enable_if<std::is_base_of<Expr, T>::value>::type> {
public:
    Patch(const Text& TheTxt) : Txt{TheTxt} {}

    std::string Apply(const std::vector<T>& Exprs) const {
        FUN_ENTRY(GL_LOG_DEBUG);
        return DoApply(Location(), Twine(), Exprs.begin(), Exprs.end());
    }

private:
    using ExprIt = typename std::vector<T>::const_iterator;

    std::string DoApply(Location Curr, const Twine Root, ExprIt It, ExprIt End) const {
        if (It == End) {
            // Attach rest of Src
            auto Idx = Txt.index(Curr);
            auto D = Txt.src().data() + Idx;
            auto L = Txt.src().length() - Idx;
            return (Root + StringRef(D, L)).str();
        }

        const auto& E = *It++;
        auto S = Txt.Segment(Curr, E.Begin);
        // String for this expression is generated here to ensure
        // all strings stay on stack.
        return DoApply(E.End, Root + S + Twine(E.Lit()), It, End);
    }

    const Text &Txt;
};

#endif
