# GLOVE Coding Style

## GENERAL

**Always use 4 spaces for indentation. Never use tabs**

**Use typically trailing braces everywhere (if, else, functions, structures, typedefs, class definitions, etc.)**

```
if(x) {
}
```

**The else statement starts on the same line as the last closing brace.**

```
if(x) {
} else {
}
```

Do NOT do this:
```
if(x) {
/*
...
*/
} else
    // One line
```

NOR this
```
if(x) {
/*
...
*/
}
else {
}
```


**Do NOT pad parenthesized expressions with spaces. Also pad with spaces trailing braces.**

```
if(x) {
}
```
Instead of
```
if ( x ) {
}
```

**Similar rules apply to function declarations, definitions and calling**

```
// Declaration
void FooFunction(int bar, float barbar);

// Definition
void
FooFunction(int bar, float barbar)
{
}

// Call
FooFunction(5, 3.0f);
```

**For function declarations write in a separate lines the template and return type from function name and arguments**

Do this:
```
char *
Shader::GetShaderSource(void) const
{
/* ... */
}
```

```
template<typename T>
void
VertexAttribInterface::SetGenericVertexAttribute(int index, const T *ptr)
{
/* ... */
}
```

**Pad (logical arithmetic and bitwise) operators with spaces. Also pad with spaces the = operator**

```
x = y * 0.5f;
```
Instead of
```
x=y*0.5f;
```

**Use precision specification for floating point values unless there is an explicit need for a double.**

```
float f = 0.5f;
```
Instead of
```
float f = 0.5;
```
And
```
float f = 1.0f;
```
Instead of
```
float f = 1.f;
```

**Function names start with an upper case.**

```
void Function(void);
```

**In multi-word function names each word starts with an upper case:**

```
void ThisFunctionDoesSomething(void);
```

**Variable names start with a lower case character.**

```
float x;
```

**Typedef names use the same naming convention as variables, however they always end with "_t".**

```
typedef float                                           glsl_float_t;
typedef struct glsl_vec2_t  { glsl_float_t f[2]; }      glsl_vec2_t;
typedef struct glsl_vec3_t  { glsl_float_t f[3]; }      glsl_vec3_t;
typedef struct glsl_vec4_t  { glsl_float_t f[4]; }      glsl_vec4_t;
typedef struct glsl_mat4_t  { glsl_vec4_t fm[4]; }      glsl_mat4_t;
```

**Struct names use the same naming convention as variables, however they always end with "_t".**

```
struct uniformBlock_t;
```

**Enum names use the same naming convention as variables, however they always end with  "_e". The enum constants use all upper case characters. Multiple words are separated with an underscore.**

```
typedef enum {
    INVALID_SHADER,
    SHADER_TYPE_VERTEX,
    SHADER_TYPE_FRAGMENT
} shaderType_e;
```

**Names of recursive functions end with "_r"**

```
void WalkBSP_r(int node);
```

**Defined names use all upper case characters. Multiple words are separated with an underscore.**

```
#define FRONT_FACING    0
```

**Use ‘const’ as much as possible.**

Use:
```
const int *p;         // pointer to const int
int * const p;        // const pointer to int
const int * const p;  // const pointer to const int
```
Don’t use:
```
int const *p;
```

## CLASSES


**Class names start with a capital letter**

```
class BufferObject;
```

**Class variable member names start with a lower case m character. This applies to classes/structs that do have function members.**

```
class BufferObject {
    size_t        mSize;
    GLenum        mGlTarget;
}
```

**Class methods have the same naming convention as functions.**

```
class BufferObject {
    void AllocateBufferObject(GLsizeiptr size, const void *data);
}
```

**Indent the names of class variables and class methods to make nice columns. The variable type or method return type is in the first column and the variable name or method name is in the second column.**

```
class BufferObject {
    size_t        mSize;
    GLenum        mGlTarget;
    void *        mpData;

    void   ReleaseBufferObject(void);
    void   SetGlTarget(GLenum target)               { FUN_ENTRY(DEBUG_DEPTH); mGlTarget = target; }
    size_t GetSize(void)                    const   { FUN_ENTRY(DEBUG_DEPTH); return mSize; }
    GLenum GetTarget(void)                  const   { FUN_ENTRY(DEBUG_DEPTH); return mGlTarget; }
}
```
**The * of the pointer is in the first column because it improves readability when considered part of the type.**


**Ordering of class variables and methods should be as follows:**

1. list of friend classes
2. private variables
3. private methods
4. protected variables
5. protected methods
6. public variables
7. public methods


**Always make class methods ‘const’ when they do not modify any class variables.**

**Avoid use of ‘const_cast’.  When object is needed to be modified, but only const versions are accessible, create a function that clearly gives an editable version of the object. This keeps the control of the ‘const-ness’ in the hands of the object and not the user.**

**Return ‘const’ objects unless the general usage of the object is to change its state.**

## FILE NAMES


**Each class should be in a seperate source file unless it makes sense to group several smaller classes.**
```
class BufferObject;
```
files:
```
bufferObject.cpp
bufferObject.h
```

**When a class spans across multiple files these files have a name that starts with the name of the class, followed by a subsection name.**
```
class Context;
```

files:
```
contextTextures.cpp
contextShaders.cpp
context.cpp
context.h
```
