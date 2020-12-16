INTERNAL_FUNCTION inline f32 GetLineBase()
{
    ui_params* Params = &GlobalUI->Params;
    
    f32 Result = Params->Font->Ascent * Params->Scale;
    
    return(Result);
}


INTERNAL_FUNCTION inline f32 GetLineAdvance()
{
    ui_params* Params = &GlobalUI->Params;
    
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

INTERNAL_FUNCTION rc2 PrintText_(render_commands* Commands, 
                                 font* Font, 
                                 char* Text, 
                                 v3 Left, v3 Up,
                                 v3 P, 
                                 v2 Offset, 
                                 u32 Flags,
                                 f32 Scale = 1.0f, 
                                 v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f))
{
    char* At = Text;
    
    v2 AtP = V2(P.x, P.y);
    
    rect_buffer* Buffer = &Commands->Rects2D;
    
    int IndexToTransformMatrix = Buffer->IdentityMatrixIndex;
    
    b32 Is3D = (Flags & PrintText_3D) != 0;
    if(Is3D)
    {
        Buffer = &Commands->Rects3D;
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
    
    while(*At)
    {
        int GlyphIndex = *At - ' ';
        
        glyph* Glyph = &Font->Glyphs[GlyphIndex];
        
        f32 TargetHeight = (f32)Glyph->Image.Height * Scale;
        
        v2 ImageP = AtP + V2(Glyph->XOffset, Glyph->YOffset) * Scale + Offset;
        PushGlyph(Buffer, Glyph, ImageP, 
                  TargetHeight, 
                  IndexToTransformMatrix, C);
        
        f32 Kerning = GetKerning(Font, *At, *(At + 1)); 
        
        //AtP.x += Glyph->Advance * Scale;
        AtP.x += (Glyph->Advance + Kerning) * Scale;
        
        At++;
    }
    
    Bounds.Max.x = AtP.x;
    
    return(Bounds);
}

INTERNAL_FUNCTION rc2 PrintText(render_commands* Commands, 
                                font* Font, 
                                char* Text, 
                                v2 P, 
                                f32 Scale = 1.0f, 
                                v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f), 
                                b32 WithShadow = true)
{
    if(WithShadow)
    {
        PrintText_(Commands, Font, Text, V3_Left(), V3_Up(), V3(P, 0.0f), V2(1.0f, 1.0f), 0, Scale, V4(0.0f, 0.0f, 0.0f, 1.0f));
    }
    rc2 Result = PrintText_(Commands, Font, Text, V3_Left(), V3_Up(), V3(P, 0.0f), V2(0.0f, 0.0f), 0, Scale, C);
    
    return(Result);
}

INTERNAL_FUNCTION void PrintText3D(render_commands* Commands, 
                                   font* Font, 
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
        PrintText_(Commands, Font, Text, Left, Up, P, V2(1.0f, 1.0f), PrintText_3D, Scale, V4(0.0f, 0.0f, 0.0f, 1.0f));
    }
#endif
    PrintText_(Commands, Font, Text, Left, Up, P, V2(0.0f, 0.0f), PrintText_3D, Scale, C);
}

INTERNAL_FUNCTION void BeginLayout(v2 At = V2(0.0f, 0.0f))
{
    GlobalUI->CurrentLayout = &GlobalUI->Layout;
    
    GlobalUI->CurrentLayout->At = At;
    GlobalUI->CurrentLayout->At.y += GetLineBase();
}

INTERNAL_FUNCTION void EndLayout()
{
    GlobalUI->CurrentLayout = 0;
}

INTERNAL_FUNCTION void Text(char* Txt, b32 Highligth = false)
{
    Assert(GlobalUI);
    
    ui_params* Params = &GlobalUI->Params;
    
    v4 Color = ColorWhite();
    if(Highligth)
    {
        Color = ColorYellow();
    }
    
    rc2 Bounds = PrintText(Params->Commands, 
                           Params->Font, 
                           Txt, 
                           GlobalUI->CurrentLayout->At, 
                           Params->Scale, Color);
    
    if(Highligth)
    {
        PushRectOutline(Params->Commands,
                        Bounds, 2,
                        Color);
    }
    
    GlobalUI->CurrentLayout->At.y += GetLineAdvance();
}

#include "flower_ui_graphs.cpp"

INTERNAL_FUNCTION inline void SetParamsUI(ui_params Params)
{
    GlobalUI->Params = Params;
}

INTERNAL_FUNCTION void InitUI(memory_arena* Arena)
{
    GlobalUI = PushStruct(Arena, ui_state);
}