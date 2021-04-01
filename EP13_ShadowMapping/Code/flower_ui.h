#ifndef FLOWER_UI_H
#define FLOWER_UI_H

#define UI_TAB_SPACES 2

#define UI_GRAPH_WIDTH_UNITS 40.0f
#define UI_GRAPH_HEIGHT_SMALL_UNITS 6.0f
#define UI_GRAPH_HEIGHT_BIG_UNITS 20.0f

enum print_text_flags
{
    PrintText_3D = (1 << 0),
    PrintText_StyleShadow = (1 << 1),
    PrintText_StyleOutline = (1 << 2),
    PrintText_IsGetSizePass = (1 << 3),
};

enum text_align_type
{
    TextAlign_Left,
    TextAlign_Right,
    
    TextAlign_Top,
    TextAlign_Bottom,
    
    TextAlign_Center,
};


// NOTE(Dima): Bounding boxes
struct ui_element_bb
{
    // NOTE(Dima): This is for example for button Area
    rc2 Active;
    
    // NOTE(Dima): This is for total element rect (like button and it's text, line,  etc)
    rc2 Total;
};

enum ui_element_type
{
    UIElement_Root,
    UIElement_Static,
    UIElement_TreeNode,
    UIElement_Cached,
};

struct ui_element
{
#define UI_ELEMENT_NAME_SIZE 256
    char DisplayName[UI_ELEMENT_NAME_SIZE];
    char IdName[UI_ELEMENT_NAME_SIZE];
    u32 IdNameHash;
    u32 Id;
    
    b32 IsOpen;
    u32 Type;
    
    ui_element* Parent;
    
    ui_element* Next;
    ui_element* Prev;
    
    ui_element* NextAlloc;
    ui_element* PrevAlloc;
    
    ui_element* ChildSentinel;
};

enum ui_row_or_column_advance_behaviour
{
    RowColumnAdvanceBehaviour_None,
    RowColumnAdvanceBehaviour_Advanced,
    RowColumnAdvanceBehaviour_Block,
    RowColumnAdvanceBehaviour_ShouldBeAdvanced,
};

struct ui_row_or_column
{
    b32 IsRow;
    
    v2 StartAt;
    rc2 Bounds;
    
    // 0 - Nothing
    // 1 - Adnvanced
    // 2 - Block
    int AdvanceBehaviour;
};

// NOTE(Dima): Layouts
struct ui_layout
{
    const char* Name;
    v2 InitAt;
    v2 At;
    
    ui_row_or_column RowOrColumns[64];
    int RowOrColumnIndex;
    int CurrentTreeDepth;
    
    ui_element* CurrentElement;
    ui_element* Root;
    
    ui_layout* Next;
};

struct ui_params
{
    // NOTE(Dima): Ui params
    render_commands* Commands;
    font* Font;
    u32 FontStyle;
    f32 Scale;
    
    f32 ScaleStack[64];
    int ScaleStackIndex;
    
    font* FontStack[64];
    int FontStackIndex;
    
    window_dimensions WindowDimensions;
};

inline u32 UIGetPrintFlagsFromFontStyle(u32 FontStyle)
{
    u32 FontStyleFlag = 0;
    if(FontStyle == FontStyle_Shadow)
    {
        FontStyleFlag = PrintText_StyleShadow;
    }
    else if(FontStyle == FontStyle_Outline)
    {
        FontStyleFlag = PrintText_StyleOutline;
    }
    
    return(FontStyleFlag);
}

struct ui_graph
{
    v2 Origin;
    v2 UnitMin;
    v2 UnitMax;
    f32 PixelsPerUnit;
    b32 ShowIntLabels;
    
    v2 AxisX;
    v2 AxisY;
    f32 AxisThickness;
    
    v4 AxisXColor;
    v4 AxisYColor;
};

struct ui_slider_graph
{
    rc2 Rect;
    v2 Dim;
    
    f32 MinValue;
    f32 MaxValue;
    
    b32 ValueScreenPCalculated;
    v2 ValueScreenP;
};

struct ui_cell_grid_graph
{
    int HorzCount;
    int VertCount;
    
    rc2 Rect;
    
    f32 CellDim;
};

// NOTE(Dima): Colors
enum ui_color_type
{
    UIColor_Text,
    UIColor_TextHot,
    UIColor_ButtonBackground,
    UIColor_ButtonBackgroundInactive,
    UIColor_Borders,
    UIColor_GraphBackground,
    
    UIColor_GraphFrameNew,
    UIColor_GraphFrameOld,
    UIColor_GraphFrameCollation,
    UIColor_GraphFrameView,
    
    UIColor_Count,
};

struct ui_colors
{
    v4 Colors[UIColor_Count];
};

// NOTE(Dima): Interactions
enum ui_interaction_priority
{
    InteractionPriority_SuperLow,
    InteractionPriority_Low,
    InteractionPriority_Avg,
    InteractionPriority_High,
    InteractionPriority_SuperHigh,
};

struct ui_interaction_ctx
{
    u32 Id;
    char* Name;
    u32 Priority;
};

struct ui_interaction
{
    ui_element* Owner;
    
    ui_interaction_ctx Context;
    
    b32 WasHotInInteraction;
    b32 WasActiveInInteraction;
};

inline ui_interaction CreateInteraction(ui_element* Owner,
                                        u32 Priority = InteractionPriority_Low)
{
    ui_interaction Result = {};
    
    Result.Owner = Owner;
    
    Result.Context.Id = Owner->Id;
    Result.Context.Name = Owner->IdName;
    Result.Context.Priority = Priority;
    
    return(Result);
}

// NOTE(Dima): UI state
struct ui_state
{
    memory_arena* Arena;
    
    ui_layout* CurrentLayout;
    
    ui_graph Graph;
    ui_graph* CurrentGraph;
    
    ui_params Params;
    
    // NOTE(Dima): Immediate mode stuff
    ui_colors Colors;
    
    ui_interaction_ctx HotInteraction;
    ui_interaction_ctx ActiveInteraction;
    
    char StringFormatBuffer[2048];
    
    ui_layout* FirstLayout;
    
    ui_element ElementsUseSentinel;
    ui_element ElementsFreeSentinel;
};

#endif //FLOWER_UI_H
