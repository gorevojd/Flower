struct nearest_frames
{
    int Prev;
    int Next;
    
    f32 Percentage;
};

INTERNAL_FUNCTION nearest_frames FindNearestFrames(f32* Times, int TimesCount,
                                                   f32 CurrentTick,
                                                   f32 AnimDurationTicks,
                                                   u32 Behaviour)
{
    nearest_frames Result = {};
    
    if(TimesCount)
    {
        int FirstFrameIndex = 0;
        int LastFrameIndex = TimesCount - 1;
        
        f32 FirstFrameTime = Times[FirstFrameIndex];
        f32 LastFrameTime = Times[LastFrameIndex];
        
        if(CurrentTick < FirstFrameTime)
        {
            // NOTE(Dima): Pre-behaviour case
            switch(Behaviour)
            {
                case AnimBehaviour_Closest:
                {
                    Result.Prev = FirstFrameIndex;
                    Result.Next = FirstFrameIndex;
                }break;
                
                case AnimBehaviour_Repeat:
                {
                    Result.Prev = LastFrameIndex;
                    Result.Next = FirstFrameIndex;
                    
                    f32 A = AnimDurationTicks - LastFrameTime;
                    f32 B = FirstFrameTime;
                    f32 C = CurrentTick;
                    
                    Result.Percentage = (A + C) / (A + B);
                }break;
            }
        }
        
        else if(CurrentTick > LastFrameTime)
        {
            // NOTE(Dima): Post-behaviour case
            switch(Behaviour)
            {
                case AnimBehaviour_Closest:
                {
                    Result.Prev = LastFrameIndex;
                    Result.Next = LastFrameIndex;
                }break;
                
                case AnimBehaviour_Repeat:
                {
                    Result.Prev = LastFrameIndex;
                    Result.Next = FirstFrameIndex;
                    
                    f32 A = AnimDurationTicks - LastFrameTime;
                    f32 B = FirstFrameTime;
                    f32 C = CurrentTick - LastFrameTime;
                    
                    Result.Percentage = C / (A + B);
                }break;
            }
        }
        
        else
        {
            for(int TimeIndex = 0; TimeIndex < TimesCount; TimeIndex++)
            {
                if(Times[TimeIndex] > CurrentTick)
                {
                    Result.Next = TimeIndex;
                    Result.Prev = TimeIndex - 1;
                    
                    f32 A = CurrentTick - Times[Result.Prev];
                    f32 B = Times[Result.Next] - Times[Result.Prev];
                    
                    Result.Percentage = A / B;
                    
                    break;
                }
            }
        }
        
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline v3 InterpolateVectorKeys(nearest_frames* Nearest,
                                                  v3* Values,
                                                  int ValuesCount)
{
    v3 Result = Values[Nearest->Prev];
    
    if(Nearest->Prev != Nearest->Next)
    {
        v3 A = Values[Nearest->Prev];
        v3 B = Values[Nearest->Next];
        
        Result = Lerp(A, B, Nearest->Percentage);
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline quat InterpolateQuaternionKeys(nearest_frames* Nearest,
                                                        quat* Values,
                                                        int ValuesCount)
{
    quat Result = Values[Nearest->Prev];
    
    if(Nearest->Prev != Nearest->Next)
    {
        quat A = Values[Nearest->Prev];
        quat B = Values[Nearest->Next];
        
        Result = Lerp(A, B, Nearest->Percentage);
    }
    
    return(Result);
}

INTERNAL_FUNCTION inline b32 NeedInterpolateKeys(int ValuesCount,
                                                 nearest_frames* Nearest)
{
    b32 Result = ValuesCount;
    
    return(Result);
}

struct playing_animation
{
    animation* Animation;
    
    f32 StartTime;
    f32 Speed;
};

void UpdateAnimation(animation* Animation, f32 Time, m44* NodeMatrices)
{
    asset_animation_shared* Shared = &Animation->Shared;
    
    f32 CurrentTick = fmod(Time * Shared->TicksPerSecond, 
                           Shared->DurationTicks);
    u32 AnimBehaviour = AnimBehaviour_Repeat;
    
    for(int NodeAnimIndex = 0;
        NodeAnimIndex < Shared->NumNodeAnims;
        NodeAnimIndex++)
    {
        node_animation* NodeAnim = &Animation->NodeAnims[NodeAnimIndex];
        
        // NOTE(Dima): Finding frames that we will interpolate between
        nearest_frames NearestP = FindNearestFrames(NodeAnim->PositionTimes,
                                                    NodeAnim->NumPos,
                                                    CurrentTick,
                                                    Shared->DurationTicks,
                                                    AnimBehaviour);
        
        nearest_frames NearestR = FindNearestFrames(NodeAnim->RotationTimes,
                                                    NodeAnim->NumRot,
                                                    CurrentTick,
                                                    Shared->DurationTicks,
                                                    AnimBehaviour);
        
        nearest_frames NearestS = FindNearestFrames(NodeAnim->ScalingTimes,
                                                    NodeAnim->NumScl,
                                                    CurrentTick,
                                                    Shared->DurationTicks,
                                                    AnimBehaviour);
        
        // NOTE(Dima): Checking and interpolating
        v3 NewP = V3(0.0f);
        if(NeedInterpolateKeys(NodeAnim->NumPos, &NearestP))
        {
            NewP = InterpolateVectorKeys(&NearestP, 
                                         NodeAnim->PositionKeys,
                                         NodeAnim->NumPos);
        }
        
        v3 NewS = V3(1.0f);
        if(NeedInterpolateKeys(NodeAnim->NumScl, &NearestS))
        {
            NewS = InterpolateVectorKeys(&NearestS,
                                         NodeAnim->ScalingKeys,
                                         NodeAnim->NumScl);
        }
        
        quat NewR = IdentityQuaternion();
        if(NeedInterpolateKeys(NodeAnim->NumRot, &NearestR))
        {
            NewR = InterpolateQuaternionKeys(&NearestR,
                                             NodeAnim->RotationKeys,
                                             NodeAnim->NumRot);
        }
        
        m44 AnimatedTransform = ScalingMatrix(NewS) * QuaternionToMatrix4(NewR) * TranslationMatrix(NewP);
        
        // NOTE(Dima): Calculating transformation matrix
        NodeMatrices[NodeAnim->NodeIndex] = AnimatedTransform;
    }
}

INTERNAL_FUNCTION void CalculateToModelTransforms(model* Model, m44* ToModel)
{
    m44* ArrayToParent = Model->Node_ToParent;
    
    for(int TranIndex = 0;
        TranIndex < Model->Shared.NumNodes;
        TranIndex++)
    {
        int ParentIndex = Model->Node_ParentIndex[TranIndex];
        
        if(ParentIndex == -1)
        {
            ToModel[TranIndex] = ArrayToParent[TranIndex];
        }
        else
        {
            ToModel[TranIndex] = ArrayToParent[TranIndex] * ToModel[ParentIndex];
        }
    }
}

INTERNAL_FUNCTION void CalculateSkinningMatrices(model* Model, 
                                                 m44* NodeToModel, 
                                                 m44* SkinningMatrices)
{
    for(int BoneIndex = 0;
        BoneIndex < Model->Shared.NumBones;
        BoneIndex++)
    {
        int NodeIndex = Model->Bone_NodeIndex[BoneIndex];
        const m44& InvBindPos = Model->Bone_InvBindPose[BoneIndex];
        
        SkinningMatrices[BoneIndex] = InvBindPos * NodeToModel[NodeIndex];
    }
}