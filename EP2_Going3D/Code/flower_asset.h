#ifndef FLOWER_ASSET_H
#define FLOWER_ASSET_H

struct vertex
{
    v3 P;
    v2 UV;
    v3 N;
    v3 C;
};

struct mesh_handles
{
    u64 ArrayObject;
    u64 BufferObject;
    u64 ElementBufferObject;
};

struct mesh
{
    vertex* Vertices;
    int VertexCount;
    
    u32* Indices;
    int IndexCount;
    
    mesh_handles ApiHandles;
};


inline b32 MeshInitialized(mesh* Mesh)
{
    b32 Result = Mesh->ApiHandles.ArrayObject != 0;
    
    return(Result);
}


struct image
{
    void* Pixels;
    
    int Width;
    int Height;
    
    float WidthOverHeight;
    
    u64 ApiHandle;
};

#endif //FLOWER_ASSET_H
