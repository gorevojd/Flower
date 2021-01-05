#ifndef FLOWER_UI_H
#define FLOWER_UI_H

enum text_align_type
{
    TextAlign_Left,
    TextAlign_Right,
    
    TextAlign_Top,
    TextAlign_Bottom,
    
    TextAlign_Center,
};

struct ui_layout
{
    v2 At;
};

struct ui_params
{
    // NOTE(Dima): Ui params
    render_commands* Commands;
    font* Font;
    f32 Scale;
    
    window_dimensions* WindowDims;
};

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

struct ui_state
{
    ui_layout Layout;
    ui_layout* CurrentLayout;
    
    ui_graph Graph;
    ui_graph* CurrentGraph;
    
    ui_params Params;
};

#endif //FLOWER_UI_H
