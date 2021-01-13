/*
// NOTE(Dima): What we've done in this episode:
--- Loaded skeleton information
--- Loaded skinning information
--- Loaded animation information
--- FindNearestFrames finds frames between which we need to interpolate
--- Interpolated nearest frames to calculate transform
--- Used transform matrices to calculate skinning matrices
--- Used skinning matrices in shader to apply skinning

--- Packed Color from 3 floats to single Int in vertex structure

// TODO(Dima): 
*/

#include <iostream>

#include <SDL.h>
#include <vector>
#include <algorithm>

#include "flower_defines.h"

#include "flower.h"

#include "flower.cpp"
#include "flower_standard.cpp"

struct app_state
{
    bool Running;
    
    window_dimensions WndDims; 
    
    SDL_Window* Window;
    b32 IsFullscreen;
    
    SDL_GameController* Gamepads[MAX_GAMEPADS];
    f32 GamepadsCheckCounter;
    
    SDL_GLContext OpenGLContext;
};

GLOBAL_VARIABLE app_state* App;

#include "flower_opengl.cpp"

GLOBAL_VARIABLE memory_arena GlobalArena;

platform_api Platform;

// NOTE(Dima): Memory block functions
PLATFORM_ALLOCATE_BLOCK(AppAllocBlock)
{
    mi SizeToAlloc = Align(Size, 32) + sizeof(memory_block);
    void* Memory = (void*)calloc(1, SizeToAlloc);
    
    void* Data = Memory;
    void* BlockData = ((u8*)Memory + (SizeToAlloc - sizeof(memory_block)));
    
    struct memory_block* Block = (memory_block*)BlockData;
    
    Block->Used = 0;
    Block->Size = Size;
    Block->Base = Data;
    
    return(Block);
}

PLATFORM_DEALLOCATE_BLOCK(AppDeallocBlock)
{
    if(Block)
    {
        free(Block->Base);
    }
}

// NOTE(Dima): Get thread ID
INTERNAL_FUNCTION inline  PLATFORM_GET_THREAD_ID(SDLGetThreadID)
{
    SDL_threadID ThreadID = SDL_ThreadID();
    
    return(ThreadID);
}

INTERNAL_FUNCTION void SetFullscreen(b32 Fullscreen)
{
    u32 Flags = 0;
    
    if(Fullscreen)
    {
        Flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }
    
    SDL_SetWindowFullscreen(App->Window, Flags);
    SDL_ShowCursor(!Fullscreen);
    
    App->IsFullscreen = Fullscreen;
}

INTERNAL_FUNCTION inline void ProcessKeyState(key_state* Key, b32 TransitionHappened, b32 IsDown)
{
    if(TransitionHappened && (Key->EndedDown != IsDown))
    {
        Key->InTransitionTime = 0.0f;
        
        Key->EndedDown = IsDown;
        Key->TransitionHappened = TransitionHappened;
    }
}

INTERNAL_FUNCTION inline void PreProcessKeyState(input_system* Input, key_state* Key)
{
    Key->TransitionHappened = false;
    Key->InTransitionTime += Global_Time->DeltaTime;
}

INTERNAL_FUNCTION void PreProcessInput(input_system* Input)
{
    Input->MouseScroll = 0.0f;
    
    // NOTE(Dima): Preprocess keyboard
    keyboard_controller* Keyboard = &Input->Keyboard;
    
    for(int KeyIndex = 0; KeyIndex < Key_Count; KeyIndex++)
    {
        PreProcessKeyState(Input, &Keyboard->KeyStates[KeyIndex]);
    }
    
    // NOTE(Dima): Preprocess gamepads
    for(int GamepadIndex = 0; GamepadIndex < MAX_GAMEPADS; GamepadIndex++)
    {
        gamepad_controller* Gamepad = &Input->Gamepads[GamepadIndex];
        
        if(Gamepad->Connected)
        {
            for(int KeyIndex = 0; 
                KeyIndex < GamepadKey_Count; 
                KeyIndex++)
            {
                PreProcessKeyState(Input, &Gamepad->Keys[KeyIndex].PressState);
            }
        }
    }
    
    // NOTE(Dima): Preprocess virtual buttons
    for(int ControlIndex = 0; 
        ControlIndex < Input->ControllerCount; 
        ControlIndex++)
    {
        controller* Control = &Input->Controllers[ControlIndex];
        
        for(int ButtonIndex = 0;
            ButtonIndex < Button_Count;
            ButtonIndex++)
        {
            PreProcessKeyState(Input, &Control->Buttons[ButtonIndex].PressState);
        }
    }
}

INTERNAL_FUNCTION void ProcessMouseButtonEvents(input_system* Input, SDL_MouseButtonEvent* MouseButEvent)
{
    keyboard_controller* Keyboard = &Input->Keyboard;
    
    key_state* ProcessKey = 0;
    switch(MouseButEvent->button)
    {
        case SDL_BUTTON_LEFT:
        {
            ProcessKey = &Keyboard->KeyStates[KeyMouse_Left];
        } break;
        
        case SDL_BUTTON_RIGHT:
        {
            ProcessKey = &Keyboard->KeyStates[KeyMouse_Right];
        } break;
        
        case SDL_BUTTON_MIDDLE:
        {
            ProcessKey = &Keyboard->KeyStates[KeyMouse_Middle];
        } break;
        
        case SDL_BUTTON_X1:
        {
            ProcessKey = &Keyboard->KeyStates[KeyMouse_X1];
        } break;
        
        case SDL_BUTTON_X2:
        {
            ProcessKey = &Keyboard->KeyStates[KeyMouse_X2];
        } break;
    }
    
    if(ProcessKey)
    {
        b32 IsDown = MouseButEvent->state == SDL_PRESSED;
        b32 TransitionHappened = IsDown != ProcessKey->EndedDown;
        
        ProcessKeyState(ProcessKey, TransitionHappened, IsDown);
    }
}

INTERNAL_FUNCTION void ProcessKeyboardEvents(input_system* Input, SDL_KeyboardEvent* KeyEvent)
{
    keyboard_controller* Keyboard = &Input->Keyboard;
    
    SDL_Keysym Keysym = KeyEvent->keysym;
    
    // NOTE(Dima): Processing mods
    b32 LeftShift = (Keysym.mod & KMOD_LSHIFT) != 0;
    b32 RightShift = (Keysym.mod & KMOD_RSHIFT) != 0;
    b32 AnyShift = (LeftShift & RightShift) != 0;
    
    b32 LeftAlt = (Keysym.mod & KMOD_LALT) != 0;
    b32 RightAlt = (Keysym.mod & KMOD_RALT) != 0;
    b32 AnyAlt = (LeftAlt | RightAlt) != 0;
    
    b32 LeftControl = (Keysym.mod & KMOD_LCTRL) != 0;
    b32 RightControl = (Keysym.mod & KMOD_RCTRL) != 0;
    b32 AnyControl = (LeftControl | RightControl) != 0;
    
    b32 Caps = (Keysym.mod & KMOD_CAPS) != 0;
    b32 Num = (Keysym.mod & KMOD_NUM) != 0;
    
    u32 Mods = 0;
    Mods |= (LeftShift) << 1;
    Mods |= (RightShift) << 2;
    Mods |= (LeftControl) << 3;
    Mods |= (RightControl) << 4;
    Mods |= (LeftAlt) << 5;
    Mods |= (RightAlt) << 6;
    Mods |= (Caps) << 7;
    Mods |= (Num) << 8;
    
    Keyboard->Mods = Mods;
    
    b32 AltIsDown = AnyAlt;
    b32 ShiftIsDown = AnyShift;
    b32 CtrlISDown = AnyControl;
    
    b32 IsDown = (KeyEvent->state == SDL_PRESSED);
    b32 TransitionHappened = (KeyEvent->repeat == 0);
    
    SDL_Keycode KeyCode = Keysym.sym;
    
    key_state* ProcessKey = 0;
    
    switch (KeyCode) {
        case(SDLK_LEFT): {
            ProcessKey = &Keyboard->KeyStates[Key_Left];
        }break;
        case(SDLK_RIGHT): {
            ProcessKey = &Keyboard->KeyStates[Key_Right];
        }break;
        case(SDLK_UP): {
            ProcessKey = &Keyboard->KeyStates[Key_Up];
        }break;
        case(SDLK_DOWN): {
            ProcessKey = &Keyboard->KeyStates[Key_Down];
        }break;
        
        case(SDLK_BACKSPACE): {
            ProcessKey = &Keyboard->KeyStates[Key_Backspace];
        }break;
        case(SDLK_TAB): {
            ProcessKey = &Keyboard->KeyStates[Key_Tab];
        }break;
        case(SDLK_RETURN): {
            ProcessKey = &Keyboard->KeyStates[Key_Return];
        }break;
        
        case(SDLK_LSHIFT): {
            ProcessKey = &Keyboard->KeyStates[Key_LeftShift];
        }break;
        case(SDLK_RSHIFT): {
            ProcessKey = &Keyboard->KeyStates[Key_RightShift];
        }break;
        
        case(SDLK_LCTRL): {
            ProcessKey = &Keyboard->KeyStates[Key_LeftControl];
        }break;
        case(SDLK_RCTRL): {
            ProcessKey = &Keyboard->KeyStates[Key_RightControl];
        }break;
        
        case(SDLK_ESCAPE): {
            ProcessKey = &Keyboard->KeyStates[Key_Escape];
        }break;
        case(SDLK_SPACE): {
            ProcessKey = &Keyboard->KeyStates[Key_Space];
        }break;
        case(SDLK_HOME): {
            ProcessKey = &Keyboard->KeyStates[Key_Home];
        }break;
        case(SDLK_END): {
            ProcessKey = &Keyboard->KeyStates[Key_End];
        }break;
        case(SDLK_INSERT): {
            ProcessKey = &Keyboard->KeyStates[Key_Insert];
        }break;
        case(SDLK_DELETE): {
            ProcessKey = &Keyboard->KeyStates[Key_Delete];
        }break;
        case(SDLK_HELP): {
            ProcessKey = &Keyboard->KeyStates[Key_Help];
        }break;
        
        case(SDLK_0): {
            ProcessKey = &Keyboard->KeyStates[Key_0];
        }break;
        case(SDLK_1): {
            ProcessKey = &Keyboard->KeyStates[Key_1];
        }break;
        case(SDLK_2): {
            ProcessKey = &Keyboard->KeyStates[Key_2];
        }break;
        case(SDLK_3): {
            ProcessKey = &Keyboard->KeyStates[Key_3];
        }break;
        case(SDLK_4): {
            ProcessKey = &Keyboard->KeyStates[Key_4];
        }break;
        case(SDLK_5): {
            ProcessKey = &Keyboard->KeyStates[Key_5];
        }break;
        case(SDLK_6): {
            ProcessKey = &Keyboard->KeyStates[Key_6];
        }break;
        case(SDLK_7): {
            ProcessKey = &Keyboard->KeyStates[Key_7];
        }break;
        case(SDLK_8): {
            ProcessKey = &Keyboard->KeyStates[Key_8];
        }break;
        case(SDLK_9): {
            ProcessKey = &Keyboard->KeyStates[Key_9];
        }break;
        
        case SDLK_a:{
            ProcessKey = &Keyboard->KeyStates[Key_A];
        } break;
        case SDLK_b:{
            ProcessKey = &Keyboard->KeyStates[Key_B];
        } break;
        case SDLK_c:{
            ProcessKey = &Keyboard->KeyStates[Key_C];
        } break;
        case SDLK_d:{
            ProcessKey = &Keyboard->KeyStates[Key_D];
        } break;
        case SDLK_e:{
            ProcessKey = &Keyboard->KeyStates[Key_E];
        } break;
        case SDLK_f:{
            ProcessKey = &Keyboard->KeyStates[Key_F];
        } break;
        case SDLK_g:{
            ProcessKey = &Keyboard->KeyStates[Key_G];
        } break;
        case SDLK_h:{
            ProcessKey = &Keyboard->KeyStates[Key_H];
        } break;
        case SDLK_i:{
            ProcessKey = &Keyboard->KeyStates[Key_I];
        } break;
        case SDLK_j:{
            ProcessKey = &Keyboard->KeyStates[Key_J];
        } break;
        case SDLK_k:{
            ProcessKey = &Keyboard->KeyStates[Key_K];
        } break;
        case SDLK_l:{
            ProcessKey = &Keyboard->KeyStates[Key_L];
        } break;
        case SDLK_m:{
            ProcessKey = &Keyboard->KeyStates[Key_M];
        } break;
        case SDLK_n:{
            ProcessKey = &Keyboard->KeyStates[Key_N];
        } break;
        case SDLK_o:{
            ProcessKey = &Keyboard->KeyStates[Key_O];
        } break;
        case SDLK_p:{
            ProcessKey = &Keyboard->KeyStates[Key_P];
        } break;
        case SDLK_q:{
            ProcessKey = &Keyboard->KeyStates[Key_Q];
        } break;
        case SDLK_r:{
            ProcessKey = &Keyboard->KeyStates[Key_R];
        } break;
        case SDLK_s:{
            ProcessKey = &Keyboard->KeyStates[Key_S];
        } break;
        case SDLK_t:{
            ProcessKey = &Keyboard->KeyStates[Key_T];
        } break;
        case SDLK_u:{
            ProcessKey = &Keyboard->KeyStates[Key_U];
        } break;
        case SDLK_v:{
            ProcessKey = &Keyboard->KeyStates[Key_V];
        } break;
        case SDLK_w:{
            ProcessKey = &Keyboard->KeyStates[Key_W];
        } break;
        case SDLK_x:{
            ProcessKey = &Keyboard->KeyStates[Key_X];
        } break;
        case SDLK_y:{
            ProcessKey = &Keyboard->KeyStates[Key_Y];
        } break;
        case SDLK_z:{
            ProcessKey = &Keyboard->KeyStates[Key_Z];
        } break;
        
        // NOTE(Dima): Num pad
        case SDLK_KP_0:{
            ProcessKey = &Keyboard->KeyStates[Key_Num0];
        } break;
        case SDLK_KP_1:{
            ProcessKey = &Keyboard->KeyStates[Key_Num1];
        } break;
        case SDLK_KP_2:{
            ProcessKey = &Keyboard->KeyStates[Key_Num2];
        } break;
        case SDLK_KP_3:{
            ProcessKey = &Keyboard->KeyStates[Key_Num3];
        } break;
        case SDLK_KP_4:{
            ProcessKey = &Keyboard->KeyStates[Key_Num4];
        } break;
        case SDLK_KP_5:{
            ProcessKey = &Keyboard->KeyStates[Key_Num5];
        } break;
        case SDLK_KP_6:{
            ProcessKey = &Keyboard->KeyStates[Key_Num6];
        } break;
        case SDLK_KP_7:{
            ProcessKey = &Keyboard->KeyStates[Key_Num7];
        } break;
        case SDLK_KP_8:{
            ProcessKey = &Keyboard->KeyStates[Key_Num7];
        } break;
        case SDLK_KP_9:{
            ProcessKey = &Keyboard->KeyStates[Key_Num7];
        } break;
        
        // NOTE(Dima): Function keys
        case(SDLK_F1): {
            ProcessKey = &Keyboard->KeyStates[Key_F1];
        }break;
        case(SDLK_F2): {
            ProcessKey = &Keyboard->KeyStates[Key_F2];
        }break;
        case(SDLK_F3): {
            ProcessKey = &Keyboard->KeyStates[Key_F3];
        }break;
        case(SDLK_F4): {
            ProcessKey = &Keyboard->KeyStates[Key_F4];
        }break;
        case(SDLK_F5): {
            ProcessKey = &Keyboard->KeyStates[Key_F5];
        }break;
        case(SDLK_F6): {
            ProcessKey = &Keyboard->KeyStates[Key_F6];
        }break;
        case(SDLK_F7): {
            ProcessKey = &Keyboard->KeyStates[Key_F7];
        }break;
        case(SDLK_F8): {
            ProcessKey = &Keyboard->KeyStates[Key_F8];
        }break;
        case(SDLK_F9): {
            ProcessKey = &Keyboard->KeyStates[Key_F9];
        }break;
        case(SDLK_F10): {
            ProcessKey = &Keyboard->KeyStates[Key_F10];
        }break;
        case(SDLK_F11): {
            ProcessKey = &Keyboard->KeyStates[Key_F11];
        }break;
        case(SDLK_F12): {
            ProcessKey = &Keyboard->KeyStates[Key_F12];
        }break;
        
        case(SDLK_BACKQUOTE): {
            ProcessKey = &Keyboard->KeyStates[Key_Backquote];
        }break;
        
        default: {
            
        }break;
    }
    
    if(ProcessKey)
    {
        ProcessKeyState(ProcessKey, TransitionHappened, IsDown);
    }
    
    if (TransitionHappened) {
        
        if (IsDown)
        {
            if (KeyCode == SDLK_RETURN && AltIsDown)
            {
                SetFullscreen(!App->IsFullscreen);
            }
            
            if(KeyCode == SDLK_BACKQUOTE && AltIsDown)
            {
                Input->CapturingMouse = !Input->CapturingMouse;
                
                SDL_SetRelativeMouseMode(Input->CapturingMouse ? SDL_TRUE : SDL_FALSE);
            }
        }
        else
        {
            
        }
    }
}

INTERNAL_FUNCTION void ProcessEvents(input_system* Input)
{
    keyboard_controller* Keyboard = &Input->Keyboard;
    
    SDL_Event Event;
    while(SDL_PollEvent(&Event))
    {
        switch(Event.type)
        {
            case SDL_KEYUP:
            case SDL_KEYDOWN:
            {
                ProcessKeyboardEvents(Input, &Event.key);
            }break;
            
            
            case(SDL_MOUSEBUTTONUP):
            case(SDL_MOUSEBUTTONDOWN): 
            {
                ProcessMouseButtonEvents(Input, &Event.button);
            }break;
            
            case SDL_MOUSEWHEEL:
            {
                Input->MouseScroll = Event.wheel.y;
            }break;
            
            case(SDL_WINDOWEVENT): 
            {
                SDL_WindowEvent* WindowEvent = &Event.window;
                switch (WindowEvent->event) {
                    /*Close is sent to window*/
                    case(SDL_WINDOWEVENT_CLOSE): {
                        App->Running = false;
                    }break;
                    
                    /*Mouse entered window*/
                    case SDL_WINDOWEVENT_ENTER: {
                        
                    }break;
                    
                    /*Mouse leaved window*/
                    case SDL_WINDOWEVENT_LEAVE: {
                        
                    }break;
                };
            }break;
            
            default:
            {
                
            }break;
        }
    }
}

INTERNAL_FUNCTION void ProcessVirtualButtons(input_system* Input)
{
    // NOTE(Dima): Processing all virtual buttons
    for(int ControlIndex = 0; 
        ControlIndex < Input->ControllerCount; 
        ControlIndex++)
    {
        controller* Controller = &Input->Controllers[ControlIndex];
        
        for(int ButtonIndex = 0;
            ButtonIndex < Button_Count;
            ButtonIndex++)
        {
            button_state* But = &Controller->Buttons[ButtonIndex];
            
            key_state* Dst = &But->PressState;
            
            if(But->KeyCount)
            {
                b32 AtLeastOneWasDown = false;
                
                for(int KeyIndex = 0; KeyIndex < But->KeyCount; KeyIndex++)
                {
                    int Key = But->Keys[KeyIndex];
                    
                    key_state* Src = GetKeyOnController(Controller, Key);
                    
                    if(Src != 0)
                    {
                        if(Src->EndedDown)
                        {
                            AtLeastOneWasDown = true;
                            
                            break;
                        }
                    }
                }
                
                b32 IsDown = AtLeastOneWasDown;
                b32 TransitionHappened = Dst->EndedDown != IsDown;
                
                ProcessKeyState(Dst, TransitionHappened, IsDown);
            }
            else
            {
                ProcessKeyState(Dst, false, false);
            }
        }
    }
}

INTERNAL_FUNCTION void ProcessMouse(input_system* Input)
{
    int MouseWindowP_X;
    int MouseWindowP_Y;
    SDL_GetMouseState(&MouseWindowP_X, &MouseWindowP_Y);
    
    int MouseDeltaX;
    int MouseDeltaY;
    SDL_GetRelativeMouseState(&MouseDeltaX, &MouseDeltaY);
    
    int WindowWidth;
    int WindowHeight;
    SDL_GetWindowSize(App->Window, &WindowWidth, &WindowHeight);
    
    Input->MouseWindowP = V2(MouseWindowP_X, MouseWindowP_Y);
    Input->MouseDeltaP = -V2(MouseDeltaX, MouseDeltaY) * Input->MouseDefaultSpeed;
    Input->MouseUV = V2((f32)MouseWindowP_X / (f32)WindowWidth, 
                        (f32)MouseWindowP_Y / (f32)WindowHeight);
}

INTERNAL_FUNCTION inline void ProcessGamepadAxis(input_system* Input, int PadIndex,
                                                 u32 OurAxis, SDL_GameControllerAxis SDLAxis)
{
    SDL_GameController* SDLPad = App->Gamepads[PadIndex];
    
    s16 AxisInt = SDL_GameControllerGetAxis(SDLPad, SDLAxis);
    
    int DeadZone = 5000;
    if(std::abs(AxisInt) < 5000)
    {
        AxisInt = 0;
    }
    
    float AxisResult = (float)AxisInt / 32768.0f;
    if(AxisInt < 0)
    {
        AxisResult = (float)AxisInt / 32768.0f;
    }
    
    AxisResult *= -1.0f;
    
    gamepad_controller* Pad = &Input->Gamepads[PadIndex];
    Pad->Axes[OurAxis] = AxisResult;
}

INTERNAL_FUNCTION inline void ProcessGamepadButton(input_system* Input, int PadIndex,
                                                   u32 OurKey, SDL_GameControllerButton SDLButton)
{
    SDL_GameController* SDLPad = App->Gamepads[PadIndex];
    gamepad_controller* Pad = &Input->Gamepads[PadIndex];
    
    Uint8 Pressed = SDL_GameControllerGetButton(SDLPad, SDLButton);
    
    gamepad_key* OurPadKey = &Pad->Keys[OurKey];
    
    b32 IsDown = Pressed == 1;
    b32 TransitionHappened = IsDown != OurPadKey->PressState.EndedDown;
    
    ProcessKeyState(&OurPadKey->PressState, TransitionHappened, IsDown);
}

INTERNAL_FUNCTION inline void SetButtonNameForController(input_system* Input, 
                                                         int PadIndex,
                                                         u32 OurButton,
                                                         u32 SDLButton)
{
    gamepad_controller* Pad = &Input->Gamepads[PadIndex];
    
    char* ButtonName = (char*)SDL_GameControllerGetStringForButton((SDL_GameControllerButton)SDLButton);
    
    gamepad_key* But = &Pad->Keys[OurButton];
    CopyStringsSafe(But->Name, sizeof(But->Name), ButtonName);
}

INTERNAL_FUNCTION void SetButtonNamesForGameController(input_system* Input, 
                                                       int PadIndex)
{
    SetButtonNameForController(Input, PadIndex, GamepadKey_A, SDL_CONTROLLER_BUTTON_A);
    SetButtonNameForController(Input, PadIndex, GamepadKey_B, SDL_CONTROLLER_BUTTON_B);
    SetButtonNameForController(Input, PadIndex, GamepadKey_X, SDL_CONTROLLER_BUTTON_X);
    SetButtonNameForController(Input, PadIndex, GamepadKey_Y, SDL_CONTROLLER_BUTTON_Y);
    
    SetButtonNameForController(Input, PadIndex, GamepadKey_Back, SDL_CONTROLLER_BUTTON_BACK);
    SetButtonNameForController(Input, PadIndex, GamepadKey_Guide, SDL_CONTROLLER_BUTTON_GUIDE);
    SetButtonNameForController(Input, PadIndex, GamepadKey_Start, SDL_CONTROLLER_BUTTON_START);
    
    SetButtonNameForController(Input, PadIndex, GamepadKey_LeftStick, SDL_CONTROLLER_BUTTON_LEFTSTICK);
    SetButtonNameForController(Input, PadIndex, GamepadKey_RightStick, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
    SetButtonNameForController(Input, PadIndex, GamepadKey_LeftShoulder, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    SetButtonNameForController(Input, PadIndex, GamepadKey_RightShoulder, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    
    SetButtonNameForController(Input, PadIndex, GamepadKey_DpadUp, SDL_CONTROLLER_BUTTON_DPAD_UP);
    SetButtonNameForController(Input, PadIndex, GamepadKey_DpadDown, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    SetButtonNameForController(Input, PadIndex, GamepadKey_DpadLeft, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    SetButtonNameForController(Input, PadIndex, GamepadKey_DpadRight, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
}

INTERNAL_FUNCTION void ProcessGamepads(input_system* Input)
{
    for(int PadIndex = 0; PadIndex < MAX_GAMEPADS; PadIndex++)
    {
        gamepad_controller* Pad = &Input->Gamepads[PadIndex];
        
        if(Pad->Connected)
        {
            // NOTE(Dima): Processing gamepad axes
            ProcessGamepadAxis(Input, PadIndex, GamepadAxis_LeftX, SDL_CONTROLLER_AXIS_LEFTX);
            ProcessGamepadAxis(Input, PadIndex, GamepadAxis_LeftY, SDL_CONTROLLER_AXIS_LEFTY);
            ProcessGamepadAxis(Input, PadIndex, GamepadAxis_RightX, SDL_CONTROLLER_AXIS_RIGHTX);
            ProcessGamepadAxis(Input, PadIndex, GamepadAxis_RightY, SDL_CONTROLLER_AXIS_RIGHTY);
            ProcessGamepadAxis(Input, PadIndex, GamepadAxis_TriggerLeft, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
            ProcessGamepadAxis(Input, PadIndex, GamepadAxis_TriggerRight, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
            
            // NOTE(Dima): Processing gampad buttons
            ProcessGamepadButton(Input, PadIndex, GamepadKey_A, SDL_CONTROLLER_BUTTON_A);
            ProcessGamepadButton(Input, PadIndex, GamepadKey_B, SDL_CONTROLLER_BUTTON_B);
            ProcessGamepadButton(Input, PadIndex, GamepadKey_X, SDL_CONTROLLER_BUTTON_X);
            ProcessGamepadButton(Input, PadIndex, GamepadKey_Y, SDL_CONTROLLER_BUTTON_Y);
            
            ProcessGamepadButton(Input, PadIndex, GamepadKey_Back, SDL_CONTROLLER_BUTTON_BACK);
            ProcessGamepadButton(Input, PadIndex, GamepadKey_Guide, SDL_CONTROLLER_BUTTON_GUIDE);
            ProcessGamepadButton(Input, PadIndex, GamepadKey_Start, SDL_CONTROLLER_BUTTON_START);
            
            ProcessGamepadButton(Input, PadIndex, GamepadKey_LeftStick, SDL_CONTROLLER_BUTTON_LEFTSTICK);
            ProcessGamepadButton(Input, PadIndex, GamepadKey_RightStick, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
            ProcessGamepadButton(Input, PadIndex, GamepadKey_LeftShoulder, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            ProcessGamepadButton(Input, PadIndex, GamepadKey_RightShoulder, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
            
            ProcessGamepadButton(Input, PadIndex, GamepadKey_DpadUp, SDL_CONTROLLER_BUTTON_DPAD_UP);
            ProcessGamepadButton(Input, PadIndex, GamepadKey_DpadDown, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            ProcessGamepadButton(Input, PadIndex, GamepadKey_DpadLeft, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            ProcessGamepadButton(Input, PadIndex, GamepadKey_DpadRight, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
        }
    }
}

INTERNAL_FUNCTION inline SDL_JoystickID SDLGetGameControlJoyID(SDL_GameController* Pad)
{
    SDL_JoystickID Result = -1;
    
    if(Pad)
    {
        SDL_Joystick* PadJoystick = SDL_GameControllerGetJoystick(Pad);
        
        Result = SDL_JoystickInstanceID(PadJoystick);
    }
    
    return(Result);
}

INTERNAL_FUNCTION b32 SDLGameControllerAlreadyConnected(SDL_GameController* Pad)
{
    b32 Result = false;
    
    SDL_JoystickID PadID = SDLGetGameControlJoyID(Pad);
    
    for(int PadIndex = 0; PadIndex < MAX_GAMEPADS; PadIndex++)
    {
        SDL_GameController* CurPad = App->Gamepads[PadIndex];
        
        SDL_JoystickID CurPadID = SDLGetGameControlJoyID(CurPad);
        
        if(CurPadID == PadID)
        {
            Result = true;
            break;
        }
    }
    
    return(Result);
}

INTERNAL_FUNCTION void CheckGamepadConnections(input_system* Input)
{
    for(int JoystickIndex = 0;
        JoystickIndex < SDL_NumJoysticks(); 
        JoystickIndex++)
    {
        SDL_Joystick* Joy = SDL_JoystickOpen(JoystickIndex);
        const char* JoystickName = SDL_JoystickName(Joy);
        
        SDL_JoystickID JoyID = SDL_JoystickInstanceID(Joy);
        
        if(SDL_IsGameController(JoystickIndex))
        {
            //SDL_Log("Detected gamepad with joystick index: %d\n", JoystickIndex);
            
            SDL_GameController* Controller = SDL_GameControllerOpen(JoystickIndex);
            if(Controller)
            {
                b32 AlreadyConnected = SDLGameControllerAlreadyConnected(Controller);
                
                if(!AlreadyConnected)
                {
                    int PadIndex = TryAddGamepadController((char*)JoystickName);
                    
                    if(PadIndex != -1)
                    {
                        App->Gamepads[PadIndex] = Controller;
                        
                        SetButtonNamesForGameController(Input, PadIndex);
                        
                        SDL_Log("Gamepad connected successfully on slot: %d\n", PadIndex);
                    }
                    else
                    {
                        SDL_Log("Could not add any more Gamepads\n");
                        
                        break;
                    }
                }
            }
            else
            {
                SDL_Log("Could not open Game Controller %i\n", JoystickIndex);
            }
            
        }
        else{
            SDL_Log("Joystick %d is not a gamepad: %d\n", JoystickIndex);
        }
    }
}

INTERNAL_FUNCTION void CheckGamepadDisconnect(input_system* Input)
{
    for (int PadIndex = 0; PadIndex < MAX_GAMEPADS; PadIndex += 1)
    {
        SDL_GameController* CurSDLPad = App->Gamepads[PadIndex];
        if(CurSDLPad != 0)
        {
            SDL_JoystickID PadID = SDLGetGameControlJoyID(CurSDLPad);
            
            b32 Found = false;
            
            for(int JoystickIndex = 0;
                JoystickIndex < SDL_NumJoysticks(); 
                JoystickIndex++)
            {
                SDL_Joystick* Joy = SDL_JoystickOpen(JoystickIndex);
                
                SDL_JoystickID JoyID = SDL_JoystickInstanceID(Joy);
                
                if(SDL_IsGameController(JoystickIndex))
                {
                    SDL_GameController* Controller = SDL_GameControllerOpen(JoystickIndex);
                    if(Controller)
                    {
                        SDL_JoystickID CurPadJoyID = SDLGetGameControlJoyID(Controller);
                        
                        if(PadID == CurPadJoyID)
                        {
                            Found = true;
                            break;
                        }
                    }
                }
            }
            
            if(!Found)
            {
                SDL_Log("Gamepad was disconnected on slot: %d\n", PadIndex);
                
                App->Gamepads[PadIndex] = 0;
                
                DisconnectGamepadController(PadIndex);
            }
        }
    }
}

INTERNAL_FUNCTION void ProcessInput()
{
    App->GamepadsCheckCounter += Global_Time->DeltaTime;
    if(App->GamepadsCheckCounter >= 1.5f)
    {
        //SDL_Log("Checking gamepad connections and disconnections\n");
        
        CheckGamepadConnections(Global_Input);
        CheckGamepadDisconnect(Global_Input);
        
        App->GamepadsCheckCounter = 0.0f;
    }
    
    // NOTE(Dima): Getting window dimensions
    SDL_GetWindowSize(App->Window, 
                      &App->WndDims.Width, 
                      &App->WndDims.Height);
    
    // NOTE(Dima): Processing input
    PreProcessInput(Global_Input);
    ProcessEvents(Global_Input);
    ProcessMouse(Global_Input);
    ProcessGamepads(Global_Input);
    ProcessVirtualButtons(Global_Input);
}

int main(int ArgsCount, char** Args)
{
    int SDLInitCode = SDL_Init(SDL_INIT_EVERYTHING);
    
    Platform = {};
    Platform.AllocateBlock = AppAllocBlock;
    Platform.DeallocateBlock = AppDeallocBlock;
    Platform.ProcessInput = ProcessInput;
    Platform.Render = OpenGLRender;
    Platform.SwapBuffers = OpenGLSwapBuffers;
    Platform.ReadFileAndNullTerminate = StandardReadFileAndNullTerminate;
    Platform.ReadFile = StandardReadFile;
    Platform.AllocateMemory = StandardAllocateMemory;
    Platform.FreeMemory = StandardFreeMemory;
    Platform.GetThreadID = SDLGetThreadID;
    
    DEBUGInitGlobalTable(&GlobalArena);
    
    App = PushStruct(&GlobalArena, app_state);
    
    memory_arena GameArena = {};
    game* Game = PushStruct(&GameArena, game);
    InitGame(Game, &GameArena);
    
#if 0    
    App->WndDims.InitWindowWidth = 1920;
    App->WndDims.InitWindowHeight = 1080;
#else
    App->WndDims.InitWidth = 1600;
    App->WndDims.InitHeight = 900;
#endif
    App->WndDims.Width = App->WndDims.InitWidth;
    App->WndDims.Height = App->WndDims.InitHeight;
    
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    App->Window = SDL_CreateWindow("Flower",
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   App->WndDims.InitWidth,
                                   App->WndDims.InitHeight,
                                   SDL_WINDOW_OPENGL);
    
    // NOTE(Dima): Do not track mouse outside window
    SDL_CaptureMouse(SDL_FALSE);
    SDL_SetRelativeMouseMode(Global_Input->CapturingMouse ? SDL_TRUE : SDL_FALSE);
    
    App->OpenGLContext = SDL_GL_CreateContext(App->Window);
    
    OpenGLInit();
    
    Global_Time->PerformanceFrequency = SDL_GetPerformanceFrequency();
    Global_Time->OneOverPerformanceFrequency = 1.0 / (f64)Global_Time->PerformanceFrequency;
    Global_Time->Time = 0.0f;
    Global_Time->DeltaTime = 0.001f;
    u64 LastClocks = SDL_GetPerformanceCounter();
    
    App->Running = true;
    while(App->Running)
    {
        FRAME_BARRIER(Global_Time->DeltaTime);
        
        BEGIN_TIMING(FRAME_UPDATE_NODE_NAME);
        
        // NOTE(Dima): Processing time
        u64 NewClocks = SDL_GetPerformanceCounter();
        u64 ClocksElapsed = NewClocks - LastClocks;
        LastClocks = NewClocks;
        
        Global_Time->DeltaTime = (f32)((f64)ClocksElapsed * Global_Time->OneOverPerformanceFrequency); 
        if(Global_Time->DeltaTime < 0.000001f)
        {
            Global_Time->DeltaTime = 0.000001f;
        }
        Global_Time->Time += Global_Time->DeltaTime;
        Global_Time->SinTime = Sin(Global_Time->Time);
        Global_Time->CosTime = Cos(Global_Time->Time);
        
        // NOTE(Dima): Updating a game
        Global_RenderCommands->WindowDimensions = App->WndDims;
        
        UpdateGame(Game);
        
        END_TIMING();
    }
    
    OpenGLFree();
    
    SDL_GL_DeleteContext(App->OpenGLContext);
    SDL_DestroyWindow(App->Window);
    
    FreeArena(&GlobalArena);
    
    return(0);
}
