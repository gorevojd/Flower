void AddAxis(render_commands* Commands,
             f32 UnitMin, f32 UnitMax,
             v2 Origin,
             v2 Axis,
             v4 AxisColor,
             f32 PixelsPerUnit,
             f32 Thickness)
{
    f32 DefaultLowerUnitOffset = 0.5f;
    f32 DefaultUpperUnitOffset = 0.75f;
    
    f32 LowerUnits = (std::abs(UnitMin) + DefaultLowerUnitOffset) * PixelsPerUnit;
    f32 UpperUnits = (std::abs(UnitMax) + DefaultUpperUnitOffset) * PixelsPerUnit;
    
    f32 HatchWidth = Thickness * 4;
    int UnitMinInt = std::ceil(UnitMin);
    int UnitMaxInt = std::floor(UnitMax);
    
    // NOTE(Dima): Pushing axis
    PushArrow2D(Commands,
                Origin - Axis * LowerUnits,
                Origin + Axis * UpperUnits,
                Thickness,
                AxisColor);
    
    // NOTE(Dima): Pushing hatches
    for(int UnitIndex = UnitMinInt; UnitIndex <= UnitMaxInt; UnitIndex++)
    {
        if(UnitIndex != 0)
        {
            v2 HatchOrigin = Origin + Axis * (f32)UnitIndex * PixelsPerUnit;
            v2 HatchPerp = V2(Axis.y, Axis.x);
            v2 HatchBegin = HatchOrigin + HatchPerp * HatchWidth * 0.5f;
            v2 HatchEnd = HatchOrigin - HatchPerp * HatchWidth * 0.5f;
            
            PushLine2D(Commands, HatchBegin, HatchEnd, Thickness, AxisColor);
        }
    }
}

INTERNAL_FUNCTION ui_graph BeginGraph(v2 Origin, 
                                      v2 UnitMin, v2 UnitMax, 
                                      f32 PixelsPerUnit = 50.0f, 
                                      f32 Thickness = RENDER_DEFAULT_2D_LINE_THICKNESS)
{
    // NOTE(Dima): Init graph 
    ui_graph Graph = {};
    
    Graph.Origin = Origin;
    Graph.UnitMin = UnitMin;
    Graph.UnitMax = UnitMax;
    Graph.PixelsPerUnit = PixelsPerUnit;
    Graph.AxisX = V2(1.0f, 0.0f);
    Graph.AxisY = V2(0.0f, -1.0f);
    
    AddAxis(GlobalUI->Params.Commands, 
            UnitMin.x, UnitMax.x,
            Origin,
            Graph.AxisX,
            ColorRed(),
            PixelsPerUnit,
            Thickness);
    
    AddAxis(GlobalUI->Params.Commands, 
            UnitMin.y, UnitMax.y,
            Origin,
#if 0                
            V2(0.0f, -1.0f),
#else
            Graph.AxisY,
#endif
            ColorGreen(),
            PixelsPerUnit,
            Thickness);
    
    return(Graph);
}

INTERNAL_FUNCTION void AddGraphPoint(ui_graph* Graph, v2 UnitP, float PixelRadius = 10.0f, v4 Color = ColorWhite())
{
    v2 TargetP = Graph->Origin + (-Graph->AxisX * UnitP.x + Graph->AxisY * UnitP.y) * Graph->PixelsPerUnit;
    
    PushCircle2D(GlobalUI->Params.Commands,
                 TargetP,
                 PixelRadius,
                 Color);
}

INTERNAL_FUNCTION void AddGraphCircle(ui_graph* Graph, v2 UnitP, f32 UnitRadius, v4 Color = ColorWhite())
{
    v2 TargetP = Graph->Origin + (-Graph->AxisX * UnitP.x + Graph->AxisY * UnitP.y) * Graph->PixelsPerUnit;
    f32 PixelRadius = UnitRadius * Graph->PixelsPerUnit;
    
    f32 HalfThick = RENDER_DEFAULT_2D_LINE_THICKNESS * 0.5f;
    
    PushCircleInternal2D(GlobalUI->Params.Commands,
                         TargetP,
                         PixelRadius - HalfThick,
                         PixelRadius + HalfThick,
                         Color,
                         32);
}

INTERNAL_FUNCTION void AddGraphMoveVector(v2 MoveVector)
{
    ui_graph Graph = BeginGraph(V2(1200, 250), V2(-1, -1), V2(1, 1), 100);
    AddGraphCircle(&Graph, V2(0.0f, 0.0f), 1, V4(0.5f, 0.5f, 0.5f, 1.0f));
    AddGraphPoint(&Graph, V2(MoveVector.x, MoveVector.y), 8);
    
    char HorizontalBuffer[64];
    stbsp_sprintf(HorizontalBuffer, "Horizontal = %.2f", -MoveVector.x);
    
    char VerticalBuffer[64];
    stbsp_sprintf(VerticalBuffer, "Vertical = %.2f", MoveVector.y);
    
    BeginLayout(V2(800, 100));
    Text(HorizontalBuffer);
    Text(VerticalBuffer);
    Text("Left", GetButton(Button_Left));
    Text("Right", GetButton(Button_Right));
    Text("Up", GetButton(Button_Up));
    Text("Down", GetButton(Button_Down));
    EndLayout();
}