#include <iostream>

#include <SDL.h>

#include "flower_defines.h"
#include "flower_math.h"

#include "flower_opengl.h"
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
    
    SDL_GLContext OpenGLContext;
};

GlobalVariable app_state App;

int main(int ArgsCount, char** Args)
{
    int SDLInitCode = SDL_Init(SDL_INIT_EVERYTHING);
    
    App.WindowWidth = 1920;
    App.WindowHeight = 1080;
    
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    App.Window = SDL_CreateWindow("Flower",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  App.WindowWidth,
                                  App.WindowHeight,
                                  SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);
    
    App.OpenGLContext = SDL_GL_CreateContext(App.Window);
    
    OpenGLInit();
    
    App.PerformanceFrequency = SDL_GetPerformanceFrequency();
    App.Time = 0.0f;
    App.DeltaTime = 0.0001f;
    u64 LastClocks = SDL_GetPerformanceCounter();
    
    App.Running = true;
    while(App.Running)
    {
        
        u64 NewClocks = SDL_GetPerformanceCounter();
        u64 ClocksElapsed = NewClocks - LastClocks;
        LastClocks = NewClocks;
        
        App.DeltaTime = (f32)((f64)ClocksElapsed / (f64)App.PerformanceFrequency); 
        App.Time += App.DeltaTime;
        
        SDL_Event Event;
        while(SDL_PollEvent(&Event))
        {
            switch(Event.type)
            {
                case SDL_KEYUP:
                case SDL_KEYDOWN:
                {
                    SDL_KeyboardEvent* KeyEvent = &Event.key;
                }break;
                
                case(SDL_WINDOWEVENT): {
                    SDL_WindowEvent* WindowEvent = &Event.window;
                    switch (WindowEvent->event) {
                        /*Close is sent to window*/
                        case(SDL_WINDOWEVENT_CLOSE): {
                            App.Running = false;
                        }break;
                        
						/*Mouse entered window*/
                        case SDL_WINDOWEVENT_ENTER: {
                            
                        }break;
                        
						/*Mouse leaved window*/
                        case SDL_WINDOWEVENT_LEAVE: {
                            
                        }break;
                    };
                }break;
                
                case(SDL_MOUSEBUTTONUP):
                case(SDL_MOUSEBUTTONDOWN): 
                {
                    
                }break;
                
                default:
                {
                    
                }break;
            }
        }
        
        OpenGLRender(App.WindowWidth, App.WindowHeight, App.Time);
        
        SDL_GL_SwapWindow(App.Window);
    }
    
    OpenGLFree();
    
    SDL_GL_DeleteContext(App.OpenGLContext);
    SDL_DestroyWindow(App.Window);
    
    return(0);
}
