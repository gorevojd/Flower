// NOTE(Dima): Internal calls
#define GET_BUTTON_COMMON(name) b32 name(key_state* Key)
typedef GET_BUTTON_COMMON(get_button_common);

inline GET_BUTTON_COMMON(GetKeyInternal)
{
    b32 Result = Key->EndedDown;
    
    return(Result);
}

inline GET_BUTTON_COMMON(GetKeyUpInternal)
{
    b32 Result = !Key->EndedDown && Key->TransitionHappened;
    
    return(Result);
}

inline GET_BUTTON_COMMON(GetKeyDownInternal)
{
    b32 Result = Key->EndedDown && Key->TransitionHappened;
    
    return(Result);
}

// NOTE(Dima): Keyboard keys
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

// NOTE(Dima): Gamepad axes
inline f32 GetAxisOnGamepadController(gamepad_controller* Pad, u32 GamepadAxis)
{
    f32 Result = 0.0f;
    
    if(Pad->Connected)
    {
        Result = Pad->Axes[GamepadAxis];
    }
    
    return(Result);
}

inline f32 GetAxisOnGamepad(u32 GamepadAxis, int GamepadIndex = -1)
{
    f32 Result = 0.0f;
    
    if(GamepadIndex == -1)
    {
        for(int PadIndex = 0;
            PadIndex < MAX_GAMEPADS;
            PadIndex++)
        {
            gamepad_controller* Gamepad = &GlobalInput->Gamepads[PadIndex];
            
            Result = GetAxisOnGamepadController(Gamepad, GamepadAxis);
            
            if(std::abs(Result) > 0.00001f)
            {
                break;
            }
        }
    }
    else
    {
        gamepad_controller* Gamepad = &GlobalInput->Gamepads[GamepadIndex];
        
        Result = GetAxisOnGamepadController(Gamepad, GamepadAxis);
    }
    
    return(Result);
}

// NOTE(Dima): Gamepad keys
inline b32 GetOnGamepadCommon(get_button_common* Func, u32 GamepadButton, int GamepadIndex)
{
    b32 Result = false;
    
    if(GamepadIndex == -1)
    {
        for(int PadIndex = 0;
            PadIndex < MAX_GAMEPADS;
            PadIndex++)
        {
            gamepad_controller* Gamepad = &GlobalInput->Gamepads[PadIndex];
            
            if(Gamepad->Connected)
            {
                Result = Func(&Gamepad->Buttons[GamepadButton].PressState);
                
                if(Result)
                {
                    break;
                }
            }
        }
    }
    else
    {
        if(GamepadIndex >= 0 && 
           GamepadIndex < MAX_GAMEPADS)
        {
            gamepad_controller* Gamepad = &GlobalInput->Gamepads[GamepadIndex];
            
            if(Gamepad->Connected)
            {
                Result = Func(&Gamepad->Buttons[GamepadButton].PressState);
            }
        }
    }
    
    return(Result);
}

inline b32 GetButtonOnGamepad(u32 GamepadButton, int GamepadIndex = -1)
{
    b32 Result = GetOnGamepadCommon(GetKeyInternal, GamepadButton, GamepadIndex);
    
    return(Result);
}

inline b32 GetButtonDownOnGamepad(u32 GamepadButton, int GamepadIndex = -1)
{
    b32 Result = GetOnGamepadCommon(GetKeyDownInternal, GamepadButton, GamepadIndex);
    
    return(Result);
}

inline b32 GetButtonUpOnGamepad(u32 GamepadButton, int GamepadIndex = -1)
{
    b32 Result = GetOnGamepadCommon(GetKeyUpInternal, GamepadButton, GamepadIndex);
    
    return(Result);
}

// NOTE(Dima): Get virtual buttons functions
inline b32 GetButtonCommon(get_button_common* Func, u32 Button, int ControllerIndex = -1)
{
    b32 Result = false;
    
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
        if(ControllerIndex >= 0 && 
           ControllerIndex < GlobalInput->ControllerCount)
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


INTERNAL_FUNCTION inline void AssignKeyOnButton(controller* Controller, u32 Key, u32 Button)
{
    button_state* But = &Controller->Buttons[Button];
    
    Assert(But->KeyCount < MAX_KEYS_PER_BUTTON);
    
    But->Keys[But->KeyCount++] = Key;
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
    Result->GamepadIndex = -1;
    
    return(Result);
}

INTERNAL_FUNCTION inline void AddGamepadController(int PadIndex, char* Name)
{
    controller* NewControl = AddController(Controller_Gamepad);
    NewControl->GamepadIndex = PadIndex;
    
    gamepad_controller* Pad = &GlobalInput->Gamepads[PadIndex];
    
    CopyStringsSafe(Pad->Name, sizeof(Pad->Name), Name);
    
    Pad->Connected = true;
    
    AssignKeyOnButton(NewControl, GamepadButton_DpadUp, Button_Up);
    AssignKeyOnButton(NewControl, GamepadButton_DpadDown, Button_Down);
    AssignKeyOnButton(NewControl, GamepadButton_DpadLeft, Button_Left);
    AssignKeyOnButton(NewControl, GamepadButton_DpadRight, Button_Right);
}

INTERNAL_FUNCTION inline void DisconnectGamepadController(int PadIndex)
{
    gamepad_controller* Pad = &GlobalInput->Gamepads[PadIndex];
    
    Pad->Connected = false;
}

// NOTE(Dima): Returns index of returned gamepad or -1 if failed
INTERNAL_FUNCTION int TryAddGamepadController(char* Name)
{
    int Result = -1;
    
    for(int GamepadIndex = 0;
        GamepadIndex < MAX_GAMEPADS;
        GamepadIndex++)
    {
        gamepad_controller* Pad = &GlobalInput->Gamepads[GamepadIndex];
        
        if(Pad->Connected == false)
        {
            Result = GamepadIndex;
            
            AddGamepadController(Result, Name);
            
            break;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline controller* AddKeyboardController()
{
    controller* Result = AddController(Controller_Keyboard);
    
    return(Result);
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
            gamepad_controller* Pad = &GlobalInput->Gamepads[Controller->GamepadIndex];
            if(Pad->Connected)
            {
                gamepad_button* PadBut = &Pad->Buttons[Key];
                Result = &PadBut->PressState;
            }
            
        } break;
    }
    
    return(Result);
}

INTERNAL_FUNCTION f32 GetAxis(u32 AxisVector, int ControllerIndex = -1)
{
    f32 Result = 0.0f;
    
    int GamepadIndex = -1;
    if(ControllerIndex != -1)
    {
        if(ControllerIndex >= 0 &&
           ControllerIndex < GlobalInput->ControllerCount)
        {
            controller* Control = &GlobalInput->Controllers[ControllerIndex];
            
            if(Control->Type == Controller_Gamepad)
            {
                GamepadIndex = Control->GamepadIndex;
            }
        }
    }
    
    f32 GamepadAxis = 0.0f;
    switch(AxisVector)
    {
        case Axis_Horizontal:
        {
            GamepadAxis = GetAxisOnGamepad(GamepadAxis_LeftX, GamepadIndex);
            
            f32 Left = GetButton(Button_Left, ControllerIndex) ? 1.0f : 0.0f;
            f32 Right = GetButton(Button_Right, ControllerIndex) ? -1.0f : 0.0f;
            
            Result = Clamp(Left + Right + GamepadAxis, -1.0f, 1.0f);
        } break;
        
        case Axis_Vertical:
        {
            GamepadAxis = GetAxisOnGamepad(GamepadAxis_LeftY, GamepadIndex);
            
            f32 Up = GetButton(Button_Up, ControllerIndex) ? 1.0f : 0.0f;
            f32 Down = GetButton(Button_Down, ControllerIndex) ? -1.0f : 0.0f;
            
            Result = Clamp(Up + Down + GamepadAxis, -1.0f, 1.0f);
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

INTERNAL_FUNCTION void InitInput(memory_arena* Arena)
{
    GlobalInput = PushStruct(Arena, input_system);
    
    GlobalInput->CapturingMouse = true;
    GlobalInput->MouseDefaultSpeed = 0.1f;
    
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