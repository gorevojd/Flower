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
    
    scene_init* Init;
    scene_update* Update;
    scene_ongui* OnGUI;
    
    memory_arena* Arena;
    struct game* Game;
    
    void* SceneState;
    b32 Initialized;
};

#endif //FLOWER_SCENE_H
