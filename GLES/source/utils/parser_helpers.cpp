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
 *  @file       parser_helpers.cpp
 *  @author     Think Silicon
 *  @date       25/07/2018
 *  @version    1.0
 *
 *  @brief      String parser helper functions
 *
 */

#include "parser_helpers.h"

const char *SAMPLER_TYPES[] =   { "sampler1D",
                                  "sampler2D",
                                  "sampler3D",
                                  "samplerCube",
                                  "sampler2DRect",
                                  "sampler1DArray",
                                  "sampler2DArray",
                                  "samplerCubeArray"
                                  "samplerBuffer",
                                  "sampler2DMS",
                                  "sampler2DMSArray",
                                  "samplerExternalOES" };

const char *IMAGE_TYPES[] =     { "image1D",
                                  "image2D",
                                  "image3D",
                                  "imageCube",
                                  "image2DRect",
                                  "image1DArray",
                                  "image2DArray",
                                  "imageCubeArray",
                                  "imageBuffer",
                                  "image2DMS",
                                  "image2DMSArray" };

inline bool
IsChar(char c)
{
    return ((c >= 'a' && c <= 'z') ||
            (c >= 'A' && c <= 'Z') ||
            (c >= '0' && c <= '9') ||
            (c == '_')
          );
}

inline bool
IsWhiteSpace(char c)
{
    return (c == ' ' || c == '\t' || c == '\n');
}

string::size_type
SkipWhiteSpaces(const std::string &source, std::string::size_type pos)
{
    while(IsWhiteSpace(source[pos++]));
    return pos - 1;
}

string::size_type
FindToken(const std::string &token, const std::string &source, std::string::size_type pos)
{
    pos = source.find(token, pos);

    if(pos == std::string::npos) {
        return std::string::npos;
    }

    while(pos != std::string::npos && IsChar(source[pos + token.length()])) {
        pos += token.length();
        pos = source.find(token, pos);
    }

    return pos;
}

std::string
GetNextToken(const std::string &source, std::string::size_type start)
{
    std::string::size_type pos = start;
    while(!IsWhiteSpace(source[pos]) && IsChar(source[pos])) {
        ++pos;
    }
    return std::string(source, start, pos - start);
}

int32_t
RemoveBrackets(std::string &source)
{
    int32_t index = -1;
    size_t  firstOpenBracket = source.find_first_of("[");
    size_t  firstCloseBracket;
    while(firstOpenBracket != string::npos) {
        firstCloseBracket = source.find_first_of("]") + 1;
        index  = stoi(string(source, firstOpenBracket + 1, firstCloseBracket - firstOpenBracket - 1));
        source = string(source, 0, firstOpenBracket)  + string(source, firstCloseBracket, source.size());
        firstOpenBracket = source.find_first_of("[");
    }

    return index;
}

void
ReplaceString(const std::string &s_in, const std::string &s_out, std::string &source)
{
    size_t first = source.find_first_of(s_in);

    if(first != string::npos) {
        source = s_out + string(source, first, source.size());
    }
}

bool
IsPrecisionQualifier(const std::string &token)
{
    return (!token.compare("highp")   ||
            !token.compare("mediump") ||
            !token.compare("lowp"));
}

bool
CanTypeBeInUniformBlock(const std::string &token)
{
    int i;

    for(i = 0; i < (int)(sizeof(SAMPLER_TYPES) / sizeof(char *)); ++i) {
        if(!token.compare(SAMPLER_TYPES[i])) {
            return false;
        }
    }

    for(i = 0; i < (int)(sizeof(IMAGE_TYPES) / sizeof(char *)); ++i) {
        if(!token.compare(IMAGE_TYPES[i])) {
            return false;
        }
    }

    return true;
}

bool
IsBuildInUniform(const std::string &token)
{
  return (!token.compare("gl_DepthRange.near") ||
          !token.compare("gl_DepthRange.far")  ||
          !token.compare("gl_DepthRange.diff"));
}

void
ReplaceAll(string& hays, const string& from, const string& to)
{
    auto pos = hays.find(from);

    while(pos != string::npos) {
        hays.replace(pos, from.size(), to);
        pos = hays.find(from, pos + to.size());
    }
}
