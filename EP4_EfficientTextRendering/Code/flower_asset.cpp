#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

INTERNAL_FUNCTION void AddGlyphToAtlas(glyph* Glyph)
{
    image* Src = &Glyph->Image;
    image* Dst = &GlobalAssetSystem->FontsAtlas;
    
    int SrcW = Src->Width;
    int SrcH = Src->Height;
    
    int DstSize = GlobalAssetSystem->FontsAtlas.Width;
    
    int DstPx = std::ceil(GlobalAssetSystem->FontAtlasAtP.x);
    int DstPy = std::ceil(GlobalAssetSystem->FontAtlasAtP.y);
    
    if(DstPx + SrcW >= DstSize)
    {
        DstPx = 0;
        DstPy = GlobalAssetSystem->FontAtlasMaxRowY;
    }
    
    Assert(DstPy + SrcH < DstSize);
    
    f32 OneOverSize = 1.0f / DstSize;
    
    Glyph->MinUV = V2(DstPx, DstPy) * OneOverSize;
    Glyph->MaxUV = V2(DstPx + SrcW, DstPy + SrcH) * OneOverSize;
    
    for(int y = 0; y < SrcH; y++)
    {
        for(int x = 0; x < SrcW; x++)
        {
            int DstPixelY = DstPy + y;
            int DstPixelX = DstPx + x;
            
            u32* DstPixel = (u32*)Dst->Pixels + DstPixelY * DstSize + DstPixelX;
            u32* SrcPixel = (u32*)Src->Pixels + y * SrcW + x;
            
            *DstPixel = *SrcPixel;
        }
    }
    
    GlobalAssetSystem->FontAtlasAtP = V2(DstPx + SrcW, DstPy);
    GlobalAssetSystem->FontAtlasMaxRowY = std::max(GlobalAssetSystem->FontAtlasMaxRowY,
                                                   DstPy + SrcH);
}

INTERNAL_FUNCTION void AddFontToAtlas(font* Font)
{
    for(int GlyphIndex = 0;
        GlyphIndex < Font->GlyphCount;
        GlyphIndex++)
    {
        glyph* Glyph = &Font->Glyphs[GlyphIndex];
        
        AddGlyphToAtlas(Glyph);
    }
}

INTERNAL_FUNCTION image AllocateImageInternal(u32 Width, u32 Height, void* PixData)
{
    image Result = {};
    
	Result.Width = Width;
	Result.Height = Height;
    
	Result.WidthOverHeight = (float)Width / (float)Height;
    
	Result.Pixels = PixData;
    
	return(Result);
}

INTERNAL_FUNCTION void InitAssetSystem(memory_arena* Arena)
{
    GlobalAssetSystem = PushStruct(Arena, asset_system);
    
    // NOTE(Dima): Font atlas initializing
    int FontAtlasSize = 2048;
    void* FontsAtlasMem = calloc(FontAtlasSize * FontAtlasSize * sizeof(u32), 1);
    GlobalAssetSystem->FontsAtlas = AllocateImageInternal(FontAtlasSize,
                                                          FontAtlasSize,
                                                          FontsAtlasMem);
    
    
}

INTERNAL_FUNCTION image LoadImageFile(char* FilePath)
{
    int Width;
    int Height;
    int Channels;
    
    unsigned char* Image = stbi_load(FilePath,
                                     &Width,
                                     &Height,
                                     &Channels,
                                     STBI_rgb_alpha);
    
    Assert(Image);
    
    int PixCount = Width * Height;
    int ImageSize = PixCount * 4;
    
    void* OurImageMem = malloc(ImageSize);
    image Result = AllocateImageInternal(Width, Height, OurImageMem);
    
    for(int PixelIndex = 0;
        PixelIndex < PixCount;
        PixelIndex++)
    {
        u32 Pix = *((u32*)Image + PixelIndex);
        
        v4 Color = UnpackRGBA(Pix);
        Color.r *= Color.a;
        Color.g *= Color.a;
        Color.b *= Color.a;
        
        u32 PackedColor = PackRGBA(Color);
        
        *((u32*)Result.Pixels + PixelIndex) = PackedColor;
    }
    
    stbi_image_free(Image);
    
    return(Result);
}

INTERNAL_FUNCTION font LoadFontFromBuffer(u8* Buffer)
{
    font Result = {};
    
    int FontSize = 30;
    Result.Size = 30;
    
    stbtt_fontinfo StbFont;
    stbtt_InitFont(&StbFont, Buffer, 0);
    
    f32 Scale = stbtt_ScaleForPixelHeight(&StbFont, FontSize);
    
    int StbAscent;
    int StbDescent;
    int StbLineGap;
    stbtt_GetFontVMetrics(&StbFont, &StbAscent, &StbDescent, &StbLineGap);
    
    Result.Ascent = (f32)StbAscent * Scale;
    Result.Descent = (f32)StbDescent * Scale;
    Result.LineGap = (f32)StbLineGap * Scale;
    
    Result.LineAdvance = Result.Ascent - Result.Descent + Result.LineGap;
    
    std::vector<glyph> Glyphs;
    
    for(int CharIndex = ' ';
        CharIndex <= '~';
        CharIndex++)
    {
        glyph Glyph = {};
        
        int StbAdvance;
        int StbLeftBearing;
        stbtt_GetCodepointHMetrics(&StbFont, 
                                   CharIndex, 
                                   &StbAdvance, 
                                   &StbLeftBearing);
        
        Glyph.Codepoint = CharIndex;
        Glyph.Advance = (f32)StbAdvance * Scale;
        Glyph.LeftBearing = (f32)StbLeftBearing * Scale;
        
        int StbW, StbH;
        int StbXOffset, StbYOffset;
        unsigned char* StbBitmap = stbtt_GetCodepointBitmap(&StbFont,
                                                            0, Scale,
                                                            CharIndex,
                                                            &StbW, &StbH,
                                                            &StbXOffset, 
                                                            &StbYOffset);
        
        if(StbW > 20000)
        {
            StbW = 0;
        }
        
        
        if(StbH > 20000)
        {
            StbH = 0;
        }
        
        int Border = 3;
        int GlyphWidth = StbW + 2 * Border;
        int GlyphHeight = StbH + 2 * Border;
        
        int ImageSize = GlyphWidth * GlyphHeight * 4;
        
        void* OurImageMem = calloc(ImageSize, 1);
        Glyph.Image = AllocateImageInternal(GlyphWidth, GlyphHeight, OurImageMem);
        Glyph.XOffset = StbXOffset - Border;
        Glyph.YOffset = StbYOffset - Border;
        
        for(int y = 0; y < StbH; y++)
        {
            for(int x = 0; x < StbW; x++)
            {
                int PixelIndex = y * StbW + x;
                unsigned char CurTone = StbBitmap[PixelIndex];
                
                f32 Tone01 = (f32)CurTone / 255.0f;
                
                v4 Color = V4(Tone01, Tone01, Tone01, Tone01);
                u32 PackedColor = PackRGBA(Color);
                
                int TargetPixel = (y + Border) * GlyphWidth + (x + Border);
                u32* OurImagePixels = (u32*)OurImageMem;
                OurImagePixels[TargetPixel] = PackedColor;
                
            }
        }
        
        stbtt_FreeBitmap(StbBitmap, 0);
        
        Glyphs.push_back(Glyph);
    }
    
    // NOTE(Dima): Copying glyphs
    Result.Glyphs = (glyph*)calloc(Glyphs.size(), sizeof(glyph));
    Result.GlyphCount = Glyphs.size();
    memcpy(Result.Glyphs, &Glyphs[0], Glyphs.size() * sizeof(glyph));
    
    // NOTE(Dima): Loading kerning
    Result.KerningPairs = (f32*)malloc(Result.GlyphCount * Result.GlyphCount * sizeof(f32));
    
    for(int i = 0; i < Result.GlyphCount; i++)
    {
        for(int j = 0; j < Result.GlyphCount; j++)
        {
            int Index = i * Result.GlyphCount + j;
            
            u32 A = Glyphs[i].Codepoint;
            u32 B = Glyphs[j].Codepoint;
            
            int ExtractedKern = stbtt_GetCodepointKernAdvance(&StbFont, A, B);
            
            if(ExtractedKern != 0)
            {
                int a = 1;
            }
            
            Result.KerningPairs[Index] = Scale * (f32)ExtractedKern;
            //Result.KerningPairs[Index] = 10.0f;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION font LoadFontFile(char* FilePath)
{
    u8* Buffer = 0;
    
    SDL_RWops* File = SDL_RWFromFile(FilePath, "rb");
    if(File)
    {
        i64 FileSize = SDL_RWsize(File);
        
        Buffer = (u8*)malloc(FileSize);
        
        SDL_RWread(File, Buffer, FileSize, 1);
        
        SDL_RWclose(File);
    }
    
    
    font Result = {};
    if(Buffer)
    {
        Result = LoadFontFromBuffer(Buffer);
    }
    
    free(Buffer);
    
    return(Result);
}

INTERNAL_FUNCTION mesh MakeMesh(const std::vector<v3>& Vertices,
                                const std::vector<v2>& TexCoords,
                                const std::vector<v3>& Normals,
                                const std::vector<v3>& Colors,
                                const std::vector<u32>& Indices)
{
    mesh Result = {};
    
    u32 VerticesSize = sizeof(vertex) * Vertices.size();
    u32 IndexSize = sizeof(u32) * Indices.size();
    
    Result.Vertices = (vertex*)malloc(VerticesSize);
    Result.VertexCount = Vertices.size();
    
    Result.Indices = (u32*)malloc(IndexSize);
    Result.IndexCount = Indices.size();
    
    for(int VertexIndex = 0;
        VertexIndex < Vertices.size();
        VertexIndex++)
    {
        vertex* Target = &Result.Vertices[VertexIndex];
        
        Target->P = Vertices[VertexIndex];
        Target->UV = TexCoords[VertexIndex];
        Target->N = Normals[VertexIndex];
        Target->C = Colors[VertexIndex];
    }
    
    for (int Index = 0; Index < Indices.size(); Index += 1)
    {
        Result.Indices[Index] = Indices[Index];
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline void PushUnitCubeSide(std::vector<v3>& Vertices,
                                               std::vector<v2>& TexCoords,
                                               std::vector<v3>& Normals,
                                               std::vector<v3>& Colors,
                                               std::vector<u32>& Indices,
                                               int Index0, 
                                               int Index1,
                                               int Index2,
                                               int Index3,
                                               v3 Normal)
{
    v3 CubeVertices[8] =
    {
        V3(-1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, -1.0f, 1.0f),
        V3(-1.0f, -1.0f, 1.0f),
        
        V3(-1.0f, 1.0f, -1.0f),
        V3(1.0f, 1.0f, -1.0f),
        V3(1.0f, -1.0f, -1.0f),
        V3(-1.0f, -1.0f, -1.0f),
    };
    
    v3 CubeColors[8] =
    {
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
        V3(1.0f, 1.0f, 1.0f),
    };
    
    int IndexBase = Indices.size();
    int VertexBase = Vertices.size();
    
    Indices.push_back(VertexBase + 0);
    Indices.push_back(VertexBase + 1);
    Indices.push_back(VertexBase + 2);
    Indices.push_back(VertexBase + 0);
    Indices.push_back(VertexBase + 2);
    Indices.push_back(VertexBase + 3);
    
    Vertices.push_back(CubeVertices[Index0]);
    Vertices.push_back(CubeVertices[Index1]);
    Vertices.push_back(CubeVertices[Index2]);
    Vertices.push_back(CubeVertices[Index3]);
    
    Normals.push_back(Normal);
    Normals.push_back(Normal);
    Normals.push_back(Normal);
    Normals.push_back(Normal);
    
    TexCoords.push_back(V2(0.0f, 1.0f));
    TexCoords.push_back(V2(1.0f, 1.0f));
    TexCoords.push_back(V2(1.0f, 0.0f));
    TexCoords.push_back(V2(0.0f, 0.0f));
    
    Colors.push_back(CubeColors[Index0]);
    Colors.push_back(CubeColors[Index1]);
    Colors.push_back(CubeColors[Index2]);
    Colors.push_back(CubeColors[Index3]);
}

INTERNAL_FUNCTION mesh MakeUnitCube()
{
    std::vector<v3> Vertices;
    std::vector<v2> TexCoords;
    std::vector<v3> Normals;
    std::vector<v3> Colors;
    std::vector<u32> Indices;
    
    // NOTE(Dima): Forward side
    PushUnitCubeSide(Vertices,
                     TexCoords,
                     Normals,
                     Colors,
                     Indices,
                     0, 1, 2, 3, 
                     V3_Forward());
    
    // NOTE(Dima): Left side
    PushUnitCubeSide(Vertices,
                     TexCoords,
                     Normals,
                     Colors,
                     Indices,
                     1, 5, 6, 2, 
                     V3_Left());
    
    // NOTE(Dima): Back side
    PushUnitCubeSide(Vertices,
                     TexCoords,
                     Normals,
                     Colors,
                     Indices,
                     5, 4, 7, 6, 
                     V3_Back());
    
    // NOTE(Dima): Right side
    PushUnitCubeSide(Vertices,
                     TexCoords,
                     Normals,
                     Colors,
                     Indices,
                     4, 0, 3, 7, 
                     V3_Right());
    
    // NOTE(Dima): Up
    PushUnitCubeSide(Vertices,
                     TexCoords,
                     Normals,
                     Colors,
                     Indices,
                     4, 5, 1, 0, 
                     V3_Up());
    
    // NOTE(Dima): Down
    PushUnitCubeSide(Vertices,
                     TexCoords,
                     Normals,
                     Colors,
                     Indices,
                     3, 2, 6, 7, 
                     V3_Down());
    
    mesh Result = MakeMesh(Vertices,
                           TexCoords,
                           Normals,
                           Colors,
                           Indices);
    
    return(Result);
}
