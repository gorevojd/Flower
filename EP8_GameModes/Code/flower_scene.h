#ifndef FLOWER_SCENE_H
#define FLOWER_SCENE_H

#define SCENE_INIT(name) void name##Init(struct scene* Scene)
typedef void scene_init(struct scene* Scene);

#define SCENE_UPDATE(name) void name##Update(struct scene* Scene)
typedef void scene_update(struct scene* Scene);

struct scene
{
    char Name[64];
    
    scene_init* Init_;
    scene_update* Update_;
    
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
};

#endif //FLOWER_SCENE_H
