#ifndef TUTY_OPENGL_H
#define TUTY_OPENGL_H

#include "GL/glew.h"

inline b32 OpenGLAttribIsValid(GLint Attrib)
{
    b32 Result = Attrib != -1;
    
    return(Result);
}

#define OPENGL_LOAD_ATTRIB(name) Result.##name##Attr = glGetAttribLocation(Result.ID, "In"#name)

struct opengl_shader
{
    GLuint ID;
    
    char Name[64];
    
    GLint PositionAttr;
    GLint TexCoordsAttr;
    GLint NormalAttr;
    GLint ColorAttr;
};

struct opengl_state
{
    opengl_shader StdShader;
};

#endif