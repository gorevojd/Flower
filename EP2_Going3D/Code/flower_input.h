#ifndef FLOWER_INPUT_H
#define FLOWER_INPUT_H

enum axis_type
{
    Axis_Horizontal,
    Axis_Vertical,
    
    Axis_MouseX,
    Axis_MouseY,
};

enum function_key_type{
    FuncKey_F1, 
    FuncKey_F2,
    FuncKey_F3,
    FuncKey_F4,
    FuncKey_F5,
    FuncKey_F6,
    FuncKey_F7,
    FuncKey_F8,
    FuncKey_F9,
    FuncKey_F10,
    FuncKey_F11,
    FuncKey_F12,
    
    FuncKey_Count,
};

enum key_code{
    // NOTE(Dima): arrows
    Key_Up,
    Key_Down,
    Key_Left,
    Key_Right,
    
    Key_Backspace,
    Key_Tab,
    Key_Return,
    Key_LeftShift,
    Key_RightShift,
    Key_LeftControl,
    Key_RightControl,
    Key_Escape,
    Key_Space,
    Key_Home,
    Key_End,
    Key_Insert,
    Key_Delete,
    Key_Help,
    
    // NOTE(Dima): Nums
    Key_0,
    Key_1,
    Key_2,
    Key_3,
    Key_4,
    Key_5,
    Key_6,
    Key_7,
    Key_8,
    Key_9,
    
    // NOTE(Dima): Letters
    Key_A,
    Key_B,
    Key_C,
    Key_D,
    Key_E,
    Key_F,
    Key_G,
    Key_H,
    Key_I,
    Key_J,
    Key_K,
    Key_L,
    Key_M,
    Key_N,
    Key_O,
    Key_P,
    Key_Q,
    Key_R,
    Key_S,
    Key_T,
    Key_U,
    Key_V,
    Key_W,
    Key_X,
    Key_Y,
    Key_Z,
    
    // NOTE(Dima): Numpad keys
    Key_Num0,
    Key_Num1,
    Key_Num2,
    Key_Num3,
    Key_Num4,
    Key_Num5,
    Key_Num6,
    Key_Num7,
    Key_Num8,
    Key_Num9,
    
    Key_F1,
    Key_F2,
    Key_F3,
    Key_F4,
    Key_F5,
    Key_F6,
    Key_F7,
    Key_F8,
    Key_F9,
    Key_F10,
    Key_F11,
    Key_F12,
    
    Key_Backquote, // NOTE(Dima): It's a ` or ~
    
    KeyMouse_Left,
    KeyMouse_Right,
    KeyMouse_Middle,
    KeyMouse_X1,
    KeyMouse_X2,
    
    Key_Count,
};

struct key_state
{
    b32 EndedDown;
    b32 TransitionHappened;
    
    f32 InTransitionTime;
};

enum key_mod
{
    KeyMod_Null = 0,
    
    KeyMod_LeftShift = (1 << 1),
    KeyMod_RightShift = (1 << 2),
    
    KeyMod_LeftControl = (1 << 3),
    KeyMod_RightControl = (1 << 4),
    
    KeyMod_LeftAlt = (1 << 5),
    KeyMod_RightAlt = (1 << 6),
    
    KeyMod_Caps = (1 << 7),
    KeyMod_NumLock = (1 << 8),
    
    KeyMod_Shift = KeyMod_RightShift | KeyMod_LeftShift,
    KeyMod_Control = KeyMod_RightControl | KeyMod_LeftControl,
    KeyMod_Alt = KeyMod_RightAlt | KeyMod_LeftAlt,
};

struct keyboard_controller
{
    key_state KeyStates[Key_Count];
    b32 FunctionKeysWasPressed[FuncKey_Count];
    
    // NOTE(Dima): Usage: Controller.Mods & KeyMod_LeftShift
    u32 Mods;
};

struct gamepad_controller
{
    
};

enum button_type
{
    Button_Left,
    Button_Right,
    Button_Up,
    Button_Down,
    
    Button_Jump,
    Button_Reload,
    Button_Interact,
    
    Button_OK,
    Button_Back,
    
    Button_MouseLeft,
    Button_MouseRight,
    Button_MouseUp,
    Button_MouseDown,
    
    Button_Count,
};

struct button_state
{
    key_state PressState;
    
    // NOTE(Dima): These mean keys on physical controller(keyboard or gamepad
#define MAX_KEYS_PER_BUTTON 8
    int Keys[MAX_KEYS_PER_BUTTON];
    int KeyCount;
};

enum controller_type
{
    Controller_None,
    
    Controller_Keyboard,
    Controller_Gamepad,
};

struct controller
{
    u32 Type;
    
    // NOTE(Dima): Will be used only in gamepad controller
    int GamepadIndex;
    
    button_state Buttons[Button_Count];
};

struct input_system
{
    keyboard_controller Keyboard;
    
#define MAX_GAMEPADS 4
    gamepad_controller Gamepads[MAX_GAMEPADS];
    
#define MAX_INPUT_CONTROLLERS 8
    controller Controllers[MAX_INPUT_CONTROLLERS];
    int ControllerCount;
    
    f32 AppDeltaTime;
    f64 AppTime;
    
    b32 CapturingMouse;
    float MouseDefaultSpeed;
    v2 MouseWindowP;
    v2 MouseUV;
    v2 MouseDeltaP;
};

#endif //FLOWER_INPUT_H
