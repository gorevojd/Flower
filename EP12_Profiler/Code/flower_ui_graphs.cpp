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
                                  ColorBlack(), false);
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
    
    UIPushScale(1.1f);
    
    PrintTextAligned(ABuf, APos, 
                     TextAlign_Center,
                     TextAlign_Center,
                     ColorGreen());
    
    PrintTextAligned(BBuf, BPos, 
                     TextAlign_Center,
                     TextAlign_Center,
                     ColorBlue());
    
    UIPopScale();
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
    
#if 0    
    if(BeginLayout())
    {
        ShowTextUnformatted(HorizontalBuffer);
        ShowTextUnformatted(VerticalBuffer);
        ShowTextUnformatted("Left");
        ShowTextUnformatted("Right");
        ShowTextUnformatted("Up");
        ShowTextUnformatted("Down");
        
        EndLayout();
    }
#endif
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

INTERNAL_FUNCTION void RandomNumberGraph()
{
    PushClear(V3(0.1f));
    
    ui_params InitParams = Global_UI->Params;
    Global_UI->Params.Font = &Global_Assets->BerlinSans;
    
    char* HeaderText = "Random bilateral [-1, 1] float";
    
    v2 HeaderTextP = UVToScreenPoint(0.5f, 0.1f);
    
    random_generation Random = SeedRandom(1232);
    
    // NOTE(Dima): Printing header text
    PrintTextAligned(HeaderText, 
                     HeaderTextP,
                     TextAlign_Center,
                     TextAlign_Center,
                     V4(1.0f, 1.0f, 1.0f, 1.0f),
                     false);
    
    int ColumnsCount = 5;
    int RowsCount = 4;
    
    f32 StartUVx = 0.1f;
    f32 EndUVx = 0.9f;
    f32 StartUVy = 0.25f;
    f32 EndUVy = 0.9f;
    
    f32 StepUVx = (EndUVx - StartUVx) / (f32)(ColumnsCount - 1);
    f32 StepUVy = (EndUVy - StartUVy) / (f32)(RowsCount - 1);
    
    // NOTE(Dima): Printing random numbers
    for(int y = 0; y < RowsCount; y++)
    {
        for(int x = 0; x < ColumnsCount; x++)
        {
            v2 NumP = UVToScreenPoint(StartUVx + (f32)x * StepUVx,
                                      StartUVy + (f32)y * StepUVy);
            
            char NumText[64];
            
            f32 RandomNum = RandomBilateral(&Random);
            
            stbsp_sprintf(NumText, "%.4f", RandomNum);
            
            // NOTE(Dima): Printing header text
            Global_UI->Params.Scale = 0.8f;
            PrintTextAligned(NumText, 
                             NumP,
                             TextAlign_Center,
                             TextAlign_Center,
                             V4(1.0f, 1.0f, 1.0f, 1.0f),
                             false);
        }
    }
    
    Global_UI->Params = InitParams;
}

INTERNAL_FUNCTION void GraphDisplaySamples(v2* Samples, 
                                           int SampleCount,
                                           f32 Radius = 4.0f)
{
    // NOTE(Dima): Displaying samples
    for(int SampleIndex = 0;
        SampleIndex < SampleCount;
        SampleIndex++)
    {
        v2 SampleP = Samples[SampleIndex];
        
        int SegmentsCount = 8;
        
        PushCircle2D(SampleP, 
                     Radius, ColorGreen(),
                     SegmentsCount);
    }
}

INTERNAL_FUNCTION rc2 GraphShowQuad(f32* OutPixelHeight)
{
    // NOTE(Dima): Drawing quad on screen
    window_dimensions Dims = Global_RenderCommands->WindowDimensions;
    f32 AspectRatio = Dims.Width / Dims.Height;
    
    f32 UVHeight = 0.8f;
    f32 PixelHeight = UVToScreenPoint(0.0f, UVHeight).y;
    f32 HalfDim = PixelHeight * 2.0f;
    
    v2 GraphCenter = UVToScreenPoint(0.52f, 0.5f);
    v2 GraphDim = V2(PixelHeight, PixelHeight);
    
    // NOTE(Dima): Pushing rect
    rc2 GraphRect = RectCenterDim(GraphCenter, GraphDim);
    PushRectOutline(GraphRect, 5, ColorWhite());
    
    if(OutPixelHeight)
    {
        *OutPixelHeight = PixelHeight;
    }
    
    return(GraphRect);
}

INTERNAL_FUNCTION void GraphNoiseSamples(memory_arena* Arena,
                                         v2* Samples, int Count)
{
    PushClear(V3(0.1f));
    
    f32 PixelHeight = 0;
    rc2 GraphRect = GraphShowQuad(&PixelHeight);
    
    static v2* DisplaySamples = 0;
    static int CurCount = Count;
    static b32 ShowGrid = true;
    
    static f32 ShowStart = 999999.0f;
    
    int GridDim = 10;
    if(ShowGrid)
    {
        f32 LineThick = 3.0f;
        v4 LineColor = ColorGray(0.5f);
        
        for(int x = 1; x < GridDim; x++)
        {
            f32 tx = (f32)x / (f32)(GridDim);
            
            f32 CoordX = Lerp(GraphRect.Min.x, GraphRect.Max.x, tx);
            f32 CoordY = Lerp(GraphRect.Min.y, GraphRect.Max.y, tx);
            
            PushLine2D(V2(CoordX, GraphRect.Min.y), 
                       V2(CoordX, GraphRect.Max.y),
                       LineThick, LineColor);
            
            PushLine2D(V2(GraphRect.Min.x, CoordY),
                       V2(GraphRect.Max.x, CoordY),
                       LineThick, LineColor);
        }
    }
    
    if(GetKeyDown(Key_Space))
    {
        ShowStart = Global_Time->Time;
    }
    
    f32 Delay = 0.16f;
    int CountToShow = Clamp((Global_Time->Time - ShowStart) / Delay, 0, Count);
    
    if(!DisplaySamples)
    {
        DisplaySamples = PushArray(Arena, v2, Count);
        
        for(int i = 0; i < Count; i++)
        {
            DisplaySamples[i] = V2(Lerp(GraphRect.Min.x, GraphRect.Max.x, Samples[i].x),
                                   Lerp(GraphRect.Min.y, GraphRect.Max.y, Samples[i].y));
        }
    }
    
    int Mode = 1;
    if(Mode == 0)
    {
        GraphDisplaySamples(DisplaySamples, CountToShow, 6);
    }
    else if(Mode == 1)
    {
        int CheckSampleCount = 5;
        GraphDisplaySamples(DisplaySamples, CheckSampleCount, 6);
        
        int InspectX = 5;
        int InspectY = 3;
        
        f32 OneQuadieLen = PixelHeight / (f32)GridDim;
        rc2 QuadieRect = RectMinDim(GraphRect.Min + V2(InspectX, InspectY) * OneQuadieLen, V2(OneQuadieLen));
        rc2 GrewRect = GrowRect(QuadieRect, 4.0f);
        
        PushRect(GrewRect, V4(1.0f, 0.5f, 0.0f, 0.2f));
        PushRectOutline(QuadieRect, 4.0f, ColorYellow());
        
        v2 SampleP = ClampInRect(Global_Input->MouseWindowP, QuadieRect);
        
        v4 ColorCheckCircle = V4(0.0f, 1.0f, 0.0f, 0.3f);
        
        f32 PixelR = OneQuadieLen * F_SQRT_TWO;
        for(int i = 0; i < CheckSampleCount; i++)
        {
            v2 Diff = SampleP - DisplaySamples[i];
            
            if(SqMagnitude(Diff) < PixelR * PixelR)
            {
                ColorCheckCircle = V4(1.0f, 0.0f, 0.0f, 0.3f);
                
                break;
            }
        }
        
        PushCircle2D(SampleP, PixelR, 
                     ColorCheckCircle,
                     32);
        
        // NOTE(Dima): Pushing sample
        PushCircle2D(SampleP, 6, ColorGreen(), 10);
    }
}

enum graph_quad_type
{
    GraphQuad_Simple,
    GraphQuad_Grid,
    GraphQuad_NaivePoiss,
};

INTERNAL_FUNCTION void GraphSamplesInQuad(memory_arena* Arena)
{
    PushClear(V3(0.1f));
    
    u32 Type = GraphQuad_Simple;
    
    f32 PixelHeight = 0;
    rc2 GraphRect = GraphShowQuad(&PixelHeight);
    
    // NOTE(Dima): Samples
    int SampleCount = 200;
    f32 SpawnDelay = 0.05f;
    int GridDim = std::roundf(std::sqrt(SampleCount));
    
    static random_generation Random = SeedRandom(123);
    static b32 ShowGrid = true;
    static rc2 LastQuadieRect = RectMinDim(V2(-100), V2(0.0f));
    static f32 NextSpawnTime = 99999.0f;
    
    static b32 Initialized = false;
    static v2* Samples = 0;
    static int CurSampleCount = 0;
    
    if(!Initialized)
    {
        Samples = PushArray(Arena, v2, SampleCount);
        
        Initialized = true;
    }
    
    if(GetKeyDown(Key_G))
    {
        ShowGrid = !ShowGrid;
    }
    
    if(ShowGrid)
    {
        f32 LineThick = 3.0f;
        v4 LineColor = ColorGray(0.5f);
        
        for(int x = 1; x < GridDim; x++)
        {
            f32 tx = (f32)x / (f32)(GridDim);
            
            f32 CoordX = Lerp(GraphRect.Min.x, GraphRect.Max.x, tx);
            f32 CoordY = Lerp(GraphRect.Min.y, GraphRect.Max.y, tx);
            
            PushLine2D(V2(CoordX, GraphRect.Min.y), 
                       V2(CoordX, GraphRect.Max.y),
                       LineThick, LineColor);
            
            PushLine2D(V2(GraphRect.Min.x, CoordY),
                       V2(GraphRect.Max.x, CoordY),
                       LineThick, LineColor);
        }
        
        PushRectOutline(LastQuadieRect, 4.0f, ColorYellow());
    }
    
    if(GetKeyDown(Key_Space))
    {
        NextSpawnTime = Global_Time->Time + SpawnDelay;
    }
    
    struct graph_quad_arrow
    {
        v2 Begin;
        v2 End;
        b32 Failed;
    };
    
    static std::vector<graph_quad_arrow> Arrows;
    static v2 LastSample = V2(-10);;
    
    if(Initialized)
    {
        if((Global_Time->Time > NextSpawnTime) && (CurSampleCount < SampleCount))
        {
            f32 AdditionalTimeInc = 0.0f;
            
            // NOTE(Dima): Generating sample if needed
            switch(Type)
            {
                case GraphQuad_NaivePoiss:
                {
                    Arrows.clear();
                    
                    v2 InUV = RandomInUV(&Random);
                    v2 Sample = V2(Lerp(GraphRect.Min.x, GraphRect.Max.x, InUV.x),
                                   Lerp(GraphRect.Min.y, GraphRect.Max.y, InUV.y));
                    LastSample = Sample;
                    
                    
                    f32 MinDist = 40.0f;
                    b32 ShouldInsert = true;
                    for(int i = 0; i < CurSampleCount; i++)
                    {
                        v2 End = Samples[i];
                        
                        graph_quad_arrow Arrow = {};
                        Arrow.Begin = Sample;
                        Arrow.End = End;
                        Arrow.Failed = false;
                        
                        v2 Offset = End - Sample;
                        
                        if(SqMagnitude(Offset) < MinDist * MinDist)
                        {
                            ShouldInsert = false;
                            
                            Arrow.Failed = true;
                            AdditionalTimeInc = 1.0f;
                        }
                        
                        Arrows.push_back(Arrow);
                    }
                    
                    if(ShouldInsert)
                    {
                        Samples[CurSampleCount++] = Sample;
                    }
                }break;
                
                case GraphQuad_Simple:
                {
                    v2 InUV = RandomInUV(&Random);
                    
                    v2 Sample = V2(Lerp(GraphRect.Min.x, GraphRect.Max.x, InUV.x),
                                   Lerp(GraphRect.Min.y, GraphRect.Max.y, InUV.y));
                    
                    Samples[CurSampleCount++] = Sample;
                }break;
                
                case GraphQuad_Grid:
                {
                    int x = CurSampleCount % GridDim;
                    int y = CurSampleCount / GridDim;
                    
                    f32 OneQuadieLen = PixelHeight / (f32)GridDim;
                    rc2 QuadieRect = RectMinDim(GraphRect.Min + V2(x, y) * OneQuadieLen, V2(OneQuadieLen));
                    
                    LastQuadieRect = QuadieRect;
                    
                    v2 InUV = RandomInUV(&Random);
                    
                    Samples[CurSampleCount++] = V2(Lerp(QuadieRect.Min.x, QuadieRect.Max.x, InUV.x),
                                                   Lerp(QuadieRect.Min.y, QuadieRect.Max.y, InUV.y));
                }break;
            }
            
            NextSpawnTime = Global_Time->Time + SpawnDelay + AdditionalTimeInc;
        }
        
        // NOTE(Dima): Success arrows
        for(auto& Arrow : Arrows)
        {
            v4 SuccessColor = V4(0.1f, 0.7f, 0.1f, 1.0f);
            v4 FailColor = V4(0.9f, 0.1f, 0.1f, 1.0f);
            
            if(!Arrow.Failed)
            {
                PushArrow2D(Arrow.Begin, Arrow.End, 2.0f, SuccessColor);
            }
        }
        
        // NOTE(Dima): Fail arrows
        for(auto& Arrow : Arrows)
        {
            v4 SuccessColor = V4(0.1f, 0.7f, 0.1f, 1.0f);
            v4 FailColor = V4(0.9f, 0.1f, 0.1f, 1.0f);
            
            if(Arrow.Failed)
            {
                PushArrow2D(Arrow.Begin, Arrow.End, 4.0f, FailColor);
            }
        }
        
        GraphDisplaySamples(Samples, CurSampleCount);
        
        PushCircle2D(LastSample, 7, ColorYellow(), 16);
    }
}

INTERNAL_FUNCTION void GraphSamplesInCircle(memory_arena* Arena, f32 SpawnDelay = 0.1f)
{
    PushClear(V3(0.1f));
    
    f32 PixelRadius = UVToScreenPoint(0.0f, 0.45f).y;
    v2 CenterP = UVToScreenPoint(0.5f, 0.5f);
    
    PushCircleOutline2D(CenterP,
                        PixelRadius,
                        5.0f,
                        ColorWhite(),
                        64);
    
    static b32 Initialized = false;
    static v2* Samples = 0;
    static int CurSampleCount = 0;
    static f32 NextSpawnTime = 99999.0f;
    static random_generation Random = SeedRandom(123);
    
    int SampleCount = 400;
    
    if(!Initialized)
    {
        Samples = PushArray(Arena, v2, SampleCount);
        
        Initialized = true;
    }
    
    if(GetKeyDown(Key_Space))
    {
        NextSpawnTime = Global_Time->Time + SpawnDelay;
    }
    
    if(Initialized)
    {
        if((Global_Time->Time > NextSpawnTime) && (CurSampleCount < SampleCount))
        {
            Samples[CurSampleCount++] = CenterP + RandomInUnitCircle(&Random) * PixelRadius;
            
            NextSpawnTime = Global_Time->Time + SpawnDelay;
        }
        
        GraphDisplaySamples(Samples, CurSampleCount);
    }
}

INTERNAL_FUNCTION void GraphRadioactiveSource()
{
    struct radioactive_particle
    {
        v2 Direction;
        v2 P;
        f32 Radius;
        b32 IsAlive;
    };
    
    static radioactive_particle Particles[64];
    static int CurParticle;
    static f32 NextSpawnTime = 0.0f;
    static random_generation Random = SeedRandom(123);
    
    f32 SpawnDelayMin = 0.05f;
    f32 SpawnDelayMax = 0.5f;
    
    PushClear(V3(0.1f));
    
    f32 PixelRadius = UVToScreenPoint(0.0f, 0.25f).y;
    v2 CenterP = UVToScreenPoint(0.5f, 0.5f);
    
    v4 ParticleInC = V4(0.8f, 0.2f, 0.1f, 1.0f);
    v4 ParticleOutC = V4(0.2f, 0.1f, 0.75f, 1.0f);
    
    if(Global_Time->Time > NextSpawnTime)
    {
        radioactive_particle* Particle = &Particles[CurParticle];
        
        Particle->IsAlive = true;
        Particle->Radius = RandomBetweenFloats(&Random, 
                                               PixelRadius * 0.1f, 
                                               PixelRadius * 0.25f);
        Particle->Direction = RandomOnUnitCircle(&Random);
        Particle->P = CenterP;
        
        CurParticle++;
        if(CurParticle >= ArrayCount(Particles))
        {
            CurParticle = 0;
        }
        NextSpawnTime = Global_Time->Time + RandomBetweenFloats(&Random, 
                                                                SpawnDelayMin, 
                                                                SpawnDelayMax);
    }
    
    // NOTE(Dima): Drawing particles;
    for(int ParticleIndex = 0;
        ParticleIndex < ArrayCount(Particles);
        ParticleIndex++)
    {
        radioactive_particle* Particle = &Particles[ParticleIndex];
        
        if(Particle->IsAlive)
        {
            Particle->P += Particle->Direction * Global_Time->DeltaTime * 200.0f;
            
            // NOTE(Dima): Drawing nucleous
            PushOutlinedCircle2D(Particle->P, 
                                 Particle->Radius,
                                 4.0f,
                                 ParticleInC,
                                 ParticleOutC,
                                 32);
        }
    }
    
    // NOTE(Dima): Drawing nucleous
    PushOutlinedCircle2D(CenterP, 
                         PixelRadius,
                         6.0f,
                         ParticleInC,
                         ParticleOutC,
                         36);
    
}

INTERNAL_FUNCTION void GraphCoinsThrow(memory_arena* Arena, 
                                       image* CoinHead,
                                       image* CoinTail)
{
    PushClear(V3(0.1f));
    
    ui_params InitParams = Global_UI->Params;
    Global_UI->Params.Font = &Global_Assets->BerlinSans;
    
    static int HeadsCount;
    static int TailsCount;
    static random_generation Random = SeedRandom(1231);
    static v2* CoinsP = 0;
    static b32* IsHead = 0;
    static f32* LifeTime = 0;
    static int CoinsCount = 0;
    static f32 NextSpawnTime = 999999.0f;
    f32 ThrowDelay = 0.07f;
    
    char HeadsText[128];
    stbsp_sprintf(HeadsText, "Heads: %d", HeadsCount);
    
    char TailsText[128];
    stbsp_sprintf(TailsText, "Tails: %d", TailsCount);
    
    int HeadsPerc = 0;
    int TailsPerc = 0;
    
    if(HeadsCount + TailsCount)
    {
        f32 Sum = HeadsCount + TailsCount;
        HeadsPerc = std::roundf((f32)HeadsCount / Sum * 100.0f);
        TailsPerc = std::roundf((f32)TailsCount / Sum * 100.0f);
    }
    
    char RatioText[128];
    stbsp_sprintf(RatioText, "Ratio %%: %d/%d",
                  HeadsPerc, TailsPerc);
    
    f32 HeaderTextUVy = 0.1f;
    f32 TextScale = 1.0f;
    v4 TextColor = V4(1.0f, 1.0f, 1.0f, 1.0f);
    v2 HeadsTextP = UVToScreenPoint(0.2f, HeaderTextUVy);
    v2 TailsTextP = UVToScreenPoint(0.4f, HeaderTextUVy);
    v2 PercTextP = UVToScreenPoint(0.6f, HeaderTextUVy);
    
    // NOTE(Dima): Printing headers text
    PrintTextAligned(HeadsText, 
                     HeadsTextP,
                     TextAlign_Left,
                     TextAlign_Center,
                     TextColor,
                     false);
    
    PrintTextAligned(TailsText, 
                     TailsTextP,
                     TextAlign_Left,
                     TextAlign_Center,
                     TextColor,
                     false);
    
    PrintTextAligned(RatioText, 
                     PercTextP,
                     TextAlign_Left,
                     TextAlign_Center,
                     TextColor,
                     false);
    
    // NOTE(Dima): 
    
    int ColumnsCount = 20;
    int RowsCount = 12;
    
    int MaxCoinCount = ColumnsCount * RowsCount;
    
    f32 StartUVx = 0.1f;
    f32 EndUVx = 0.9f;
    f32 StartUVy = 0.25f;
    f32 EndUVy = 0.9f;
    
    f32 StepUVx = (EndUVx - StartUVx) / (f32)(ColumnsCount - 1);
    f32 StepUVy = (EndUVy - StartUVy) / (f32)(RowsCount - 1);
    f32 ScaleImage = 1.0f;
    
    if(!CoinsP)
    {
        CoinsP = PushArray(Arena, v2, MaxCoinCount);
    }
    
    if(!IsHead)
    {
        IsHead = PushArray(Arena, b32, MaxCoinCount);
    }
    
    if(!LifeTime)
    {
        LifeTime = PushArray(Arena, f32, MaxCoinCount);
    }
    
    if(GetKeyDown(Key_Space))
    {
        NextSpawnTime = Global_Time->Time + ThrowDelay;
    }
    
    if((Global_Time->Time > NextSpawnTime) && (CoinsCount < MaxCoinCount))
    {
        int x = CoinsCount % ColumnsCount;
        int y = CoinsCount / ColumnsCount;
        
        CoinsP[CoinsCount] = UVToScreenPoint(StartUVx + (f32)x * StepUVx,
                                             StartUVy + (f32)y * StepUVy);
        
        b32 ThisIsHead = RandomBool(&Random);
        IsHead[CoinsCount] = ThisIsHead;
        
        if(ThisIsHead)
        {
            HeadsCount++;
        }
        else
        {
            TailsCount++;
        }
        
        LifeTime[CoinsCount] = 0.0f;
        
        CoinsCount++;
        NextSpawnTime = Global_Time->Time + ThrowDelay;
    }
    
    // NOTE(Dima): Drawing coins
    for(int CoinIndex = 0;
        CoinIndex < CoinsCount;
        CoinIndex++)
    {
        v2 CoinP = CoinsP[CoinIndex];
        
        image* ToPush = CoinTail;
        if(IsHead[CoinIndex])
        {
            ToPush = CoinHead;
        }
        
        f32 Alpha = Clamp01(LifeTime[CoinIndex] / 0.5f);
        
        f32 ImagePixelHeight = UVToScreenPoint(0.0f, StepUVy).y * 0.9f;
        PushCenteredImage(ToPush, CoinP, 
                          ImagePixelHeight,
                          V4(1.0f, 1.0f, 1.0f, Alpha));
        
        LifeTime[CoinIndex] += Global_Time->DeltaTime;
    }
    
    Global_UI->Params = InitParams;
}