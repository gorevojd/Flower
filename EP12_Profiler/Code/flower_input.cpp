// NOTE(Dima): MOuse stuff
inline b32 MouseInRect(rc2 Rect)
{
    b32 Result = PointInRect(Global_Input->MouseWindowP, Rect);
    
    return(Result);
}

inline v2 GetMouseP()
{
    v2 Result = Global_Input->MouseWindowP;
    
    return(Result);
}

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
    b32 Result = GetKeyInternal(&Global_Input->Keyboard.KeyStates[KeyCode]);
    
    return(Result);
}

inline b32 GetKeyUp(u32 KeyCode)
{
    b32 Result = GetKeyUpInternal(&Global_Input->Keyboard.KeyStates[KeyCode]);
    
    return(Result);
}

inline b32 GetKeyDown(u32 KeyCode)
{
    b32 Result = GetKeyDownInternal(&Global_Input->Keyboard.KeyStates[KeyCode]);
    
    return(Result);
}

inline b32 GetMod(u32 Mod)
{
    b32 Result = (Global_Input->Keyboard.Mods & Mod) != 0;
    
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
            gamepad_controller* Gamepad = &Global_Input->Gamepads[PadIndex];
            
            Result = GetAxisOnGamepadController(Gamepad, GamepadAxis);
            
            if(std::abs(Result) > 0.00001f)
            {
                break;
            }
        }
    }
    else
    {
        gamepad_controller* Gamepad = &Global_Input->Gamepads[GamepadIndex];
        
        Result = GetAxisOnGamepadController(Gamepad, GamepadAxis);
    }
    
    return(Result);
}

// NOTE(Dima): Gamepad keys
inline b32 GetOnGamepadCommon(get_button_common* Func, u32 GamepadKey, int GamepadIndex)
{
    b32 Result = false;
    
    if(GamepadIndex == -1)
    {
        for(int PadIndex = 0;
            PadIndex < MAX_GAMEPADS;
            PadIndex++)
        {
            gamepad_controller* Gamepad = &Global_Input->Gamepads[PadIndex];
            
            if(Gamepad->Connected)
            {
                Result = Func(&Gamepad->Keys[GamepadKey].PressState);
                
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
            gamepad_controller* Gamepad = &Global_Input->Gamepads[GamepadIndex];
            
            if(Gamepad->Connected)
            {
                Result = Func(&Gamepad->Keys[GamepadKey].PressState);
            }
        }
    }
    
    return(Result);
}

inline b32 GetKeyOnGamepad(u32 GamepadKey, int GamepadIndex = -1)
{
    b32 Result = GetOnGamepadCommon(GetKeyInternal, GamepadKey, GamepadIndex);
    
    return(Result);
}

inline b32 GetKeyDownOnGamepad(u32 GamepadKey, int GamepadIndex = -1)
{
    b32 Result = GetOnGamepadCommon(GetKeyDownInternal, GamepadKey, GamepadIndex);
    
    return(Result);
}

inline b32 GetKeyUpOnGamepad(u32 GamepadKey, int GamepadIndex = -1)
{
    b32 Result = GetOnGamepadCommon(GetKeyUpInternal, GamepadKey, GamepadIndex);
    
    return(Result);
}

// NOTE(Dima): Get virtual buttons functions
inline b32 GetButtonCommon(get_button_common* Func, u32 Button, int ControllerIndex = -1)
{
    b32 Result = false;
    
    if(ControllerIndex == -1)
    {
        for(int ControlIndex = 0; 
            ControlIndex < Global_Input->ControllerCount;
            ControlIndex++)
        {
            controller* Controller = &Global_Input->Controllers[ControlIndex];
            
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
           ControllerIndex < Global_Input->ControllerCount)
        {
            controller* Controller = &Global_Input->Controllers[ControllerIndex];
            
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
    controller* Control = &Global_Input->Controllers[ControllerIndex];
    
    Control->Type = Controller_Gamepad;
    Control->GamepadIndex = GamepadIndex;
}

INTERNAL_FUNCTION inline controller* AddController(u32 Type)
{
    Assert(Global_Input->ControllerCount < ArrayCount(Global_Input->Controllers));
    
    controller* Result = &Global_Input->Controllers[Global_Input->ControllerCount++];
    
    Result->Type = Type;
    Result->GamepadIndex = -1;
    
    return(Result);
}

INTERNAL_FUNCTION inline void AddGamepadController(int PadIndex, char* Name)
{
    controller* NewControl = AddController(Controller_Gamepad);
    NewControl->GamepadIndex = PadIndex;
    
    gamepad_controller* Pad = &Global_Input->Gamepads[PadIndex];
    
    CopyStringsSafe(Pad->Name, sizeof(Pad->Name), Name);
    
    Pad->Connected = true;
    
    AssignKeyOnButton(NewControl, GamepadKey_DpadUp, Button_Up);
    AssignKeyOnButton(NewControl, GamepadKey_DpadDown, Button_Down);
    AssignKeyOnButton(NewControl, GamepadKey_DpadLeft, Button_Left);
    AssignKeyOnButton(NewControl, GamepadKey_DpadRight, Button_Right);
}

INTERNAL_FUNCTION inline void DisconnectGamepadController(int PadIndex)
{
    gamepad_controller* Pad = &Global_Input->Gamepads[PadIndex];
    
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
        gamepad_controller* Pad = &Global_Input->Gamepads[GamepadIndex];
        
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
            Result = &Global_Input->Keyboard.KeyStates[Key];
        } break;
        
        case Controller_Gamepad:
        {
            gamepad_controller* Pad = &Global_Input->Gamepads[Controller->GamepadIndex];
            if(Pad->Connected)
            {
                gamepad_key* PadBut = &Pad->Keys[Key];
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
           ControllerIndex < Global_Input->ControllerCount)
        {
            controller* Control = &Global_Input->Controllers[ControllerIndex];
            
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
            Result = Global_Input->MouseDeltaP.x;
        } break;
        
        case Axis_MouseY:
        {
            Result = Global_Input->MouseDeltaP.y;
        } break;
        
        case Axis_MouseScroll:
        {
            Result = Global_Input->MouseScroll;
        }break;
    }
    
    return(Result);
}

INTERNAL_FUNCTION void InitInput(memory_arena* Arena)
{
    Global_Input = PushStruct(Arena, input_system);
    
    Global_Input->CapturingMouse = true;
    Global_Input->MouseDefaultSpeed = 0.1f;
    
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
    
    // NOTE(Dima): Init gamepads
    for(int GamepadIndex = 0;
        GamepadIndex < MAX_GAMEPADS;
        GamepadIndex++)
    {
        gamepad_controller* Pad = &Global_Input->Gamepads[GamepadIndex];
        
        Pad->Connected = false;
    }
}