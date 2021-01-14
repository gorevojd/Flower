struct mouse_cheese_state
{
    
};

SCENE_INIT(MouseCheese)
{
    mouse_cheese_state* State = GetSceneState(mouse_cheese_state);
    
    
}

b32 MouseButton(v2 P, v2 Dim)
{
    b32 Result = false;
    
    rc2 ButtonRect = RectCenterDim(P, Dim);
    v4 RectColor = ColorGray(0.76f);
    const char* ButtonText = "Go to me!";
    if(MouseInRect(ButtonRect))
    {
        ButtonText = "Click Me!";
        RectColor = ColorOrange();
        
        if(GetKeyDown(KeyMouse_Left))
        {
            Result = true;
        }
    }
    
    PushRect(ButtonRect, RectColor);
    PrintTextAligned((char*)ButtonText, 
                     P,
                     TextAlign_Center,
                     TextAlign_Center, 
                     3.0f,
                     ColorWhite(), 
                     true);
    
    PushRectOutline(ButtonRect, 3, ColorBlack());
    
    return(Result);
}

SCENE_UPDATE(MouseCheese)
{
    mouse_cheese_state* State = GetSceneState(mouse_cheese_state);
    
    image* Mouse = &Global_Assets->Mouse;
    image* Cheese = &Global_Assets->Cheese;
    
    v2 MouseP = Global_Input->MouseWindowP;
    LOCAL_PERSIST v2 CheeseP = V2(0.0f, 0.0f);
    
    f32 CheeseHeight = 100.0f;
    rc2 CheeseRect = RectCenterDim(CheeseP, 
                                   V2(Cheese->WidthOverHeight * CheeseHeight, CheeseHeight));
    
    v2 ButtonDimension = UVToScreenPoint(0.4f, 0.24f);
    
    LOCAL_PERSIST b32 CheeseIsVisible = false;
    LOCAL_PERSIST b32 MouseEatCheese = false;
    LOCAL_PERSIST b32 EatCount = 0;
    
    const char* Phrases[] = 
    {
        "Omnomnom!!!",
        "So tasty!",
        "I will give this one to my children",
        "Thank you!",
        "So delicious",
    };
    
    v2 CheesePositions[] = {
        UVToScreenPoint(0.1f, 0.8f),
        UVToScreenPoint(0.3f, 0.8f),
        UVToScreenPoint(0.5f, 0.8f),
        UVToScreenPoint(0.7f, 0.8f),
        UVToScreenPoint(0.9f, 0.8f),
    };
    
    // NOTE(Dima): COpying button 3 times
    if(MouseButton(UVToScreenPoint(0.5f, 0.2f),
                   ButtonDimension))
    {
        CheeseIsVisible = true;
        MouseEatCheese = false;
        
        CheeseP = CheesePositions[EatCount % ArrayCount(CheesePositions)];
    }
    
    
#if 0    
    if(MouseButton(UVToScreenPoint(0.5f, 0.5f),
                   ButtonDimension))
    {
        CheeseIsVisible = true;
        MouseEatCheese = false;
        
        CheeseP = CheesePositions[EatCount % ArrayCount(CheesePositions)];
    }
    
    
    if(MouseButton(UVToScreenPoint(0.5f, 0.8f),
                   ButtonDimension))
    {
        CheeseIsVisible = true;
        MouseEatCheese = false;
        
        CheeseP = CheesePositions[EatCount % ArrayCount(CheesePositions)];
    }
#endif
    
    // NOTE(Dima): Processing cheese
    if(CheeseIsVisible)
    {
        if(MouseInRect(CheeseRect) && !MouseEatCheese)
        {
            MouseEatCheese = true;
            EatCount++;
        }
        
        if(!MouseEatCheese)
        {
            PushCenteredImage(Cheese, CheeseP, CheeseHeight);
        }
    }
    
    if(MouseEatCheese)
    {
        char* LabelText = (char*)Phrases[EatCount % ArrayCount(Phrases)];
        
        AddPointLabel(LabelText,
                      MouseP,
                      270,
                      100,
                      TextAlign_Center,
                      TextAlign_Bottom,
                      1.5f);
    }
    
    PushCenteredImage(Mouse, MouseP, 200);
}