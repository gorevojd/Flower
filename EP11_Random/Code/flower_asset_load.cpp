#include "flower_utils.cpp"

struct loading_params
{
    u32 AssetType;
    
    b32 Image_FlipVertically;
    b32 Image_FilteringIsClosest;
    
    f32 Model_DefaultScale;
    b32 Model_FixInvalidRotation;
    
    int Font_PixelHeight;
};

inline loading_params DefaultLoadingParams()
{
    loading_params Result = {};
    
    // NOTE(Dima): Image
    Result.Image_FlipVertically = true;
    Result.Image_FilteringIsClosest = false;
    
    // NOTE(Dima): Model
    Result.Model_FixInvalidRotation = false;
    Result.Model_DefaultScale = 1.0f;
    
    // NOTE(Dima): Font
    Result.Font_PixelHeight = 30;
    
    return(Result);
}

INTERNAL_FUNCTION image LoadImageFile(char* FilePath, loading_params Params = DefaultLoadingParams())
{
    int Width;
    int Height;
    int Channels;
    
    stbi_set_flip_vertically_on_load(Params.Image_FlipVertically);
    
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
    Result.FilteringIsClosest = Params.Image_FilteringIsClosest;
    
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

INTERNAL_FUNCTION font LoadFontFromBuffer(u8* Buffer, int FontSize)
{
    font Result = {};
    
    Result.Size = FontSize;
    
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
    std::vector<image> GlyphImages;
    
    int GlyphCount = ('~' - ' ' + 1);
    
    for(int Index = 0;
        Index < GlyphCount;
        Index++)
    {
        glyph Glyph = {};
        
        int Codepoint = ' ' + Index;
        
        int StbAdvance;
        int StbLeftBearing;
        stbtt_GetCodepointHMetrics(&StbFont, 
                                   Codepoint, 
                                   &StbAdvance, 
                                   &StbLeftBearing);
        
        Glyph.Codepoint = Codepoint;
        Glyph.Advance = (f32)StbAdvance * Scale;
        Glyph.LeftBearing = (f32)StbLeftBearing * Scale;
        
        int StbW, StbH;
        int StbXOffset, StbYOffset;
        unsigned char* StbBitmap = stbtt_GetCodepointBitmap(&StbFont,
                                                            0, Scale,
                                                            Codepoint,
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
        Glyph.ImageIndex = Index;
        GlyphImages.push_back(AllocateImageInternal(GlyphWidth, GlyphHeight, OurImageMem));
        
        Glyph.XOffset = StbXOffset - Border;
        Glyph.YOffset = StbYOffset - Border;
        Glyph.WidthOverHeight = (f32)GlyphWidth / (f32)GlyphHeight;
        
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
    
    Assert(Glyphs.size() == GlyphImages.size());
    
    // NOTE(Dima): Copying glyphs
    Result.Glyphs = (glyph*)calloc(Glyphs.size(), sizeof(glyph));
    Result.GlyphCount = Glyphs.size();
    memcpy(Result.Glyphs, &Glyphs[0], Glyphs.size() * sizeof(glyph));
    
    // NOTE(Dima): Copy images
    Result.GlyphImages = (image*)calloc(GlyphImages.size(), sizeof(image));
    memcpy(Result.GlyphImages, &GlyphImages[0], GlyphImages.size() * sizeof(image));
    
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

INTERNAL_FUNCTION font LoadFontFile(char* FilePath, 
                                    loading_params Params = DefaultLoadingParams())
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
        Result = LoadFontFromBuffer(Buffer, Params.Font_PixelHeight);
    }
    
    free(Buffer);
    
    return(Result);
}

INTERNAL_FUNCTION mesh MakeMesh(const helper_mesh& HelperMesh)
{
    mesh Result = {};
    
    // NOTE(Dima): Copy name
    const char* SrcName = HelperMesh.Name.c_str();
    CopyStringsSafe(Result.Name, ArrayCount(Result.Name), (char*)SrcName);
    
    mesh_offsets* Offsets = &Result.Offsets;
    helper_byte_buffer Help = {};
    
    Offsets->OffsetP = Help.AddPlace("P", HelperMesh.Vertices.size(), sizeof(v3));
    Offsets->OffsetUV = Help.AddPlace("UV", HelperMesh.TexCoords.size(), sizeof(v2));
    Offsets->OffsetN = Help.AddPlace("N", HelperMesh.Normals.size(), sizeof(v3));
    Offsets->OffsetC = Help.AddPlace("C", HelperMesh.Colors.size(), sizeof(u32));
    Offsets->OffsetBoneWeights = Help.AddPlace("BoneWeights", HelperMesh.BoneWeights.size(), sizeof(v4));
    Offsets->OffsetBoneIndices = Help.AddPlace("BoneIndices", HelperMesh.BoneIndices.size(), sizeof(u32));
    Help.AddPlace("Indices", HelperMesh.Indices.size(), sizeof(u32));
    
    Help.Generate();
    
    Result.P = (v3*)Help.GetPlace("P");
    Result.UV = (v2*)Help.GetPlace("UV");
    Result.N = (v3*)Help.GetPlace("N");
    Result.C = (u32*)Help.GetPlace("C");
    Result.BoneWeights = (v4*)Help.GetPlace("BoneWeights");
    Result.BoneIndices = (u32*)Help.GetPlace("BoneIndices");
    
    // NOTE(Dima): Saving vertices data buffer
    Result.Free = Help.Data;
    Result.FreeSize = Help.DataSize;
    Result.VertexCount = HelperMesh.Vertices.size();
    Result.IsSkinned = HelperMesh.IsSkinned;
    
    // NOTE(Dima): Allocating indices
    Result.Indices = (u32*)Help.GetPlace("Indices");;
    Result.IndexCount = HelperMesh.Indices.size();
    
    // NOTE(Dima): Storing vertex data
    for(int VertexIndex = 0;
        VertexIndex < HelperMesh.Vertices.size();
        VertexIndex++)
    {
        Result.P[VertexIndex] = HelperMesh.Vertices[VertexIndex];
        Result.UV[VertexIndex] = HelperMesh.TexCoords[VertexIndex];
        Result.N[VertexIndex] = HelperMesh.Normals[VertexIndex];
        Result.C[VertexIndex] = PackRGB(HelperMesh.Colors[VertexIndex]);
        if(HelperMesh.IsSkinned)
        {
            Result.BoneWeights[VertexIndex] = HelperMesh.BoneWeights[VertexIndex];
            Result.BoneIndices[VertexIndex] = HelperMesh.BoneIndices[VertexIndex];
        }
    }
    
    for (int Index = 0; Index < HelperMesh.Indices.size(); Index += 1)
    {
        Result.Indices[Index] = HelperMesh.Indices[Index];
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline void PushTriangle(helper_mesh& HelperMesh,
                                           v3 A, v3 B, v3 C,
                                           v3 N, v3 Color)
{
    int VertexBase = HelperMesh.Vertices.size();
    
    auto& Vertices = HelperMesh.Vertices;
    auto& TexCoords = HelperMesh.TexCoords;
    auto& Normals = HelperMesh.Normals;
    auto& Colors = HelperMesh.Colors;
    auto& Indices = HelperMesh.Indices;
    
    Vertices.push_back(A);
    Vertices.push_back(B);
    Vertices.push_back(C);
    
    TexCoords.push_back(V2(0.0f, 1.0f));
    TexCoords.push_back(V2(1.0f, 1.0f));
    TexCoords.push_back(V2(1.0f, 0.0f));
    
    Normals.push_back(N);
    Normals.push_back(N);
    Normals.push_back(N);
    
    Colors.push_back(Color);
    Colors.push_back(Color);
    Colors.push_back(Color);
    
    Indices.push_back(VertexBase + 0);
    Indices.push_back(VertexBase + 1);
    Indices.push_back(VertexBase + 2);
}

INTERNAL_FUNCTION inline void PushFlatPolygon(helper_mesh& HelperMesh,
                                              v3 A, v3 B, v3 C, v3 D,
                                              v3 N, v3 Color)
{
    int VertexBase = HelperMesh.Vertices.size();
    
    auto& Vertices = HelperMesh.Vertices;
    auto& TexCoords = HelperMesh.TexCoords;
    auto& Normals = HelperMesh.Normals;
    auto& Colors = HelperMesh.Colors;
    auto& Indices = HelperMesh.Indices;
    
    Vertices.push_back(A);
    Vertices.push_back(B);
    Vertices.push_back(C);
    Vertices.push_back(D);
    
    TexCoords.push_back(V2(0.0f, 1.0f));
    TexCoords.push_back(V2(1.0f, 1.0f));
    TexCoords.push_back(V2(1.0f, 0.0f));
    TexCoords.push_back(V2(0.0f, 0.0f));
    
    Normals.push_back(N);
    Normals.push_back(N);
    Normals.push_back(N);
    Normals.push_back(N);
    
    Colors.push_back(Color);
    Colors.push_back(Color);
    Colors.push_back(Color);
    Colors.push_back(Color);
    
    Indices.push_back(VertexBase + 0);
    Indices.push_back(VertexBase + 1);
    Indices.push_back(VertexBase + 2);
    Indices.push_back(VertexBase + 0);
    Indices.push_back(VertexBase + 2);
    Indices.push_back(VertexBase + 3);
}

INTERNAL_FUNCTION inline void PushUnitCubeSide(helper_mesh& HelperMesh,
                                               int Index0, 
                                               int Index1,
                                               int Index2,
                                               int Index3,
                                               v3 Normal,
                                               v3 Color,
                                               f32 SideLen)
{
    f32 HalfSideLen = SideLen * 0.5f;
    
    v3 CubeVertices[8] =
    {
        HalfSideLen * V3(-1.0f, 1.0f, 1.0f),
        HalfSideLen * V3(1.0f, 1.0f, 1.0f),
        HalfSideLen * V3(1.0f, -1.0f, 1.0f),
        HalfSideLen * V3(-1.0f, -1.0f, 1.0f),
        
        HalfSideLen * V3(-1.0f, 1.0f, -1.0f),
        HalfSideLen * V3(1.0f, 1.0f, -1.0f),
        HalfSideLen * V3(1.0f, -1.0f, -1.0f),
        HalfSideLen * V3(-1.0f, -1.0f, -1.0f),
    };
    
    PushFlatPolygon(HelperMesh,
                    CubeVertices[Index0],
                    CubeVertices[Index1],
                    CubeVertices[Index2],
                    CubeVertices[Index3],
                    Normal, Color);
}

INTERNAL_FUNCTION mesh MakeUnitCube(f32 SideLen = 2.0f)
{
    helper_mesh HelperMesh = {};
    
    v3 CubeColor = V3(1.0f);
    
    // NOTE(Dima): Forward side
    PushUnitCubeSide(HelperMesh,
                     0, 1, 2, 3, 
                     V3_Forward(), 
                     CubeColor,
                     SideLen);
    
    // NOTE(Dima): Left side
    PushUnitCubeSide(HelperMesh,
                     1, 5, 6, 2, 
                     V3_Left(), 
                     CubeColor,
                     SideLen);
    
    // NOTE(Dima): Back side
    PushUnitCubeSide(HelperMesh,
                     5, 4, 7, 6, 
                     V3_Back(), 
                     CubeColor,
                     SideLen);
    
    // NOTE(Dima): Right side
    PushUnitCubeSide(HelperMesh,
                     4, 0, 3, 7, 
                     V3_Right(), 
                     CubeColor,
                     SideLen);
    
    // NOTE(Dima): Up
    PushUnitCubeSide(HelperMesh,
                     4, 5, 1, 0, 
                     V3_Up(), 
                     CubeColor,
                     SideLen);
    
    // NOTE(Dima): Down
    PushUnitCubeSide(HelperMesh,
                     3, 2, 6, 7, 
                     V3_Down(), 
                     CubeColor,
                     SideLen);
    
    mesh Result = MakeMesh(HelperMesh);
    
    return(Result);
}

mesh MakePlane()
{
    helper_mesh HelperMesh = {};
    
    v3 Points[4] = 
    {
        V3(1.0f, 0.0f, 1.0f),
        V3(-1.0f, 0.0f, 1.0f),
        V3(-1.0f, 0.0f, -1.0f),
        V3(1.0f, 0.0f, -1.0f)
    };
    
    PushFlatPolygon(HelperMesh,
                    Points[0], Points[1],
                    Points[2], Points[3],
                    V3_Up(), V3_One());
    
    mesh Result = MakeMesh(HelperMesh);
    
    return(Result);
}

#include "atool_assimp.cpp"