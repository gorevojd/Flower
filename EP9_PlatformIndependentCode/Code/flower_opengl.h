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
    GLint WeightsAttr;
    GLint BoneIDsAttr;
    
    // NOTE(Dima): Uniforms
    GLint ProjectionLoc;
    GLint ViewLoc;
    GLint ModelLoc;
    GLint SkinningMatricesLoc;
    GLint SkinningMatricesCountLoc;
    GLint MeshIsSkinnedLoc;
    GLint InstanceModelMatricesLoc;
    GLint UseInstancingLoc;
    
    GLint MultColorLoc;
    GLint TexDiffuseLoc;
    GLint HasDiffuseLoc;
    GLint MaterialMissingLoc;
    
    GLint IsImageLoc;
    GLint ImageLoc;
    GLint RectsColorsLoc;
    GLint RectsTypesLoc;
    GLint Is3DLoc;
    GLint IsBatchLoc;
    GLint RectsTransformsLoc;
    GLint RectsIndicesToTransformsLoc;
    GLint RectOrthoMatrixIndexLoc;
    GLint RectPerspMatrixIndexLoc;
};

struct opengl_state
{
    opengl_shader StdShader;
    opengl_shader UIRectShader;
};

#endif