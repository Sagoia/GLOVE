#ifndef __SLANGCOMPILER_H__
#define __SLANGCOMPILER_H__

#include "utils/globals.h"
#include "utils/glsl_types.h"

class SlangCompiler {

public:
    SlangCompiler() {}
    virtual ~SlangCompiler() {}

    virtual bool CompileShader(const char* const* source, shader_type_t language) = 0;
    virtual const char* GetInfoLog() = 0;
    virtual const char* GetSource() const = 0;
};

#endif // __SLANGCOMPILER_H__
