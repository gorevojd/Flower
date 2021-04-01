INTERNAL_FUNCTION inline v2 GetGraphPoint(ui_graph* Graph, v2 UnitP)
{
    v2 Result = Graph->Origin + (-Graph->AxisX * -UnitP.x + Graph->AxisY * UnitP.y) * Graph->PixelsPerUnit;
    
    return(Result);
}

INTERNAL_FUNCTION inline v2 GetGraphPoint(ui_slider_graph* Graph, f32 UnitValue)
{
    v2 RectDim = GetDim(Graph->Rect);
    
    float Percentage = (UnitValue - Graph->MinValue) / (Graph->MaxValue - Graph->MinValue);
    
    v2 Result = V2(Graph->Rect.Min.x + Percentage * RectDim.x,
                   Graph->Rect.Min.y + RectDim.y * 0.5f);
    
    return(Result);
}

INTERNAL_FUNCTION v2 AddGraphPoint(ui_graph* Graph, v2 UnitP, float PixelRadius = 10.0f, v4 Color = ColorWhite())
{
    v2 TargetP = GetGraphPoint(Graph, UnitP);
    
    PushOutlinedCircle2D(TargetP,
                         PixelRadius,
                         2.0f,
                         Color);
    
    return(TargetP);
}

INTERNAL_FUNCTION v2 AddGraphPoint(ui_slider_graph* Graph,
                                   f32 PointRadius,
                                   f32 Value,
                                   v4 Color)
{
    v2 RectDim = GetDim(Graph->Rect);
    
    f32 TargetX = Graph->Rect.Min.x + RectDim.x * ((Value - Graph->MinValue) / (Graph->MaxValue - Graph->MinValue)); 
    f32 TargetY = Graph->Rect.Min.y + RectDim.y * 0.5f;
    
    v2 CircleCenter = V2(TargetX, TargetY);
    
    PushOutlinedCircle2D(CircleCenter,
                         PointRadius,
                         4,
                         Color);
    
    return(CircleCenter);
}

INTERNAL_FUNCTION rc2 AddPointLabel(char* Text,
                                    v2 PointCenter,
                                    f32 AngleDegrees,
                                    f32 Radius,
                                    u32 AlignX = TextAlign_Center,
                                    u32 AlignY = TextAlign_Center,
                                    f32 TextScale = 1.0f)
{
    v2 Offset = V2(Cos(AngleDegrees * F_DEG2RAD), 
                   Sin(AngleDegrees * F_DEG2RAD)) * Radius;
    
    UIPushScale(TextScale);
    rc2 Result = PrintTextAligned(Text, 
                                  PointCenter + Offset,
                                  AlignX,
                                  AlignY, 
                                  ColorBlack());
    UIPopScale();
    
#if 0    
    PushRectOutline(Global_UI->Params.Commands,
                    Result, 3,
                    ColorRed());
#endif
    
    return(Result);
}

INTERNAL_FUNCTION void AddHatchToAxis(ui_graph* Graph,
                                      b32 IsX, 
                                      int UnitIndex)
{
    render_commands* Commands = Global_UI->Params.Commands;
    
    v2 Axis = IsX ? Graph->AxisX : Graph->AxisY;
    v4 AxisColor = IsX ? Graph->AxisXColor : Graph->AxisYColor;
    
    f32 HatchWidth = Graph->AxisThickness * 4.0f;
    
    v2 HatchOrigin = GetGraphPoint(Graph, Axis * (f32)UnitIndex);
    v2 HatchPerp = V2(Axis.y, Axis.x);
    v2 HatchBegin = HatchOrigin + HatchPerp * HatchWidth * 0.5f;
    v2 HatchEnd = HatchOrigin - HatchPerp * HatchWidth * 0.5f;
    
    PushLine2D(HatchBegin, HatchEnd, Graph->AxisThickness, AxisColor);
    
    if(Graph->ShowIntLabels)
    {
        char Buffer[16];
        stbsp_sprintf(Buffer, "%d", UnitIndex);
        
        u32 HorizontalAlign = TextAlign_Left;
        u32 VerticalAlign = TextAlign_Center;
        f32 Angle = 0;
        if(IsX)
        {
            HorizontalAlign = TextAlign_Center;
            VerticalAlign = TextAlign_Bottom;
            Angle = 270;
        }
        
        AddPointLabel(Buffer, 
                      HatchOrigin,
                      Angle, 30,
                      HorizontalAlign,
                      VerticalAlign);
    }
}

void AddAxis(ui_graph* Graph,
             b32 IsX,
             b32 ShowCenterHatch = false)
{
    render_commands* Commands = Global_UI->Params.Commands;
    
    // NOTE(Dima): Axis color
    v4 AxisColor = IsX ? Graph->AxisXColor : Graph->AxisYColor;
    v2 Axis = IsX ? Graph->AxisX : Graph->AxisY;
    
    // NOTE(Dima): Axis value
    
    f32 DefaultLowerUnitOffset = 0.5f;
    f32 DefaultUpperUnitOffset = 0.75f;
    
    float UnitMin = Graph->UnitMin.y;
    float UnitMax = Graph->UnitMax.y;
    if(IsX)
    {
        UnitMin = Graph->UnitMin.x;
        UnitMax = Graph->UnitMax.x;
    }
    
    f32 LowerUnits = (std::abs(UnitMin) + DefaultLowerUnitOffset) * Graph->PixelsPerUnit;
    f32 UpperUnits = (std::abs(UnitMax) + DefaultUpperUnitOffset) * Graph->PixelsPerUnit;
    
    int UnitMinInt = std::ceil(UnitMin);
    int UnitMaxInt = std::floor(UnitMax);
    
    // NOTE(Dima): Pushing axis
    PushArrow2D(Graph->Origin - Axis * LowerUnits,
                Graph->Origin + Axis * UpperUnits,
                Graph->AxisThickness,
                AxisColor);
    
    // NOTE(Dima): Pushing hatches
    for(int UnitIndex = UnitMinInt; UnitIndex <= UnitMaxInt; UnitIndex++)
    {
        if(UnitIndex != 0)
        {
            AddHatchToAxis(Graph, IsX, UnitIndex);
        }
        else
        {
            if(ShowCenterHatch)
            {
                AddHatchToAxis(Graph, IsX, UnitIndex);
            }
        }
    }
}

INTERNAL_FUNCTION ui_graph BeginGraph(v2 Origin, 
                                      v2 UnitMin, v2 UnitMax, 
                                      f32 PixelsPerUnit = 50.0f, 
                                      f32 Thickness = RENDER_DEFAULT_2D_LINE_THICKNESS,
                                      b32 ShowVerticalAxis = true,
                                      b32 ShowIntLabels = false)
{
    // NOTE(Dima): Init graph 
    ui_graph Graph = {};
    
    Graph.Origin = Origin;
    Graph.UnitMin = UnitMin;
    Graph.UnitMax = UnitMax;
    Graph.PixelsPerUnit = PixelsPerUnit;
    Graph.AxisX = V2(1.0f, 0.0f);
    Graph.AxisY = V2(0.0f, -1.0f);
    Graph.AxisThickness = Thickness;
    Graph.AxisXColor = ColorRed();
    Graph.AxisYColor = ColorGreen();
    Graph.ShowIntLabels = ShowIntLabels;
    
    b32 ShowCenterHatch = !ShowVerticalAxis;
    
    AddAxis(&Graph, true, ShowCenterHatch);
    
    if(ShowVerticalAxis)
    {
        AddAxis(&Graph, false, !ShowVerticalAxis);
    }
    
    return(Graph);
}

INTERNAL_FUNCTION v2 AddGraphCircle(ui_graph* Graph, v2 UnitP, f32 UnitRadius, v4 Color = ColorWhite())
{
    v2 TargetP = GetGraphPoint(Graph, UnitP);
    
    f32 PixelRadius = UnitRadius * Graph->PixelsPerUnit;
    
    f32 HalfThick = RENDER_DEFAULT_2D_LINE_THICKNESS * 0.5f;
    
    PushCircleInternal2D(TargetP,
                         PixelRadius - HalfThick,
                         PixelRadius + HalfThick,
                         Color,
                         32);
    
    return(TargetP);
}

INTERNAL_FUNCTION ui_cell_grid_graph CreateCellGridGraph(int HorzCount,
                                                         int VertCount,
                                                         f32 GraphHeight)
{
    ui_cell_grid_graph Result = {};
    
    Result.HorzCount = HorzCount;
    Result.VertCount = HorzCount;
    Result.CellDim = GraphHeight / (f32)VertCount;
    
    v2 GraphDim = V2(GraphHeight / (f32)VertCount * (f32)HorzCount);
    Result.Rect = RectCenterDim(UVToScreenPoint(0.5f, 0.5f), GraphDim);
    
    return(Result);
}

INTERNAL_FUNCTION ui_cell_grid_graph CreateCellGridGraph(v2 GraphDim,
                                                         f32 CellDim,
                                                         b32 IsCustomP = false,
                                                         v2 CustomP = V2(0.0f, 0.0f))
{
    ui_cell_grid_graph Result = {};
    
    Result.HorzCount = std::floor(GraphDim.x / (f32)CellDim);
    Result.VertCount = std::floor(GraphDim.y / (f32)CellDim);
    Result.CellDim = CellDim;
    
    v2 ActualGraphDim = V2(Result.HorzCount, Result.VertCount) * CellDim;
    
    if(IsCustomP)
    {
        Result.Rect = RectMinDim(CustomP, ActualGraphDim);
    }
    else
    {
        Result.Rect = RectCenterDim(UVToScreenPoint(0.5f, 0.5f), ActualGraphDim);
    }
    
    return(Result);
}

INTERNAL_FUNCTION void ShowCellGrid(ui_cell_grid_graph* Graph, 
                                    f32 LineThickness = 3.0f,
                                    v4 LineColor = ColorGray(0.3f))
{
    v2 GridDim = GetDim(Graph->Rect);
    
    // NOTE(Dima): Horizontal lines
    for(int x = 1; x < Graph->HorzCount; x++)
    {
        f32 CoordX = Graph->Rect.Min.x + x * Graph->CellDim;
        
        PushLine2D(V2(CoordX, Graph->Rect.Min.y), 
                   V2(CoordX, Graph->Rect.Max.y),
                   LineThickness, LineColor);
    }
    
    // NOTE(Dima): Vertical lines
    for(int y = 1; y < Graph->VertCount; y++)
    {
        f32 CoordY = Graph->Rect.Min.y + y * Graph->CellDim;
        
        PushLine2D(V2(Graph->Rect.Min.x, CoordY),
                   V2(Graph->Rect.Max.x, CoordY),
                   LineThickness, LineColor);
    }
    
    // NOTE(Dima): Outer lines
    PushRectOutline(Graph->Rect, LineThickness, LineColor);
}

INTERNAL_FUNCTION rc2 GetCellRect(ui_cell_grid_graph* Graph, int x, int y)
{
    v2 Min = Graph->Rect.Min + Graph->CellDim * V2(x, y);
    
    rc2 Result = RectMinDim(Min, V2(Graph->CellDim));
    
    return(Result);
}

INTERNAL_FUNCTION rc2 GetCellRectByIndex(ui_cell_grid_graph* Graph, int Index)
{
    int x = Index % Graph->HorzCount;
    int y = Index / Graph->HorzCount;
    
    rc2 Result = GetCellRect(Graph, x, y);
    
    return(Result);
}
