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
    
    rc2 Result = PrintTextAligned(Text, 
                                  PointCenter + Offset,
                                  AlignX,
                                  AlignY, 
                                  TextScale,
                                  ColorBlack(), false);
    
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


typedef float function_prototype_rounding(float);

INTERNAL_FUNCTION void RoundingGraph(function_prototype_rounding* RoundFunc, 
                                     char* FunctionName,
                                     float Value)
{
    v2 MinUnit = V2(-4);
    v2 MaxUnit = V2(4);
    
    ui_graph Graph = BeginGraph(UVToScreenPoint(0.5f, 0.5f), 
                                MinUnit, 
                                MaxUnit, 150,
                                RENDER_DEFAULT_2D_LINE_THICKNESS,
                                false, true);
    
    int ResultI = RoundFunc(Value);
    
    f32 Result = (f32)ResultI;
    
    AddGraphPoint(&Graph, V2(Result, 0.0f), 20, ColorBlue());
    AddGraphPoint(&Graph, V2(Value, 0.0f), 15, ColorGreen());
    
    char ABuf[64];
    char BBuf[64];
    
    stbsp_sprintf(ABuf, "float A = %.2f", Value);
    stbsp_sprintf(BBuf, "int B = %s(A) = %d", FunctionName, ResultI);
    
    v2 APos = UVToScreenPoint(0.5f, 0.65f);
    v2 BPos = UVToScreenPoint(0.5f, 0.73f);
    
    f32 TextScale = 1.1f;
    
    PrintTextAligned(ABuf, APos, 
                     TextAlign_Center,
                     TextAlign_Center,
                     TextScale,
                     ColorGreen());
    
    PrintTextAligned(BBuf, BPos, 
                     TextAlign_Center,
                     TextAlign_Center,
                     TextScale,
                     ColorBlue());
}

INTERNAL_FUNCTION void MoveVectorGraph(v2 MoveVector)
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

// NOTE(Dima): Params are in pixels
INTERNAL_FUNCTION ui_slider_graph SliderGraph(v2 CenterP, 
                                              v2 Dim,
                                              f32 Min,
                                              f32 Max,
                                              b32 ShowRect = true)
{
    ui_slider_graph Result = {};
    
    Result.Rect = RectCenterDim(CenterP, Dim);
    Result.Dim = GetDim(Result.Rect);
    Result.MinValue = Min;
    Result.MaxValue = Max;
    
    if(ShowRect)
    {
        PushRect(Result.Rect, 
                 ColorGray());
    }
    
    return(Result);
}

INTERNAL_FUNCTION ui_slider_graph ValueSlider(f32 Value,
                                              f32 Min,
                                              f32 Max,
                                              v2 Center,
                                              v2 Dim,
                                              char* ValueLabelBuffer,
                                              b32 ShowSliderBack = true,
                                              b32 ShowMinMaxLabels = true,
                                              f32 ValueLabelAngle = 90.0f, 
                                              f32 ValueLabelOffset = 40.0f)
{
    ui_slider_graph TGraph = SliderGraph(Center, Dim,
                                         Min, Max, ShowSliderBack);
    
    char MinBuffer[16];
    char MaxBuffer[16];
    
    f32 MinFract = Min - (f32)((int)Min);
    f32 MaxFract = Max - (f32)((int)Max);
    b32 MinIsInt = std::abs(MinFract) < 0.000001f;
    b32 MaxIsInt = std::abs(MaxFract) < 0.000001f;
    
    if(MinIsInt)
    {
        stbsp_sprintf(MinBuffer, "%d", (int)Min);
    }
    else
    {
        stbsp_sprintf(MinBuffer, "%.2f", TGraph.MinValue);
    }
    
    if(MaxIsInt)
    {
        stbsp_sprintf(MaxBuffer, "%d", (int)Max);
    }
    else
    {
        stbsp_sprintf(MaxBuffer, "%.2f", TGraph.MaxValue);
    }
    
    f32 TextScale = 1.0f;
    
    if(ShowMinMaxLabels)
    {
        AddPointLabel(MinBuffer, 
                      TGraph.Rect.Min + V2(0, TGraph.Dim.y * 0.5f),
                      180, 30,
                      TextAlign_Right,
                      TextAlign_Center,
                      TextScale);
        
        AddPointLabel(MaxBuffer,
                      V2(TGraph.Rect.Max.x,
                         TGraph.Rect.Min.y) + V2(0, TGraph.Dim.y * 0.5f),
                      0, 30,
                      TextAlign_Left,
                      TextAlign_Center,
                      TextScale);
    }
    
    v2 TPos = AddGraphPoint(&TGraph, 
                            20, Value,
                            ColorWhite());
    
    AddPointLabel(ValueLabelBuffer,
                  TPos,
                  ValueLabelAngle, 
                  ValueLabelOffset,
                  TextAlign_Center,
                  TextAlign_Top,
                  TextScale);
    
    TGraph.ValueScreenPCalculated = true;
    TGraph.ValueScreenP = TPos;
    
    return(TGraph);
}

INTERNAL_FUNCTION void LerpSliderT(f32 t)
{
    char TBuffer[16];
    stbsp_sprintf(TBuffer, "t = %.2f (%d%%)", t, (int)(t * 100.0f));
    
    ValueSlider(t, 0, 1, 
                UVToScreenPoint(V2(0.5f, 0.65f)),
                UVToScreenPoint(V2(0.6f, 0.02f)),
                TBuffer);
}

INTERNAL_FUNCTION void LerpGraph(f32 t, b32 ShowLabels = true)
{
    LerpSliderT(t);
    
    float TextScale = 1.3f;
    
    // NOTE(Dima): Value
    ui_slider_graph ValueGraph = SliderGraph(UVToScreenPoint(V2(0.5f, 0.4f)),
                                             UVToScreenPoint(V2(0.75f, 0.02f)),
                                             -20, 46, false);
    
    
    // NOTE(Dima): Min Value point
    v2 MinValuePos = AddGraphPoint(&ValueGraph, 
                                   30, ValueGraph.MinValue,
                                   ColorRed());
    
    // NOTE(Dima): Max value point
    v2 MaxValuePos = AddGraphPoint(&ValueGraph, 
                                   30, ValueGraph.MaxValue,
                                   ColorGreen());
    
    // NOTE(Dima): Interpolated point
    f32 LerpValue = Lerp(ValueGraph.MinValue, ValueGraph.MaxValue, t);
    v2 ValuePos = AddGraphPoint(&ValueGraph,
                                20, 
                                LerpValue,
                                ColorBlue());
    
    // NOTE(Dima): Labels
    char ValueMinBuf[16];
    FloatToString(ValueMinBuf, ValueGraph.MinValue, 2);
    
    char ValueMaxBuf[16];
    FloatToString(ValueMaxBuf, ValueGraph.MaxValue, 2);
    
    char LerpValueBuf[16];
    stbsp_sprintf(LerpValueBuf, "P = %.2f", LerpValue);
    
    if(ShowLabels)
    {
        
        AddPointLabel(ValueMinBuf, 
                      MinValuePos,
                      180, 30,
                      TextAlign_Right,
                      TextAlign_Center,
                      TextScale);
        
        AddPointLabel(ValueMaxBuf, 
                      MaxValuePos,
                      0, 30,
                      TextAlign_Left,
                      TextAlign_Center,
                      TextScale);
        
        AddPointLabel(LerpValueBuf, 
                      ValuePos,
                      90, 20,
                      TextAlign_Center,
                      TextAlign_Top,
                      TextScale);
    }
}

INTERNAL_FUNCTION void LerpGraphColors(float t, 
                                       v4 Min = ColorRed(), 
                                       v4 Max = ColorGreen(), 
                                       b32 ShowLabels = true)
{
    render_commands* Commands = Global_UI->Params.Commands;
    
    LerpSliderT(t);
    
    float TextScale = 0.6f;
    
    v2 MinColorP = UVToScreenPoint(0.2f, 0.45f);
    v2 MaxColorP = UVToScreenPoint(0.8f, 0.45f);
    
    v2 MinMaxDim = V2(80.0f);
    
    v4 MinColor = Min;
    v4 MaxColor = Max;
    
    PushOutlinedRect(RectCenterDim(MinColorP, MinMaxDim),
                     2,
                     MinColor);
    
    PushOutlinedRect(RectCenterDim(MaxColorP, MinMaxDim),
                     2,
                     MaxColor);
    
    v4 LerpedColor = Lerp(MinColor, MaxColor, t);
    
    v2 LerpColorP = Lerp(MinColorP, MaxColorP, t);
    
    PushOutlinedRect(RectCenterDim(LerpColorP, MinMaxDim),
                     2,
                     LerpedColor);
    
    if(ShowLabels)
    {
        // NOTE(Dima): Labels
        char ValueMinBuf[32];
        stbsp_sprintf(ValueMinBuf, "RGB(%.2f, %.2f, %.2f)", 
                      MinColor.r,
                      MinColor.g,
                      MinColor.b);
        
        char ValueMaxBuf[32];
        stbsp_sprintf(ValueMaxBuf, "RGB(%.2f, %.2f, %.2f)",
                      MaxColor.r,
                      MaxColor.g,
                      MaxColor.b);
        
        char LerpValueBuf[32];
        stbsp_sprintf(LerpValueBuf, "RGB(%.2f, %.2f, %.2f)", 
                      LerpedColor.r,
                      LerpedColor.g,
                      LerpedColor.b);
        
        AddPointLabel(ValueMinBuf, 
                      MinColorP,
                      180, 40,
                      TextAlign_Right,
                      TextAlign_Center,
                      TextScale);
        
        AddPointLabel(ValueMaxBuf, 
                      MaxColorP,
                      0, 40,
                      TextAlign_Left,
                      TextAlign_Center,
                      TextScale);
        
        AddPointLabel(LerpValueBuf, 
                      LerpColorP,
                      270, 40,
                      TextAlign_Center,
                      TextAlign_Bottom,
                      TextScale);
    }
}

INTERNAL_FUNCTION void LerpGraphPositions(float t, b32 ShowLabels = true)
{
    LerpSliderT(t);
    
    float TextScale = 1.0f;
    
    ui_graph Graph = BeginGraph(UVToScreenPoint(V2(0.16f, 0.5f)), 
                                V2(0.0f, 0.0f), V2(20.0f, 7.0f), 
                                50.0f, 
                                6);
    
#if 1    
    v2 MinP = V2(2.0f, 6.0f);
    v2 MaxP = V2(13.0f, 1.0f);
#else
    v2 MinP = V2(3.0f, 1.0f);
    v2 MaxP = V2(9.0f, 6.0f);
#endif
    
    f32 LerpPx = Lerp(MinP.x, MaxP.x, t);
    f32 LerpPy = Lerp(MinP.y, MaxP.y, t);
    
    v2 LerpP = V2(LerpPx, LerpPy);
    
    v2 GraphMinP = GetGraphPoint(&Graph, MinP);
    v2 GraphMaxP = GetGraphPoint(&Graph, MaxP);
    v2 GraphLerpP = GetGraphPoint(&Graph, LerpP);
    
    AddGraphPoint(&Graph, MinP, 10.0f, ColorRed());
    AddGraphPoint(&Graph, MaxP, 10.0f, ColorGreen());
    
    b32 DrawLine = false;
    
    if(DrawLine)
    {
        PushRoundLine2D(GraphMinP, GraphLerpP,
                        11.0f,
                        ColorBlue());
    }
    
    AddGraphPoint(&Graph, LerpP, 10.0f, ColorBlue());
    
    if(ShowLabels)
    {
        
        // NOTE(Dima): Labels
        char ValueMinBuf[32];
        stbsp_sprintf(ValueMinBuf, "(%.2f, %.2f)", 
                      MinP.x,
                      MinP.y);
        
        char ValueMaxBuf[32];
        stbsp_sprintf(ValueMaxBuf, "(%.2f, %.2f)",
                      MaxP.x,
                      MaxP.y);
        
        char LerpValueBuf[32];
        stbsp_sprintf(LerpValueBuf, "(%.2f, %.2f)", 
                      LerpP.x,
                      LerpP.y);
        
        AddPointLabel(ValueMinBuf, 
                      GraphMinP,
                      180, 30,
                      TextAlign_Right,
                      TextAlign_Center,
                      TextScale);
        
        AddPointLabel(ValueMaxBuf, 
                      GraphMaxP,
                      0, 30,
                      TextAlign_Left,
                      TextAlign_Center,
                      TextScale);
        
        AddPointLabel(LerpValueBuf, 
                      GraphLerpP,
                      270, 20,
                      TextAlign_Center,
                      TextAlign_Bottom,
                      TextScale);
    }
}

INTERNAL_FUNCTION void ClampGraph(f32 Value, 
                                  f32 ValueMin, f32 ValueMax,
                                  f32 ClampMin, f32 ClampMax)
{
    char TBuffer[32];
    stbsp_sprintf(TBuffer, "Value = %.2f", Value);
    
    float ClampedValue = Clamp(Value, ClampMin, ClampMax);
    
    char ResBuffer[32];
    stbsp_sprintf(ResBuffer, "Result = %.2f", ClampedValue);
    
    ValueSlider(Value, ValueMin, ValueMax, 
                UVToScreenPoint(V2(0.5f, 0.7f)),
                UVToScreenPoint(V2(0.6f, 0.02f)),
                TBuffer);
    
    v2 ResultScreenP = UVToScreenPoint(V2(0.5f, 0.4f)); 
    ui_slider_graph ResultSlider = ValueSlider(ClampedValue, 
                                               ValueMin, ValueMax, 
                                               ResultScreenP,
                                               UVToScreenPoint(V2(0.6f, 0.02f)),
                                               ResBuffer, false, false);
    
#if 0    
    v2 MinClampP = GetGraphPoint(&ResultSlider, ClampMin);
    v2 MaxClampP = GetGraphPoint(&ResultSlider, ClampMax);
#endif
    
    v2 MinClampP = AddGraphPoint(&ResultSlider, 
                                 14, ClampMin,
                                 ColorRed());
    v2 MaxClampP = AddGraphPoint(&ResultSlider,
                                 14, ClampMax,
                                 ColorGreen());
    
    char MinClampBuffer[16];
    stbsp_sprintf(MinClampBuffer, "Min = %.2f", ClampMin);
    
    char MaxClampBuffer[16];
    stbsp_sprintf(MaxClampBuffer, "Max = %.2f", ClampMax);
    
    
    
    AddPointLabel(MinClampBuffer,
                  MinClampP,
                  250, 30,
                  TextAlign_Right,
                  TextAlign_Bottom,
                  1.0f);
    
    AddPointLabel(MaxClampBuffer,
                  MaxClampP,
                  290, 30,
                  TextAlign_Left,
                  TextAlign_Bottom,
                  1.0f);
}

INTERNAL_FUNCTION void MappingRangeGraph(v2 Range1,
                                         v2 Range2,
                                         f32 t)
{
    f32 RangeRad = 20.0f;
    b32 IsLettersLabels = false;
    b32 ShowSlidersBack = true;
    
    char Range1Buf[32];
    stbsp_sprintf(Range1Buf, "X", t);
    
    char Range2Buf[32];
    stbsp_sprintf(Range2Buf, "Y = ?", t);
    
    f32 Range1Value = Lerp(Range1.x, Range1.y, t);
    f32 Range2Value = Lerp(Range2.x, Range2.y, t);
    
    if(!IsLettersLabels)
    {
        stbsp_sprintf(Range1Buf, "X = %.2f", Range1Value);
        stbsp_sprintf(Range2Buf, "Y = %.2f", Range2Value);
    }
    
    f32 Range1Magn = Range1.Max - Range1.Min;
    f32 Range2Magn = Range2.Max - Range2.Min;
    
    // NOTE(Dima): Range1
    ui_slider_graph Graph1 = ValueSlider(Range1Value, 
                                         Range1.x, Range1.y, 
                                         UVToScreenPoint(V2(0.35f, 0.7f)),
                                         UVToScreenPoint(V2(Range1Magn * 0.08f, 0.02f)),
                                         Range1Buf,
                                         ShowSlidersBack,
                                         !IsLettersLabels);
    
    v2 R1Min = AddGraphPoint(&Graph1, RangeRad, Range1.Min, ColorRed());
    v2 R1Max = AddGraphPoint(&Graph1, RangeRad, Range1.Max, ColorRed());
    
    // NOTE(Dima): Range2
    ui_slider_graph Graph2 = ValueSlider(Range2Value, 
                                         Range2.x, Range2.y, 
                                         UVToScreenPoint(V2(0.65f, 0.4f)),
                                         UVToScreenPoint(V2(Range2Magn * 0.06f, 0.02f)),
                                         Range2Buf,
                                         ShowSlidersBack,
                                         !IsLettersLabels);
    
    v2 R2Min = AddGraphPoint(&Graph2, RangeRad, Range2.Min, ColorBlue());
    v2 R2Max = AddGraphPoint(&Graph2, RangeRad, Range2.Max, ColorBlue());
    
    // NOTE(Dima): Labels
    if(IsLettersLabels)
    {
        AddPointLabel("A",
                      R1Min,
                      180, 30,
                      TextAlign_Right,
                      TextAlign_Center,
                      1.0f);
        
        AddPointLabel("B",
                      R1Max,
                      0, 30,
                      TextAlign_Left,
                      TextAlign_Center,
                      1.0f);
        
        AddPointLabel("C",
                      R2Min,
                      180, 30,
                      TextAlign_Right,
                      TextAlign_Center,
                      1.0f);
        
        AddPointLabel("D",
                      R2Max,
                      0, 30,
                      TextAlign_Left,
                      TextAlign_Center,
                      1.0f);
    }
    
    
#if 1    
    // NOTE(Dima): Misc
    if(Graph1.ValueScreenPCalculated && Graph2.ValueScreenPCalculated)
    {
        PushDashedLine2D(Graph1.ValueScreenP,
                         Graph2.ValueScreenP,
                         RENDER_DEFAULT_2D_LINE_DASH_LENGTH,
                         RENDER_DEFAULT_2D_LINE_DASH_SPACING,
                         RENDER_DEFAULT_2D_LINE_THICKNESS,
                         ColorBlack());
    }
#endif
    
}


INTERNAL_FUNCTION void UniBiMappingRangeGraph(f32 t)
{
    f32 RangeRad = 20.0f;
    b32 IsLettersLabels = false;
    b32 ShowSlidersBack = true;
    
    v2 Range1 = V2(0.0f, 1.0f);
    v2 Range2 = V2(-1.0f, 1.0f);
    
    float HorizDim = 0.45f;
    
    char Range1Buf[32];
    stbsp_sprintf(Range1Buf, "X", t);
    
    char Range2Buf[32];
    stbsp_sprintf(Range2Buf, "Y = ?", t);
    
    f32 Range1Value = Lerp(Range1.x, Range1.y, t);
    f32 Range2Value = Lerp(Range2.x, Range2.y, t);
    
    if(!IsLettersLabels)
    {
        stbsp_sprintf(Range1Buf, "X = %.2f", Range1Value);
        stbsp_sprintf(Range2Buf, "Y = %.2f", Range2Value);
    }
    
    f32 Range1Magn = Range1.Max - Range1.Min;
    f32 Range2Magn = Range2.Max - Range2.Min;
    
    // NOTE(Dima): Range1
    ui_slider_graph Graph1 = ValueSlider(Range1Value, 
                                         Range1.x, Range1.y, 
                                         UVToScreenPoint(V2(0.5f + HorizDim * 0.25f, 0.65f)),
                                         UVToScreenPoint(V2(HorizDim * 0.5f, 0.02f)),
                                         Range1Buf,
                                         ShowSlidersBack,
                                         !IsLettersLabels);
    
    v2 R1Min = AddGraphPoint(&Graph1, RangeRad, Range1.Min, ColorRed());
    v2 R1Max = AddGraphPoint(&Graph1, RangeRad, Range1.Max, ColorRed());
    
    // NOTE(Dima): Range2
    ui_slider_graph Graph2 = ValueSlider(Range2Value, 
                                         Range2.x, Range2.y, 
                                         UVToScreenPoint(V2(0.5f, 0.35f)),
                                         UVToScreenPoint(V2(HorizDim, 0.02f)),
                                         Range2Buf,
                                         ShowSlidersBack,
                                         !IsLettersLabels);
    
    v2 R2Min = AddGraphPoint(&Graph2, RangeRad, Range2.Min, ColorBlue());
    v2 R2Max = AddGraphPoint(&Graph2, RangeRad, Range2.Max, ColorBlue());
    
    // NOTE(Dima): Labels
    if(IsLettersLabels)
    {
        AddPointLabel("A",
                      R1Min,
                      180, 30,
                      TextAlign_Right,
                      TextAlign_Center,
                      1.0f);
        
        AddPointLabel("B",
                      R1Max,
                      0, 30,
                      TextAlign_Left,
                      TextAlign_Center,
                      1.0f);
        
        AddPointLabel("C",
                      R2Min,
                      180, 30,
                      TextAlign_Right,
                      TextAlign_Center,
                      1.0f);
        
        AddPointLabel("D",
                      R2Max,
                      0, 30,
                      TextAlign_Left,
                      TextAlign_Center,
                      1.0f);
    }
    
    
#if 0    
    // NOTE(Dima): Misc
    if(Graph1.ValueScreenPCalculated && Graph2.ValueScreenPCalculated)
    {
        PushDashedLine2D(Global_UI->Params.Commands,
                         Graph1.ValueScreenP,
                         Graph2.ValueScreenP,
                         RENDER_DEFAULT_2D_LINE_DASH_LENGTH,
                         RENDER_DEFAULT_2D_LINE_DASH_SPACING,
                         RENDER_DEFAULT_2D_LINE_THICKNESS,
                         ColorBlack());
    }
#endif
    
}