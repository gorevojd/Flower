struct game_camera
{
    v3 P;
    
    v3 EulerAngles;
    
    m33 Transform;
};

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

// NOTE(Dima): Look at matrix
m44 GetViewMatrix(game_camera* Camera)
{
    m44 Result = LookAt(Camera->P, Camera->P + Camera->Transform.Rows[2], V3_Up());
    
    return(Result);
}

void RenderModel(render_commands* Commands,
                 model* Model,
                 v3 P,
                 f32 Time,
                 animation* Animation = 0)
{
    m44* SkinningMatrices = 0;
    int SkinningMatricesCount = 0;
    
    if(Animation != 0)
    {
        UpdateAnimation(Animation, Time, Model->NodeToParent);
        CalculateToModelTransforms(Model);
        CalculateSkinningMatrices(Model);
        
        SkinningMatrices = Model->SkinningMatrices;
        SkinningMatricesCount = Model->NumBones;
    }
    
    m44 ModelToWorld = TranslationMatrix(P);
    
    for(int NodeIndex = 0;
        NodeIndex < Model->NumNodes;
        NodeIndex++)
    {
        model_node* Node = &Model->Nodes[NodeIndex];
        
        m44 NodeTran = Model->NodeToModel[NodeIndex] * ModelToWorld;
        
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
                
                PushMesh(Commands, 
                         Mesh,
                         Model->Materials[Mesh->MaterialIndexInModel],
                         MeshTran,
                         V3(1.0f),
                         SkinningMatrices,
                         SkinningMatricesCount);
            }
        }
        
#if 0        
        if(BoneMesh != 0)
        {
            m44 BoneTran = ScalingMatrix(0.05f) * TranslationMatrix((V4(0, 0, 0, 1) * NodeTran).xyz);
            
            PushMesh(Commands, BoneMesh,
                     0,
                     BoneTran);
        }
#endif
    }
}