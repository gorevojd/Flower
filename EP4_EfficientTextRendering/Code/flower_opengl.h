#ifndef TUTY_OPENGL_H
#define TUTY_OPENGL_H

#include "GL/glew.h"

inline b32 OpenGLAttribIsValid(GLint Attrib)
{
    b32 Result = Attrib != -1;
    
    return(Result);
}

#define OPENGL_LOAD_ATTRIB(name) Result.##name##Attr = glGetAttribLocation(Result.ID, "In"#name)
#define OPENGL_LOAD_UNIFORM(name) Result.##name##Loc = glGetUniformLocation(Result.ID, #name)

struct opengl_shader
{
    GLuint ID;
    
    char Name[64];
    
    // NOTE(Dima): Attribs
    GLint PositionAttr;
    GLint TexCoordsAttr;
    GLint NormalAttr;
    GLint ColorAttr;
    
    // NOTE(Dima): Uniforms
    GLint ProjectionLoc;
    GLint ViewLoc;
    GLint ModelLoc;
    
    GLint MultColorLoc;
    GLint TexDiffuseLoc;
    
    GLint IsImageLoc;
    GLint ImageLoc;
    GLint GlyphsColorsLoc;
    GLint GlyphsTypesLoc;
    GLint Is3DLoc;
    GLint IsBatchLoc;
    GLint GlyphsTransformsLoc;
    GLint GlyphsIndicesToTransformsLoc;
    GLint GlyphDefaultProjectionMatrixIndexLoc;
    GLint GlyphOrthoMatrixIndexLoc;
    GLint GlyphPerspMatrixIndexLoc;
};

struct opengl_state
{
    opengl_shader StdShader;
    opengl_shader UIRectShader;
};

#endif