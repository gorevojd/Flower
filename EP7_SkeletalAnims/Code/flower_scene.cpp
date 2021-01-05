INTERNAL_FUNCTION inline void* InitSceneState(scene* Scene, mi StateSize)
{
    if(!Scene->SceneState)
    {
        Scene->SceneState = AllocateFromArena(Scene->Arena, StateSize);
    }
    
    void* Result = Scene->SceneState;
    
    return(Result);
}

#define GetSceneState(state_name) (state_name*)InitSceneState(Scene, sizeof(state_name)) 
