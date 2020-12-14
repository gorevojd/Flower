#ifndef FLOWER_UI_H
#define FLOWER_UI_H

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
};

struct ui_graph
{
    v2 Origin;
    v2 UnitMin;
    v2 UnitMax;
    f32 PixelsPerUnit;
    
    v2 AxisX;
    v2 AxisY;
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
