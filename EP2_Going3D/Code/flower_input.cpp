input_system* GlobalInput;

inline b32 GetKeyInternal(key_state* Key)
{
    b32 Result = Key->EndedDown;
    
    return(Result);
}

inline b32 GetKeyUpInternal(key_state* Key)
{
    b32 Result = !Key->EndedDown && Key->TransitionHappened;
    
    return(Result);
}

inline b32 GetKeyDownInternal(key_state* Key)
{
    b32 Result = Key->EndedDown && Key->TransitionHappened;
    
    return(Result);
}

inline b32 GetKey(u32 KeyCode)
{
    b32 Result = GetKeyInternal(&GlobalInput->Keyboard.KeyStates[KeyCode]);
    
    return(Result);
}

inline b32 GetKeyUp(u32 KeyCode)
{
    b32 Result = GetKeyUpInternal(&GlobalInput->Keyboard.KeyStates[KeyCode]);
    
    return(Result);
}

inline b32 GetKeyDown(u32 KeyCode)
{
    b32 Result = GetKeyDownInternal(&GlobalInput->Keyboard.KeyStates[KeyCode]);
    
    return(Result);
}

#define GET_BUTTON_COMMON(name) b32 name(key_state* Key)
typedef GET_BUTTON_COMMON(get_button_common);

inline b32 GetButtonCommon(get_button_common* Func, u32 Button, int ControllerIndex = -1)
{
    b32 Result = false;
    
    // NOTE(Dima): Seconds choice
    if(ControllerIndex == -1)
    {
        for(int ControlIndex = 0; 
            ControlIndex < GlobalInput->ControllerCount;
            ControlIndex++)
        {
            controller* Controller = &GlobalInput->Controllers[ControlIndex];
            
            button_state* But = &Controller->Buttons[Button];
            
            Result = Func(&But->PressState);
            if(Result)
            {
                break;
            }
        }
    }
    else
    {
        if(ControllerIndex < MAX_INPUT_CONTROLLERS)
        {
            controller* Controller = &GlobalInput->Controllers[ControllerIndex];
            
            button_state* But = &Controller->Buttons[Button];
            
            Result = Func(&But->PressState); 
        }
    }
    
    return(Result);
}

inline b32 GetButton(u32 Button, int ControllerIndex = -1)
{
    b32 Result = GetButtonCommon(GetKeyInternal, Button, ControllerIndex);
    
    return(Result);
}

inline b32 GetButtonUp(u32 Button, int ControllerIndex = -1)
{
    b32 Result = GetButtonCommon(GetKeyUpInternal, Button, ControllerIndex);
    
    return(Result);
}

inline b32 GetButtonDown(u32 Button, int ControllerIndex = -1)
{
    b32 Result = GetButtonCommon(GetKeyDownInternal, Button, ControllerIndex);
    
    return(Result);
}

INTERNAL_FUNCTION inline void AddGamepadToController(int ControllerIndex, int GamepadIndex)
{
    controller* Control = &GlobalInput->Controllers[ControllerIndex];
    
    Control->Type = Controller_Gamepad;
    Control->GamepadIndex = GamepadIndex;
}

INTERNAL_FUNCTION inline controller* AddController(u32 Type)
{
    Assert(GlobalInput->ControllerCount < ArrayCount(GlobalInput->Controllers));
    
    controller* Result = &GlobalInput->Controllers[GlobalInput->ControllerCount++];
    
    Result->Type = Type;
    
    return(Result);
}

INTERNAL_FUNCTION inline controller* AddKeyboardController()
{
    controller* Result = AddController(Controller_Keyboard);
    
    return(Result);
}

INTERNAL_FUNCTION inline void AssignKeyOnButton(controller* Controller, u32 Key, u32 Button)
{
    button_state* But = &Controller->Buttons[Button];
    
    Assert(But->KeyCount < MAX_KEYS_PER_BUTTON);
    
    But->Keys[But->KeyCount++] = Key;
}

INTERNAL_FUNCTION inline key_state* GetKeyOnController(controller* Controller, u32 Key)
{
    key_state* Result = 0;
    
    switch(Controller->Type)
    {
        case Controller_Keyboard:
        {
            Result = &GlobalInput->Keyboard.KeyStates[Key];
        } break;
        
        case Controller_Gamepad:
        {
            
        } break;
    }
    
    return(Result);
}

INTERNAL_FUNCTION f32 GetAxis(u32 AxisType, int ControllerIndex = -1)
{
    f32 Result = 0.0f;
    
    switch(AxisType)
    {
        case Axis_Horizontal:
        {
            f32 Left = GetButton(Button_Left, ControllerIndex) ? 1.0f : 0.0f;
            f32 Right = GetButton(Button_Right, ControllerIndex) ? -1.0f : 0.0f;
            
            Result = Left + Right;
        } break;
        
        case Axis_Vertical:
        {
            f32 Up = GetButton(Button_Up, ControllerIndex) ? 1.0f : 0.0f;
            f32 Down = GetButton(Button_Down, ControllerIndex) ? -1.0f : 0.0f;
            
            Result = Up + Down;
        } break;
        
        case Axis_MouseX:
        {
            Result = GlobalInput->MouseDeltaP.x;
        } break;
        
        case Axis_MouseY:
        {
            Result = GlobalInput->MouseDeltaP.y;
        } break;
    }
    
    return(Result);
}

INTERNAL_FUNCTION void InitInput(input_system* Input)
{
    GlobalInput = Input;
    
    Input->CapturingMouse = true;
    Input->MouseDefaultSpeed = 0.1f;
    
    controller* Kb1 = AddKeyboardController();
    AssignKeyOnButton(Kb1, Key_W, Button_Up);
    AssignKeyOnButton(Kb1, Key_A, Button_Left);
    AssignKeyOnButton(Kb1, Key_S, Button_Down);
    AssignKeyOnButton(Kb1, Key_D, Button_Right);
    
    AssignKeyOnButton(Kb1, Key_Up, Button_Up);
    AssignKeyOnButton(Kb1, Key_Left, Button_Left);
    AssignKeyOnButton(Kb1, Key_Down, Button_Down);
    AssignKeyOnButton(Kb1, Key_Right, Button_Right);
    
    
    controller* Kb2 = AddKeyboardController();
    AssignKeyOnButton(Kb2, Key_Up, Button_Up);
    AssignKeyOnButton(Kb2, Key_Left, Button_Left);
    AssignKeyOnButton(Kb2, Key_Down, Button_Down);
    AssignKeyOnButton(Kb2, Key_Right, Button_Right);
}