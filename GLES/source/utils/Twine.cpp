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
 *  @file       Twine.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      LLVM's Twine. Simplified.
 *
 */
#include "Twine.h"

Twine Twine::concat(const Twine& Suffix) const {
    Child NewLHS, NewRHS;
    NewLHS.twine = this;
    NewRHS.twine = &Suffix;
    NodeKind NewLHSKind = TwineKind, NewRHSKind = TwineKind;
    if (isUnary()) {
        NewLHS = LHS;
        NewLHSKind = LHSKind;
    }
    if (Suffix.isUnary()) {
        NewRHS = Suffix.LHS;
        NewRHSKind = Suffix.LHSKind;
    }
    return Twine(NewLHS, NewLHSKind, NewRHS, NewRHSKind);
}

std::string Twine::str() const {
    std::ostringstream OS;
    print(OS);
    return OS.str();
}
