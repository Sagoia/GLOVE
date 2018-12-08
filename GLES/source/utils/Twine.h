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
 *  @file       Twine.h
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      LLVM's Twine. Simplified.
 *
 */
#ifndef _TWINE_H
#define _TWINE_H

#include <sstream>
#include <string>

#include <cstring>
#include <cassert>

class StringRef {
public:
    using const_iterator = const char *;
    using size_type = size_t;

    StringRef() = default;

    StringRef(std::nullptr_t) = delete;

    StringRef(const char *data, size_t length) :
        Data(data), Length(length) {}

    StringRef(const std::string& Str) :
        Data(Str.data()), Length(Str.length()) {}

    StringRef(const char *str) :
        Data(str), Length(::strlen(str)) {}

    const_iterator begin() const { return Data; }
    const_iterator end() const { return Data + Length; }

    template <typename T>
    typename std::enable_if<std::is_same<T, std::string>::value,
                            StringRef>::type &
    operator=(T &&Str) = delete;

    char operator[](size_t Index) const {
        assert(Index < Length && "Invalid index!");
        return Data[Index];
    }

    const char *data() const { return Data; }
    size_t size() const { return Length; }
    size_t length() const { return Length; }
    bool empty() const { return Data == nullptr; }

    std::string str() const {
        if (!Data) {
            return std::string();
        }
        return std::string(Data, Length);
    }

private:
    const char *Data = nullptr;
    size_t Length = 0;
};

class Twine {
    enum NodeKind {
        NullKind,
        StringRefKind,
        TwineKind
    };

    union Child {
        const Twine *twine;
        const StringRef *ref;
    };

    Child LHS;
    Child RHS;

    NodeKind LHSKind = NullKind;
    NodeKind RHSKind = NullKind;

    explicit Twine(const Twine& LHS, const Twine& RHS)
        : LHSKind(TwineKind), RHSKind(TwineKind) {
        this->LHS.twine = &LHS;
        this->RHS.twine = &RHS;
    }

    explicit Twine(Child LHS, NodeKind LHSKind, Child RHS, NodeKind RHSKind)
                : LHS(LHS), RHS(RHS), LHSKind(LHSKind), RHSKind(RHSKind) {}


    bool isUnary() const {
        return RHSKind == NullKind;
    }

    void printChild(std::ostringstream& OS, Child C, NodeKind Kind) const {
        if (Kind == StringRefKind) {
            OS << C.ref->str();
        } else if (Kind == TwineKind) {
            C.twine->print(OS);
        }
    }

    void print(std::ostringstream& OS) const {
        printChild(OS, LHS, LHSKind);
        printChild(OS, RHS, RHSKind);
    }

public:
    Twine() {}
    Twine(const Twine&) = default;

    Twine(const StringRef& Str) : LHSKind(StringRefKind) {
        LHS.ref = &Str;
    }

    Twine &operator=(const Twine&) = delete;
    Twine concat(const Twine& Suffix) const;
    std::string str() const;

};

inline Twine operator+(const Twine& LHS, const Twine& RHS) {
    return LHS.concat(RHS);
}

#endif
