/*
// NOTE(Dima): What we've done in this episode:
  
- render commands buffer
- memory arenas

*/

#include <iostream>

#include <SDL.h>

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
#include "flower_asset.h"
#include "flower_opengl.h"
#include "flower_render.h"

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

struct render_params
{
    int WindowWidth;
    int WindowHeight;
    
    m44* View;
    m44* Projection;
    
    mesh* Cube;
    image* CubeTexture;
    
    f64 Time;
    
    render_commands* Commands;
};

#include "flower_input.cpp"
#include "flower_gameplay.cpp"
#include "flower_asset.cpp"
#include "flower_render.cpp"
#include "flower_opengl.cpp"

struct app_state
{
    bool Running;
    
    int WindowWidth;
    int WindowHeight;
    
    f64 Time;
    f32 DeltaTime;
    u64 PerformanceFrequency;
    
    SDL_Window* Window;
    b32 IsFullscreen;
    
    SDL_GameController* Gamepads[4];
    int GamepadCount;
    
    SDL_GLContext OpenGLContext;
};

GLOBAL_VARIABLE app_state* App;
GLOBAL_VARIABLE input_system* Input;
GLOBAL_VARIABLE render_commands RenderCommands;


GLOBAL_VARIABLE game_camera Camera;
GLOBAL_VARIABLE memory_arena GlobalArena;

INTERNAL_FUNCTION void SetFullscreen(b32 Fullscreen)
{
    u32 Flags = 0;
    
    if(Fullscreen)
    {
        Flags |= SDL_WINDOW_FULLSCREEN;
    }
    
    SDL_SetWindowFullscreen(App->Window, Flags);
    SDL_ShowCursor(!Fullscreen);
    
    App->IsFullscreen = Fullscreen;
}

inline void ProcessKeyState(key_state* Key, b32 TransitionHappened, b32 IsDown)
{
    if(TransitionHappened && (Key->EndedDown != IsDown))
    {
        Key->InTransitionTime = 0.0f;
        
        Key->EndedDown = IsDown;
        Key->TransitionHappened = TransitionHappened;
    }
}

INTERNAL_FUNCTION void PreProcessInput(input_system* Input)
{
    // NOTE(Dima): Preprocess keyboard
    keyboard_controller* Keyboard = &Input->Keyboard;
    
    for(int KeyIndex = 0; KeyIndex < Key_Count; KeyIndex++)
    {
        key_state* Key = &Keyboard->KeyStates[KeyIndex];
        
        Key->TransitionHappened = false;
        Key->InTransitionTime += Input->AppDeltaTime;
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
            button_state* But = &Control->Buttons[ButtonIndex];
            
            key_state* Key = &But->PressState;
            
            Key->TransitionHappened = false;
            Key->InTransitionTime += Input->AppDeltaTime;
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

INTERNAL_FUNCTION void ProcessInput(input_system* Input)
{
    PreProcessInput(Input);
    
    ProcessEvents(Input);
    
    ProcessMouse(Input);
    ProcessVirtualButtons(Input);
}

int main(int ArgsCount, char** Args)
{
    int SDLInitCode = SDL_Init(SDL_INIT_EVERYTHING);
    
    PlatformAllocateBlock = AppAllocBlock;
    PlatformDeallocateBlock = AppDeallocBlock;
    
    App = PushStruct(&GlobalArena, app_state);
    
    InitInput(&GlobalArena);
    
    image BoxTexture = LoadImageFile("../Data/Textures/container_diffuse.png");
    mesh Cube = MakeUnitCube();
    
#if 0    
    App->WindowWidth = 1920;
    App->WindowHeight = 1080;
#else
    App->WindowWidth = 1366;
    App->WindowHeight = 768;
#endif
    
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
                                   App->WindowWidth,
                                   App->WindowHeight,
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
    Camera.P = V3(0.0f, 0.0f, 5.0f);
    quat InitRot = LookRotation(V3_Back(), V3_Up());
    Camera.EulerAngles = Quat2Euler(InitRot);
    
    App->GamepadCount = 0;
    for(int JoystickIndex = 0;
        JoystickIndex < SDL_NumJoysticks(); 
        JoystickIndex++)
    {
        if(SDL_IsGameController(JoystickIndex))
        {
            SDL_Log("Detected gamepad with joystick index: %d\n", JoystickIndex);
            
            SDL_GameController* Controller = SDL_GameControllerOpen(JoystickIndex);
            if(Controller)
            {
                App->Gamepads[App->GamepadCount++] = Controller;
            }
            else
            {
                SDL_Log("Could not open Game Controller %i\n", JoystickIndex);
            }
            
            if(App->GamepadCount >= ArrayCount(App->Gamepads))
            {
                SDL_Log("Could not open any more Gamepads\n");
                
                break;
            }
        }
        else{
            SDL_Log("Joystick %d is not a gamepad: %d\n", JoystickIndex);
        }
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
        
        // NOTE(Dima): Process camera movement
        
        f32 HorzMove = GetAxis(Axis_Horizontal, 0);
        f32 VertMove = GetAxis(Axis_Vertical, 0);
        
        v3 Moves = V3(HorzMove, 0.0f, VertMove);
        
        float SpeedMultiplier = 1.0f;
        if(GetKey(Key_LeftShift))
        {
            SpeedMultiplier *= 5.0f;
        }
        if(GetKey(Key_Space))
        {
            SpeedMultiplier *= 5.0f;
        }
        
        v3 MoveVector = NOZ(V3(Moves.x, 0.0f, Moves.z)) * GlobalInput->AppDeltaTime;
        
        if(GlobalInput->CapturingMouse)
        {
            f32 MouseDeltaX = GetAxis(Axis_MouseX);
            f32 MouseDeltaY = GetAxis(Axis_MouseY);
            
            UpdateCameraRotation(&Camera, MouseDeltaY, MouseDeltaX, 0.0f);
        }
        
        float CameraSpeed = 5.0f * SpeedMultiplier;
        MoveVector = CameraSpeed * MoveVector * Camera.Transform;
        
        Camera.P += MoveVector;
        
        m44 View = GetViewMatrix(&Camera);
        m44 Projection = PerspectiveProjection(App->WindowWidth, 
                                               App->WindowHeight,
                                               500.0f, 0.5f);
        
        
        v3 Color00 = V3(1.0f, 1.0f, 1.0f);
        v3 Color01 = V3(1.0f, 0.0f, 0.0f);
        v3 Color10 = V3(1.0f, 1.0f, 0.0f);
        v3 Color11 = V3(0.3f, 0.1f, 0.9f);
        
        float CubeSpacing = 4.0f;
        
        int SideLen = 20;
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
                
                PushMesh(&RenderCommands, &Cube, P, VertColor);
            }
        }
        
        render_params RenderParams = {};
        
        RenderParams.WindowWidth = App->WindowWidth;
        RenderParams.WindowHeight = App->WindowHeight;
        RenderParams.View = &View;
        RenderParams.Projection = &Projection;
        RenderParams.Cube = &Cube;
        RenderParams.CubeTexture = &BoxTexture;
        RenderParams.Time = App->Time;
        RenderParams.Commands = &RenderCommands;
        
        BeginRender(&RenderCommands);
        
        OpenGLRender(&RenderParams);
        
        EndRender(&RenderCommands);
        
        SDL_GL_SwapWindow(App->Window);
    }
    
    OpenGLFree();
    
    SDL_GL_DeleteContext(App->OpenGLContext);
    SDL_DestroyWindow(App->Window);
    
    FreeArena(&GlobalArena);
    
    return(0);
}
