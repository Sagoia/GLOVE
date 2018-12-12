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
 *  @file       Text.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      plain text search
 *
 */
#include "Patch.h"
#include "utils/glLogger.h"

#include <iostream>
#include <cstring>
#include <cassert>

std::ostream &operator<<(std::ostream &OS, const Location& L) {
    return OS << L.Line << ":" << L.Column;
}

void Text::init() {
    // scan whole Src to get width of each row
    size_t I {};
    for (const auto& C : Src) {
        // `\n' is counted
        ++I;
        if (C == '\n') {
            Width.push_back(I);
            I = 0;
        }
    }
}

// Build a StringRef [From, To)
StringRef Text::Segment(Location From, Location To) const {
    FUN_ENTRY(GL_LOG_TRACE);

    if (To < From) {
        std::cerr << "Illegal segment: (" << From << ", " << To << ")\n";
        ::abort();
    }
    if (To == From) {
        return StringRef();
    }

    size_t Start = index(From);
    size_t End = index(To);
    return StringRef(Src.data() + Start, End - Start);
}

Location Text::location(size_t index) const {
    size_t Len = 0, Line = 1;
    for (auto W : Width) {
        Len += W;
        if (Len > index) {
            size_t Col = W - (Len - index) + 1;
            return Location{Line, Col};
        }
        ++Line;
    }
    return Location{};
}

Location Text::FindLocation(const char *Needle, Location From, bool Fwd) const {
    FUN_ENTRY(GL_LOG_DEBUG);

    auto Index = index(From);
    const char *P = Src.data() + Index;
    const char *Begin = Src.data();
    const char *End = Src.data() + Src.length();
    const auto Len = ::strlen(Needle);
    const int Step = Fwd ? 1 : -1;

    // The Needle is usually small, and usually stays very
    // close to From. Use naive search.
    while (::memcmp(P, Needle, Len) != 0) {
        if (P == Begin || P == End) {
            return Location {};
        }
        P += Step;
    }
    return location(P - Begin);
}
