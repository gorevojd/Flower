#ifndef FLOWER_RENDER_PRIMITIVES_H
#define FLOWER_RENDER_PRIMITIVES_H

#include "flower_asset_types_shared.h"

enum renderer_handle_type
{
    RendererHandle_Invalid,
    
    RendererHandle_Image,
    RendererHandle_Mesh,
    RendererHandle_TextureBuffer,
    RendererHandle_Cubemap,
};

struct renderer_handle
{
    b32 Initialized;
    b32 Invalidated;
    u32 Type;
    
    union
    {
        struct
        {
            u32 TextureObject;
        } Image;
        
        struct 
        {
            u32 BufferObject;
            u32 TextureObject;
        } TextureBuffer;
        
        struct 
        {
            u64 ArrayObject;
            u64 BufferObject;
            u64 ElementBufferObject;
        } Mesh;
        
        struct 
        {
            u32 Handle;
        } Cubemap;
    };
};

inline void InitRendererHandle(renderer_handle* Handle, u32 Type)
{
    Handle->Initialized = false;
    Handle->Invalidated = false;
    Handle->Type = Type;
}

inline renderer_handle CreateRendererHandle(u32 Type)
{
    renderer_handle Result = {};
    
    InitRendererHandle(&Result, Type);
    
    return(Result);
}

inline b32 ShouldDeleteHandleStorage(renderer_handle* Handle)
{
    b32 Result = false;
    
    if(Handle->Invalidated && Handle->Initialized)
    {
        Result = true;
    }
    
    return(Result);
}

inline void InvalidateHandle(renderer_handle* Handle)
{
    Handle->Invalidated = true;
}

struct render_mesh_offsets
{
    u32 OffsetP;
    u32 OffsetUV;
    u32 OffsetN;
    u32 OffsetC;
    u32 OffsetBoneWeights;
    u32 OffsetBoneIndices;
};

struct mesh
{
    char Name[64];
    
    // NOTE(Dima): This pointer holds data for all the mesh (this includes vertices and index data)
    void* Free;
    u32 FreeSize;
    
    v3* P;
    v2* UV;
    v3* N;
    u32* C;
    v4* BoneWeights;
    u32* BoneIndices;
    int VertexCount;
    
    u32* Indices;
    int IndexCount;
    
    b32 IsSkinned;
    int MaterialIndexInModel;
    
    renderer_handle Handle;
    render_mesh_offsets Offsets;
};

struct voxel_mesh
{
    u32* Vertices;
    u32* PerFaceData;
    
    int VerticesCount;
    int FaceCount;
    
    void* Free;
    
    renderer_handle Handle;
    renderer_handle PerFaceBufHandle;
};

enum image_format
{
    ImageFormat_RGBA,
    ImageFormat_RGB,
    ImageFormat_Grayscale,
    
    ImageFormat_Count,
};

GLOBAL_VARIABLE int ImageFormatPixelSizes[] = 
{
    4, 
    3,
    1,
};

struct image
{
    void* Pixels;
    
    int Width;
    int Height;
    
    float WidthOverHeight;
    u32 Format;
    
    b32 FilteringIsClosest;
    
    renderer_handle Handle;
};

struct cubemap
{
    image Left;
    image Right;
    image Down;
    image Top;
    image Front;
    image Back;
    
    renderer_handle Handle;
};

struct material
{
    image* Diffuse;
};

struct node_animation
{
    v3* PositionKeys;
    quat* RotationKeys;
    v3* ScalingKeys;
    
    f32* PositionTimes;
    f32* RotationTimes;
    f32* ScalingTimes;
    
    int NumPos;
    int NumRot;
    int NumScl;
    
    int NodeIndex;
};

enum animation_behaviour
{
    AnimBehaviour_Closest,
    AnimBehaviour_Repeat,
};

struct animation
{
    char Name[64];
    asset_animation_shared Shared;
    
    node_animation* NodeAnims;
    
    void* Free;
};

enum font_style
{
    FontStyle_Regular, 
    FontStyle_Shadow,
    FontStyle_Outline,
    
    FontStyle_Count,
};

struct glyph_style
{
    int ImageIndex;
    f32 WidthOverHeight;
    
    v2 MinUV;
    v2 MaxUV;
};

inline glyph_style CreateGlyphStyle(int ImageIndex, int Width, int Height)
{
    glyph_style Result = {};
    
    Result.ImageIndex = ImageIndex;
    Result.WidthOverHeight = (f32)Width / (f32)Height;
    
    return(Result);
}

struct glyph
{
    glyph_style Styles[FontStyle_Count];
    
    u32 Codepoint;
    
    f32 Advance;
    f32 LeftBearing;
    
    f32 XOffset;
    f32 YOffset;
};

struct font
{
    image* GlyphImages;
    glyph* Glyphs;
    int GlyphCount;
    
    float* KerningPairs;
    
    f32 Ascent;
    f32 Descent;
    f32 LineGap;
    
    f32 LineAdvance;
    
    f32 PixelsPerMeter;
};

struct model_node
{
    char Name[64];
    
    int ChildIndices[32];
    int MeshIndices[8];
    
    int NumChildIndices;
    int NumMeshIndices;
};

struct model
{
    mesh** Meshes;
    material** Materials;
    
    // NOTE(Dima): Nodes
    model_node* Nodes;
    m44* Node_ToParent;
    int* Node_ParentIndex;
    
    // NOTE(Dima): Bones
    m44* Bone_InvBindPose;
    int* Bone_NodeIndex;
    
    asset_model_shared Shared;
    
    void* Free;
};

#endif //FLOWER_RENDER_PRIMITIVES_H
