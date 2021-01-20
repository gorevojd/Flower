INTERNAL_FUNCTION inline f32 GetLineBase()
{
    ui_params* Params = &Global_UI->Params;
    
    f32 Result = Params->Font->Ascent * Params->Scale;
    
    return(Result);
}

INTERNAL_FUNCTION inline f32 GetLineAdvance()
{
    ui_params* Params = &Global_UI->Params;
    
    f32 Result = Params->Font->LineAdvance * Params->Scale;
    
    return(Result);
}


INTERNAL_FUNCTION inline f32 GetKerning(font* Font, u32 CodepointFirst, u32 CodepointSecond)
{
    int GlyphIndex = CodepointFirst - ' ';
    int NextGlyphIndex = -1;
    
    if(CodepointSecond != 0)
    {
        NextGlyphIndex = CodepointSecond - ' ';
    }
    
    f32 Kerning = 0.0f;
    if(NextGlyphIndex != -1)
    {
        Kerning = Font->KerningPairs[GlyphIndex * Font->GlyphCount + NextGlyphIndex];
        
        if(Kerning > 0.0001f)
        {
            int a = 1;
        }
    }
    
    return(Kerning);
}

enum print_text_flags
{
    PrintText_3D = (1 << 0),
};

INTERNAL_FUNCTION rc2 PrintText_(font* Font, 
                                 char* Text, 
                                 v3 Left, v3 Up,
                                 v3 P, 
                                 v2 Offset, 
                                 u32 Flags,
                                 f32 Scale = 1.0f, 
                                 b32 IsGetSizePass = false,
                                 v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
    char* At = Text;
    
    v2 AtP = V2(P.x, P.y);
    
    rect_buffer* Buffer = &Global_RenderCommands->Rects2D;
    
    int IndexToTransformMatrix = Buffer->IdentityMatrixIndex;
    
    b32 Is3D = (Flags & PrintText_3D) != 0;
    if(Is3D)
    {
        Buffer = &Global_RenderCommands->Rects3D;
        AtP = {};
        
        Up = -Up;
        Left = -Left;
        
        Scale *= 1.0f / (f32)Font->Size;
        
        m44 TextTransform = Matrix4FromRows(V4(Left, 0.0f), 
                                            V4(Up, 0.0f), 
                                            V4(NOZ(Cross(Left, Up)), 0.0f),
                                            V4(P.x, P.y, P.z, 1.0f));
        IndexToTransformMatrix = PushRectTransform(Buffer, &TextTransform);
    }
    
    rc2 Bounds;
    Bounds.Min.x = AtP.x;
    Bounds.Min.y = AtP.y - Font->Ascent * Scale;
    Bounds.Max.y = AtP.y - Font->Descent * Scale;
    
    image* GlyphImages = Font->GlyphImages;
    
    while(*At)
    {
        int GlyphIndex = *At - ' ';
        
        glyph* Glyph = &Font->Glyphs[GlyphIndex];
        
        if(!IsGetSizePass)
        {
            image* Image = &GlyphImages[Glyph->ImageIndex];
            
            f32 TargetHeight = (f32)Image->Height * Scale;
            
            v2 ImageP = AtP + V2(Glyph->XOffset, Glyph->YOffset) * Scale + Offset;
            PushGlyph(Buffer, Glyph, ImageP, 
                      TargetHeight, 
                      IndexToTransformMatrix, C);
        }
        
        f32 Kerning = GetKerning(Font, *At, *(At + 1)); 
        
        //AtP.x += Glyph->Advance * Scale;
        AtP.x += (Glyph->Advance + Kerning) * Scale;
        
        At++;
    }
    
    Bounds.Max.x = AtP.x;
    
    return(Bounds);
}

INTERNAL_FUNCTION rc2 PrintText(font* Font, 
                                char* Text, 
                                v2 P, 
                                f32 Scale = 1.0f, 
                                v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f), 
                                b32 WithShadow = true)
{
    if(WithShadow)
    {
        PrintText_(Font, Text, 
                   V3_Left(), V3_Up(), 
                   V3(P, 0.0f), V2(1.0f, 1.0f), 
                   0, Scale, false, 
                   V4(0.0f, 0.0f, 0.0f, 1.0f));
    }
    rc2 Result = PrintText_( Font, Text, 
                            V3_Left(), V3_Up(), 
                            V3(P, 0.0f), V2(0.0f, 0.0f), 
                            0, Scale, false,  C);
    
    return(Result);
}

INTERNAL_FUNCTION void PrintText3D(font* Font, 
                                   char* Text,
                                   v3 Left, v3 Up,
                                   v3 P, 
                                   v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f), 
                                   f32 Scale = 1.0f, 
                                   b32 WithShadow = true)
{
    
#if 0    
    if(WithShadow)
    {
        PrintText_(Font, Text, 
                   Left, Up, P, 
                   V2(1.0f, 1.0f), 
                   PrintText_3D, 
                   Scale, 
                   V4(0.0f, 0.0f, 0.0f, 1.0f));
    }
#endif
    
    PrintText_(Font, Text, 
               Left, Up, P, 
               V2(0.0f, 0.0f), 
               PrintText_3D, 
               Scale, 
               false, C);
}

INTERNAL_FUNCTION inline v2 GetTextSize(char* Text, f32 TextScale)
{
    render_commands* Commands = Global_UI->Params.Commands;
    font* Font = Global_UI->Params.Font;
    
    rc2 TextRect = PrintText_(Font, 
                              Text, 
                              V3_Left(), V3_Up(), 
                              V3(0.0f, 0.0f, 0.0f), V2(0.0f, 0.0f), 0, TextScale, true);
    
    v2 Result = GetDim(TextRect);
    
    return(Result);
}

INTERNAL_FUNCTION inline f32 GetPrintHorizontalPosition(f32 Min, f32 Max, 
                                                        f32 TextDimX, u32 Align,
                                                        f32 TextScale)
{
    f32 Result = Min;
    
    switch(Align)
    {
        case TextAlign_Right:
        {
            Result = Max - TextDimX;
        }break;
        
        case TextAlign_Center:
        {
            Result = Min + (Max - Min) * 0.5f - TextDimX * 0.5f;
        }break;
        
        case TextAlign_Left:
        {
            Result = Min;
        }break;
        
        default:
        {
            InvalidCodePath;
        }break;
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline f32 GetPrintVerticalPosition(f32 Min, f32 Max, 
                                                      f32 TextDimY, u32 Align, 
                                                      f32 TextScale)
{
    f32 Result = Min;
    
    font* Font = Global_UI->Params.Font;
    
    switch(Align)
    {
        case TextAlign_Top:
        {
            Result = Min + Font->Ascent * TextScale;
        }break;
        
        case TextAlign_Bottom:
        {
            Result = Max + Font->Descent * TextScale;
        }break;
        
        case TextAlign_Center:
        {
            f32 DimY = (Font->Ascent - Font->Descent) * TextScale;
            
            f32 CenterY = Min + (Max - Min) * 0.5f;
            
            Result = CenterY - DimY * 0.5f + Font->Ascent * TextScale;
        }break;
        
        default:
        {
            InvalidCodePath;
        }break;
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline v2 GetPrintPositionInRect(rc2 Rect,
                                                   v2 TextDim,
                                                   u32 AlignX,
                                                   u32 AlignY,
                                                   f32 TextScale)
{
    v2 Result;
    
    Result.x = GetPrintHorizontalPosition(Rect.Min.x, Rect.Max.x,
                                          TextDim.x, AlignX, TextScale);
    Result.y = GetPrintVerticalPosition(Rect.Min.y, Rect.Max.y,
                                        TextDim.y, AlignY, TextScale);
    
    return(Result);
}

INTERNAL_FUNCTION rc2 PrintText(char* Text,
                                v2 P,
                                f32 Scale = 1.0f,
                                v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f),
                                b32 WithShadow = true)
{
    render_commands* Commands = Global_UI->Params.Commands;
    font* Font = Global_UI->Params.Font;
    
    rc2 Result = PrintText(Font,
                           Text,
                           P,
                           Scale, C,
                           WithShadow);
    
    return(Result);
}

INTERNAL_FUNCTION rc2 PrintTextAligned(char* Text, 
                                       rc2 Rect,
                                       u32 AlignX = TextAlign_Center,
                                       u32 AlignY = TextAlign_Center,
                                       f32 Scale = 1.0f,
                                       v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f),
                                       b32 WithShadow = true)
{
    v2 TextSize = GetTextSize(Text, Scale);
    
    v2 PrintP = GetPrintPositionInRect(Rect, TextSize,
                                       AlignX, AlignY,
                                       Scale);
    
    rc2 Result = PrintText(Text, PrintP, Scale, C, WithShadow);
    
    return(Result);
}


INTERNAL_FUNCTION rc2 PrintTextAligned(char* Text, 
                                       v2 Point,
                                       u32 AlignX = TextAlign_Center,
                                       u32 AlignY = TextAlign_Center,
                                       f32 Scale = 1.0f,
                                       v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f),
                                       b32 WithShadow = true)
{
    rc2 Result = PrintTextAligned(Text, RectMinMax(Point, Point), 
                                  AlignX, AlignY,
                                  Scale,
                                  C, WithShadow);
    
    return(Result);
}

// NOTE(Dima): Get pixel point on screen from top left corner
INTERNAL_FUNCTION inline v2 UVToScreenPoint(float x, float y)
{
    v2 Result;
    
    Result.x = x * Global_UI->Params.WindowDims->Width;
    Result.y = y * Global_UI->Params.WindowDims->Height;
    
    return(Result);
}


INTERNAL_FUNCTION inline v2 UVToScreenPoint(v2 UV)
{
    v2 Result = UVToScreenPoint(UV.x, UV.y);
    
    return(Result);
}

INTERNAL_FUNCTION void BeginLayout(v2 At = V2(0.0f, 0.0f))
{
    Global_UI->CurrentLayout = &Global_UI->Layout;
    
    Global_UI->CurrentLayout->At = At;
    Global_UI->CurrentLayout->At.y += GetLineBase();
}

INTERNAL_FUNCTION void EndLayout()
{
    Global_UI->CurrentLayout = 0;
}

INTERNAL_FUNCTION void Text(char* Txt, b32 Highligth = false)
{
    Assert(Global_UI);
    
    ui_params* Params = &Global_UI->Params;
    
    v4 Color = ColorWhite();
    if(Highligth)
    {
        Color = ColorYellow();
    }
    
    rc2 Bounds = PrintText(Params->Font, 
                           Txt, 
                           Global_UI->CurrentLayout->At, 
                           Params->Scale, Color);
    
    if(Highligth)
    {
        PushRectOutline(Bounds, 2,
                        Color);
    }
    
    Global_UI->CurrentLayout->At.y += GetLineAdvance();
}

#include "flower_ui_graphs.cpp"

INTERNAL_FUNCTION inline void SetParamsUI(ui_params Params)
{
    Global_UI->Params = Params;
}

INTERNAL_FUNCTION void InitUI(memory_arena* Arena)
{
    Global_UI = PushStruct(Arena, ui_state);
}