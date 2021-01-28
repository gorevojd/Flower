#ifndef FLOWER_SCENE_H
#define FLOWER_SCENE_H

#define SCENE_INIT(name) void name##_Init(struct scene* Scene)
typedef void scene_init(struct scene* Scene);

#define SCENE_UPDATE(name) void name##_Update(struct scene* Scene)
typedef void scene_update(struct scene* Scene);

#define SCENE_ONGUI(name) void name##_OnGUI(struct scene* Scene)
typedef void scene_ongui(struct scene* Scene);

struct scene
{
    char Name[64];
    
    scene_init* Init_;
    scene_update* Update_;
    scene_ongui* OnGUI_;
    
    memory_arena* Arena;
    struct game* Game;
    
    void* SceneState;
    b32 Initialized;
    
    void Init()
    {
        if(Init_)
        {
            Init_(this);
        }
    }
    
    void Update()
    {
        FUNCTION_TIMING();
        
        if(!Initialized)
        {
            Init();
            
            Initialized = true;
        }
        
        if(Update_)
        {
            Update_(this);
        }
    }
    
    void OnGUI()
    {
        if(OnGUI_)
        {
            OnGUI_(this);
        }
    }
};

#endif //FLOWER_SCENE_H
