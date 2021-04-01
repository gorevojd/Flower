#include "prj_minecraft.h"

#define STB_PERLIN_IMPLEMENTATION
#include "stb_perlin.h"

inline minc_block_texture_indices CreateBlockAll(int Index)
{
    minc_block_texture_indices Result = {};
    
    Result.Front = Index;
    Result.Back = Index;
    Result.Left = Index;
    Result.Right = Index;
    Result.Up = Index;
    Result.Down = Index;
    
    return(Result);
}

inline minc_block_texture_indices CreateBlockSide(int SideIndex, int UpdownIndex)
{
    minc_block_texture_indices Result = {};
    
    Result.Front = SideIndex;
    Result.Back = SideIndex;
    Result.Left = SideIndex;
    Result.Right = SideIndex;
    Result.Up = UpdownIndex;
    Result.Down = UpdownIndex;
    
    return(Result);
}

inline minc_block_texture_indices CreateBlockUpDown(int SideIndex, int Up, int Down)
{
    minc_block_texture_indices Result = {};
    
    Result.Front = SideIndex;
    Result.Back = SideIndex;
    Result.Left = SideIndex;
    Result.Right = SideIndex;
    Result.Up = Up;
    Result.Down = Down;
    
    return(Result);
}

inline minc_block_texture_indices CreateBlock(int Front,
                                              int Back,
                                              int Left,
                                              int Right,
                                              int Up,
                                              int Down)
{
    minc_block_texture_indices Result = {};
    
    Result.Front = Front;
    Result.Back = Back;
    Result.Left = Left;
    Result.Right = Right;
    Result.Up = Up;
    Result.Down = Down;
    
    return(Result);
}

INTERNAL_FUNCTION void InitMinecraftBlockTextures(minecraft* Mine)
{
    // NOTE(Dima): Init blocks texture indices
    Mine->BlocksTextureIndices[MincBlock_Ground] = CreateBlockAll(MincTexture_Ground);
    Mine->BlocksTextureIndices[MincBlock_GroundGrass] = CreateBlockUpDown(MincTexture_GroundSide,
                                                                          MincTexture_GroundGrass,
                                                                          MincTexture_Ground);
    Mine->BlocksTextureIndices[MincBlock_Stone] = CreateBlockAll(MincTexture_Stone);
    Mine->BlocksTextureIndices[MincBlock_StoneWeak] = CreateBlockAll(MincTexture_StoneWeak);
    Mine->BlocksTextureIndices[MincBlock_Sand] = CreateBlockAll(MincTexture_Sand);
    Mine->BlocksTextureIndices[MincBlock_Brick] = CreateBlockAll(MincTexture_Brick);
    Mine->BlocksTextureIndices[MincBlock_Bomb] = CreateBlockUpDown(MincTexture_BombSide,
                                                                   MincTexture_BombUp,
                                                                   MincTexture_BombDown);
    
    Mine->BlocksTextureIndices[MincBlock_BlockIron] = CreateBlockAll(MincTexture_BlockIron);
    Mine->BlocksTextureIndices[MincBlock_BlockGold] = CreateBlockAll(MincTexture_BlockGold);
    Mine->BlocksTextureIndices[MincBlock_BlockDiamond] = CreateBlockAll(MincTexture_BlockDiam);
    
    Mine->BlocksTextureIndices[MincBlock_Snow] = CreateBlockAll(MincTexture_GroundSnow);
    Mine->BlocksTextureIndices[MincBlock_SnowGround] = CreateBlockUpDown(MincTexture_GroundSnowSide,
                                                                         MincTexture_GroundSnow,
                                                                         MincTexture_Ground);
    
    Mine->BlocksTextureIndices[MincBlock_TreeWood] = CreateBlockSide(MincTexture_WoodTreeSide,
                                                                     MincTexture_WoodTreeUp);
    Mine->BlocksTextureIndices[MincBlock_TreeWoodBirch] = CreateBlockSide(MincTexture_WoodTreeBirch,
                                                                          MincTexture_WoodTreeUp);
    Mine->BlocksTextureIndices[MincBlock_TreeWoodDark] = CreateBlockSide(MincTexture_WoodTreeDark,
                                                                         MincTexture_WoodTreeUp);
    Mine->BlocksTextureIndices[MincBlock_TreeWoodGrass] = CreateBlockSide(MincTexture_WoodTreeGrass,
                                                                          MincTexture_WoodTreeUp);
    Mine->BlocksTextureIndices[MincBlock_TreeWoodGrassSnow] = CreateBlockSide(MincTexture_WoodTreeGrassSnow,
                                                                              MincTexture_WoodTreeUp);
    
    Mine->BlocksTextureIndices[MincBlock_TreeLeaves] = CreateBlockAll(MincTexture_Leaves);
    Mine->BlocksTextureIndices[MincBlock_TreeLeavesSnow] = CreateBlockAll(MincTexture_LeavesSnow);
}

INTERNAL_FUNCTION inline void MincFaceOffsetOnCube(minecraft* Minecraft,
                                                   int Normal, 
                                                   int A, int B, int C, int D)
{
    minc_offsets_to_vertex Cube[8] =
    {
        {1, 1, 0},
        {0, 1, 0},
        {0, 1, 1},
        {1, 1, 1},
        {1, 0, 0},
        {0, 0, 0},
        {0, 0, 1},
        {1, 0, 1},
    };
    
    minc_face_offsets_to_vertex* Face = &Minecraft->OffsetsToV[Normal];
    
    Face->VertexOffsets[0] = Cube[A];
    Face->VertexOffsets[1] = Cube[B];
    Face->VertexOffsets[2] = Cube[C];
    Face->VertexOffsets[3] = Cube[A];
    Face->VertexOffsets[4] = Cube[C];
    Face->VertexOffsets[5] = Cube[D];
}

INTERNAL_FUNCTION void InitMinecraftTextureOffsets(minecraft* Mine)
{
    MincFaceOffsetOnCube(Mine,
                         MincFaceNormal_Front,
                         2, 3, 7, 6);
    
    MincFaceOffsetOnCube(Mine,
                         MincFaceNormal_Back,
                         0, 1, 5, 4);
    
    MincFaceOffsetOnCube(Mine,
                         MincFaceNormal_Left,
                         3, 0, 4, 7);
    
    MincFaceOffsetOnCube(Mine,
                         MincFaceNormal_Right,
                         1, 2, 6, 5);
    
    MincFaceOffsetOnCube(Mine,
                         MincFaceNormal_Up,
                         3, 2, 1, 0);
    
    MincFaceOffsetOnCube(Mine,
                         MincFaceNormal_Down,
                         4, 5, 6, 7);
}

INTERNAL_FUNCTION inline minc_biome* CreateDefaultBiome(minecraft* Mine, 
                                                        u32 Type, 
                                                        f32 Weight)
{
    minc_biome* Result = &Mine->Biomes[Type];
    
    Result->LayerBlocks[0] = MincBlock_GroundGrass;
    Result->LayerBlocks[1] = MincBlock_Ground;
    Result->LayerBlocks[2] = MincBlock_Stone;
    Result->LayerBlocks[3] = MincBlock_Stone;
    
    Result->StartWeight = Mine->BiomesTotalWeight;
    Result->EndWeight = Mine->BiomesTotalWeight + Weight;
    
    Mine->BiomesTotalWeight += Weight;
    Result->BaseHeight = 64.0f;
    Result->NoiseFrequency = 256.0f;
    Result->NoiseScale = 20.0f;
    
    // NOTE(Dima): Trees
    Result->TreeDensity = 0.002f;
    Result->HasTrees = true;
    Result->TrunkMinH = 5;
    Result->TrunkMaxH = 10;
    Result->CrownMinH = 5;
    Result->CrownMaxH = 12;
    Result->CrownMinRad = 1;
    Result->CrownMaxRad = 3;
    
    Result->TreeCrownBlock = MincBlock_TreeLeaves;
    Result->TreeTrunkBlock = MincBlock_TreeWoodDark;
    
    return(Result);
}

INTERNAL_FUNCTION void InitMinecraftBiomes(minecraft* Mine)
{
    Mine->BiomesTotalWeight = 0.0f;
    
    minc_biome* Desert = CreateDefaultBiome(Mine, MincBiome_Desert, 10);
    Desert->NoiseFrequency = 1024.0f;
    Desert->NoiseScale = 16.0f;
    Desert->BaseHeight = 40.0f;
    Desert->LayerBlocks[0] = MincBlock_Sand;
    Desert->LayerBlocks[1] = MincBlock_Sand;
    Desert->LayerBlocks[2] = MincBlock_Ground;
    Desert->LayerBlocks[3] = MincBlock_Stone;
    Desert->HasTrees = false;
    
    minc_biome* Standard = CreateDefaultBiome(Mine, MincBiome_Standard, 20);
    Standard->NoiseFrequency = 300.0f;
    Standard->NoiseScale = 16.0f;
    Standard->BaseHeight = 40.0f;
    Standard->TreeCrownBlock = MincBlock_TreeLeaves;
    Standard->TreeTrunkBlock = MincBlock_TreeWoodBirch;
    
    minc_biome* Standard2 = CreateDefaultBiome(Mine, MincBiome_Standard2, 28);
    Standard2->NoiseFrequency = 800.0f;
    Standard2->NoiseScale = 15.0f;
    Standard2->BaseHeight = 27.0f;
    Standard2->TreeCrownBlock = MincBlock_TreeLeaves;
    Standard2->TreeTrunkBlock = MincBlock_TreeWoodBirch;
    
    minc_biome* SnowTaiga = CreateDefaultBiome(Mine, MincBiome_SnowTaiga, 40);
    SnowTaiga->NoiseFrequency = 256.0f;
    SnowTaiga->NoiseScale = 45.0f;
    SnowTaiga->BaseHeight = 60.0f;
    SnowTaiga->LayerBlocks[0] = MincBlock_SnowGround;
    SnowTaiga->TreeDensity = 0.004f;
    SnowTaiga->TreeCrownBlock = MincBlock_TreeLeavesSnow;
    SnowTaiga->TreeTrunkBlock = MincBlock_TreeWood;
    
    // NOTE(Dima): Backpropagate biome weights
    f32 OneOverTotalWeight = 1.0f / Mine->BiomesTotalWeight;
    for(int BiomeIndex = 0;
        BiomeIndex < MincBiome_Count;
        BiomeIndex++)
    {
        minc_biome* Biome = Mine->Biomes + BiomeIndex;
        
        Biome->StartWeight *= OneOverTotalWeight;
        Biome->EndWeight *= OneOverTotalWeight;
    }
}

#define GET_BLOCK_INDEX_IN_CHUNK(x, y, z) (y * (MINC_CHUNK_WIDTH * MINC_CHUNK_WIDTH) + z * MINC_CHUNK_WIDTH + x)

INTERNAL_FUNCTION inline int MincGetBlockIndex(int X, int Y, int Z)
{
    int Result = GET_BLOCK_INDEX_IN_CHUNK(X, Y, Z);
    
    return(Result);
}

INTERNAL_FUNCTION inline u8 GetBlockInChunk(minc_chunk* Chunk,
                                            int X, int Y, int Z)
{
    int Index = GET_BLOCK_INDEX_IN_CHUNK(X, Y, Z);
    
    u8 Result = Chunk->Blocks[Index];
    
    return(Result);
}

INTERNAL_FUNCTION inline u8 GetBlockInChunkSide(minc_chunk_side* Side,
                                                int X, int Y)
{
    u8 Result = Side->Blocks[Y * MINC_CHUNK_WIDTH + X];
    
    return(Result);
}

struct minc_generate_chunks
{
    minc_chunk* Dst;
    
    minc_chunk_side* ChunksSides[6];
    minc_chunk* Chunks[6];
};

// NOTE(Dima): Returns false if there were not enough vertices
INTERNAL_FUNCTION b32 GenerateChunkMesh(minecraft* Minecraft,
                                        minc_temp_mesh* Mesh,
                                        minc_generate_chunks Chunks)
{
    minc_chunk* Chunk = Chunks.Dst;
    
    Mesh->VerticesCount = 0;
    Mesh->FaceCount = 0;
    
    int TotalVerticesCount = 0;
    int TotalFaceCount = 0;
    
    f32 MaxY = -1;
    f32 MinY = MINC_CHUNK_HEIGHT + 1;
    
    for(int y = 0; y < MINC_CHUNK_HEIGHT; y++)
    {
        for(int z = 0; z < MINC_CHUNK_WIDTH; z++)
        {
            for(int x = 0; x < MINC_CHUNK_WIDTH; x++)
            {
                int BlockIndex = GET_BLOCK_INDEX_IN_CHUNK(x, y, z);
                
                u8 BlockType = Chunk->Blocks[BlockIndex];
                
                b32 IsOuterX = (x == 0) || (x == MINC_CHUNK_WIDTH - 1);
                b32 IsOuterY = (y == 0) || (y == MINC_CHUNK_HEIGHT - 1);
                b32 IsOuterZ = (z == 0) || (z == MINC_CHUNK_WIDTH - 1);
                
                b32 BlockIsOuter = IsOuterX || IsOuterY || IsOuterZ;
                b32 BlockIsLeaves = ((BlockType == MincBlock_TreeLeaves) || 
                                     (BlockType == MincBlock_TreeLeavesSnow));
                
                if(BlockType != MincBlock_Empty)
                {
                    for(int NormalDir = 0;
                        NormalDir < MincFaceNormal_Count;
                        NormalDir++)
                    {
                        u8 NeighbourBlock = MincBlock_Ground;
                        
                        minc_chunk_side* SideChunk = Chunks.ChunksSides[NormalDir];
                        
                        switch(NormalDir)
                        {
                            case MincFaceNormal_Left:
                            {
                                if(x + 1 < MINC_CHUNK_WIDTH)
                                {
                                    NeighbourBlock = GetBlockInChunk(Chunk, x + 1, y, z);
                                }
                                else
                                {
                                    if(SideChunk)
                                    {
                                        NeighbourBlock = GetBlockInChunkSide(SideChunk, z, y);
                                    }
                                }
                            }break;
                            
                            case MincFaceNormal_Right:
                            {
                                if(x - 1 >= 0)
                                {
                                    NeighbourBlock = GetBlockInChunk(Chunk, x - 1, y, z);
                                }
                                else
                                {
                                    if(SideChunk)
                                    {
                                        NeighbourBlock = GetBlockInChunkSide(SideChunk, z, y);
                                    }
                                }
                            }break;
                            
                            case MincFaceNormal_Front:
                            {
                                if(z + 1 < MINC_CHUNK_WIDTH)
                                {
                                    NeighbourBlock = GetBlockInChunk(Chunk, x, y, z + 1);
                                }
                                else
                                {
                                    if(SideChunk)
                                    {
                                        NeighbourBlock = GetBlockInChunkSide(SideChunk, x, y);
                                    }
                                }
                            }break;
                            
                            case MincFaceNormal_Back:
                            {
                                if(z - 1 >= 0)
                                {
                                    NeighbourBlock = GetBlockInChunk(Chunk, x, y, z - 1);
                                }
                                else
                                {
                                    if(SideChunk)
                                    {
                                        NeighbourBlock = GetBlockInChunkSide(SideChunk, x, y);
                                    }
                                }
                            }break;
                            
                            case MincFaceNormal_Up:
                            {
                                if(y + 1 < MINC_CHUNK_HEIGHT)
                                {
                                    NeighbourBlock = GetBlockInChunk(Chunk, x, y + 1, z);
                                }
                                else
                                {
                                    if(SideChunk)
                                    {
                                        NeighbourBlock = GetBlockInChunkSide(SideChunk, x, z);
                                    }
                                }
                            }break;
                            
                            case MincFaceNormal_Down:
                            {
                                if(y - 1 >= 0)
                                {
                                    NeighbourBlock = GetBlockInChunk(Chunk, x, y - 1, z);
                                }
                                else
                                {
#if 0
                                    if(SideChunk)
                                    {
                                        NeighbourBlock = GetBlockInChunkSide(SideChunk, x, z);
                                    }
#else
                                    NeighbourBlock = 1;
#endif
                                    
                                }
                            }break;
                        }
                        
                        if(NeighbourBlock == MincBlock_Empty)
                        {
                            // NOTE(Dima): Generating and setting vertices
                            minc_block_texture_indices* TexIndices = &Minecraft->BlocksTextureIndices[BlockType];
                            int TexIndex = TexIndices->Sides[NormalDir];
                            
                            minc_face_offsets_to_vertex* OffsetToV = &Minecraft->OffsetsToV[NormalDir];
                            
                            for(int VertexIndex = 0;
                                VertexIndex < 6;
                                VertexIndex++)
                            {
                                minc_offsets_to_vertex* Offset = &OffsetToV->VertexOffsets[VertexIndex];
                                
                                if(Mesh->VerticesCount < Mesh->MaxVerticesCount)
                                {
                                    
                                    u32 Vertex = 0;
                                    
                                    int VertexX = x + Offset->x;
                                    int VertexY = y + Offset->y;
                                    int VertexZ = z + Offset->z;
                                    
                                    Vertex |= (VertexX & 63);
                                    Vertex |= (VertexZ & 63) << 6;
                                    Vertex |= (VertexY & 255) << 12;
                                    
                                    Mesh->Vertices[Mesh->VerticesCount++] = Vertex;
                                    
                                    if((f32)VertexY > MaxY)
                                    {
                                        MaxY = VertexY;
                                    }
                                    
                                    if((f32)VertexY < MinY)
                                    {
                                        MinY = VertexY;
                                    }
                                }
                                TotalVerticesCount++;
                            }
                            
                            if(Mesh->FaceCount < Mesh->MaxFaceCount)
                            {
                                // NOTE(Dima): Generating and Setting per face data
                                u32 PerFace = NormalDir;
                                PerFace |= ((TexIndex & 255) << 3);
                                PerFace |= (BlockIsLeaves & 1) << 11;
                                
                                Mesh->PerFaceData[Mesh->FaceCount++] = PerFace;
                            }
                            TotalFaceCount++;
                        }
                    }
                }
                
            } // Loop x
        } // Loop z
    } // Loop y
    
    Chunk->ExpectedVerticesCount = TotalVerticesCount;
    
    b32 Result = true;
    if(TotalVerticesCount > Mesh->VerticesCount)
    {
        Result = false;
    }
    
    // NOTE(Dima): Getting mesh bounding box
    v3 ChunkWorldP = V3(Chunk->CoordX * MINC_CHUNK_WIDTH,
                        0.0f,
                        Chunk->CoordZ * MINC_CHUNK_WIDTH);
    
    
    f32 YRad = std::abs(MaxY - MinY) * 0.5f;
    f32 SideRad = (f32)MINC_CHUNK_WIDTH * 0.5f;
    Chunk->BoundingSphereR = Sqrt(YRad * YRad + 2.0f * SideRad * SideRad);
    Chunk->BoundingSphereCenter = V3(((f32)Chunk->CoordX + 0.5f) * (f32)MINC_CHUNK_WIDTH,
                                     MinY + (MaxY - MinY) * 0.5f,
                                     ((f32)Chunk->CoordZ + 0.5f) * (f32)MINC_CHUNK_WIDTH);
    Chunk->BoundingSphereGenerated = true;
    
    return(Result);
}


INTERNAL_FUNCTION void MincCopyTempMeshToMesh(voxel_mesh* Mesh, minc_temp_mesh* TempMesh)
{
    if(Mesh->Handle.Initialized)
    {
        // NOTE(Dima): If mesh is already initialized - then just invalidate to reallocate it.
        InvalidateHandle(&Mesh->Handle);
    }
    
    Mesh->VerticesCount = TempMesh->VerticesCount;
    Mesh->FaceCount = TempMesh->FaceCount;
    
    mi DataSize = TempMesh->VerticesCount * sizeof(u32) + TempMesh->FaceCount * sizeof(u32);
    if(DataSize > 0)
    {
        Mesh->Free = malloc(DataSize);
        
        // NOTE(Dima): Copy vertices;
        memcpy(Mesh->Free, 
               TempMesh->Vertices, 
               TempMesh->VerticesCount * sizeof(u32));
        
        // NOTE(Dima): Copy per face data
        memcpy((u8*)Mesh->Free + Mesh->VerticesCount * sizeof(u32),
               TempMesh->PerFaceData,
               TempMesh->FaceCount * sizeof(u32));
        
        // NOTE(Dima): Reset pointers
        Mesh->Vertices = (u32*)Mesh->Free;
        Mesh->PerFaceData = (u32*)((u8*)Mesh->Free + Mesh->VerticesCount * sizeof(u32));
    }
    else
    {
        Mesh->Vertices = 0;
        Mesh->PerFaceData = 0;
        
        Mesh->Free = 0;
    }
}


INTERNAL_FUNCTION inline void MincSetColumn(minc_chunk* Blocks,
                                            int MinIndex,
                                            int MaxIndex,
                                            int X, int Z,
                                            u8 Block)
{
    int Min = Clamp(MinIndex, 0, MINC_CHUNK_HEIGHT);
    int Max = Clamp(MaxIndex, 0, MINC_CHUNK_HEIGHT);
    
    for(int Index = Min;
        Index <= Max;
        Index++)
    {
        Blocks->Blocks[MincGetBlockIndex(X, Index, Z)] = Block;
    }
}

inline u64 MincGetKey(int X, int Y, int Z)
{
    u64 Key = (u32)X | ((u64)((u32)Z) << 32);
    //u64 Key = (u64)X | ((u64)Z << 32);
    
    return(Key);
}

INTERNAL_FUNCTION inline minc_chunk_meta_slot* MincFindSlot(minecraft* Mine, 
                                                            int X, int Z)
{
    u64 Key = MincGetKey(X, 0, Z);
    u32 KeyHash = Hash32Slow(Key);
    int Index = KeyHash % MINC_META_TABLE_SIZE;
    
    minc_chunk_meta_slot* Result = 0;
    
    minc_chunk_meta_slot* At = Mine->MetaTable[Index];
    while(At)
    {
        if(At->Key == Key)
        {
            Result = At;
            break;
        }
        
        At = At->NextInHash;
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline minc_chunk_meta* MincFindChunkMeta(minecraft* Mine, 
                                                            int X, int Z)
{
    minc_chunk_meta* Meta = 0;
    
    minc_chunk_meta_slot* Slot = MincFindSlot(Mine, X, Z);
    if(Slot)
    {
        Meta = Slot->Meta;
    }
    
    return(Meta);
}

INTERNAL_FUNCTION void CopyChunkMeta(minc_chunk_meta* Dst,
                                     minc_chunk_meta* Src)
{
    for(int i = 0; i < MINC_CHUNK_WIDTH * MINC_CHUNK_WIDTH; i++)
    {
        Dst->BiomeMap[i] = Src->BiomeMap[i];
        Dst->HeightMap[i] = Src->HeightMap[i];
    }
}

struct minc_generate_maps_work
{
    minc_chunk_meta* Meta;
    minecraft* Mine;
    task_memory* Task;
    
    int X;
    int Z;
};

INTERNAL_FUNCTION PLATFORM_CALLBACK(MincGenerateMapsWork)
{
    minc_generate_maps_work* Work = (minc_generate_maps_work*)Data;
    
    minc_chunk_meta* Meta = Work->Meta;
    minecraft* Mine = Work->Mine;
    
    u8* BiomeMap = Meta->BiomeMap;
    u16* HeightMap = Meta->HeightMap;
    
    v3 ChunkWorldP = V3(Work->X * MINC_CHUNK_WIDTH,
                        0.0f,
                        Work->Z * MINC_CHUNK_WIDTH);
    
    for(int z = 0; z < MINC_CHUNK_WIDTH; z++)
    {
        for(int x = 0; x < MINC_CHUNK_WIDTH; x++)
        {
            int TargetIndex = z * MINC_CHUNK_WIDTH + x;
            
            // NOTE(Dima): Generating and finding biome
            f32 BiomeNoiseInit = stb_perlin_noise3((ChunkWorldP.x + x) / 512.0f, 
                                                   0.0f, 
                                                   (ChunkWorldP.z + z) / 512.0f,
                                                   0, 0, 0);
            f32 BiomeNoise = BiomeNoiseInit * 0.5f + 0.5f;
            
            // NOTE(Dima): Finding corresponding biome
            int Biome = -1;
            for(int BiomeIndex = 0;
                BiomeIndex < MincBiome_Count;
                BiomeIndex++)
            {
                minc_biome* CurBiome = &Mine->Biomes[BiomeIndex];
                
                if((BiomeNoise >= CurBiome->StartWeight) && 
                   (BiomeNoise <= CurBiome->EndWeight))
                {
                    // NOTE(Dima): Biome found
                    Biome = BiomeIndex;
                    break;
                }
            }
            
            Assert(Biome != -1);
            
            // NOTE(Dima): Set in biome map
            BiomeMap[TargetIndex] = Biome;
            
            // NOTE(Dima): Generating & Setting noise in the noise map
            minc_biome* CurBiome = Mine->Biomes + Biome;
            f32 TempX = (ChunkWorldP.x + x) / CurBiome->NoiseFrequency;
            f32 TempZ = (ChunkWorldP.z + z) / CurBiome->NoiseFrequency;
            
            f32 Noise = stb_perlin_fbm_noise3(TempX, 0.0f, TempZ, 2.0f, 0.5f, 6);
            
            // NOTE(Dima): Generating height and setting it in height map
            f32 ScaledNoise = Noise * CurBiome->NoiseScale;
            int Height = CurBiome->BaseHeight + (int)ScaledNoise;
            int CurrentHeight = ClampFloat(Height, 0, MINC_CHUNK_HEIGHT - 1);
            HeightMap[TargetIndex] = CurrentHeight;
        }
    }
    
    Meta->State.store(MincChunk_ReadyToFixBiomeGaps);
    FreeTaskMemory(Work->Task);
}

struct minc_fix_gaps_work
{
    minc_chunk_meta* Meta;
    minecraft* Mine;
    task_memory* Task;
    
    int X;
    int Z;
    
    minc_lookup_chunks_metas LookupMetas;
};

INTERNAL_FUNCTION PLATFORM_CALLBACK(MincFixBiomeGapsWork)
{
    minc_fix_gaps_work* Work = (minc_fix_gaps_work*)Data;
    
    v3 ChunkWorldP = V3(Work->X * MINC_CHUNK_WIDTH,
                        0.0f,
                        Work->Z * MINC_CHUNK_WIDTH);
    
    minc_chunk_meta* Meta = Work->Meta;
    minecraft* Mine = Work->Mine;
    
    int X = Work->X;
    int Z = Work->Z;
    
    u8* BiomeMap = Meta->BiomeMap;
    u16* HeightMap = Meta->HeightMap;
    
    f32 AvgNearHeights[MINC_CHUNK_WIDTH * MINC_CHUNK_WIDTH];
    f32 BiomeNearPerc[MINC_CHUNK_WIDTH * MINC_CHUNK_WIDTH];
    
    // NOTE(Dima): Fixing biome gaps. First - calc avg biome nearby percentage and avg height
    for(int z = 0; z < MINC_CHUNK_WIDTH; z++)
    {
        for(int x = 0; x < MINC_CHUNK_WIDTH; x++)
        {
            int IndexInMap = z * MINC_CHUNK_WIDTH + x;
            
            // NOTE(Dima): Lookup into neighbour cells
            int TotalHeightNear = 0.0f;
            f32 TotalCurBiome = 0.0f;
            
            u8 CurBiome = BiomeMap[IndexInMap];
            
            int CountCheckedNear = 0;
            
            int r = 5;
            for(int a = -r; a <= r; a++)
            {
                int LookupZ = a + z;
                int LookupChunkZ = 1;
                if(LookupZ < 0)
                {
                    LookupZ = MINC_CHUNK_WIDTH + LookupZ;
                    LookupChunkZ--;
                }
                else if(LookupZ >= MINC_CHUNK_WIDTH)
                {
                    LookupZ = LookupZ - MINC_CHUNK_WIDTH;
                    LookupChunkZ++;
                }
                
                for(int b = -r; b <= r; b++)
                {
                    int LookupX = b + x;
                    int LookupChunkX = 1;
                    if(LookupX < 0)
                    {
                        LookupX = MINC_CHUNK_WIDTH + LookupX;
                        LookupChunkX--;
                    }
                    else if(LookupX >= MINC_CHUNK_WIDTH)
                    {
                        LookupX = LookupX - MINC_CHUNK_WIDTH;
                        LookupChunkX++;
                    }
                    
                    minc_chunk_meta* Lookup = Work->LookupMetas.LookupChunks[LookupChunkZ * 3 + LookupChunkX];
                    
                    if(Lookup)
                    {
                        int LookupIndexInMap = LookupZ * MINC_CHUNK_WIDTH + LookupX;
                        
                        if(CurBiome == Lookup->BiomeMap[LookupIndexInMap])
                        {
                            TotalCurBiome += 1.0f;
                        }
                        
                        TotalHeightNear += Lookup->HeightMap[LookupIndexInMap];
                        
                        CountCheckedNear++;
                    }
                }
            }
            
            BiomeNearPerc[IndexInMap] = TotalCurBiome / (f32)CountCheckedNear;
            AvgNearHeights[IndexInMap] = (f32)TotalHeightNear / (f32)CountCheckedNear;
        }
    }
    
    for(int z = 0; z < MINC_CHUNK_WIDTH; z++)
    {
        for(int x = 0; x < MINC_CHUNK_WIDTH; x++)
        {
            int IndexInMap = z * MINC_CHUNK_WIDTH + x;
            
            int TargetHeight = HeightMap[IndexInMap];
            if(BiomeNearPerc[IndexInMap] < 0.99f)
            {
                TargetHeight = AvgNearHeights[IndexInMap];
            }
            
            HeightMap[IndexInMap] = TargetHeight;
        }
    }
    
    Meta->State.store(MincChunk_ReadyToGenerateChunk);
    FreeTaskMemory(Work->Task);
}

struct minc_generate_chunk_work
{
    minecraft* Minecraft;
    task_memory* TaskMemory;
    minc_chunk* Chunk;
    minc_chunk_meta* Meta;
};

INTERNAL_FUNCTION PLATFORM_CALLBACK(MincGenerateChunkWork)
{
    minc_generate_chunk_work* Work = (minc_generate_chunk_work*)Data;
    
    minecraft* Minecraft = Work->Minecraft;
    minc_chunk_meta* Meta = Work->Meta;
    minc_chunk* Chunk = Work->Chunk;
    task_memory* TaskMemory = Work->TaskMemory;
    
    random_generation Random = SeedRandom(Hash32(MincGetKey(Chunk->CoordX, 0, Chunk->CoordZ)));
    
    for(int BlockIndex = 0;
        BlockIndex < MINC_CHUNK_COUNT;
        BlockIndex++)
    {
        Chunk->Blocks[BlockIndex] = MincBlock_Empty;
    }
    
    // NOTE(Dima): Heightmaps and all other stuff should be generated
    Assert(Meta);
    
    for(int z = 0; z < MINC_CHUNK_WIDTH; z++)
    {
        for(int x = 0; x < MINC_CHUNK_WIDTH; x++)
        {
            int IndexInMap = z * MINC_CHUNK_WIDTH + x;
            
            minc_biome* Biome = Minecraft->Biomes + Meta->BiomeMap[IndexInMap];
            
            // NOTE(Dima): Setting block type at height
            int CurrentHeight = Meta->HeightMap[IndexInMap];
            
            if(CurrentHeight < 35)
            {
                int GroundLayerHeight = RandomBetweenU32(&Random, 5, 7);
                MincSetColumn(Chunk, 
                              CurrentHeight - (GroundLayerHeight - 1), 
                              CurrentHeight,
                              x, z,
                              MincBlock_Sand);
                
                // NOTE(Dima): Setting stones
                CurrentHeight -= GroundLayerHeight;
                MincSetColumn(Chunk,
                              0, CurrentHeight,
                              x, z,
                              Biome->LayerBlocks[3]);
            }
            else if(Biome->HasTrees)
            {
                f32 RandomVal = RandomUnilateral(&Random);
                
                if(RandomVal < Biome->TreeDensity)
                {
                    int TrunkHeight = RandomBetweenU32(&Random, Biome->TrunkMinH, Biome->TrunkMaxH + 1);
                    int CrownHeight = RandomBetweenU32(&Random, Biome->CrownMinH, Biome->CrownMaxH + 1);
                    int CrownRadius = RandomBetweenU32(&Random, Biome->CrownMinRad, Biome->CrownMaxRad + 1);
                    
                    MincSetColumn(Chunk, 
                                  CurrentHeight, 
                                  CurrentHeight + TrunkHeight - 1,
                                  x, z, 
                                  Biome->TreeTrunkBlock);
                    
                    for(int dz = -CrownRadius; dz <= CrownRadius; dz++)
                    {
                        for(int dx = -CrownRadius; dx <= CrownRadius; dx++)
                        {
                            int TargetX = x + dx;
                            int TargetZ = z + dz;
                            
                            if(TargetX >= 0 && TargetX < MINC_CHUNK_WIDTH &&
                               TargetZ >= 0 && TargetZ < MINC_CHUNK_WIDTH)
                            {
                                int AdditionalH = 0;
                                if(dx == 0 && dz == 0)
                                {
                                    AdditionalH = 1;
                                }
                                
                                MincSetColumn(Chunk,
                                              CurrentHeight + TrunkHeight - 1 + AdditionalH,
                                              CurrentHeight + TrunkHeight + CrownHeight - 2,
                                              TargetX, TargetZ,
                                              Biome->TreeCrownBlock);
                            }
                            else
                            {
                                // TODO(Dima): Do something to push this column into neighbour chunk
                            }
                        }
                    }
                }
                
                Chunk->Blocks[MincGetBlockIndex(x, CurrentHeight, z)] = Biome->LayerBlocks[0];
                
                // NOTE(Dima): 
                --CurrentHeight;
                int GroundLayerHeight = RandomBetweenU32(&Random, 5, 7);
                MincSetColumn(Chunk, 
                              CurrentHeight - (GroundLayerHeight - 1), 
                              CurrentHeight,
                              x, z,
                              Biome->LayerBlocks[1]);
                
                // NOTE(Dima): Setting stones
                CurrentHeight -= GroundLayerHeight;
                MincSetColumn(Chunk,
                              0, CurrentHeight,
                              x, z,
                              Biome->LayerBlocks[3]);
            }
        }
    }
    
    Meta->State = MincChunk_ReadyToGenerateMesh;
    
    FreeTaskMemory(TaskMemory);
}

struct minc_generate_mesh_work
{
    minecraft* Minecraft;
    task_memory* TaskMemory;
    minc_chunk* Chunk;
    minc_chunk_meta* Meta;
    
    minc_generate_chunks GenerationChunks;
    
    minc_temp_mesh* TempMesh;
};

INTERNAL_FUNCTION PLATFORM_CALLBACK(MincGenerateMeshWork)
{
    minc_generate_mesh_work* Work = (minc_generate_mesh_work*)Data;
    
    minecraft* Minecraft = Work->Minecraft;
    task_memory* TaskMemory = Work->TaskMemory;
    minc_chunk* Chunk = Work->Chunk;
    minc_chunk_meta* Meta = Work->Meta;
    
    Chunk->SucceededGenerateMesh = GenerateChunkMesh(Minecraft, 
                                                     Work->TempMesh,
                                                     Work->GenerationChunks);
    
    Meta->State = MincChunk_GeneratingMeshFinalizing;
}

INTERNAL_FUNCTION inline minc_chunk_side* GetFreeChunkSide(minecraft* Mine)
{
    minc_chunk_side* Result = Mine->ChunkSidePool;
    
    if(Result)
    {
        Mine->ChunkSidePool = Result->NextInList;
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline void ReturnUsedChunkSide(minecraft* Mine, minc_chunk_side* Side)
{
    Side->NextInList = Mine->ChunkSidePool;
    Mine->ChunkSidePool = Side;
}

INTERNAL_FUNCTION inline void MincGetGeneratedChunk(minecraft* Mine, 
                                                    int X, 
                                                    int Z,
                                                    minc_generate_chunks* GenerateChunks,
                                                    int Direction,
                                                    int* MissedCount,
                                                    b32 JustCountChunks)
{
    minc_chunk_meta_slot* Slot = MincFindSlot(Mine, X, Z);
    
    minc_chunk_side* Result = 0;
    minc_chunk* ResultChunk = 0;
    
    if(Slot)
    {
        if(Slot->Chunk)
        {
            if(Slot->Meta->State > MincChunk_GeneratingChunk)
            {
                // NOTE(Dima): Getting source chunk
                minc_chunk* Src = Slot->Chunk;
                
                ResultChunk = Src;
                
                if(!JustCountChunks)
                {
                    
                    Result = GetFreeChunkSide(Mine);
                    
                    if(Result)
                    {
                        // NOTE(Dima): Getting copy of adjacent neighbours chunk blocks
                        switch(Direction)
                        {
                            case MincFaceNormal_Left:
                            {
                                for(int y = 0; y < MINC_CHUNK_HEIGHT; y++)
                                {
                                    for(int z = 0; z < MINC_CHUNK_WIDTH; z++)
                                    {
                                        int DstIndex = y * MINC_CHUNK_WIDTH + z;
                                        
                                        Result->Blocks[DstIndex] = GetBlockInChunk(Src, 0, y, z);
                                    }
                                }
                            }break;
                            
                            case MincFaceNormal_Right:
                            {
                                for(int y = 0; y < MINC_CHUNK_HEIGHT; y++)
                                {
                                    for(int z = 0; z < MINC_CHUNK_WIDTH; z++)
                                    {
                                        int DstIndex = y * MINC_CHUNK_WIDTH + z;
                                        
                                        Result->Blocks[DstIndex] = GetBlockInChunk(Src, 
                                                                                   MINC_CHUNK_WIDTH - 1, y, z);
                                    }
                                }
                            }break;
                            
                            case MincFaceNormal_Front:
                            {
                                for(int y = 0; y < MINC_CHUNK_HEIGHT; y++)
                                {
                                    for(int x = 0; x < MINC_CHUNK_WIDTH; x++)
                                    {
                                        int DstIndex = y * MINC_CHUNK_WIDTH + x;
                                        
                                        Result->Blocks[DstIndex] = GetBlockInChunk(Src, x, y, 0);
                                    }
                                }
                            }break;
                            
                            case MincFaceNormal_Back:
                            {
                                for(int y = 0; y < MINC_CHUNK_HEIGHT; y++)
                                {
                                    for(int x = 0; x < MINC_CHUNK_WIDTH; x++)
                                    {
                                        int DstIndex = y * MINC_CHUNK_WIDTH + x;
                                        
                                        Result->Blocks[DstIndex] = GetBlockInChunk(Src, 
                                                                                   x, y, 
                                                                                   MINC_CHUNK_WIDTH - 1);
                                    }
                                }
                            }break;
                            
                            case MincFaceNormal_Up:
                            {
                                for(int z = 0; z < MINC_CHUNK_WIDTH; z++)
                                {
                                    for(int x = 0; x < MINC_CHUNK_WIDTH; x++)
                                    {
                                        int DstIndex = z * MINC_CHUNK_WIDTH + x;
                                        
                                        Result->Blocks[DstIndex] = GetBlockInChunk(Src, x, 0, z);
                                    }
                                }
                            }break;
                            
                            case MincFaceNormal_Down:
                            {
                                for(int z = 0; z < MINC_CHUNK_WIDTH; z++)
                                {
                                    for(int x = 0; x < MINC_CHUNK_WIDTH; x++)
                                    {
                                        int DstIndex = z * MINC_CHUNK_WIDTH + x;
                                        
                                        Result->Blocks[DstIndex] = GetBlockInChunk(Src, x, MINC_CHUNK_HEIGHT - 1, z);
                                    }
                                }
                            }break;
                        }
                    }
                    else
                    {
                        if(MissedCount)
                        {
                            (*MissedCount)++;
                        }
                    }
                }
            }
        }
    }
    
    GenerateChunks->ChunksSides[Direction] = Result;
    GenerateChunks->Chunks[Direction] = ResultChunk;
}

INTERNAL_FUNCTION void MincGetGenerationChunks(minecraft* Mine, 
                                               minc_generate_chunks* Chunks,
                                               minc_chunk* Chunk,
                                               int* OutChunksCount,
                                               int* OutMissedCount)
{
    int X = Chunk->CoordX;
    int Z = Chunk->CoordZ;
    
    b32 JustCount = OutMissedCount == 0;
    
    for(int i = 0; i < 6; i++)
    {
        Chunks->ChunksSides[i] = 0;
        Chunks->Chunks[i] = 0;
    }
    
    int MissedCount = 0;
    
    Chunks->Dst = Chunk;
    
    MincGetGeneratedChunk(Mine, X + 1, Z,
                          Chunks,
                          MincFaceNormal_Left,
                          &MissedCount,
                          JustCount);
    
    if(!MissedCount)
    {
        
        MincGetGeneratedChunk(Mine, X - 1, Z, 
                              Chunks,
                              MincFaceNormal_Right,
                              &MissedCount,
                              JustCount);
    }
    
    if(!MissedCount)
    {
        
        MincGetGeneratedChunk(Mine, X, Z + 1, 
                              Chunks,
                              MincFaceNormal_Front,
                              &MissedCount,
                              JustCount);
    }
    
    if(!MissedCount)
    {
        
        MincGetGeneratedChunk(Mine, X, Z - 1, 
                              Chunks,
                              MincFaceNormal_Back,
                              &MissedCount,
                              JustCount);
    }
    
    int GenerationChunksCount = 0;
    for(int i = 0; i < 6; i++)
    {
        if(Chunks->Chunks[i])
        {
            GenerationChunksCount++;
        }
    }
    
    *OutChunksCount = GenerationChunksCount;
    if(OutMissedCount)
    {
        *OutMissedCount = MissedCount;
    }
}

INTERNAL_FUNCTION void MincReturnUsedChunkSides(minecraft* Mine, minc_generate_chunks* GenerationChunks)
{
    // NOTE(Dima): Freing used chunk sides
    for(int i = 0; i < MincFaceNormal_Count; i++)
    {
        minc_chunk_side* ChunkSide = GenerationChunks->ChunksSides[i];
        
        if(ChunkSide)
        {
            ReturnUsedChunkSide(Mine, ChunkSide);
        }
    }
}



GLOBAL_VARIABLE int Minc_MaxVertCount[] = 
{
    25002,
    50004,
    100008,
    500004,
    MINC_MAX_VERTS_COUNT,
};

GLOBAL_VARIABLE int Minc_TempMeshCountInList[] =
{
    80,
    40,
    20,
    10,
    2,
};

INTERNAL_FUNCTION minc_temp_mesh* GetFreeTempMesh(minecraft* Mine, int VerticesCount)
{
    minc_temp_mesh* Result = 0;
    
    for(int ListIndex = 0;
        ListIndex < ARC(Mine->TempMeshLists);
        ListIndex++)
    {
        minc_temp_mesh* At = Mine->TempMeshLists[ListIndex];
        
        if(At && (At->MaxVerticesCount >= VerticesCount))
        {
            Result = At;
            
            Mine->TempMeshLists[ListIndex] = Result->NextInList;
            Result->NextInList = 0;
            
            break;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION void ReturnFreeTempMesh(minecraft* Mine, minc_temp_mesh* Mesh)
{
    
    int TargetListIndex = Mesh->TempMeshListIndex;
    Assert(Mesh->MaxVerticesCount == Minc_MaxVertCount[TargetListIndex]);
    
    // NOTE(Dima): Storing back into singly linked list
    Mesh->NextInList = Mine->TempMeshLists[TargetListIndex];
    Mine->TempMeshLists[TargetListIndex] = Mesh;
}

INTERNAL_FUNCTION inline void MincFreeChunkMesh(minc_chunk* Chunk)
{
    // NOTE(Dima): Free chunk mesh data
    if(Chunk->Mesh.Free && Chunk->Mesh.VerticesCount > 0)
    {
        free(Chunk->Mesh.Free);
    }
    Chunk->Mesh.Free = 0;
}

INTERNAL_FUNCTION inline b32 GetLookupChunksMetas(minecraft* Mine, 
                                                  minc_lookup_chunks_metas* Metas,
                                                  int X, int Z,
                                                  u32 MinimumState)
{
    int CountOK = 0;
    for(int j = -1; j <= 1; j++)
    {
        for(int i = -1; i <= 1; i++)
        {
            int LookupIndex = (j + 1) * 3 + (i + 1);
            Metas->LookupChunks[LookupIndex] = 0;
            
            minc_chunk_meta* LookupChunkMeta = MincFindChunkMeta(Mine, X + i, Z + j);
            if(LookupChunkMeta)
            {
                if(LookupChunkMeta->State >= MinimumState)
                {
                    Metas->LookupChunks[LookupIndex] = LookupChunkMeta;
                    CountOK++;
                }
            }
        }
    }
    
    b32 AllOK = (CountOK == 9);
    
    return(AllOK);
}

INTERNAL_FUNCTION void UpdateChunkAtIndex(minecraft* Minecraft,
                                          int X, int Z,
                                          int PlayerChunkX,
                                          int PlayerChunkZ)
{
    minc_chunk_meta_slot* ChunkMetaSlot = MincFindSlot(Minecraft, X, Z);
    
    v3 ChunkWorldP = V3(X * MINC_CHUNK_WIDTH,
                        0.0f,
                        Z * MINC_CHUNK_WIDTH);
    
    if(ChunkMetaSlot)
    {
        minc_chunk* Chunk = ChunkMetaSlot->Chunk;
        minc_chunk_meta* Meta = ChunkMetaSlot->Meta;
        
        switch(Meta->State)
        {
            case MincChunk_Unloaded:
            {
                Assert(Chunk == 0);
                
                // NOTE(Dima): Start generating maps work
                task_memory* Task = GetTaskMemoryForUse(Minecraft->TaskPool, sizeof(minc_generate_maps_work));
                Assert(Task);
                
                minc_generate_maps_work* Work = PushStruct(&Task->Arena, minc_generate_maps_work);
                
                Work->Meta = Meta;
                Work->Mine = Minecraft;
                Work->Task = Task;
                Work->X = X;
                Work->Z = Z;
                
                Meta->State = MincChunk_GeneratingMaps;
                KickJob(MincGenerateMapsWork, Work, JobPriority_High);
            }break;
            
            case MincChunk_ReadyToFixBiomeGaps:
            {
                minc_lookup_chunks_metas LookupMetas;
                b32 CanFix = GetLookupChunksMetas(Minecraft, &LookupMetas, X, Z, MincChunk_ReadyToFixBiomeGaps);
                
                if(CanFix)
                {
                    task_memory* Task = GetTaskMemoryForUse(Minecraft->TaskPool, sizeof(minc_fix_gaps_work));
                    Assert(Task);
                    
                    minc_fix_gaps_work* Work = PushStruct(&Task->Arena, minc_fix_gaps_work);
                    
                    Work->Meta = Meta;
                    Work->Mine = Minecraft;
                    Work->Task = Task;
                    Work->X = X;
                    Work->Z = Z;
                    Work->LookupMetas = LookupMetas;
                    
                    Meta->State = MincChunk_FixingBiomeGaps;
                    KickJob(MincFixBiomeGapsWork, Work, JobPriority_High);
                }
            }break;
            
            case MincChunk_ReadyToGenerateChunk:
            {
                
                // NOTE(Dima): Allocating chunk
                Assert(ChunkMetaSlot->Chunk == 0);
                
                ChunkMetaSlot->Chunk = (minc_chunk*)malloc(sizeof(minc_chunk));
                Chunk = ChunkMetaSlot->Chunk;
                memset(Chunk->Blocks, 0, MINC_CHUNK_COUNT);
                Chunk->Mesh.Free = 0;
                Chunk->ExpectedVerticesCount = 0;
                Chunk->BoundingSphereGenerated = false;
                
                // NOTE(Dima): Setting chunk
                Chunk->CoordX = X;
                Chunk->CoordZ = Z;
                
                // NOTE(Dima): Starting work to generate chunk
                task_memory* Task = GetTaskMemoryForUse(Minecraft->TaskPool, sizeof(minc_generate_chunk_work));
                Assert(Task);
                
                minc_generate_chunk_work* Work = PushStruct(&Task->Arena, minc_generate_chunk_work);
                
                Work->Minecraft = Minecraft;
                Work->TaskMemory = Task;
                Work->Chunk = Chunk;
                Work->Meta = Meta;
                
                Meta->State = MincChunk_GeneratingChunk;
                KickJob(MincGenerateChunkWork, Work, JobPriority_High);
            }break;
            
            case MincChunk_ReadyToGenerateMesh:
            {
                Assert(Chunk->ExpectedVerticesCount <= MINC_MAX_VERTS_COUNT);
                
                minc_temp_mesh* TempMesh = GetFreeTempMesh(Minecraft, Chunk->ExpectedVerticesCount);
                if(TempMesh)
                {
                    task_memory* Task = GetTaskMemoryForUse(Minecraft->TaskPool, sizeof(minc_generate_mesh_work));
                    if(Task)
                    {
                        minc_generate_mesh_work* Work = PushStruct(&Task->Arena, minc_generate_mesh_work);
                        Chunk->GenerateMeshWork = Work;
                        
                        // NOTE(Dima): Getting side chunks
                        Work->GenerationChunks = {};
                        int MissedChunksSides;
                        MincGetGenerationChunks(Minecraft, 
                                                &Work->GenerationChunks,
                                                Chunk,
                                                &Chunk->GenerationSidesCount,
                                                &MissedChunksSides);
                        
                        if(MissedChunksSides > 0)
                        {
                            MincReturnUsedChunkSides(Minecraft, &Work->GenerationChunks);
                            
                            FreeTaskMemory(Task);
                            ReturnFreeTempMesh(Minecraft, TempMesh);
                        }
                        else
                        {
                            // NOTE(Dima): Filling work parameters
                            Work->Chunk = Chunk;
                            Work->Minecraft = Minecraft;
                            Work->TaskMemory = Task;
                            Work->Meta = Meta;
                            Work->Chunk->Mesh.Handle = CreateRendererHandle(RendererHandle_Mesh);
                            Work->Chunk->Mesh.PerFaceBufHandle = CreateRendererHandle(RendererHandle_TextureBuffer);
                            Work->TempMesh = TempMesh;
                            
                            Meta->State = MincChunk_GeneratingMesh;
                            KickJob(MincGenerateMeshWork, Work, JobPriority_High);
                        }
                    }
                    else
                    {
                        ReturnFreeTempMesh(Minecraft, TempMesh);
                    }
                }
            }break;
            
            case MincChunk_GeneratingMeshFinalizing:
            {
                Assert(Chunk->GenerateMeshWork);
                Assert(Chunk->Mesh.Free == 0);
                
                minc_generate_mesh_work* Work = Chunk->GenerateMeshWork;
                task_memory* TaskMemory = Work->TaskMemory;
                minc_chunk_meta* Meta = Work->Meta;
                
                MincReturnUsedChunkSides(Minecraft, &Work->GenerationChunks);
                
                if(Chunk->SucceededGenerateMesh)
                {
                    MincCopyTempMeshToMesh(&Chunk->Mesh, Work->TempMesh);
                    ReturnFreeTempMesh(Minecraft, Work->TempMesh);
                    FreeTaskMemory(TaskMemory);
                    
                    Meta->State = MincChunk_MeshGenerated;
                    
                    Chunk->GenerateMeshWork = 0;
                    
                    // NOTE(Dima): Invalidate handles so that they are reallocated if were allocated previously
                    InvalidateHandle(&Chunk->Mesh.Handle);
                    InvalidateHandle(&Chunk->Mesh.PerFaceBufHandle);
                }
                else
                {
                    ReturnFreeTempMesh(Minecraft, Work->TempMesh);
                    FreeTaskMemory(TaskMemory);
                    
                    // NOTE(Dima): Try to generate mesh once again
                    Meta->State = MincChunk_ReadyToGenerateMesh;
                    
                    Chunk->GenerateMeshWork = 0;
                }
            }break;
        }
        
        if(Chunk)
        {
            int DiffX = Chunk->CoordX - PlayerChunkX;
            int DiffZ = Chunk->CoordZ - PlayerChunkZ;
            
            int DistToChunk = Sqrt(DiffX * DiffX + DiffZ * DiffZ);
            
            if(DistToChunk > Minecraft->ChunksViewDistance)
            {
                // NOTE(Dima): Deallocating mesh
                if(Chunk->Mesh.Handle.Initialized &&
                   Chunk->Mesh.PerFaceBufHandle.Initialized)
                {
                    u32 DesiredState = MincChunk_ReadyToGenerateMesh;
                    u32 CompareState = MincChunk_MeshGenerated;
                    
                    if(ChunkMetaSlot->Meta->State.compare_exchange_weak(CompareState, DesiredState))
                    {
                        RenderPushDeallocateHandle(&Chunk->Mesh.Handle);
                        RenderPushDeallocateHandle(&Chunk->Mesh.PerFaceBufHandle);
                        
                        MincFreeChunkMesh(Chunk);
                    }
                }
                
#if 1                
                // NOTE(Dima): We can only free chunk if it's handles were freed
                if((Chunk->Mesh.Handle.Initialized == false) &&
                   (Chunk->Mesh.PerFaceBufHandle.Initialized == false))
                {
                    // NOTE(Dima): Deallocating chunk
                    u32 DesiredState = MincChunk_ReadyToGenerateChunk;
                    u32 CompareState = MincChunk_ReadyToGenerateMesh;
                    
                    if(ChunkMetaSlot->Meta->State.compare_exchange_weak(CompareState, DesiredState))
                    {
                        free(Chunk);
                        
                        // NOTE(Dima): Remove this chunk from map
                        ChunkMetaSlot->Chunk = 0;
                    }
                }
#endif
            }
            else
            {
                if(Meta->State == MincChunk_MeshGenerated)
                {
                    minc_generate_chunks GenChunks = {};
                    int GenSidesCount;
                    int MissedCount;
                    MincGetGenerationChunks(Minecraft, &GenChunks, Chunk,
                                            &GenSidesCount, 0);
                    b32 Changed = GenSidesCount != Chunk->GenerationSidesCount;
                    
                    // NOTE(Dima): If chunks sides changed then we can reallocate mesh
                    if(Changed)
                    {
                        u32 DesiredState = MincChunk_ReadyToGenerateMesh;
                        u32 CompareState = MincChunk_MeshGenerated;
                        
                        if(Meta->State.compare_exchange_weak(CompareState, DesiredState))
                        {
                            Chunk->GenerationSidesCount = GenSidesCount;
                            MincFreeChunkMesh(Chunk);
                        }
                    }
                    else
                    {
                        // NOTE(Dima): Render chunk
                        PushVoxelChunkMesh(&Chunk->Mesh, 
                                           ChunkWorldP, 
                                           CullingInfo(Chunk->BoundingSphereCenter,
                                                       Chunk->BoundingSphereR,
                                                       true));
                    }
                }
            }
        }
    }
}

INTERNAL_FUNCTION void MincTryInitChunk(minecraft* Minecraft, 
                                        int X, int Z)
{
    minc_chunk_meta_slot* ChunkMetaSlot = MincFindSlot(Minecraft, X, Z);
    
    if(!ChunkMetaSlot)
    {
        // NOTE(Dima): Insert meta to table
        u64 Key = MincGetKey(X, 0, Z);
        u32 KeyHash = Hash32Slow(Key);
        int Index = KeyHash % MINC_META_TABLE_SIZE;
        
        minc_chunk_meta_slot* At = Minecraft->MetaTable[Index];
        
        while(At)
        {
            // NOTE(Dima): It can not exist here. Otherwise it should have been found previously
            Assert(At->Key != Key);
            
            At = At->NextInHash;
        }
        
        // NOTE(Dima): Allocating meta
        minc_chunk_meta_slot* NewMetaSlot = PushStruct(Minecraft->Arena, minc_chunk_meta_slot);
        NewMetaSlot->NextInHash = Minecraft->MetaTable[Index];
        NewMetaSlot->Key = Key;
        
        Minecraft->MetaTable[Index] = NewMetaSlot;
        
        // NOTE(Dima): Allocating chunk meta
        minc_chunk_meta* ChunkMeta = PushStructSafe(Minecraft->Arena, minc_chunk_meta);
        ChunkMeta->CoordX = X;
        ChunkMeta->CoordZ = Z;
        
        // NOTE(Dima): Setting data
        NewMetaSlot->Meta = ChunkMeta;
        NewMetaSlot->Chunk = 0;
        
        // NOTE(Dima): Starting generating meta for chunk
        ChunkMeta->State.store(MincChunk_Unloaded, std::memory_order_relaxed);
    }
}

INTERNAL_FUNCTION void MincInitTempMeshList(minecraft* Mine, 
                                            int ListIndex, 
                                            int MaxVertsCount,
                                            int MeshesCount)
{
    Mine->TempMeshLists[ListIndex] = 0;
    
    for(int MeshIndex = 0;
        MeshIndex < MeshesCount;
        MeshIndex++)
    {
        minc_temp_mesh* New = PushStruct(Mine->Arena, minc_temp_mesh);
        
        New->MaxVerticesCount = MaxVertsCount;
        New->MaxFaceCount = MaxVertsCount / 6;
        
        New->Vertices = PushArray(Mine->Arena, u32, MaxVertsCount);
        New->PerFaceData = PushArray(Mine->Arena, u32, MaxVertsCount / 6);
        
        New->TempMeshListIndex = ListIndex;
        New->NextInList = Mine->TempMeshLists[ListIndex];
        Mine->TempMeshLists[ListIndex] = New;
    }
}

INTERNAL_FUNCTION void MincInitTempMeshLists(minecraft* Mine)
{
    for(int ListIndex = 0;
        ListIndex < ARC(Mine->TempMeshLists);
        ListIndex++)
    {
        MincInitTempMeshList(Mine, ListIndex,
                             Minc_MaxVertCount[ListIndex], 
                             Minc_TempMeshCountInList[ListIndex]);
    }
}

INTERNAL_FUNCTION void MincInitChunksSidesPool(minecraft* Mine)
{
    // NOTE(Dima): 5000 of theese will take take ~20mb.
    int PoolSize = 5000;
    
    minc_chunk_side* Pool = PushArray(Mine->Arena, minc_chunk_side, PoolSize);
    
    Mine->ChunkSidePool = 0;
    
    for(int EntryIndex = 0;
        EntryIndex < PoolSize;
        EntryIndex++)
    {
        minc_chunk_side* Entry = &Pool[EntryIndex];
        
        // NOTE(Dima): Inserting to list
        Entry->NextInList = Mine->ChunkSidePool;
        Mine->ChunkSidePool = Entry;
    }
}


INTERNAL_FUNCTION void CreateMinecraft(memory_arena* Arena, minecraft* Mine)
{
    Mine->Arena = Arena;
    Mine->ChunksViewDistance = 30;
    
    InitMinecraftBlockTextures(Mine);
    InitMinecraftTextureOffsets(Mine);
    InitMinecraftBiomes(Mine);
    
    Mine->TaskPool = CreateTaskMemoryPoolDynamic(Arena);
    MincInitTempMeshLists(Mine);
    MincInitChunksSidesPool(Mine);
    
    // NOTE(Dima): Init meta table
    for(int MetaIndex = 0;
        MetaIndex < MINC_META_TABLE_SIZE;
        MetaIndex++)
    {
        Mine->MetaTable[MetaIndex] = 0;
    }
}

INTERNAL_FUNCTION void UpdateMinecraft(minecraft* Mine, v3 PlayerP)
{
    int vd = Mine->ChunksViewDistance;
    
    int PlayerChunkX = Floor(PlayerP.x / (f32)MINC_CHUNK_WIDTH);
    int PlayerChunkZ = Floor(PlayerP.z / (f32)MINC_CHUNK_WIDTH);
    
    // NOTE(Dima): Iterating to remove unseen
    for(int i = 0; i < MINC_META_TABLE_SIZE; i++)
    {
        minc_chunk_meta_slot* At = Mine->MetaTable[i];
        
        while(At)
        {
            minc_chunk* Chunk = At->Chunk;
            
            UpdateChunkAtIndex(Mine, 
                               At->Meta->CoordX,
                               At->Meta->CoordZ,
                               PlayerChunkX,
                               PlayerChunkZ);
            
            At = At->NextInHash;
        }
    }
    
    {
        BLOCK_TIMING("Minc: Update Visible");
        
        // NOTE(Dima): Update all chunks within view distance
        for(int j = -vd; j <= vd; j++)
        {
            for(int i = -vd; i <= vd; i++)
            {
                int DistToChunk = Sqrt(i * i + j * j);
                
                if(DistToChunk <= vd)
                {
                    int X = PlayerChunkX + i;
                    int Z = PlayerChunkZ + j;
                    
                    MincTryInitChunk(Mine, X, Z);
                }
            }
        }
    }
}