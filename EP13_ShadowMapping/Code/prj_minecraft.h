#ifndef PRJ_MINECRAFT_H
#define PRJ_MINECRAFT_H

/*

 Vertex structure:
6, 8, 6 bits - for x y z

Per-face structure
3 bits - normal
8 bits - bitmap ID
1 bit - if geometry is floating

*/

#define MINC_CHUNK_WIDTH 32
#define MINC_CHUNK_HEIGHT 128

#define MINC_CHUNK_COUNT (MINC_CHUNK_WIDTH * MINC_CHUNK_WIDTH * MINC_CHUNK_HEIGHT)
#define MINC_MAX_VERTS_COUNT (MINC_CHUNK_COUNT / 2 * 6 * 6)

#define MINC_ATLAS_TEXTURE_INDEX(x, y) (y * 16 + x)
enum minc_atlas_texture
{
    MincTexture_GroundGrass = MINC_ATLAS_TEXTURE_INDEX(0, 0),
    MincTexture_Ground = MINC_ATLAS_TEXTURE_INDEX(2, 0),
    MincTexture_GroundSide = MINC_ATLAS_TEXTURE_INDEX(3, 0),
    
    MincTexture_Stone = MINC_ATLAS_TEXTURE_INDEX(1, 0),
    MincTexture_StoneWeak = MINC_ATLAS_TEXTURE_INDEX(0, 1),
    MincTexture_Sand = MINC_ATLAS_TEXTURE_INDEX(2, 1),
    
    MincTexture_WoodTreeUp = MINC_ATLAS_TEXTURE_INDEX(5, 1),
    MincTexture_WoodTreeSide = MINC_ATLAS_TEXTURE_INDEX(4, 1),
    MincTexture_WoodTreeBirch = MINC_ATLAS_TEXTURE_INDEX(5, 7),
    MincTexture_WoodTreeDark = MINC_ATLAS_TEXTURE_INDEX(4, 7),
    MincTexture_WoodTreeGrassSnow = MINC_ATLAS_TEXTURE_INDEX(4, 8),
    MincTexture_WoodTreeGrass = MINC_ATLAS_TEXTURE_INDEX(5, 8),
    
    MincTexture_LeavesSnow = MINC_ATLAS_TEXTURE_INDEX(4, 3),
    MincTexture_Leaves = MINC_ATLAS_TEXTURE_INDEX(5, 3),
    
    MincTexture_BlockIron = MINC_ATLAS_TEXTURE_INDEX(6, 1),
    MincTexture_BlockGold = MINC_ATLAS_TEXTURE_INDEX(7, 1),
    MincTexture_BlockDiam = MINC_ATLAS_TEXTURE_INDEX(8, 1),
    
    MincTexture_Wood = MINC_ATLAS_TEXTURE_INDEX(4, 0),
    MincTexture_Brick = MINC_ATLAS_TEXTURE_INDEX(7, 0),
    
    MincTexture_BombSide = MINC_ATLAS_TEXTURE_INDEX(8, 0),
    MincTexture_BombUp = MINC_ATLAS_TEXTURE_INDEX(9, 0),
    MincTexture_BombDown = MINC_ATLAS_TEXTURE_INDEX(10, 0),
    
    MincTexture_GroundSnow = MINC_ATLAS_TEXTURE_INDEX(2, 4),
    MincTexture_GroundSnowSide = MINC_ATLAS_TEXTURE_INDEX(4, 4),
    
    MincTexture_IceWater = MINC_ATLAS_TEXTURE_INDEX(3, 4),
    
    MincTexture_Lava1 = MINC_ATLAS_TEXTURE_INDEX(13, 14),
    MincTexture_Lava2 = MINC_ATLAS_TEXTURE_INDEX(14, 14),
    MincTexture_Lava3 = MINC_ATLAS_TEXTURE_INDEX(15, 14),
    MincTexture_Lava4 = MINC_ATLAS_TEXTURE_INDEX(14, 15),
    MincTexture_Lava5 = MINC_ATLAS_TEXTURE_INDEX(15, 15),
    
    MincTexture_Water1 = MINC_ATLAS_TEXTURE_INDEX(13, 12),
    MincTexture_Water2 = MINC_ATLAS_TEXTURE_INDEX(14, 12),
    MincTexture_Water3 = MINC_ATLAS_TEXTURE_INDEX(15, 12),
    MincTexture_Water4 = MINC_ATLAS_TEXTURE_INDEX(14, 13),
    MincTexture_Water5 = MINC_ATLAS_TEXTURE_INDEX(15, 13),
};

enum minc_block_type
{
    MincBlock_Empty,
    
    MincBlock_Ground,
    MincBlock_GroundGrass,
    MincBlock_Stone,
    MincBlock_StoneWeak,
    MincBlock_Sand,
    MincBlock_Brick,
    MincBlock_Bomb,
    
    MincBlock_BlockIron,
    MincBlock_BlockGold,
    MincBlock_BlockDiamond,
    
    MincBlock_Snow,
    MincBlock_SnowGround,
    
    MincBlock_TreeWood,
    MincBlock_TreeWoodBirch,
    MincBlock_TreeWoodDark,
    MincBlock_TreeWoodGrass,
    MincBlock_TreeWoodGrassSnow,
    
    MincBlock_TreeLeaves,
    MincBlock_TreeLeavesSnow,
    
    MincBlock_Count,
};

enum minc_normal_type
{
    MincFaceNormal_Front,
    MincFaceNormal_Back,
    MincFaceNormal_Left,
    MincFaceNormal_Right,
    MincFaceNormal_Up,
    MincFaceNormal_Down,
    
    MincFaceNormal_Count,
};

union minc_block_texture_indices
{
    struct
    {
        int Front;
        int Back;
        int Left;
        int Right;
        int Up;
        int Down;
    };
    
    int Sides[MincFaceNormal_Count];
};

struct minc_offsets_to_vertex
{
    u8 x, y, z;
};

struct minc_face_offsets_to_vertex
{
    minc_offsets_to_vertex VertexOffsets[6];
};

struct minc_temp_mesh
{
    // NOTE(Dima): At the worst case we'll have each second block * 6 faces * 6 vertex-per-face
    u32* Vertices;
    
    // NOTE(Dima): The same but except face vertices
    u32* PerFaceData;
    
    int VerticesCount;
    int FaceCount;
    
    int MaxVerticesCount;
    int MaxFaceCount;
    
    minc_temp_mesh* NextInList;
    int TempMeshListIndex;
};

enum minc_biome_type
{
    MincBiome_Desert,
    MincBiome_Standard,
    MincBiome_Standard2,
    MincBiome_SnowTaiga,
    
    MincBiome_Count,
};

struct minc_biome
{
    u8 LayerBlocks[4];
    
    f32 StartWeight;
    f32 EndWeight;
    
    f32 BaseHeight;
    f32 NoiseFrequency;
    f32 NoiseScale;
    
    // NOTE(Dima): Trees related stuff
    b32 HasTrees;
    b32 HasGrass;
    f32 TreeDensity;
    f32 GrassDensity;
    
    int TrunkMinH;
    int TrunkMaxH;
    int CrownMinH;
    int CrownMaxH;
    int CrownMinRad;
    int CrownMaxRad;
    
    u8 TreeTrunkBlock;
    u8 TreeCrownBlock;
};

struct minc_chunk_meta
{
    // NOTE(Dima): Generate biome map and height map
    u8 BiomeMap[MINC_CHUNK_WIDTH * MINC_CHUNK_WIDTH];
    u16 HeightMap[MINC_CHUNK_WIDTH * MINC_CHUNK_WIDTH];
    
    int CoordX;
    int CoordZ;
    
    std::atomic_uint32_t State;
};

struct minc_lookup_chunks_metas
{
    minc_chunk_meta* LookupChunks[9];
};

enum minc_chunk_state
{
    MincChunk_Unloaded,
    
    MincChunk_GeneratingMaps,
    
    MincChunk_ReadyToFixBiomeGaps,
    MincChunk_FixingBiomeGaps,
    
    MincChunk_ReadyToGenerateChunk,
    MincChunk_GeneratingChunk,
    
    MincChunk_ReadyToGenerateMesh,
    MincChunk_GeneratingMesh,
    MincChunk_GeneratingMeshFinalizing,
    MincChunk_MeshGenerated,
};

struct minc_chunk
{
    int CoordX;
    int CoordY;
    int CoordZ;
    
    u8 Blocks[MINC_CHUNK_COUNT];
    
    voxel_mesh Mesh;
    
    struct minc_generate_mesh_work* GenerateMeshWork;
    int GenerationSidesCount;
    int ExpectedVerticesCount;
    b32 SucceededGenerateMesh;
    
    b32 BoundingSphereGenerated;
    f32 BoundingSphereR;
    v3 BoundingSphereCenter;
};

struct minc_chunk_side
{
    u8 Blocks[MINC_CHUNK_WIDTH * MINC_CHUNK_HEIGHT];
    
    minc_chunk_side* NextInList;
};

struct minc_chunk_meta_slot
{
    u64 Key;
    minc_chunk_meta* Meta;
    minc_chunk* Chunk;
    
    minc_chunk_meta_slot* NextInHash;
};

struct minecraft
{
    memory_arena* Arena;
    
    minc_chunk Chunks[10][10];
    
#define MINC_META_TABLE_SIZE 2048
    minc_chunk_meta_slot* MetaTable[MINC_META_TABLE_SIZE];
    minc_chunk_meta_slot MetaSentinel;
    
    minc_block_texture_indices BlocksTextureIndices[MincBlock_Count];
    minc_face_offsets_to_vertex OffsetsToV[6];
    minc_biome Biomes[MincBiome_Count];
    f32 BiomesTotalWeight;
    
    task_memory_pool* TaskPool;
    
    minc_temp_mesh* TempMeshLists[5];
    minc_chunk_side* ChunkSidePool;
    
    int ChunksViewDistance;
};

#endif //PRJ_MINECRAFT_H
