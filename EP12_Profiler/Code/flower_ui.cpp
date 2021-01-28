INTERNAL_FUNCTION inline ui_params* UIGetParams()
{
    ui_params* Params = &Global_UI->Params;
    
    return(Params);
}

INTERNAL_FUNCTION inline f32 GetLineBase()
{
    ui_params* Params = UIGetParams();
    
    f32 Result = Params->Font->Ascent * Params->Scale;
    
    return(Result);
}

INTERNAL_FUNCTION inline f32 GetLineAdvance()
{
    ui_params* Params = UIGetParams();
    
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

INTERNAL_FUNCTION inline rc2 GetTextRect(char* Text, v2 P)
{
    ui_params* Params = UIGetParams();
    
    font* Font = Params->Font;
    
    rc2 Result = PrintText_(Font, 
                            Text, 
                            V3_Left(), V3_Up(), 
                            V3(P.x, P.y, 0.0f), 
                            V2(0.0f, 0.0f), 
                            0, Params->Scale, true);
    
    return(Result);
}

INTERNAL_FUNCTION inline v2 GetTextSize(char* Text)
{
    rc2 TextRect = GetTextRect(Text, V2(0.0f, 0.0f));
    
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
                                                   u32 AlignY)
{
    v2 Result;
    
    ui_params* Params = UIGetParams();
    
    Result.x = GetPrintHorizontalPosition(Rect.Min.x, Rect.Max.x,
                                          TextDim.x, AlignX, Params->Scale);
    Result.y = GetPrintVerticalPosition(Rect.Min.y, Rect.Max.y,
                                        TextDim.y, AlignY, Params->Scale);
    
    return(Result);
}

INTERNAL_FUNCTION rc2 PrintText(char* Text,
                                v2 P,
                                v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f),
                                b32 WithShadow = true)
{
    ui_params* Params = UIGetParams();
    font* Font = Params->Font;
    
    rc2 Result = PrintText(Font,
                           Text,
                           P,
                           Params->Scale, 
                           C,
                           WithShadow);
    
    return(Result);
}

INTERNAL_FUNCTION rc2 PrintTextAligned(char* Text, 
                                       rc2 Rect,
                                       u32 AlignX = TextAlign_Center,
                                       u32 AlignY = TextAlign_Center,
                                       v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f),
                                       b32 WithShadow = true)
{
    v2 TextSize = GetTextSize(Text);
    
    v2 PrintP = GetPrintPositionInRect(Rect, TextSize,
                                       AlignX, AlignY);
    
    rc2 Result = PrintText(Text, PrintP, C, WithShadow);
    
    return(Result);
}


INTERNAL_FUNCTION rc2 PrintTextAligned(char* Text, 
                                       v2 Point,
                                       u32 AlignX = TextAlign_Center,
                                       u32 AlignY = TextAlign_Center,
                                       v4 C = V4(1.0f, 1.0f, 1.0f, 1.0f),
                                       b32 WithShadow = true)
{
    rc2 Result = PrintTextAligned(Text, RectMinMax(Point, Point), 
                                  AlignX, AlignY,
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

INTERNAL_FUNCTION inline v4 UIGetColor(u32 Color)
{
    v4 Result = Global_UI->Colors.Colors[Color];
    
    return(Result);
}

INTERNAL_FUNCTION void InitUIColors()
{
    ui_colors* Colors_ =  &Global_UI->Colors;
    
    v4* Colors = Colors_->Colors;
    
    Colors[UIColor_Text] = ColorWhite();
    Colors[UIColor_TextHot] = ColorYellow();
    Colors[UIColor_ButtonBackground] = ColorRed();
    Colors[UIColor_ButtonBackgroundInactive] = V4(0.5f, 0.1f, 0.1f, 1.0f);
    Colors[UIColor_Borders] = ColorBlack();
    Colors[UIColor_GraphBackground] = V4(0.0f, 0.0f, 0.0f, 0.65f);
    
    Colors[UIColor_GraphFrameNew] = ColorRed();
    Colors[UIColor_GraphFrameOld] = V4(0.2f, 0.3f, 0.9f, 1.0f);
    Colors[UIColor_GraphFrameCollation] = ColorGreen();
    Colors[UIColor_GraphFrameView] = ColorYellow();
}

INTERNAL_FUNCTION void InitUI(memory_arena* Arena)
{
    Global_UI = PushStruct(Arena, ui_state);
    
    Global_UI->Arena = Arena;
    
    InitUIColors();
    
    Global_UI->FirstLayout = 0;
    
    // NOTE(Dima): Init sentinels
    CopyStringsSafe(Global_UI->ElementsFreeSentinel.DisplayName,
                    ArrayCount(Global_UI->ElementsFreeSentinel.DisplayName),
                    "SentinelFree");
    CopyStringsSafe(Global_UI->ElementsUseSentinel.DisplayName,
                    ArrayCount(Global_UI->ElementsUseSentinel.DisplayName),
                    "SentinelUse");
    DLIST_REFLECT_PTRS(Global_UI->ElementsFreeSentinel, NextAlloc, PrevAlloc);
    DLIST_REFLECT_PTRS(Global_UI->ElementsUseSentinel, NextAlloc, PrevAlloc);
    
    Global_UI->HotInteraction = {};
    Global_UI->ActiveInteraction = {};
}

// NOTE(Dima): UI parameters
INTERNAL_FUNCTION inline void UISetParams(ui_params Params)
{
    Global_UI->Params = Params;
}

inline void UIPushScale(f32 Scale)
{
    ui_params* Params = UIGetParams();
    
    Assert(Params->ScaleStackIndex < ArrayCount(Params->ScaleStack));
    
    Params->ScaleStack[Params->ScaleStackIndex++] = Scale;
    Params->Scale = Scale;
}

inline void UIPopScale()
{
    ui_params* Params = UIGetParams();
    
    Assert(Params->ScaleStackIndex > 0);
    
    --Params->ScaleStackIndex;
    Params->ScaleStack[Params->ScaleStackIndex] = 0.0f;
    if(Params->ScaleStackIndex - 1 >= 0)
    {
        Params->Scale = Params->ScaleStack[Params->ScaleStackIndex - 1];
    }
    else
    {
        Params->Scale = 1.0f;
    }
}

INTERNAL_FUNCTION void UIBeginFrame()
{
    window_dimensions* WndDims = &Global_RenderCommands->WindowDimensions;
    
    ui_params ParamsUI = {};
    ParamsUI.Commands = Global_RenderCommands;
    ParamsUI.Font = &Global_Assets->LiberationMono;
    ParamsUI.WindowDims = WndDims;
    
    UISetParams(ParamsUI);
    
    // NOTE(Dima): Init font scale stack
    ParamsUI.ScaleStackIndex = 0;
    UIPushScale(1.0f);
    
    // NOTE(Dima): Initializing layouts
    ui_layout* LayoutAt = Global_UI->FirstLayout;
    
    while(LayoutAt)
    {
        LayoutAt->At = V2(0.0f);
        
        LayoutAt = LayoutAt->Next;
    }
}

inline b32 InteractionIsHot(ui_interaction* Interaction)
{
    b32 Result = Global_UI->HotInteraction.Id == Interaction->Context.Id;
    
    return(Result);
}

inline void InteractionSetHot(ui_interaction* Interaction,
                              b32 ToSet)
{
    Assert(Interaction);
    
    if(ToSet)
    {
        if(Interaction->Context.Priority >= Global_UI->HotInteraction.Priority)
        {
            Global_UI->HotInteraction = Interaction->Context;
        }
    }
    else{
        if(Global_UI->HotInteraction.Id == Interaction->Context.Id){
            Global_UI->HotInteraction = {};
        }
    }
}

inline b32 InteractionIsActive(ui_interaction* Interaction)
{
    b32 Result = Global_UI->ActiveInteraction.Id == Interaction->Context.Id;
    
    return(Result);
}

inline void InteractionSetActive(ui_interaction* Interaction)
{
    if(InteractionIsHot(Interaction) && 
       Interaction->Context.Priority >= Global_UI->ActiveInteraction.Priority)
    {
        Global_UI->ActiveInteraction = Interaction->Context;
        InteractionSetHot(Interaction, false);
    }
}

inline void ReleaseInteraction(ui_interaction* Interaction)
{
    if(Interaction){
        if(InteractionIsActive(Interaction))
        {
            Global_UI->ActiveInteraction = {};
        }
    }
}

inline void ProcessMouseKeyInteractionInRect(ui_interaction* Interaction, 
                                             u32 MouseKey, 
                                             rc2 Rect,
                                             b32 Interactible = true,
                                             b32 Clickable = true)
{
    Interaction->WasHotInInteraction = false;
    Interaction->WasActiveInInteraction = false;
    
    if(MouseInRect(Rect) && Interactible)
    {
        InteractionSetHot(Interaction, true);
        Interaction->WasHotInInteraction = true;
        
        if(GetKeyDown(MouseKey) && Clickable)
        {
            InteractionSetActive(Interaction);
            Interaction->WasActiveInInteraction = true;
        }
    }
    else
    {
        InteractionSetHot(Interaction, false);
    }
    
    if(InteractionIsActive(Interaction) && GetKeyUp(MouseKey))
    {
        ReleaseInteraction(Interaction);
    }
}

// NOTE(Dima): Layouts
INTERNAL_FUNCTION inline ui_layout* GetCurrentLayout()
{
    ui_layout* Layout = Global_UI->CurrentLayout;
    
    return(Layout);
}

INTERNAL_FUNCTION inline ui_element* UIAllocateElement()
{
    if(DLIST_FREE_IS_EMPTY(Global_UI->ElementsFreeSentinel, NextAlloc))
    {
        // NOTE(Dima): Allocate new elements and insert them
        int ToAddFreeCount = 256;
        ui_element* ToAdd = PushArray(Global_UI->Arena, ui_element, ToAddFreeCount);
        
        for(int i = 0;
            i < ToAddFreeCount;
            i++)
        {
            ui_element* ElemI = &ToAdd[i];
            
            // NOTE(Dima): Inserting to free list
            DLIST_INSERT_AFTER_SENTINEL(ElemI, Global_UI->ElementsFreeSentinel, NextAlloc, PrevAlloc);
        }
    }
    
    // NOTE(Dima): Getting new element
    ui_element* Found = Global_UI->ElementsFreeSentinel.NextAlloc;
    
    // NOTE(Dima): Deleting from free list
    DLIST_REMOVE(Found, NextAlloc, PrevAlloc);
    
    // NOTE(Dima): Inserting to use list
    DLIST_INSERT_AFTER_SENTINEL(Found, Global_UI->ElementsUseSentinel, NextAlloc, PrevAlloc);
    
    return(Found);
}

INTERNAL_FUNCTION inline void UIDeallocateElement(ui_element* Element)
{
    // NOTE(Dima): Deleting from Use list
    DLIST_REMOVE(Element, NextAlloc, PrevAlloc);
    
    // NOTE(Dima): Inserting to Free list
    DLIST_INSERT_AFTER_SENTINEL(Element, Global_UI->ElementsFreeSentinel, NextAlloc, PrevAlloc);
}

INTERNAL_FUNCTION inline void UIInitChildSentinel(ui_element* Parent)
{
    ui_element* ChildSentinel = UIAllocateElement();
    
    Parent->ChildSentinel = ChildSentinel;
    ChildSentinel->Parent = Parent;
    
    DLIST_REFLECT_POINTER_PTRS(ChildSentinel, Next, Prev);
    
    char* Name = "ChildSentinel";
    CopyStringsSafe(ChildSentinel->DisplayName,
                    ArrayCount(ChildSentinel->DisplayName),
                    Name);
    CopyStringsSafe(ChildSentinel->IdName,
                    ArrayCount(ChildSentinel->IdName),
                    Name);
    ChildSentinel->IdNameHash = StringHashFNV(Name);
}

INTERNAL_FUNCTION inline b32 UIElementIsOpenedInTree(ui_element* Elem)
{
    ui_element* At = Elem->Parent;
    
    b32 Opened = true;
    while(At)
    {
        if(At->IsOpen == false)
        {
            Opened = false;
            
            break;
        }
        
        At = At->Parent;
    }
    
    return(Opened);
}

INTERNAL_FUNCTION inline int UIElementTreeDepth(ui_layout* Layout, 
                                                b32 StartFromCurrent = false)
{
    ui_element* Elem = Layout->CurrentElement;
    
    ui_element* At = Elem->Parent;
    if(StartFromCurrent)
    {
        At = Elem;
    }
    
    int Result = 0;
    while(At)
    {
        if(At->Type == UIElement_TreeNode)
        {
            Result++;
            
            break;
        }
        
        At = At->Parent;
    }
    
    return(Result);
}

INTERNAL_FUNCTION void ParseToHashString(char* ToHash, char* ToDisplay, char* From)
{
    char* At = From;
    char* AtDisplay = ToDisplay;
    char* ToCopyFrom = 0;
    
    while(At && *At)
    {
        if(At[0] == '#' &&
           At[1] == '#')
        {
            ToCopyFrom = At + 2;
            
            break;
        }
        else
        {
            *AtDisplay++ = *At++;
        }
    }
    *AtDisplay = 0;
    
    
    // NOTE(Dima): Copying to Hash
    char* Dst = ToHash;
    char* Src = From;
    
    if(ToCopyFrom)
    {
        Src = ToCopyFrom;
    }
    
    while(*Src)
    {
        *Dst++ = *Src++;
    }
    *Dst = 0;
}

/*
NOTE(Dima):

By default Name is Hashed and will be used to calculate ID;
By you can specify string that will be hashed by adding two ## symbols 
and add string that has to be hashed after them
For example "Some text ## This text is for ID calculation"

text1 - use text1 as Display name and as Name for ID calculation
text1##text2 - use text1 as Display name and text2 as name for ID calculation
*/

INTERNAL_FUNCTION ui_element* UIBeginElement(char* Name, u32 Type)
{
    ui_layout* Layout = GetCurrentLayout();
    
    ui_element* Found = 0;
    
    char IdName[UI_ELEMENT_NAME_SIZE];
    char DisplayName[UI_ELEMENT_NAME_SIZE];
    ParseToHashString(IdName, DisplayName, Name);
    u32 NameHash = StringHashFNV(IdName);
    
    ui_element* Parent = Layout->CurrentElement;
    u32 ParentID = 1;
    
    if(Parent && (Type != UIElement_Static))
    {
        ui_element* At = Parent->ChildSentinel->Next;
        
        while(At != Parent->ChildSentinel)
        {
            if(At->IdNameHash == NameHash)
            {
                if(StringsAreEqual(At->IdName, (char*)IdName))
                {
                    Found = At;
                    break;
                }
            }
            
            At = At->Next;
        }
        
        ParentID = Parent->Id;
    }
    
    if(!Found)
    {
        Found = UIAllocateElement();
        
        // NOTE(Dima): Initializing element
        CopyStringsSafe(Found->IdName, UI_ELEMENT_NAME_SIZE, IdName);
        Found->IdNameHash = NameHash;
        Found->Id = ParentID * NameHash * 479001599 + 993319;
        
        // NOTE(Dima): Allocating child sentinel
        UIInitChildSentinel(Found);
        
        Found->Parent = Parent;
        Found->IsOpen = true;
        Found->Type = Type;
        
        if(Parent)
        {
            // NOTE(Dima): Inserting to Parent's children
            DLIST_INSERT_AFTER(Found, Parent->ChildSentinel, Next, Prev);
        }
    }
    
    CopyStringsSafe(Found->DisplayName, UI_ELEMENT_NAME_SIZE, DisplayName);
    
    Layout->CurrentElement = Found;
    
    return(Found);
}

INTERNAL_FUNCTION void UIEndElement(u32 Type)
{
    ui_layout* Layout = GetCurrentLayout();
    
    ui_element* CurrentElement = Layout->CurrentElement;
    
    Assert(CurrentElement->Type == Type);
    
    ui_element* Parent = CurrentElement->Parent;
    
    if(Type == UIElement_Static)
    {
        DLIST_REMOVE(CurrentElement, Next, Prev);
        UIDeallocateElement(CurrentElement->ChildSentinel);
        UIDeallocateElement(CurrentElement);
    }
    
    Layout->CurrentElement = Parent;
}


// NOTE(Dima): Manipulation
#if 0
INTERNAL_FUNCTION inline void SameLine()
{
    ui_layout* Layout = GetCurrentLayout();
    
    Layout->StayOnSameLine = true;
}

INTERNAL_FUNCTION inline void SameColumn()
{
    ui_layout* Layout = GetCurrentLayout();
    
    Layout->StayOnSameColumn = true;
}
#endif


INTERNAL_FUNCTION inline ui_row_or_column* UIFindAlreadyAdvanced(b32 IncludingThis = false)
{
    ui_layout* Layout = GetCurrentLayout();
    
    int StartIndex = Layout->RowOrColumnIndex - 1;
    if(IncludingThis)
    {
        StartIndex = Layout->RowOrColumnIndex;
    }
    
    ui_row_or_column* Result = 0;
    for(int i = StartIndex; i >= 0 ; i--)
    {
        ui_row_or_column* RowOrColumn = &Layout->RowOrColumns[i];
        
        if(RowOrColumn->AdvanceBehaviour == RowColumnAdvanceBehaviour_Advanced)
        {
            Result = RowOrColumn;
            break;
        }
        else if(RowOrColumn->AdvanceBehaviour == RowColumnAdvanceBehaviour_Block)
        {
            break;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline void BeginRowOrColumn(b32 IsRow, 
                                               int AdvanceBehaviour = RowColumnAdvanceBehaviour_None)
{
    ui_layout* Layout = GetCurrentLayout();
    int TargetIndex = ++Layout->RowOrColumnIndex;
    
    ui_row_or_column* Src = 0;
    ui_row_or_column* Dst = &Layout->RowOrColumns[TargetIndex];
    
    if(TargetIndex - 1 >= 0)
    {
        Src = &Layout->RowOrColumns[TargetIndex - 1];
    }
    
    ui_row_or_column* AlreadyAdvanced = UIFindAlreadyAdvanced(false);
    
    Dst->StartAt = Layout->At;
    Dst->AdvanceBehaviour = AdvanceBehaviour;
    if(!AlreadyAdvanced)
    {
        Dst->StartAt.x = Layout->InitAt.x + (f32)(Layout->CurrentTreeDepth * UI_TAB_SPACES) * GetLineBase();
        Dst->AdvanceBehaviour = RowColumnAdvanceBehaviour_ShouldBeAdvanced;
    }
    
    Dst->IsRow = IsRow;
    Dst->Bounds = {};
}

INTERNAL_FUNCTION inline ui_row_or_column* GetRowOrColumnByIndex(ui_layout* Layout, int Index)
{
    ui_row_or_column* Result = 0;
    
    if(Index >= 0)
    {
        Result = &Layout->RowOrColumns[Index];
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline ui_row_or_column* GetParentRowOrColumn()
{
    ui_layout* Layout = GetCurrentLayout();
    
    ui_row_or_column* Result = GetRowOrColumnByIndex(Layout, Layout->RowOrColumnIndex - 1);
    
    return(Result);
}

INTERNAL_FUNCTION inline void EndRowOrColumn(b32 IsRow)
{
    ui_layout* Layout = GetCurrentLayout();
    
    ui_row_or_column* Src = &Layout->RowOrColumns[Layout->RowOrColumnIndex];
    
    Assert(Src->IsRow == IsRow);
    
    ui_row_or_column* Dst = GetRowOrColumnByIndex(Layout, --Layout->RowOrColumnIndex);
    
    if(Dst)
    {
        rc2 ResultBounds = Dst->Bounds;
        
        if(GetArea(Src->Bounds) > 1)
        {
            ResultBounds = Src->Bounds;
            
            if(GetArea(Dst->Bounds) > 1)
            {
                ResultBounds = UnionRect(ResultBounds, Dst->Bounds);
            }
        }
        
        Dst->Bounds = ResultBounds;
        
        if(Dst->IsRow)
        {
            Layout->At.x = Src->Bounds.Max.x + GetLineBase();
            Layout->At.y = Src->StartAt.y;
        }
        else
        {
            Layout->At.x = Src->StartAt.x;
            Layout->At.y = Src->Bounds.Max.y + GetLineBase();
        }
    }
}

INTERNAL_FUNCTION inline void BeginRow()
{
    BeginRowOrColumn(true);
}

INTERNAL_FUNCTION inline void EndRow()
{
    EndRowOrColumn(true);
}

INTERNAL_FUNCTION inline void BeginColumn()
{
    BeginRowOrColumn(false);
}

INTERNAL_FUNCTION inline void BeginTreeColumn()
{
    BeginRowOrColumn(false, RowColumnAdvanceBehaviour_Block);
}

INTERNAL_FUNCTION inline void EndColumn()
{
    EndRowOrColumn(false);
}


// NOTE(Dima): Layouts stuff
INTERNAL_FUNCTION b32 BeginLayout(const char* Name)
{
    ui_layout* Found = 0;
    
    // NOTE(Dima): FInding layout
    if(Global_UI->FirstLayout)
    {
        ui_layout* At = Global_UI->FirstLayout;
        
        while(At != 0)
        {
            if(StringsAreEqual((char*)Name, (char*)At->Name))
            {
                Found = At;
                break;
            }
            
            At = At->Next;
        }
    }
    
    // NOTE(Dima): If not found - then allocate
    b32 FoundJustAllocated = false;
    if(!Found)
    {
        Found = PushStruct(Global_UI->Arena, ui_layout);
        FoundJustAllocated = true;
    }
    
    // NOTE(Dima): Setting root element of this layout
    Assert(Global_UI->CurrentLayout == 0);
    Global_UI->CurrentLayout = Found;
    
    if(FoundJustAllocated)
    {
        Found->Name = Name;
        Found->Next = Global_UI->FirstLayout;
        Global_UI->FirstLayout = Found;
        
        Found->InitAt = V2(0.0f, 0.0f);
        
        Found->CurrentElement = 0;
        Found->Root = UIBeginElement("Root", UIElement_Root);
        Found->CurrentElement = Found->Root;
    }
    
    Found->At = Found->InitAt + V2(0.0f, GetLineBase());
    
    // NOTE(Dima): Beginning column
    Found->RowOrColumnIndex = -1;
    BeginColumn();
    
    b32 Result = true;
    return(Result);
}

INTERNAL_FUNCTION void EndLayout()
{
    ui_layout* Layout = GetCurrentLayout();
    
    Assert(Layout);
    
    EndColumn();
    
    Assert(Layout->CurrentElement->Type == UIElement_Root);
    
    Global_UI->CurrentLayout = 0;
}

INTERNAL_FUNCTION inline void StepLittleY()
{
    ui_layout* Layout = GetCurrentLayout();
    
    Layout->At.y += GetLineBase() * 0.25f;
}

INTERNAL_FUNCTION inline void PreAdvance()
{
    ui_layout* Layout = GetCurrentLayout();
    
    ui_row_or_column* RowOrColumn = &Layout->RowOrColumns[Layout->RowOrColumnIndex];
    b32 IsRow = RowOrColumn->IsRow;
    
    f32 HorizontalP = Layout->At.x;
    f32 VerticalP = Layout->At.y;
    
    b32 ShouldAdvance = true;
    
    for(int i = Layout->RowOrColumnIndex; i >= 0 ; i--)
    {
        ui_row_or_column* RowOrColumn = &Layout->RowOrColumns[i];
        
        if(RowOrColumn->AdvanceBehaviour == RowColumnAdvanceBehaviour_ShouldBeAdvanced)
        {
            RowOrColumn->AdvanceBehaviour = RowColumnAdvanceBehaviour_Advanced;
        }
        else if(RowOrColumn->AdvanceBehaviour == RowColumnAdvanceBehaviour_Advanced)
        {
            ShouldAdvance = false;
            break;
        }
        else if(RowOrColumn->AdvanceBehaviour == RowColumnAdvanceBehaviour_Block)
        {
            break;
        }
    }
    
    if(ShouldAdvance)
    {
        HorizontalP = Layout->InitAt.x + (f32)(UIElementTreeDepth(Layout) * UI_TAB_SPACES) * GetLineBase();
        RowOrColumn->AdvanceBehaviour = RowColumnAdvanceBehaviour_Advanced;
    }
    
    Layout->At.y = VerticalP;
    Layout->At.x = HorizontalP;
}

INTERNAL_FUNCTION inline void DescribeElement(rc2 ElementBounds)
{
    ui_layout* Layout = GetCurrentLayout();
    ui_row_or_column* RowOrColumn = &Layout->RowOrColumns[Layout->RowOrColumnIndex];
    
    Assert(RowOrColumn);
    
    if(GetArea(ElementBounds) > 1)
    {
        rc2 BoundsResult = ElementBounds;
        
        if(GetArea(RowOrColumn->Bounds) > 1)
        {
            BoundsResult = UnionRect(BoundsResult, RowOrColumn->Bounds);
        }
        
        RowOrColumn->Bounds = BoundsResult;
        
        if(RowOrColumn->IsRow)
        {
            Layout->At.x = ElementBounds.Max.x + GetLineBase();
        }
        else
        {
            Layout->At.y = ElementBounds.Max.y + GetLineBase();
        }
    }
}

// NOTE(Dima): Elements
INTERNAL_FUNCTION void ShowTextUnformatted(char* Text)
{
    ui_params* Params = UIGetParams();
    
    ui_element* Element = UIBeginElement(Text, UIElement_Static);
    
    if(UIElementIsOpenedInTree(Element))
    {
        PreAdvance();
        
        rc2 Bounds = PrintText(Params->Font, 
                               Text, 
                               Global_UI->CurrentLayout->At, 
                               Params->Scale, 
                               UIGetColor(UIColor_Text));
        
        DescribeElement(Bounds);
    }
    
    UIEndElement(UIElement_Static);
}

INTERNAL_FUNCTION int ShowText(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int Result = stbsp_vsnprintf(Global_UI->StringFormatBuffer, 
                                 ArrayCount(Global_UI->StringFormatBuffer), 
                                 fmt, args);
    va_end(args);
    
    ShowTextUnformatted(Global_UI->StringFormatBuffer);
    
    return(Result);
}

enum ui_button_flags
{
    TextElement_IsInteractible = (1 << 0),
    TextElement_IsClickable = (1 << 1),
    TextElement_BackgroundRectangle = (1 << 2),
    TextElement_UseCustomRectangle = (1 << 3),
    TextElement_Highlighted = (1 << 4),
};

INTERNAL_FUNCTION b32 TextElement(u32 Flags, b32* OpenedInTree, 
                                  rc2 CustomRect = {},
                                  u32 AlignHorizontal = TextAlign_Left,
                                  u32 AlignVertical = TextAlign_Top)
{
    b32 Pressed = false;
    
    ui_layout* Layout = GetCurrentLayout();
    
    ui_element* Element = Layout->CurrentElement;
    
    b32 IsOpened = UIElementIsOpenedInTree(Element);
    
    if(IsOpened)
    {
        char* NameToPrint = Element->DisplayName;
        
        // NOTE(Dima): Calculating bounds rectangle
        rc2 Bounds;
        if(BoolFlag(Flags, TextElement_UseCustomRectangle))
        {
            Bounds = CustomRect;
            
            v2 TextSize = GetTextSize(NameToPrint);
        }
        else
        {
            PreAdvance();
            
            Bounds = GetTextRect(Element->DisplayName, Layout->At);
            
            AlignHorizontal = TextAlign_Left;
            AlignVertical = TextAlign_Top;
        }
        
        // NOTE(Dima): Interaction
        ui_interaction Interaction = CreateInteraction(Element, InteractionPriority_Avg);
        
        ProcessMouseKeyInteractionInRect(&Interaction, 
                                         KeyMouse_Left, 
                                         Bounds,
                                         BoolFlag(Flags, TextElement_IsInteractible),
                                         BoolFlag(Flags, TextElement_IsClickable));
        
        v4 TextC = UIGetColor(UIColor_Text);
        if(Interaction.WasHotInInteraction)
        {
            TextC = UIGetColor(UIColor_TextHot);
            
            if(Interaction.WasActiveInInteraction)
            {
                Pressed = true;
            }
        }
        
        // NOTE(Dima): Pushing background
        if(Flags & TextElement_BackgroundRectangle)
        {
            v4 BackgroundColor = UIGetColor(UIColor_ButtonBackgroundInactive);
            if(Flags & TextElement_Highlighted)
            {
                BackgroundColor = UIGetColor(UIColor_ButtonBackground);
            }
            
            PushRect(Bounds, BackgroundColor);
        }
        
        // NOTE(Dima): Printing text
        PrintTextAligned(NameToPrint,
                         Bounds, 
                         AlignHorizontal,
                         AlignVertical,
                         TextC);
        
        // NOTE(Dima): Describing element
        DescribeElement(Bounds);
    }
    
    if(OpenedInTree)
    {
        *OpenedInTree = IsOpened;
    }
    
    return(Pressed);
}

INTERNAL_FUNCTION b32 Button(const char* Name, b32 Highlighted = true)
{
    ui_params* Params = UIGetParams();
    
    ui_element* Element = UIBeginElement((char*)Name, UIElement_Static);
    
    u32 ButtonFlags = (TextElement_IsInteractible |
                       TextElement_IsClickable |
                       TextElement_BackgroundRectangle);
    
    if(Highlighted)
    {
        ButtonFlags |= TextElement_Highlighted;
    }
    
    b32 Pressed = TextElement(ButtonFlags, 0);
    
    UIEndElement(UIElement_Static);
    
    return(Pressed);
}

INTERNAL_FUNCTION b32 BoolButton(const char* Name, b32* BoolSource, 
                                 char* PositiveText = 0, 
                                 char* NegativeText = 0)
{
    ui_params* Params = UIGetParams();
    
    BeginRow();
    
    ui_layout* Layout = GetCurrentLayout();
    ui_element* Element = UIBeginElement((char*)Name, UIElement_Static);
    
    u32 ButtonFlags = (TextElement_IsInteractible |
                       TextElement_IsClickable |
                       TextElement_BackgroundRectangle |
                       TextElement_UseCustomRectangle);
    
    if(!PositiveText)
    {
        PositiveText = "True";
    }
    
    if(!NegativeText)
    {
        NegativeText = "False";
    }
    
    v2 PositiveTextSize = GetTextSize(PositiveText);
    v2 NegativeTextSize = GetTextSize(NegativeText);
    
    PreAdvance();
    
    // NOTE(Dima): calculate button rectangle
    rc2 ButtonRect;
    v2 RectMinP = V2(Layout->At.x, Layout->At.y - GetLineBase());
    if(PositiveTextSize.x > NegativeTextSize.x)
    {
        ButtonRect = RectMinDim(RectMinP, PositiveTextSize);
    }
    else
    {
        ButtonRect = RectMinDim(RectMinP, NegativeTextSize);
    }
    
    // NOTE(Dima): Choosing which text to display
    char* DisplayText = "Null";
    if(BoolSource)
    {
        if(*BoolSource)
        {
            DisplayText = PositiveText;
        }
        else
        {
            DisplayText = NegativeText;
        }
    }
    
    CopyStringsSafe(Element->DisplayName, 
                    ArrayCount(Element->DisplayName), 
                    DisplayText);
    b32 Pressed = TextElement(ButtonFlags, 0, ButtonRect,
                              TextAlign_Center,
                              TextAlign_Top);
    
    if(Pressed && BoolSource)
    {
        *BoolSource = !*BoolSource;
    }
    
    UIEndElement(UIElement_Static);
    
    ShowTextUnformatted((char*)Name);
    
    EndRow();
    
    return(Pressed);
}

INTERNAL_FUNCTION void TreePop()
{
    EndColumn();
    UIEndElement(UIElement_TreeNode);
    
    ui_layout* Layout = GetCurrentLayout();
    Layout->CurrentTreeDepth = UIElementTreeDepth(Layout, true);
}

INTERNAL_FUNCTION b32 TreeNode(const char* Name)
{
    
    ui_element* Element = UIBeginElement((char*)Name, UIElement_TreeNode);
    ui_layout* Layout = GetCurrentLayout();
    Layout->CurrentTreeDepth = UIElementTreeDepth(Layout, true);
    
    u32 Flags = (TextElement_IsInteractible | TextElement_IsClickable);
    
    b32 OpenedInTree;
    b32 Pressed = TextElement(Flags, &OpenedInTree);
    if(Pressed)
    {
        Element->IsOpen = !Element->IsOpen;
    }
    
    BeginTreeColumn();
    
    // NOTE(Dima): Returning result
    b32 Result = OpenedInTree && Element->IsOpen;
    if(!Result)
    {
        TreePop();
    }
    
    return(Result);
}
