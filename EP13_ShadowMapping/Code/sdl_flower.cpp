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

#include "flower_defines.h"
#include "flower_platform.h"
#include "flower.h"

#include "flower_standard.cpp"
#include "flower_jobs_platform.cpp"

platform_api Platform;
GLOBAL_VARIABLE job_system* Global_Jobs;

#if !defined(PLATFORM_IS_WINDOWS)
#include "flower.cpp"
#else

GLOBAL_VARIABLE input_system* Global_Input;
GLOBAL_VARIABLE time_system* Global_Time;
GLOBAL_VARIABLE debug_global_table* Global_DebugTable;

#include <Windows.h>

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "flower_input.cpp"

#endif

struct app_state
{
    bool Running;
    
    window_dimensions WndDims; 
    
    SDL_Window* Window;
    b32 IsFullscreen;
    
    SDL_GameController* Gamepads[MAX_GAMEPADS];
    f32 GamepadsCheckCounter;
    
    SDL_GLContext OpenGLContext;
    
#if defined(PLATFORM_IS_WINDOWS)
    char GameDllFullPath[256];
    char TempDllFullPath[256];
    
    FILETIME LastGameDllWriteTime;
    
    HINSTANCE TempDllHandle;
#endif
    game_update_and_render* GameUpdateAndRender;
    game_init* GameInit;
};

GLOBAL_VARIABLE app_state* App;

#include "flower_opengl.cpp"

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

INTERNAL_FUNCTION inline PLATFORM_OUTPUT_LOG(SDLOutputLog)
{
    SDL_Log(Text);
}

// NOTE(Dima): Performance counters functions
INTERNAL_FUNCTION inline PLATFORM_GET_PERFORMANCE_COUNTER(SDLGetPerfCounter)
{
    u64 Result = SDL_GetPerformanceCounter();
    
    return(Result);
}

INTERNAL_FUNCTION inline PLATFORM_GET_ELAPSED_TIME(SDLGetElapsedTime)
{
    f64 Result = ((f64)ClocksEnd - (f64)ClocksBegin) * Platform.OneOverPerfFrequency;
    
    return(Result);
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

INTERNAL_FUNCTION inline void PreProcessKeyState(key_state* Key)
{
    Key->TransitionHappened = false;
    Key->InTransitionTime += Global_Input->DeltaTime;
}

INTERNAL_FUNCTION void PreProcessInput()
{
    Global_Input->MouseScroll = 0.0f;
    
    // NOTE(Dima): Preprocess keyboard
    keyboard_controller* Keyboard = &Global_Input->Keyboard;
    
    for(int KeyIndex = 0; KeyIndex < Key_Count; KeyIndex++)
    {
        PreProcessKeyState(&Keyboard->KeyStates[KeyIndex]);
    }
    
    // NOTE(Dima): Preprocess gamepads
    for(int GamepadIndex = 0; GamepadIndex < MAX_GAMEPADS; GamepadIndex++)
    {
        gamepad_controller* Gamepad = &Global_Input->Gamepads[GamepadIndex];
        
        if(Gamepad->Connected)
        {
            for(int KeyIndex = 0; 
                KeyIndex < GamepadKey_Count; 
                KeyIndex++)
            {
                PreProcessKeyState(&Gamepad->Keys[KeyIndex].PressState);
            }
        }
    }
    
    // NOTE(Dima): Preprocess virtual buttons
    for(int ControlIndex = 0; 
        ControlIndex < Global_Input->ControllerCount; 
        ControlIndex++)
    {
        controller* Control = &Global_Input->Controllers[ControlIndex];
        
        for(int ButtonIndex = 0;
            ButtonIndex < Button_Count;
            ButtonIndex++)
        {
            PreProcessKeyState(&Control->Buttons[ButtonIndex].PressState);
        }
    }
}

INTERNAL_FUNCTION void ProcessMouseButtonEvents(SDL_MouseButtonEvent* MouseButEvent)
{
    keyboard_controller* Keyboard = &Global_Input->Keyboard;
    
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

INTERNAL_FUNCTION void ProcessKeyboardEvents(SDL_KeyboardEvent* KeyEvent)
{
    keyboard_controller* Keyboard = &Global_Input->Keyboard;
    
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
        }
        else
        {
            
        }
    }
}

INTERNAL_FUNCTION PLATFORM_SET_CAPTURING_MOUSE(SetCapturingMouse)
{
    SDL_SetRelativeMouseMode(IsCapture ? SDL_TRUE : SDL_FALSE);
}

INTERNAL_FUNCTION void ProcessEvents()
{
    keyboard_controller* Keyboard = &Global_Input->Keyboard;
    
    SDL_Event Event;
    while(SDL_PollEvent(&Event))
    {
        switch(Event.type)
        {
            case SDL_KEYUP:
            case SDL_KEYDOWN:
            {
                ProcessKeyboardEvents(&Event.key);
            }break;
            
            
            case(SDL_MOUSEBUTTONUP):
            case(SDL_MOUSEBUTTONDOWN): 
            {
                ProcessMouseButtonEvents(&Event.button);
            }break;
            
            case SDL_MOUSEWHEEL:
            {
                Global_Input->MouseScroll = Event.wheel.y;
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

INTERNAL_FUNCTION void ProcessVirtualButtons()
{
    // NOTE(Dima): Processing all virtual buttons
    for(int ControlIndex = 0; 
        ControlIndex < Global_Input->ControllerCount; 
        ControlIndex++)
    {
        controller* Controller = &Global_Input->Controllers[ControlIndex];
        
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

INTERNAL_FUNCTION void ProcessMouse()
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
    
    Global_Input->MouseWindowP = V2(MouseWindowP_X, MouseWindowP_Y);
    Global_Input->MouseDeltaP = -V2(MouseDeltaX, MouseDeltaY) * Global_Input->MouseDefaultSpeed;
    Global_Input->MouseUV = V2((f32)MouseWindowP_X / (f32)WindowWidth, 
                               (f32)MouseWindowP_Y / (f32)WindowHeight);
}

INTERNAL_FUNCTION inline void ProcessGamepadAxis(int PadIndex,
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
    
    gamepad_controller* Pad = &Global_Input->Gamepads[PadIndex];
    Pad->Axes[OurAxis] = AxisResult;
}

INTERNAL_FUNCTION inline void ProcessGamepadButton(int PadIndex,
                                                   u32 OurKey, SDL_GameControllerButton SDLButton)
{
    SDL_GameController* SDLPad = App->Gamepads[PadIndex];
    gamepad_controller* Pad = &Global_Input->Gamepads[PadIndex];
    
    Uint8 Pressed = SDL_GameControllerGetButton(SDLPad, SDLButton);
    
    gamepad_key* OurPadKey = &Pad->Keys[OurKey];
    
    b32 IsDown = Pressed == 1;
    b32 TransitionHappened = IsDown != OurPadKey->PressState.EndedDown;
    
    ProcessKeyState(&OurPadKey->PressState, TransitionHappened, IsDown);
}

INTERNAL_FUNCTION inline void SetButtonNameForController(int PadIndex,
                                                         u32 OurButton,
                                                         u32 SDLButton)
{
    gamepad_controller* Pad = &Global_Input->Gamepads[PadIndex];
    
    char* ButtonName = (char*)SDL_GameControllerGetStringForButton((SDL_GameControllerButton)SDLButton);
    
    gamepad_key* But = &Pad->Keys[OurButton];
    CopyStringsSafe(But->Name, sizeof(But->Name), ButtonName);
}

INTERNAL_FUNCTION void SetButtonNamesForGameController(int PadIndex)
{
    SetButtonNameForController(PadIndex, GamepadKey_A, SDL_CONTROLLER_BUTTON_A);
    SetButtonNameForController(PadIndex, GamepadKey_B, SDL_CONTROLLER_BUTTON_B);
    SetButtonNameForController(PadIndex, GamepadKey_X, SDL_CONTROLLER_BUTTON_X);
    SetButtonNameForController(PadIndex, GamepadKey_Y, SDL_CONTROLLER_BUTTON_Y);
    
    SetButtonNameForController(PadIndex, GamepadKey_Back, SDL_CONTROLLER_BUTTON_BACK);
    SetButtonNameForController(PadIndex, GamepadKey_Guide, SDL_CONTROLLER_BUTTON_GUIDE);
    SetButtonNameForController(PadIndex, GamepadKey_Start, SDL_CONTROLLER_BUTTON_START);
    
    SetButtonNameForController(PadIndex, GamepadKey_LeftStick, SDL_CONTROLLER_BUTTON_LEFTSTICK);
    SetButtonNameForController(PadIndex, GamepadKey_RightStick, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
    SetButtonNameForController(PadIndex, GamepadKey_LeftShoulder, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    SetButtonNameForController(PadIndex, GamepadKey_RightShoulder, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    
    SetButtonNameForController(PadIndex, GamepadKey_DpadUp, SDL_CONTROLLER_BUTTON_DPAD_UP);
    SetButtonNameForController(PadIndex, GamepadKey_DpadDown, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    SetButtonNameForController(PadIndex, GamepadKey_DpadLeft, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    SetButtonNameForController(PadIndex, GamepadKey_DpadRight, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
}

INTERNAL_FUNCTION void ProcessGamepads()
{
    for(int PadIndex = 0; PadIndex < MAX_GAMEPADS; PadIndex++)
    {
        gamepad_controller* Pad = &Global_Input->Gamepads[PadIndex];
        
        if(Pad->Connected)
        {
            // NOTE(Dima): Processing gamepad axes
            ProcessGamepadAxis(PadIndex, GamepadAxis_LeftX, SDL_CONTROLLER_AXIS_LEFTX);
            ProcessGamepadAxis(PadIndex, GamepadAxis_LeftY, SDL_CONTROLLER_AXIS_LEFTY);
            ProcessGamepadAxis(PadIndex, GamepadAxis_RightX, SDL_CONTROLLER_AXIS_RIGHTX);
            ProcessGamepadAxis(PadIndex, GamepadAxis_RightY, SDL_CONTROLLER_AXIS_RIGHTY);
            ProcessGamepadAxis(PadIndex, GamepadAxis_TriggerLeft, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
            ProcessGamepadAxis(PadIndex, GamepadAxis_TriggerRight, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
            
            // NOTE(Dima): Processing gampad buttons
            ProcessGamepadButton(PadIndex, GamepadKey_A, SDL_CONTROLLER_BUTTON_A);
            ProcessGamepadButton(PadIndex, GamepadKey_B, SDL_CONTROLLER_BUTTON_B);
            ProcessGamepadButton(PadIndex, GamepadKey_X, SDL_CONTROLLER_BUTTON_X);
            ProcessGamepadButton(PadIndex, GamepadKey_Y, SDL_CONTROLLER_BUTTON_Y);
            
            ProcessGamepadButton(PadIndex, GamepadKey_Back, SDL_CONTROLLER_BUTTON_BACK);
            ProcessGamepadButton(PadIndex, GamepadKey_Guide, SDL_CONTROLLER_BUTTON_GUIDE);
            ProcessGamepadButton(PadIndex, GamepadKey_Start, SDL_CONTROLLER_BUTTON_START);
            
            ProcessGamepadButton(PadIndex, GamepadKey_LeftStick, SDL_CONTROLLER_BUTTON_LEFTSTICK);
            ProcessGamepadButton(PadIndex, GamepadKey_RightStick, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
            ProcessGamepadButton(PadIndex, GamepadKey_LeftShoulder, SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
            ProcessGamepadButton(PadIndex, GamepadKey_RightShoulder, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
            
            ProcessGamepadButton(PadIndex, GamepadKey_DpadUp, SDL_CONTROLLER_BUTTON_DPAD_UP);
            ProcessGamepadButton(PadIndex, GamepadKey_DpadDown, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
            ProcessGamepadButton(PadIndex, GamepadKey_DpadLeft, SDL_CONTROLLER_BUTTON_DPAD_LEFT);
            ProcessGamepadButton(PadIndex, GamepadKey_DpadRight, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
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

INTERNAL_FUNCTION void CheckGamepadConnections()
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
                        
                        SetButtonNamesForGameController(PadIndex);
                        
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

INTERNAL_FUNCTION void CheckGamepadDisconnect()
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
    App->GamepadsCheckCounter += Global_Input->DeltaTime;
    if(App->GamepadsCheckCounter >= 1.5f)
    {
        //SDL_Log("Checking gamepad connections and disconnections\n");
        
        CheckGamepadConnections();
        CheckGamepadDisconnect();
        
        App->GamepadsCheckCounter = 0.0f;
    }
    
    // NOTE(Dima): Getting window dimensions
    SDL_GetWindowSize(App->Window, 
                      &App->WndDims.Width, 
                      &App->WndDims.Height);
    
    // NOTE(Dima): Processing input
    PreProcessInput();
    ProcessEvents();
    ProcessMouse();
    ProcessGamepads();
    ProcessVirtualButtons();
}

void SetGlobalVariables(game* Game)
{
    Global_Input = Game->Input;
    Global_Time = Game->Time;
    Global_DebugTable = Game->DebugTable;
    Global_Jobs = Game->JobSystem;
}

#if defined(PLATFORM_IS_WINDOWS)

b32 GetLastWriteTimeToGameDLL(char* DllFullPath, FILETIME* ResultTime)
{
    // NOTE(Dima): Getting full file name
    HANDLE FileHandle = CreateFile(DllFullPath, GENERIC_READ, 
                                   FILE_SHARE_READ, NULL,
                                   OPEN_EXISTING, 0, 0);
    
    FILETIME TimeCreate, TimeAccess, TimeWrite;
    b32 Result = false;
    if(FileHandle != INVALID_HANDLE_VALUE)
    {
        Result = true;
        
        GetFileTime(FileHandle, 
                    &TimeCreate,
                    &TimeAccess,
                    ResultTime);
        
        CloseHandle(FileHandle);
    }
    
    return(Result);
}

void LoadGameAndFunctionsFromDll()
{
    // NOTE(Dima): Copy the DLL
    CopyFile(App->GameDllFullPath, 
             App->TempDllFullPath,
             false);
    
    SDL_Log("Game Code DLL: %s", App->TempDllFullPath);
    HMODULE LibHandle = LoadLibraryA(App->TempDllFullPath);
    Assert(LibHandle);
    
    App->GameUpdateAndRender = (game_update_and_render*)GetProcAddress(LibHandle, "GameUpdateAndRender");
    App->GameInit = (game_init*)GetProcAddress(LibHandle, "GameInit");
    
    Assert(App->GameUpdateAndRender);
    Assert(App->GameInit);
    
    App->TempDllHandle = LibHandle;
}
#endif

INTERNAL_FUNCTION b32 RealoadIfNeededGameDLL()
{
    b32 Result = false;
    
#if defined(PLATFORM_IS_WINDOWS)
    FILETIME LastSaved = App->LastGameDllWriteTime;
    FILETIME LastWriteTime;
    
    // NOTE(Dima): If get time on DLL file was success - try to compare times
    if(GetLastWriteTimeToGameDLL(App->GameDllFullPath, &LastWriteTime))
    {
        if(CompareFileTime(&LastWriteTime, &LastSaved) > 0)
        {
            // TODO(Dima): Here I can place code to finish work before DLL is reloaded
            
            
            // NOTE(Dima): Freing current loaded DLL
            BOOL FreeResult = FreeLibrary(App->TempDllHandle);
            Assert(FreeResult);
            
            // NOTE(Dima): Reloading functions
            LoadGameAndFunctionsFromDll();
            
            // NOTE(Dima): Setting new saved DLL last write time
            App->LastGameDllWriteTime = LastWriteTime;
            
            Result = true;
        }
    }
#else
    
#endif
    
    return(Result);
}

INTERNAL_FUNCTION void InitGameCodeFromDll()
{
#if defined(PLATFORM_IS_WINDOWS)
    GetFullPathNameA("../Build/Flower.dll",
                     ArrayCount(App->GameDllFullPath),
                     App->GameDllFullPath, 0);
    
    GetFullPathNameA("../Build/FlowerTemp.dll",
                     ArrayCount(App->TempDllFullPath),
                     App->TempDllFullPath, 0);
    
    b32 GetTimeResult = GetLastWriteTimeToGameDLL(App->GameDllFullPath, &App->LastGameDllWriteTime);
    Assert(GetTimeResult);
    
    LoadGameAndFunctionsFromDll();
#else
    // TODO(Dima): Add support for other platforms that may want dynamic code reloading here
    App->GameUpdateAndRender = GameUpdateAndRender;
    App->GameInit = GameInit;
#endif
}

int main(int ArgsCount, char** Args)
{
    int SDLInitCode = SDL_Init(SDL_INIT_EVERYTHING);
    
    Platform = {};
    Platform.AllocateBlock = AppAllocBlock;
    Platform.DeallocateBlock = AppDeallocBlock;
    Platform.ProcessInput = ProcessInput;
    Platform.SetCapturingMouse = SetCapturingMouse;
    Platform.Render = OpenGLRender;
    Platform.SwapBuffers = OpenGLSwapBuffers;
    Platform.ReadFileAndNullTerminate = StandardReadFileAndNullTerminate;
    Platform.ReadFile = StandardReadFile;
    Platform.AllocateMemory = StandardAllocateMemory;
    Platform.FreeMemory = StandardFreeMemory;
    Platform.GetThreadID = SDLGetThreadID;
    Platform.GetPerfCounter = SDLGetPerfCounter;
    Platform.GetElapsedTime = SDLGetElapsedTime;
    Platform.PerfFrequency = SDL_GetPerformanceFrequency();
    Platform.OneOverPerfFrequency = 1.0 / (f64)SDL_GetPerformanceFrequency();
    Platform.Log = SDLOutputLog;
    
    memory_arena GameArena = {};
    App = PushStruct(&GameArena, app_state);
    
    game* Game = PushStruct(&GameArena, game);
    InitGameCodeFromDll();
    
    // NOTE(Dima): Init job system
    int QueuesThreadCounts[] = {2, 10};
    int QueuesJobCounts[] = {DEFAULT_JOBS_COUNT, DEFAULT_JOBS_COUNT};
    Game->JobSystem = InitJobSystem(&GameArena, QueuesThreadCounts, QueuesJobCounts);
    
#if 1
    App->WndDims.InitWidth = 1920;
    App->WndDims.InitHeight = 1080;
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
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 16);
    
    App->Window = SDL_CreateWindow("Flower",
                                   SDL_WINDOWPOS_UNDEFINED,
                                   SDL_WINDOWPOS_UNDEFINED,
                                   App->WndDims.InitWidth,
                                   App->WndDims.InitHeight,
                                   SDL_WINDOW_OPENGL);
    
    // NOTE(Dima): Init game
    App->GameInit(Game, &GameArena, &Platform, App->WndDims);
    
    // NOTE(Dima): Setting up global variables after game init
    SetGlobalVariables(Game);
    
    
#if 0    
    random_generation Random = SeedRandom(1234);
    for(int i = 0; i < 100; i++)
    {
        SDL_Log("%u\n", RandomBetweenUint(&Random, 0, 10));
    }
#endif
    
    // NOTE(Dima): Do not track mouse outside window
    SDL_CaptureMouse(SDL_FALSE);
    SDL_SetRelativeMouseMode(Global_Input->CapturingMouse ? SDL_TRUE : SDL_FALSE);
    
    App->OpenGLContext = SDL_GL_CreateContext(App->Window);
    
    OpenGLInit(Game->RenderCommands, &GameArena);
    
    Global_Time->Time = 0.0f;
    Global_Time->DeltaTime = 0.001f;
    u64 LastClocks = SDL_GetPerformanceCounter();
    
    App->Running = true;
    while(App->Running)
    {
        // NOTE(Dima): Checking if need to reload game code here.
        // It's important to do it before FRAME_BARRIER, so that only one frame 
        // records are invalid.
        b32 DllWasJustReloaded = RealoadIfNeededGameDLL();
        
        FRAME_BARRIER(Global_Time->DeltaTime);
        
        BEGIN_TIMING(FRAME_UPDATE_NODE_NAME);
        
        // NOTE(Dima): Processing time
        u64 NewClocks = SDL_GetPerformanceCounter();
        u64 ClocksElapsed = NewClocks - LastClocks;
        LastClocks = NewClocks;
        
        Global_Time->DeltaTime = (f32)((f64)ClocksElapsed * Platform.OneOverPerfFrequency); 
        if(Global_Time->DeltaTime < 0.000001f)
        {
            Global_Time->DeltaTime = 0.000001f;
        }
        Global_Time->Time += Global_Time->DeltaTime;
        Global_Time->SinTime = Sin(Global_Time->Time);
        Global_Time->CosTime = Cos(Global_Time->Time);
        Global_Input->Time = Global_Time->Time;
        Global_Input->DeltaTime = Global_Time->DeltaTime;
        
        // NOTE(Dima): Tell the game that it's code has just been reloaded
        Game->ShouldReloadGameCode = DllWasJustReloaded;
        
        // NOTE(Dima): Updating a game
        Game->WindowDimensions = App->WndDims;
        App->GameUpdateAndRender(Game);
        
        END_TIMING();
    }
    
    OpenGLFree(Game->RenderCommands);
    
    SDL_GL_DeleteContext(App->OpenGLContext);
    SDL_DestroyWindow(App->Window);
    
    FreeArena(&GameArena);
    
    return(0);
}
