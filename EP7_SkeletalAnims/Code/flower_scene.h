#ifndef FLOWER_SCENE_H
#define FLOWER_SCENE_H

#define SCENE_INIT(name) void name(struct scene* Scene)
typedef SCENE_INIT(scene_init);

#define SCENE_UPDATE(name) void name(struct scene* Scene)
typedef SCENE_UPDATE(scene_update);

struct scene
{
    char Name[64];
    
    scene_init* Init_;
    scene_update* Update_;
    
    memory_arena* Arena;
    
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
