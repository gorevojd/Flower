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
    
    f32 Factor = Clamp01((Global_Time->Time - TimeStart) / FadeDuration);
    f32 t = Smoothstep(Factor);
    f32 PongTime = PingPong(Global_Time->Time / 4.0f, 1.0f);
    
#if 0        
    AddGraphMoveVector(V2(-Moves.x, Moves.z));
    LerpGraph(PongTime, false);
    
    f32 ValueMin = -0.5f;
    f32 ValueMax = 1.5f;
    ClampGraph(ValueMin + Global_Input->MouseUV.x * ((f32)ValueMax - (f32)ValueMin), 
               ValueMin, ValueMax, 
               0, 1);
    LerpGraphPositions(t);
    LerpGraphColors(t, V4(1.0f, 0.3f, 0.8f, 1.0f), V4(0.2f, 0.8f, 0.5f, 1.0f));
    
    f32 GraphValueMagnitude = 3.8f;
    f32 PassValue = -GraphValueMagnitude + Global_Input->MouseUV.x * 2.0f * GraphValueMagnitude;
    RoundingGraph(std::round, "std::round", PassValue);
    
    UniBiMappingRangeGraph(PongTime);
    
    RandomNumberGraph();
    
    GraphSamplesInQuad(Scene->Arena);
    GraphSamplesInCircle(Scene->Arena);
    GraphNoiseSamples(Scene->Arena, &State->BlueNoise[0], State->BlueNoise.size());
    GraphRadioactiveSource();
#endif
    
    GraphCoinsThrow(Scene->Arena,
                    &Global_Assets->CoinHead, 
                    &Global_Assets->CoinTail);
}
