/*
// NOTE(Dima): What we've done in this episode:

--- Imported Assimp library
--- Used Assimp to load model
--- Loaded a few textures. And applied them on models via materials.
--- Triangles, Quadrilaterals, Polygons support in UI rendering
--- Some math naming changes
--- Material assignment on loaded models
--- Added triangle, circle and lines support to the renderers

//TODO(Dima):
--- Simple particle system
*/

#include <iostream>

#include <SDL.h>
#include <vector>
#include <algorithm>

#include "flower_defines.h"

#define PLATFORM_ALLOCATE_BLOCK(name) struct memory_block* name(u32 Size)
typedef PLATFORM_ALLOCATE_BLOCK(platform_allocate_block);

#define PLATFORM_DEALLOCATE_BLOCK(name) void name(memory_block* Block)
typedef PLATFORM_DEALLOCATE_BLOCK(platform_deallocate_block);

GLOBAL_VARIABLE platform_allocate_block* PlatformAllocateBlock;
GLOBAL_VARIABLE platform_deallocate_block* PlatformDeallocateBlock;

#include "flower_math.h"
#include "flower_strings.h"
#include "flower_mem.h"

#include "flower_input.h"
#include "flower_render_primitives.h"
#include "flower_asset.h"
#include "flower_opengl.h"
#include "flower_render.h"
#include "flower_ui.h"

#define STB_SPRINTF_IMPLEMENTATION
#include "stb_sprintf.h"

GLOBAL_VARIABLE input_system* GlobalInput;
GLOBAL_VARIABLE asset_system* GlobalAssetSystem;
GLOBAL_VARIABLE ui_state* GlobalUI;

#include "flower_input.cpp"
#include "flower_gameplay.cpp"
#include "flower_asset.cpp"
#include "flower_render.cpp"
#include "flower_opengl.cpp"
#include "flower_ui.cpp"

#include "flower_world.cpp"

struct app_state
{
    bool Running;
    
    window_dimensions WndDims; 
    
    f64 Time;
    f32 DeltaTime;
    u64 PerformanceFrequency;
    
    SDL_Window* Window;
    b32 IsFullscreen;
    
    SDL_GameController* Gamepads[MAX_GAMEPADS];
    f32 GamepadsCheckCounter;
    
    SDL_GLContext OpenGLContext;
};

struct game_state
{
    game_camera Camera;
    
    world* World;
};

GLOBAL_VARIABLE app_state* App;
GLOBAL_VARIABLE render_commands RenderCommands;

GLOBAL_VARIABLE memory_arena GlobalArena;

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
    Key->InTransitionTime += Input->AppDeltaTime;
}

INTERNAL_FUNCTION void PreProcessInput(input_system* Input)
{
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

INTERNAL_FUNCTION void ProcessInput(input_system* Input)
{
    App->GamepadsCheckCounter += App->DeltaTime;
    if(App->GamepadsCheckCounter >= 1.5f)
    {
        //SDL_Log("Checking gamepad connections and disconnections\n");
        
        CheckGamepadConnections(Input);
        CheckGamepadDisconnect(Input);
        
        App->GamepadsCheckCounter = 0.0f;
    }
    
    PreProcessInput(Input);
    
    ProcessEvents(Input);
    ProcessMouse(Input);
    ProcessGamepads(Input);
    ProcessVirtualButtons(Input);
}

// TODO(Dima): Remove this from here
void RenderModel(render_commands* Commands, 
                 loaded_model* Model,
                 v3 P)
{
    m44 ModelToWorld = TranslationMatrix(P);
    
    for(int NodeIndex = 0;
        NodeIndex < Model->Nodes.size();
        NodeIndex++)
    {
        loaded_model_node* Node = &Model->Nodes[NodeIndex];
        
        v3 NodeOffsetFromModelOrigin = (V4(0.0f, 0.0f, 0.0f, 1.0f) * Node->ToModel).xyz;
        
        for(int MeshIndex = 0;
            MeshIndex < Node->MeshIndices.size();
            MeshIndex++)
        {
            int ActualMeshIndex = Node->MeshIndices[MeshIndex];
            
            v3 PushP = P + NodeOffsetFromModelOrigin;
            
            m44 NodeTran = Node->ToModel * ModelToWorld;
            
            loaded_model_mesh* Mesh = &Model->Meshes[ActualMeshIndex];
            
            PushMesh(Commands, 
                     &Mesh->Mesh,
                     Model->Materials[Mesh->MaterialIndex],
                     NodeTran);
        }
    }
}

int main(int ArgsCount, char** Args)
{
    int SDLInitCode = SDL_Init(SDL_INIT_EVERYTHING);
    
    PlatformAllocateBlock = AppAllocBlock;
    PlatformDeallocateBlock = AppDeallocBlock;
    
    App = PushStruct(&GlobalArena, app_state);
    game_state* Game = PushStruct(&GlobalArena, game_state);
    Game->World = CreateWorld(&GlobalArena);
    
    InitInput(&GlobalArena);
    InitAssetSystem(&GlobalArena);
    InitUI(&GlobalArena);
    
    mesh Cube = MakeUnitCube();
    mesh Plane = MakePlane();
    
    image BoxTexture = LoadImageFile("../Data/Textures/container_diffuse.png");
    image PlaneTexture = LoadImageFile("E:/Media/PixarTextures/png/ground/Red_gravel_pxr128.png");
    
    font FontTimes = LoadFontFile("C:/Windows/Fonts/times.ttf");
    font FontGoofy = LoadFontFile("../Data/Fonts/Life is goofy.ttf");
    font FontArial = LoadFontFile("c:/windows/fonts/arial.ttf");
    
    image Palette = LoadImageFile("E:/Development/Modeling/Pallette/MyPallette.png", true);
    
    loading_params BearParams = DefaultLoadingParams();
    BearParams.Model_DefaultScale = 0.01f;
    BearParams.Model_FixInvalidRotation = true;
    
    loading_params FoxParams = BearParams;
    
    image BearDiffuse = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Bear/Bear.tga");
    image BearNormal = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Bear/Bear Normals.tga");
    image BearEyesDiffuse = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Bear/Eye Bear.tga");
    image BearEyesShine = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Bear/Eye Shine Bear.tga");
    loaded_model Bear = LoadModelFromFile("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Bear/bear.FBX", BearParams);
    
    image FoxDiffuse = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Fox/Fox.tga");
    image FoxNormal = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Fox/Fox Normals.tga");
    image FoxEyesDiffuse = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Fox/Eye Green.tga");
    image FoxEyesShine = LoadImageFile("E:/Development/Modeling/3rdParty/ForestAnimals/Textures/Fox/Eye Shine.tga");
    loaded_model Fox = LoadModelFromFile("E:/Development/Modeling/3rdParty/ForestAnimals/FBX/Fox/Fox.fbx", FoxParams);
    
    loaded_model Supra = LoadModelFromFile("E:/Development/Modeling/Modeling challenge/ToyotaSupra/Supra.FBX");
    
    // NOTE(Dima): Bear materials
    material BearMaterial = {};
    BearMaterial.Diffuse = &BearDiffuse;
    
    material BearEyesMaterial = {};
    BearEyesMaterial.Diffuse = &BearEyesDiffuse;
    
    material BearEyesShineMaterial = {};
    BearEyesShineMaterial.Diffuse = &BearEyesShine;
    
    Bear.Materials.push_back(&BearMaterial);
    Bear.Materials.push_back(&BearEyesMaterial);
    Bear.Materials.push_back(&BearEyesShineMaterial);
    Bear.Meshes[1].MaterialIndex = 1;
    Bear.Meshes[2].MaterialIndex = 2;
    Bear.Meshes[3].MaterialIndex = 1;
    Bear.Meshes[4].MaterialIndex = 2;
    
    // NOTE(Dima): Fox materials
    material FoxMaterial = {};
    FoxMaterial.Diffuse = &FoxDiffuse;
    
    material FoxEyesMaterial = {};
    FoxEyesMaterial.Diffuse = &FoxEyesDiffuse;
    
    material FoxEyesShineMaterial = {};
    FoxEyesShineMaterial.Diffuse = &FoxEyesShine;
    
    Fox.Materials.push_back(&FoxMaterial);
    Fox.Materials.push_back(&FoxEyesMaterial);
    Fox.Materials.push_back(&FoxEyesShineMaterial);
    Fox.Meshes[1].MaterialIndex = 1;
    Fox.Meshes[2].MaterialIndex = 2;
    Fox.Meshes[3].MaterialIndex = 1;
    Fox.Meshes[4].MaterialIndex = 2;
    
    // NOTE(Dima): Other materials
    material SupraMaterial = {};
    SupraMaterial.Diffuse = &Palette;
    Supra.Materials.push_back(&SupraMaterial);
    
    material CubeMaterial = {};
    CubeMaterial.Diffuse = &BoxTexture;
    
    material PlaneMaterial = {};
    PlaneMaterial.Diffuse = &PlaneTexture;
    
    AddFontToAtlas(&FontTimes);
    AddFontToAtlas(&FontGoofy);
    AddFontToAtlas(&FontArial);
    
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
    SDL_SetRelativeMouseMode(GlobalInput->CapturingMouse ? SDL_TRUE : SDL_FALSE);
    
    App->OpenGLContext = SDL_GL_CreateContext(App->Window);
    
    OpenGLInit();
    
    App->PerformanceFrequency = SDL_GetPerformanceFrequency();
    App->Time = 0.0f;
    App->DeltaTime = 0.001f;
    u64 LastClocks = SDL_GetPerformanceCounter();
    
    // TODO(Dima): Fix init rotation bug
    Game->Camera.P = V3(0.0f, 0.0f, 5.0f);
    quat InitRot = LookRotation(V3_Back(), V3_Up());
    Game->Camera.EulerAngles = QuatToEuler(InitRot);
    
    // NOTE(Dima): Init gamepads
    for(int GamepadIndex = 0;
        GamepadIndex < MAX_GAMEPADS;
        GamepadIndex++)
    {
        gamepad_controller* Pad = &GlobalInput->Gamepads[GamepadIndex];
        
        Pad->Connected = false;
    }
    
    App->Running = true;
    while(App->Running)
    {
        u64 NewClocks = SDL_GetPerformanceCounter();
        u64 ClocksElapsed = NewClocks - LastClocks;
        LastClocks = NewClocks;
        
        App->DeltaTime = (f32)((f64)ClocksElapsed / (f64)App->PerformanceFrequency); 
        if(App->DeltaTime < 0.000001f)
        {
            App->DeltaTime = 0.000001f;
        }
        App->Time += App->DeltaTime;
        
        GlobalInput->AppDeltaTime = App->DeltaTime;
        GlobalInput->AppTime = App->Time;
        
        ProcessInput(GlobalInput);
        
        SDL_GetWindowSize(App->Window, 
                          &App->WndDims.Width, 
                          &App->WndDims.Height);
        
        // NOTE(Dima): Speed multiplyer
        float SpeedMultiplier = 1.0f;
        if(GetKey(Key_LeftShift))
        {
            SpeedMultiplier *= 5.0f;
        }
        if(GetKey(Key_Space))
        {
            SpeedMultiplier *= 5.0f;
        }
        
        // NOTE(Dima): Process movement
        f32 HorzMove = GetAxis(Axis_Horizontal);
        f32 VertMove = GetAxis(Axis_Vertical);
        
        v3 Moves = NOZ(V3(HorzMove, 0.0f, VertMove));
        
        v3 MoveVector = Moves * GlobalInput->AppDeltaTime;
        
        if(GlobalInput->CapturingMouse)
        {
            f32 MouseDeltaX = GetAxis(Axis_MouseX);
            f32 MouseDeltaY = GetAxis(Axis_MouseY);
            
            UpdateCameraRotation(&Game->Camera, MouseDeltaY, MouseDeltaX, 0.0f);
        }
        
        float CameraSpeed = 5.0f * SpeedMultiplier;
        v3 TransformedMoveVector = CameraSpeed * MoveVector * Game->Camera.Transform;
        
        if(GlobalInput->CapturingMouse)
        {
            Game->Camera.P += TransformedMoveVector;
        }
        
        m44 View = GetViewMatrix(&Game->Camera);
        m44 Projection = PerspectiveProjection(App->WndDims.Width, 
                                               App->WndDims.Height,
                                               500.0f, 0.5f);
        m44 ViewProjection = View * Projection;
        m44 ScreenOrthoProjection = OrthographicProjection(App->WndDims.Width, 
                                                           App->WndDims.Height);
        
        
#if 1
        LOCAL_PERSIST v3 CubePos;
        m44 CubeTransform = TranslationMatrix(CubePos + V3_Up());
        PushMesh(&RenderCommands, &Cube, &CubeMaterial, CubeTransform, ColorWhite().rgb);
        
        CubePos += MoveVector * 4.0f;
        
        PushMesh(&RenderCommands, 
                 &Plane, &PlaneMaterial, 
                 ScalingMatrix(20.0f), V3_One());
#endif
        
#if 0   
        // NOTE(Dima): Pushing cubes
        v3 Color00 = V3(1.0f, 1.0f, 1.0f);
        v3 Color01 = V3(1.0f, 0.0f, 0.0f);
        v3 Color10 = V3(1.0f, 1.0f, 0.0f);
        v3 Color11 = V3(0.3f, 0.1f, 0.9f);
        
        float CubeSpacing = 4.0f;
        
        CubeMaterial.Diffuse = 0;
        int SideLen = 10;
        for(int VerticalIndex = 0; VerticalIndex < SideLen; VerticalIndex++)
        {
            for(int HorizontalIndex = 0; HorizontalIndex < SideLen; HorizontalIndex++)
            {
                float HorzPercentage = (float)HorizontalIndex / (float)(SideLen - 1);
                float VertPercentage = (float)VerticalIndex / (float)(SideLen - 1);
                
                v3 ColorHorzBottom = Lerp(Color00, Color10, HorzPercentage);
                v3 ColorHorzTop = Lerp(Color01, Color11, HorzPercentage);
                v3 VertColor = Lerp(ColorHorzBottom, ColorHorzTop, VertPercentage);
                
                v3 P = V3((f32)HorizontalIndex * CubeSpacing, 
                          Sin(App->Time + (HorizontalIndex + VerticalIndex) * 12.0f), 
                          (f32)VerticalIndex * CubeSpacing);
                
                PushMesh(&RenderCommands, 
                         &Cube, 
                         &CubeMaterial, 
                         TranslationMatrix(P), 
                         VertColor);
            }
        }
#endif
        
#if 0
        PushImage(&RenderCommands, &BearDiffuse, V2(100, 100), 400);
        PushImage(&RenderCommands, &BearNormal, V2(500, 100), 400);
        PushImage(&RenderCommands, &Palette, V2(900, 100), 400);
        
        RenderModel(&RenderCommands, 
                    &Bear,
                    V3(0.0f, 0.0f, 0.0f));
        
        RenderModel(&RenderCommands, 
                    &Supra,
                    V3(5.0f, 0.0f, 0.0f));
        
        RenderModel(&RenderCommands, 
                    &Fox,
                    V3(-3.0f, 0.0f, 0.0f));
#endif
        
        //PushImage(&RenderCommands, &GlobalAssetSystem->FontsAtlas, V2(100, 100), 1200);
        
        // NOTE(Dima): Push UI
        char CharBuffer[64];
        stbsp_sprintf(CharBuffer, "Time: %.2f", App->Time);
        
        char FPSBuffer[64];
        stbsp_sprintf(FPSBuffer, "FPS: %.0f", 1.0f / App->DeltaTime);
        
        char FrameTimeBuffer[64];
        stbsp_sprintf(FrameTimeBuffer, "FrameTime ms: %.2f", App->DeltaTime * 1000.0f);
        
        ui_params ParamsUI = {};
        ParamsUI.Commands = &RenderCommands;
        ParamsUI.Font = &FontTimes;
        ParamsUI.Scale = 1.0f;
        
        SetParamsUI(ParamsUI);
        
        BeginLayout();
        Text("Hello Twitch and YouTube!");
        Text(CharBuffer);
        Text(FPSBuffer);
        Text(FrameTimeBuffer);
        EndLayout();
        
#if 1     
        ui_graph Graph = BeginGraph(V2(1200, 250), V2(-1, -1), V2(1, 1), 100);
        AddGraphCircle(&Graph, V2(0.0f, 0.0f), 1, V4(0.5f, 0.5f, 0.5f, 1.0f));
        AddGraphPoint(&Graph, V2(Moves.x, Moves.z), 8);
        
        char HorizontalBuffer[64];
        stbsp_sprintf(HorizontalBuffer, "Horizontal = %.1f", -HorzMove);
        
        char VerticalBuffer[64];
        stbsp_sprintf(VerticalBuffer, "Vertical = %.1f", VertMove);
        
        
        BeginLayout(V2(800, 100));
        Text(HorizontalBuffer);
        Text(VerticalBuffer);
        Text("Left", GetButton(Button_Left));
        Text("Right", GetButton(Button_Right));
        Text("Up", GetButton(Button_Up));
        Text("Down", GetButton(Button_Down));
        EndLayout();
#endif
        
#if 0  
        PushTriangle2D(&RenderCommands, V2(100, 100), V2(500, 500), V2(600, 100), V4(1.0f, 1.0f, 0.0f, 1.0f));
        PushQuadrilateral2D(&RenderCommands, 
                            V2(1000, 100), 
                            V2(1300, 50), 
                            V2(1350, 500),
                            V2(1050, 400),
                            V4(0.0f, 0.4f, 1.0f, 1.0f));
        
        PushCircle2D(&RenderCommands,
                     V2(500, 600),
                     100.0f + Sin(App->Time) * 20.0f,
                     V4(0.8f, 0.14f, 0.75f, 1.0f));
        
        PushCircleInternal2D(&RenderCommands,
                             V2(500, 600),
                             110 + Sin(App->Time) * 25.0f,
                             120.0f + Sin(App->Time) * 25.0f,
                             V4(0.8f, 0.14f, 0.75f, 1.0f));
        
        PushLine2D(&RenderCommands,
                   V2(100, 800),
                   V2(1500, 500),
                   4,
                   ColorBlue());
        
        PushArrow2D(&RenderCommands,
                    V2(1000 + Sin(App->Time * 2.0f) * 100.0f, 200),
                    V2(600, 800 + Sin(App->Time) * 100.0f),
                    4,
                    ColorBlue());
        
        PushArrow2D(&RenderCommands,
                    V2(50, 50),
                    V2(152, 150),
                    4,
                    ColorRed());
        
        for(int i = 0; i < 50; i++)
        {
            rc2 Rect = RectMinDim(V2(10 + Sin(App->Time + i) * 30.0f, 100 + i * 15),
                                  V2(100, 10));
            
            PushRect(&RenderCommands, 
                     Rect);
            
            PushRectOutline(&RenderCommands,
                            Rect, 2);
        }
        
        for(int i = 0; i < 50; i++)
        {
            rc2 Rect = RectMinDim(V2(210 + Sin(App->Time + i) * 30.0f, 100 + i * 15),
                                  V2(100, 10));
            
            PushRect(&RenderCommands, 
                     Rect,
                     ColorGreen());
            
            PushRectOutline(&RenderCommands,
                            Rect, 2);
        }
        
        for(int i = 0; i < 50; i++)
        {
            rc2 Rect = RectMinDim(V2(320 + Sin(App->Time + i) * 30.0f, 100 + i * 15),
                                  V2(100, 10));
            
            PushRect(&RenderCommands, 
                     Rect,
                     ColorRed());
            
            PushRectOutline(&RenderCommands,
                            Rect, 2);
        }
#endif
        
#if 1
#if 0    
        int TimeIndex = (int)(App->Time / 10.0f) - 1;
        
        switch(TimeIndex)
        {
            
            case 0:
            {
                PrintText3D(&RenderCommands, 
                            &FontArial, 
                            "Visit my twitch channel: twitch.tw/gorevojd",
                            V3_Left(), V3_Up(),
                            V3(0.0f, 4.0f, 0.0f));
                
            }break;
            
            case 1:
            {
                PrintText3D(&RenderCommands, 
                            &FontArial, 
                            "Thanks to my Patreon supporters! <3",
                            V3_Left(), V3_Up(),
                            V3(0.0f, 5.0f, 0.0f));
                
                PrintText3D(&RenderCommands, 
                            &FontTimes, 
                            "Vano97x",
                            V3_Left(), V3_Up(),
                            V3(0, 4, 0.0f),
                            V4(1.0f, 1.0f, 1.0f, 1.0f));
                
                PrintText3D(&RenderCommands, 
                            &FontTimes, 
                            "Despera",
                            V3_Left(), V3_Up(),
                            V3(0, 3, 0.0f),
                            V4(1.0f, 1.0f, 1.0f, 1.0f));
                
            }break;
            
            case 2:
            {
                PrintText3D(&RenderCommands, 
                            &FontTimes, 
                            "Hit like :)",
                            V3_Left(), V3_Up(),
                            V3(0, 4, 0.0f),
                            V4(1.0f, 1.0f, 1.0f, 1.0f));
                
                PrintText3D(&RenderCommands, 
                            &FontTimes, 
                            "And subscribe :)",
                            V3_Left(), V3_Up(),
                            V3(0, 3, 0.0f),
                            V4(1.0f, 1.0f, 1.0f, 1.0f));
            }break;
            
            case 3:
            {
                PrintText3D(&RenderCommands, 
                            &FontTimes, 
                            "Thanks for watching!",
                            V3_Left(), V3_Up(),
                            V3(0, 4, 0.0f),
                            V4(1.0f, 1.0f, 1.0f, 1.0f));
                
                PrintText3D(&RenderCommands, 
                            &FontTimes, 
                            "Goodbye!",
                            V3_Left(), V3_Up(),
                            V3(0, 3, 0.0f),
                            V4(1.0f, 1.0f, 1.0f, 1.0f));
            } break;
        }
#endif
        
#endif
        
        if(GetKeyOnGamepad(GamepadKey_DpadLeft))
        {
            
        }
        
#if 1
#if 0    
        int TimeIndex = (int)(App->Time / 10.0f) - 1;
        
        switch(TimeIndex)
        {
            
            case 0:
            {
                PrintText3D(&RenderCommands, 
                            &FontArial, 
                            "Visit my twitch channel: twitch.tw/gorevojd",
                            V3_Left(), V3_Up(),
                            V3(0.0f, 4.0f, 0.0f));
                
            }break;
            
            case 1:
            {
                PrintText3D(&RenderCommands, 
                            &FontArial, 
                            "Thanks to my Patreon supporters! <3",
                            V3_Left(), V3_Up(),
                            V3(0.0f, 5.0f, 0.0f));
                
                PrintText3D(&RenderCommands, 
                            &FontTimes, 
                            "Vano97x",
                            V3_Left(), V3_Up(),
                            V3(0, 4, 0.0f),
                            V4(1.0f, 1.0f, 1.0f, 1.0f));
                
                PrintText3D(&RenderCommands, 
                            &FontTimes, 
                            "Despera",
                            V3_Left(), V3_Up(),
                            V3(0, 3, 0.0f),
                            V4(1.0f, 1.0f, 1.0f, 1.0f));
                
            }break;
            
            case 2:
            {
                PrintText3D(&RenderCommands, 
                            &FontTimes, 
                            "Hit like :)",
                            V3_Left(), V3_Up(),
                            V3(0, 4, 0.0f),
                            V4(1.0f, 1.0f, 1.0f, 1.0f));
                
                PrintText3D(&RenderCommands, 
                            &FontTimes, 
                            "And subscribe :)",
                            V3_Left(), V3_Up(),
                            V3(0, 3, 0.0f),
                            V4(1.0f, 1.0f, 1.0f, 1.0f));
            }break;
            
            case 3:
            {
                PrintText3D(&RenderCommands, 
                            &FontTimes, 
                            "Thanks for watching!",
                            V3_Left(), V3_Up(),
                            V3(0, 4, 0.0f),
                            V4(1.0f, 1.0f, 1.0f, 1.0f));
                
                PrintText3D(&RenderCommands, 
                            &FontTimes, 
                            "Goodbye!",
                            V3_Left(), V3_Up(),
                            V3(0, 3, 0.0f),
                            V4(1.0f, 1.0f, 1.0f, 1.0f));
            } break;
        }
#endif
        
#endif
        
        
        // NOTE(Dima): Render everything
        render_params RenderParams = {};
        
        RenderParams.WndDims = App->WndDims;
        RenderParams.View = &View;
        RenderParams.ViewProjection = &ViewProjection;
        RenderParams.Projection = &Projection;
        RenderParams.ScreenOrthoProjection = &ScreenOrthoProjection;
        RenderParams.Cube = &Cube;
        RenderParams.CubeTexture = &BoxTexture;
        RenderParams.Time = App->Time;
        RenderParams.Commands = &RenderCommands;
        RenderParams.FontAtlas = &GlobalAssetSystem->FontsAtlas;
        
        BeginRender(&RenderParams);
        
        OpenGLRender(&RenderParams);
        
        EndRender(&RenderParams);
        
        SDL_GL_SwapWindow(App->Window);
    }
    
    OpenGLFree();
    
    SDL_GL_DeleteContext(App->OpenGLContext);
    SDL_DestroyWindow(App->Window);
    
    FreeArena(&GlobalArena);
    
    return(0);
}
