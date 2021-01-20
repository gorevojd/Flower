enum camera_state
{
    Camera_FlyAround,
    Camera_RotateAround,
    Camera_ShowcaseRotateZ,
};

struct game_camera
{
    v3 P;
    v3 EulerAngles;
    
    m33 Transform;
    
    // NOTE(Dima): Staff for rotating around
    f32 ViewRadius;
    f32 ViewTargetRadius;
    f32 ViewRadiusMin;
    f32 ViewRadiusMax;
    v3 ViewCenterP;
    
    u32 State;
};

inline void InitCamera(game_camera* Cam, u32 State)
{
    Cam->State = State;
    
    Cam->ViewRadiusMax = 20.0f;
    Cam->ViewRadiusMin = 1.0f;
    Cam->ViewRadius = 15.0f;
    Cam->ViewTargetRadius = 7.0f;
}

// NOTE(Dima): Updating camera rotation 
void UpdateCameraRotation(game_camera* Camera,
                          float dPitch,
                          float dYaw,
                          float dRoll)
{
    float LockEdge = 89.0f;
    
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
    Camera->ViewTargetRadius -= Global_Input->MouseScroll;
    Camera->ViewTargetRadius = Clamp(Camera->ViewTargetRadius, 
                                     Camera->ViewRadiusMin, 
                                     Camera->ViewRadiusMax);
    
    Camera->ViewRadius = Lerp(Camera->ViewRadius, Camera->ViewTargetRadius, 5.0f * Global_Time->DeltaTime);
}

void UpdateCamera(game_camera* Camera, f32 CamSpeed = 1.0f)
{
    if(Global_Input->CapturingMouse)
    {
        f32 MouseDeltaX = GetAxis(Axis_MouseX);
        f32 MouseDeltaY = GetAxis(Axis_MouseY);
        
        if(Camera->State == Camera_ShowcaseRotateZ)
        {
            MouseDeltaX = Global_Time->DeltaTime * F_RAD2DEG * F_TWO_PI / 8.0f;
            MouseDeltaY = 0.0f;
        }
        UpdateCameraRotation(Camera, MouseDeltaY, MouseDeltaX, 0.0f);
    }
    
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

INTERNAL_FUNCTION void SetMatrices(m44 ViewMatrix)
{
    window_dimensions* WndDims = &Global_RenderCommands->WindowDimensions;
    
    Global_RenderCommands->View = ViewMatrix;
    Global_RenderCommands->Projection = PerspectiveProjection(WndDims->Width, 
                                                              WndDims->Height,
                                                              500.0f, 0.5f);
    
    Global_RenderCommands->ViewProjection = 
        Global_RenderCommands->View * 
        Global_RenderCommands->Projection;
}

void RenderModel(model* Model,
                 v3 P,
                 f32 Time,
                 animation* Animation = 0)
{
    m44* SkinningMatrices = 0;
    int SkinningMatricesCount = 0;
    
    if(Animation != 0)
    {
        UpdateAnimation(Animation, Time, Model->Node_ToParent);
        
        SkinningMatrices = Model->Bone_SkinningMatrices;
        SkinningMatricesCount = Model->Shared.NumBones;
    }
    
    CalculateToModelTransforms(Model);
    CalculateSkinningMatrices(Model);
    
    m44 ModelToWorld = TranslationMatrix(P);
    
    for(int NodeIndex = 0;
        NodeIndex < Model->Shared.NumNodes;
        NodeIndex++)
    {
        model_node* Node = &Model->Nodes[NodeIndex];
        
        m44 NodeTran = Model->Node_ToModel[NodeIndex] * ModelToWorld;
        
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
                
                PushInstanceMesh(1000, 
                                 Mesh,
                                 Model->Materials[Mesh->MaterialIndexInModel],
                                 MeshTran,
                                 V3(1.0f),
                                 SkinningMatrices,
                                 SkinningMatricesCount);
                
            }
        }
    }
}