#include "flower_ui_graphs.cpp"

struct ui_graphs_state
{
    std::vector<v2> BlueNoise;
};

SCENE_INIT(GraphShow)
{
    ui_graphs_state* State = GetSceneState(ui_graphs_state);
    
    State->BlueNoise = GenerateBlueNoise(V2(1.0f), 0.1f, 126);
}


SCENE_UPDATE(GraphShow)
{
    ui_graphs_state* State = GetSceneState(ui_graphs_state);
    
    static f32 TimeStart = 5.0f;
    f32 FadeDuration = 7.0f;
    
    if(GetKeyDown(Key_R))
    {
        TimeStart = 9999999.0f;
    }
    
    if(GetKeyDown(Key_Space))
    {
        TimeStart = Global_Time->Time;
    }
    
#if 0
    if(GetKeyDown(Key_I))
    {
        u64 StartClocks = Platform.GetPerfCounter();
        InvertImageColors(&Global_Assets->Scenery);
        u64 EndClocks= Platform.GetPerfCounter();
        
        f64 ElapsedTime = Platform.GetElapsedTime(StartClocks, EndClocks);
        printf("%.4fms\n", ElapsedTime * 1000.0f);
    }
    
    PushImage(&Global_Assets->Scenery, V2(0.0f, 0.0f), 900);
#endif
}
