#include "flower_component.cpp"

inline void InitCamera(game_camera* Cam, u32 State,
                       f32 Near = 0.5f,
                       f32 Far = 500.0f)
{
    Cam->State = State;
    
    Cam->NearClipPlane = Near;
    Cam->FarClipPlane = Far;
    
    Cam->ViewRadiusMax = 20.0f;
    Cam->ViewRadiusMin = 1.0f;
    Cam->ViewRadius = 15.0f;
    Cam->ViewTargetRadius = 7.0f;
    Cam->ShowcaseRotateTime = 12.0f;
}

// NOTE(Dima): Updating camera rotation 
void UpdateCameraRotation(game_camera* Camera,
                          float dPitch,
                          float dYaw,
                          float dRoll)
{
    float LockEdge = 85.0f;
    
    Camera->EulerAngles.Pitch += dPitch;
    Camera->EulerAngles.Yaw += dYaw;
    Camera->EulerAngles.Roll += dRoll;
    
    Camera->EulerAngles.Pitch = Clamp(Camera->EulerAngles.Pitch, -LockEdge, LockEdge);
    
    v3 Front;
    
    float Yaw = Camera->EulerAngles.Yaw * F_DEG2RAD;
    float Pitch = Camera->EulerAngles.Pitch * F_DEG2RAD;
    
#if 1
    Front.x = Sin(Yaw) * Cos(Pitch);
    Front.y = Sin(Pitch);
    Front.z = Cos(Yaw) * Cos(Pitch);
    Front = NOZ(Front);
    
    v3 Left = NOZ(Cross(V3_Up(), Front));
    v3 Up = NOZ(Cross(Front, Left));
    
    Camera->Transform = Matrix3FromRows(Left, Up, Front);
#else
    
    m44 RotYaw = RotationMatrixY(Yaw);
    m44 RotPitch = RotationMatrixX(Pitch);
    
    Camera->Transform = Matrix4ToMatrix3(RotPitch * RotYaw);
#endif
}

INTERNAL_FUNCTION inline void MoveCameraToViewTarget(game_camera* Camera)
{
    Camera->ViewTargetRadius -= Global_Input->MouseScroll * 0.4f;
    Camera->ViewTargetRadius = Clamp(Camera->ViewTargetRadius, 
                                     Camera->ViewRadiusMin, 
                                     Camera->ViewRadiusMax);
    
    Camera->ViewRadius = Lerp(Camera->ViewRadius, Camera->ViewTargetRadius, 5.0f * Global_Time->DeltaTime);
}

void UpdateCamera(game_camera* Camera, f32 CamSpeed = 1.0f)
{
    f32 MouseDeltaX = 0.0f;
    f32 MouseDeltaY = 0.0f;
    
    if(Global_Input->CapturingMouse)
    {
        MouseDeltaX = GetAxis(Axis_MouseX);
        MouseDeltaY = GetAxis(Axis_MouseY);
        
        if(Camera->State == Camera_ShowcaseRotateZ)
        {
            MouseDeltaX = Global_Time->DeltaTime * F_RAD2DEG * F_TWO_PI / Camera->ShowcaseRotateTime;
            MouseDeltaY = 0.0f;
        }
    }
    
    if(Camera->State == Camera_ShowcaseRotateZ)
    {
        MouseDeltaX = Global_Time->DeltaTime * F_RAD2DEG * F_TWO_PI / Camera->ShowcaseRotateTime;
    }
    
    UpdateCameraRotation(Camera, MouseDeltaY, MouseDeltaX, 0.0f);
    
    if(Camera->State == Camera_FlyAround)
    {
        f32 HorzMove = GetAxis(Axis_Horizontal);
        f32 VertMove = GetAxis(Axis_Vertical);
        
        v3 Moves = NOZ(V3(HorzMove, 0.0f, VertMove));
        
        v3 MoveVector = Moves * Global_Time->DeltaTime;
        
        v3 TransformedMoveVector = CamSpeed * MoveVector * Camera->Transform;
        
        if(Global_Input->CapturingMouse)
        {
            Camera->P += TransformedMoveVector;
        }
    }
    else if(Camera->State == Camera_RotateAround)
    {
        MoveCameraToViewTarget(Camera);
        
        v3 CamFront = Camera->Transform.Rows[2];
        
        Camera->P = Camera->ViewCenterP - CamFront * Camera->ViewRadius;
    }
    else if(Camera->State == Camera_ShowcaseRotateZ)
    {
        MoveCameraToViewTarget(Camera);
        
        v3 CamFront = Camera->Transform.Rows[2];
        
        Camera->P = Camera->ViewCenterP - CamFront * Camera->ViewRadius;
    }
};

// NOTE(Dima): Look at matrix
m44 GetViewMatrix(game_camera* Camera)
{
    m44 Result = LookAt(Camera->P, Camera->P + Camera->Transform.Rows[2], V3_Up());
    
    return(Result);
}

void ShowLabel3D(game_camera* Camera,
                 char* Text,
                 v3 P,
                 f32 UnitHeight = 0.2f,
                 v4 Color = ColorWhite())
{
    v3 Normal = -Camera->Transform.Rows[2];
    
    PrintTextCentered3D(Text, P,
                        Normal,
                        UnitHeight,
                        Color);
}

INTERNAL_FUNCTION void SetMatrices(game_camera* Camera, 
                                   render_pass* RenderPass)
{
    window_dimensions* WndDims = &Global_RenderCommands->WindowDimensions;
    
    m44 View = GetViewMatrix(Camera);
    
    SetPerspectivePassData(RenderPass,
                           Camera->P,
                           View, 
                           WndDims->Width,
                           WndDims->Height,
                           Camera->FarClipPlane,
                           Camera->NearClipPlane);
}

// NOTE(Dima): Object pool stuff
inline game_object* AllocateGameObject(game* Game)
{
    game_object_pool* Pool = &Game->GameObjectPool;
    
    if(DLIST_FREE_IS_EMPTY(Pool->FreeSentinel, NextAlloc))
    {
        int CountNewObjects = 512;
        
        game_object* NewObjects = PushArray(Game->Arena, game_object, CountNewObjects);
        
        for(int ObjectIndex = 0;
            ObjectIndex < CountNewObjects;
            ObjectIndex++)
        {
            game_object* CurrentObj = NewObjects + ObjectIndex;
            
            CurrentObj->CreationIndex = Pool->CreatedObjectsCount + ObjectIndex;
            
            DLIST_INSERT_BEFORE_SENTINEL(CurrentObj, Pool->FreeSentinel, 
                                         NextAlloc, PrevAlloc);
        }
    }
    
    game_object* Result = Pool->FreeSentinel.NextAlloc;
    
    // NOTE(Dima): Removing from Free & inserting to Use list
    DLIST_REMOVE(Result, NextAlloc, PrevAlloc);
    DLIST_INSERT_BEFORE_SENTINEL(Result, Pool->UseSentinel,
                                 NextAlloc, PrevAlloc);
    
    return(Result);
}

inline void DeallocateGameObject(game* Game, game_object* Object)
{
    game_object_pool* Pool = &Game->GameObjectPool;
    
    // NOTE(Dima): Removing from Use & inserting to Free list
    DLIST_REMOVE(Object, NextAlloc, PrevAlloc);
    DLIST_INSERT_BEFORE_SENTINEL(Object, Pool->FreeSentinel,
                                 NextAlloc, PrevAlloc);
}

game_object* CreateGameObject(game* Game, u32 Type, 
                              char* Name = 0, 
                              game_object* Parent = 0,
                              b32 IsActive = true)
{
    game_object_pool* Pool = &Game->GameObjectPool;
    
    // NOTE(Dima): Allocating
    game_object* Result = AllocateGameObject(Game);
    Result->Type = Type;
    
    // NOTE(Dima): Init object parent
    Result->IsActive = IsActive;
    Result->Parent = Pool->Root;
    if(Parent)
    {
        Result->Parent = Parent;
    }
    
    // NOTE(Dima): Init ptrs
    if(Type == GameObject_Sentinel ||
       Type == GameObject_Root)
    {
        DLIST_REFLECT_POINTER_PTRS(Result, Next, Prev);
    }
    else
    {
        DLIST_INSERT_BEFORE(Result, Result->Parent->ChildSentinel, Next, Prev);
    }
    
    // NOTE(Dima): Init Child sentinels
    if(Type == GameObject_Sentinel)
    {
        Result->ChildSentinel = 0;
    }
    else
    {
        // NOTE(Dima): Creating child sentinel
        Result->ChildSentinel = CreateGameObject(Game, 
                                                 GameObject_Sentinel,
                                                 "Sentinel",
                                                 Result, 
                                                 false);
    }
    
    // NOTE(Dima): Init object name
    if(!Name)
    {
        stbsp_sprintf(Result->Name, "Object %d", Result->CreationIndex);
    }
    else
    {
        CopyStringsSafe(Result->Name, 
                        ARC(Result->Name),
                        Name);
    }
    
    // NOTE(Dima): Init transforms
    Result->P = V3_Zero();
    Result->R = IdentityQuaternion();
    Result->S = V3_One();
    
    return(Result);
}

void DeleteGameObjectRec(game* Game, game_object* Obj)
{
    if(Obj->ChildSentinel)
    {
        game_object* At = Obj->ChildSentinel->Next;
        while(At != Obj->ChildSentinel)
        {
            DeleteGameObjectRec(Game, At);
            
            At = At->Next;
        }
        
        DLIST_REMOVE(Obj->ChildSentinel, Next, Prev);
        DeallocateGameObject(Game, Obj->ChildSentinel);
    }
    
    // TODO(Dima): If needed - we can call destructors on object's components here
    // ....
    RemoveAllComponents(Obj);
    
    // NOTE(Dima): Deleting this object
    DLIST_REMOVE(Obj, Next, Prev);
    DeallocateGameObject(Game, Obj);
}

void DeleteGameObject(game* Game, game_object* Obj)
{
    DeleteGameObjectRec(Game, Obj);
}

void InitGameObjectPool(game* Game,
                        memory_arena* Arena)
{
    game_object_pool* Pool = &Game->GameObjectPool;
    
    DLIST_REFLECT_PTRS(Pool->UseSentinel, NextAlloc, PrevAlloc);
    DLIST_REFLECT_PTRS(Pool->FreeSentinel, NextAlloc, PrevAlloc);
    
    Pool->CreatedObjectsCount = 0;
    
    // NOTE(Dima): Init root object and it's sentinel
    Pool->Root = CreateGameObject(Game, GameObject_Root, "Root");
}

game_object* CreateModelGameObject(game* Game,
                                   model* Model,
                                   char* Name = 0)
{
    game_object* Result = CreateGameObject(Game, GameObject_Object, Name);
    
    AddComponent(Result, Component_component_model);
    component_model* ModelComp = GetComp(Result, component_model);
    ModelComp->Model = Model;
    
    helper_byte_buffer Help = {};
    component* Component = AddComponent(Result, Component_component_model);
    
    if(Model->Shared.NumNodes)
    {
        Help.AddPlace("NodeToModel", Model->Shared.NumNodes, sizeof(m44));
    }
    
    if(Model->Shared.NumBones)
    {
        Help.AddPlace("SkinningMatrices", Model->Shared.NumBones, sizeof(m44));
    }
    
    Help.Generate();
    
    ModelComp->NodeToModel = (m44*)Help.GetPlace("NodeToModel");
    ModelComp->SkinningMatrices = (m44*)Help.GetPlace("SkinningMatrices");
    
    Component->Free = Help.Data;
    Component->FreeSize = Help.DataSize;
    
    return(Result);
}

void UpdateModelGameObject(game_object* Object)
{
    component_model* CompModel = GetComp(Object, component_model);
    
    m44* SkinningMatrices = 0;
    int SkinningMatricesCount = 0;
    model* Model = CompModel->Model;
    
    component_animator* AnimatorComp = GetComp(Object, component_animator);
    animation* Animation = 0;
    
    if(AnimatorComp)
    {
        Animation = AnimatorComp->PlayingAnimation;
    }
    
    if(Animation != 0)
    {
        UpdateAnimation(Animation, Global_Time->Time, Model->Node_ToParent);
        
        SkinningMatrices = CompModel->SkinningMatrices;
        SkinningMatricesCount = Model->Shared.NumBones;
        
        CalculateToModelTransforms(Model, CompModel->NodeToModel);
        // NOTE(Dima): Setting this to false so next time they should be recomputed again
        CompModel->ToModelIsComputed = false;
        
        CalculateSkinningMatrices(Model,
                                  CompModel->NodeToModel,
                                  CompModel->SkinningMatrices);
    }
    else
    {
        if(!CompModel->ToModelIsComputed)
        {
            CalculateToModelTransforms(Model, CompModel->NodeToModel);
            // NOTE(Dima): 
            CompModel->ToModelIsComputed = true;
        }
    }
    
    m44 ModelToWorld = TranslationMatrix(Object->P);
    
    for(int NodeIndex = 0;
        NodeIndex < Model->Shared.NumNodes;
        NodeIndex++)
    {
        model_node* Node = &Model->Nodes[NodeIndex];
        
        m44 NodeTran = CompModel->NodeToModel[NodeIndex] * ModelToWorld;
        
        for(int MeshIndex = 0;
            MeshIndex < Node->NumMeshIndices;
            MeshIndex++)
        {
            int ActualMeshIndex = Node->MeshIndices[MeshIndex];
            
            mesh* Mesh = Model->Meshes[ActualMeshIndex];
            
            if(Mesh)
            {
                m44 MeshTran = NodeTran;
                if(Mesh->IsSkinned)
                {
                    MeshTran = ModelToWorld;
                }
                
                material* Material = Model->Materials[Mesh->MaterialIndexInModel];
                
                if(Mesh->IsSkinned)
                {
                    PushMesh(Mesh, Material, MeshTran,
                             V3(1.0f),
                             SkinningMatrices,
                             SkinningMatricesCount);
                }
                else
                {
                    PushMesh(Mesh, Material, MeshTran);
                }
            }
        }
    }
}

#define PROCESS_GAME_OBJECT_FUNC(name) void name(game* Game, game_object* Obj)
typedef PROCESS_GAME_OBJECT_FUNC(process_game_object_func);

PROCESS_GAME_OBJECT_FUNC(UpdateGameObject)
{
    switch(Obj->Type)
    {
        case GameObject_Object:
        {
            UpdateModelGameObject(Obj);
        }break;
        
        default:
        {
            // NOTE(Dima): Nothing to do!
        }break;
    }
}

PROCESS_GAME_OBJECT_FUNC(RenderGameObject)
{
    switch(Obj->Type)
    {
        case GameObject_Object:
        {
            // TODO(Dima): Do something
        }break;
        
        default:
        {
            // NOTE(Dima): Nothing to do!
        }break;
    }
}

void GameObjectRec(game* Game, game_object* Obj, process_game_object_func* Func)
{
    Assert(Obj->ChildSentinel);
    
    if(Obj->IsActive)
    {
        // NOTE(Dima): Iterating over children and update them recursively
        if(Obj->ChildSentinel)
        {
            game_object* At = Obj->ChildSentinel->Next;
            
            while(At != Obj->ChildSentinel)
            {
                GameObjectRec(Game, At, Func);
                
                At = At->Next;
            }
        }
        
        Func(Game, Obj);
    }
}

void UpdateGameObjects(game* Game)
{
    GameObjectRec(Game, Game->GameObjectPool.Root, UpdateGameObject);
}
