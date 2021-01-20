#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags

inline m44 AssimpToOurMatrix(aiMatrix4x4& AMat)
{
    m44 Result = {};
    
    Result.e[0] = AMat.a1;
    Result.e[1] = AMat.a2;
    Result.e[2] = AMat.a3;
    Result.e[3] = AMat.a4;
    
    Result.e[4] = AMat.b1;
    Result.e[5] = AMat.b2;
    Result.e[6] = AMat.b3;
    Result.e[7] = AMat.b4;
    
    Result.e[8] = AMat.c1;
    Result.e[9] = AMat.c2;
    Result.e[10] = AMat.c3;
    Result.e[11] = AMat.c4;
    
    Result.e[12] = AMat.d1;
    Result.e[13] = AMat.d2;
    Result.e[14] = AMat.d3;
    Result.e[15] = AMat.d4;
    
    return(Transpose(Result));
}

struct loaded_model_node
{
    std::string Name;
    
    int NodeIndex;
    int ParentNodeIndex;
    
    std::vector<int> ChildNodesIndices;
    std::vector<int> MeshIndices;
    
    m44 ToParent;
    m44 ToModel;
    
    aiNode* AssimpNode;
};

struct loaded_node_animation
{
    std::vector<v3> PositionKeys;
    std::vector<quat> RotationKeys;
    std::vector<v3> ScalingKeys;
    
    std::vector<f32> PositionTimes;
    std::vector<f32> RotationTimes;
    std::vector<f32> ScalingTimes;
    
    int NodeIndex;
    u32 DataSize;
};

struct loaded_animation
{
    std::vector<loaded_node_animation> NodeAnims;
    
    f32 DurationTicks;
    f32 TicksPerSecond;
    u32 Behaviour;
    
    std::string Name;
};

struct loaded_model
{
    int NumMeshes;
    mesh* Meshes;
    
    std::vector<loaded_model_node> Nodes;
    std::vector<material*> Materials;
    
    m44 Bone_InvBindPose[256];
    int Bone_NodeIndex[256];
    int NumBones;
    
    // NOTE(Dima): Help structures
    std::map<std::string, int> NodeNameToNodeIndex;
    std::map<std::string, int> NodeNameToBoneIndex;
    
    std::vector<loaded_animation> Animations;
    
    void* ModelFree;
    void* MeshesFree;
    void* MaterialsFree;
};

mesh ConvertAssimpMesh(const aiScene* AssimpScene, 
                       aiMesh* AssimpMesh, 
                       loaded_model* Model)
{
    helper_mesh HelperMesh = {};
    
    std::vector<v3>& Vertices = HelperMesh.Vertices;
    std::vector<v2>& TexCoords = HelperMesh.TexCoords;
    std::vector<v3>& Normals = HelperMesh.Normals;
    std::vector<v3>& Colors = HelperMesh.Colors;
    std::vector<u32>& Indices = HelperMesh.Indices;
    
    std::vector<v4>& BoneWeights = HelperMesh.BoneWeights;
    std::vector<u32>& BoneIndices = HelperMesh.BoneIndices;
    
    Vertices.reserve(AssimpMesh->mNumVertices);
    TexCoords.reserve(AssimpMesh->mNumVertices);
    Normals.reserve(AssimpMesh->mNumVertices);
    Colors.reserve(AssimpMesh->mNumVertices);
    
    // NOTE(Dima): Choosing color set to load
    int NumColorChannels = AssimpMesh->GetNumColorChannels();
    b32 CanLoadColors = false;
    int ColorSetIndex = 0;
    for(int ColorIndex = 0;
        ColorIndex < NumColorChannels;
        ColorIndex++)
    {
        CanLoadColors = AssimpMesh->HasVertexColors(ColorIndex);
        
        if(CanLoadColors)
        {
            ColorSetIndex = ColorIndex;
            break;
        }
    }
    
    // NOTE(Dima): Choosing UV set to load
    int NumUVChannels = AssimpMesh->GetNumUVChannels();
    b32 CanLoadUVs = false;
    int UVSetIndex = 0;
    for(int UVIndex = 0;
        UVIndex < NumUVChannels;
        UVIndex++)
    {
        CanLoadUVs = AssimpMesh->HasTextureCoords(UVIndex);
        
        if(CanLoadUVs)
        {
            UVSetIndex = UVIndex;
            break;
        }
    }
    
    // NOTE(Dima): Loading vertices
    for(int VertexIndex = 0;
        VertexIndex < AssimpMesh->mNumVertices;
        VertexIndex++)
    {
        // NOTE(Dima): Vertex data
        aiVector3D AssimpVertex = AssimpMesh->mVertices[VertexIndex];
        
        v3 OurVertex = V3(AssimpVertex.x,
                          AssimpVertex.y,
                          AssimpVertex.z);
        Vertices.push_back(OurVertex);
        
        // NOTE(Dima): Normal data
        aiVector3D AssimpNormal = AssimpMesh->mNormals[VertexIndex];
        v3 OurNormal = V3(AssimpNormal.x,
                          AssimpNormal.y,
                          AssimpNormal.z);
        Normals.push_back(OurNormal);
        
        // NOTE(Dima): UVs data
        v2 OurUV = V2(0.0f, 0.0f);
        if(CanLoadUVs)
        {
            aiVector3D AssimpUV = AssimpMesh->mTextureCoords[UVSetIndex][VertexIndex];
            OurUV = V2(AssimpUV.x, AssimpUV.y);
        }
        TexCoords.push_back(OurUV);
        
        // NOTE(Dima): Colors data
        v3 OurColor = V3(1.0f, 1.0f, 1.0f);
        if(CanLoadColors)
        {
            aiColor4D AssimpColor = AssimpMesh->mColors[ColorSetIndex][VertexIndex];
            OurColor = V3(AssimpColor.r,
                          AssimpColor.g,
                          AssimpColor.b);
        }
        Colors.push_back(OurColor);
    }
    
    // NOTE(Dima): Loading indices
    for(int FaceIndex = 0;
        FaceIndex < AssimpMesh->mNumFaces;
        FaceIndex++)
    {
        aiFace* Face = &AssimpMesh->mFaces[FaceIndex];
        
        if(Face->mNumIndices == 3)
        {
            Indices.push_back(Face->mIndices[0]);
            Indices.push_back(Face->mIndices[1]);
            Indices.push_back(Face->mIndices[2]);
        }
    }
    
    // NOTE(Dima): Loading weights
    struct v_weight
    {
        f32 W;
        int ID;
    };
    
    struct v_weights
    {
        std::vector<v_weight> Weights;
    };
    
    std::vector<v_weights> Weights(AssimpMesh->mNumVertices);
    
    HelperMesh.IsSkinned = AssimpMesh->HasBones() && (AssimpMesh->mNumBones > 0);
    
    if(HelperMesh.IsSkinned)
    {
        for(int MeshBoneIndex = 0;
            MeshBoneIndex < AssimpMesh->mNumBones;
            MeshBoneIndex++)
        {
            aiBone* AiBone = AssimpMesh->mBones[MeshBoneIndex];
            
            // NOTE(Dima): Finding bone ID
            std::string BoneName = std::string(AiBone->mName.C_Str());
            int BoneIndex = Model->NodeNameToBoneIndex[BoneName];
            
            // NOTE(Dima): Processing weights
            for(int WeightIndex = 0;
                WeightIndex < AiBone->mNumWeights;
                WeightIndex++)
            {
                aiVertexWeight* Weight = &AiBone->mWeights[WeightIndex];
                
                u32 VertexIndex = Weight->mVertexId;
                f32 VertexWeight = Weight->mWeight;
                
                v_weight NewWeight = {};
                NewWeight.W = VertexWeight;
                NewWeight.ID = BoneIndex;
                
                v_weights* TargetWeights = &Weights[VertexIndex];
                TargetWeights->Weights.push_back(NewWeight);
            }
        }
        
        // NOTE(Dima): Preparing weights
        for(int WeightIndex = 0;
            WeightIndex < Weights.size();
            WeightIndex++)
        {
            v_weights* CurWeights = &Weights[WeightIndex];
            
            v4 ResultWeights = V4(0.0f, 0.0f, 0.0f, 0.0f);
            u32 ResultIndices = 0;
            
            for(int InnerIndex = 0;
                InnerIndex < CurWeights->Weights.size();
                InnerIndex++)
            {
                v_weight W = CurWeights->Weights[InnerIndex];
                
                ResultWeights.e[InnerIndex] = W.W;
                ResultIndices |= (W.ID & 255) << (InnerIndex * 8);
            }
            
            BoneWeights.push_back(ResultWeights);
            BoneIndices.push_back(ResultIndices);
        }
    }
    
    // NOTE(Dima): Actual converting
    mesh Result = MakeMesh(HelperMesh);
    
    std::string MeshName = std::string(AssimpMesh->mName.C_Str());
    
    CopyStringsSafe(Result.Name, ArrayCount(Result.Name), (char*)MeshName.c_str());
    Result.MaterialIndexInModel = 0;
    
    return(Result);
}

INTERNAL_FUNCTION void AssimpProcessNode(aiScene* AssimpScene,
                                         aiNode* AssimpNode,
                                         loaded_model_node* Parent,
                                         const m44& GlobalInitTransform,
                                         loaded_model* Loaded)
{
    // NOTE(Dima): Calculating this node transforms
    m44 ThisNodeToParentTransform = GlobalInitTransform;
    m44 ThisNodeToModelTransform = GlobalInitTransform;
    if(Parent != 0)
    {
        ThisNodeToParentTransform = AssimpToOurMatrix(AssimpNode->mTransformation);
        ThisNodeToModelTransform = ThisNodeToParentTransform * Parent->ToModel;
    }
    
    // NOTE(Dima): Initializing new Node
    int ThisNodeIndex = Loaded->Nodes.size();;
    
    loaded_model_node NewNode;
    NewNode.Name = std::string(AssimpNode->mName.C_Str());
    NewNode.NodeIndex = ThisNodeIndex;
    NewNode.ToParent = ThisNodeToParentTransform;
    NewNode.ToModel = ThisNodeToModelTransform; 
    NewNode.AssimpNode = AssimpNode; 
    
    // NOTE(Dima): Setting child and parent nodes indices
    if(Parent != 0)
    {
        Parent->ChildNodesIndices.push_back(ThisNodeIndex);
        NewNode.ParentNodeIndex = Parent->NodeIndex;
    }
    else
    {
        NewNode.ParentNodeIndex = -1;
    }
    
    // NOTE(Dima): Pushing node
    int NewNodeIndex = Loaded->Nodes.size();
    Loaded->NodeNameToNodeIndex.insert(std::pair<std::string, int>(NewNode.Name, NewNodeIndex));
    Loaded->Nodes.push_back(NewNode);
    
    // NOTE(Dima): Recursive lookups
    for(int NodeIndex = 0; NodeIndex < AssimpNode->mNumChildren; NodeIndex++)
    {
        aiNode* CurChild = AssimpNode->mChildren[NodeIndex];
        
        AssimpProcessNode(AssimpScene, CurChild, 
                          &Loaded->Nodes[ThisNodeIndex], 
                          GlobalInitTransform, 
                          Loaded);
    }
}

INTERNAL_FUNCTION void LoadModelBones(const aiScene* AssimpScene,
                                      loaded_model* Model)
{
    Model->NumBones = 0;
    
    for(int MeshIndex = 0; 
        MeshIndex < AssimpScene->mNumMeshes; 
        MeshIndex++)
    {
        aiMesh* AiMesh = AssimpScene->mMeshes[MeshIndex];
        
        if(AiMesh->HasBones() && (AiMesh->mNumBones > 0))
        {
            for(int BoneIndex = 0;
                BoneIndex < AiMesh->mNumBones;
                BoneIndex++)
            {
                aiBone* AiBone = AiMesh->mBones[BoneIndex];
                int ThisBoneID = Model->NumBones;
                
                std::string BoneName = std::string(AiBone->mName.C_Str());
                
                Model->Bone_InvBindPose[ThisBoneID] = AssimpToOurMatrix(AiBone->mOffsetMatrix);
                Model->Bone_NodeIndex[ThisBoneID] = Model->NodeNameToNodeIndex[BoneName];
                
                Model->NodeNameToBoneIndex.insert(std::pair<std::string, int>(BoneName, ThisBoneID));
                
                Model->NumBones++;
            }
        }
    }
}

INTERNAL_FUNCTION void LoadModelMeshes(const aiScene* AssimpScene,
                                       loaded_model* Model)
{
    Model->NumMeshes = AssimpScene->mNumMeshes;
    Model->Meshes = (mesh*)malloc(sizeof(mesh) * Model->NumMeshes);
    
    // NOTE(Dima): Loading meshes
    for(int MeshIndex = 0;
        MeshIndex < Model->NumMeshes;
        MeshIndex++)
    {
        aiMesh* AssimpMesh = AssimpScene->mMeshes[MeshIndex];
        
        Model->Meshes[MeshIndex] = ConvertAssimpMesh(AssimpScene, AssimpMesh, Model);
    }
    
    // NOTE(Dima): Loading mesh indices
    for(int NodeIndex = 0; 
        NodeIndex < Model->Nodes.size();
        NodeIndex++)
    {
        loaded_model_node* OurNode = &Model->Nodes[NodeIndex];
        
        // NOTE(Dima): Loading this nodes meshes
        aiNode* AssimpNode = OurNode->AssimpNode;
        
        for(int MeshIndex = 0; MeshIndex < AssimpNode->mNumMeshes; MeshIndex++)
        {
            int MeshIndexInScene = AssimpNode->mMeshes[MeshIndex];
            
            OurNode->MeshIndices.push_back(MeshIndexInScene);
        }
    }
    
    Model->MeshesFree = Model->Meshes;
}

INTERNAL_FUNCTION animation ConvertToActualAnimation(loaded_animation* Load)
{
    int NumNodeAnims = Load->NodeAnims.size();
    
    animation Result = {};
    
    Result.Shared.DurationTicks = Load->DurationTicks;
    Result.Shared.TicksPerSecond = Load->TicksPerSecond;
    Result.Shared.Behaviour = Load->Behaviour;
    Result.Shared.NumNodeAnims = NumNodeAnims;
    
    CopyStringsSafe(Result.Name, ArrayCount(Result.Name), (char*)Load->Name.c_str());
    
    helper_byte_buffer Help = {};
    
    Help.AddPlace("NodeAnims", NumNodeAnims, sizeof(node_animation));
    
    std::string PKeysStr = std::string("PKeys");
    std::string RKeysStr = std::string("RKeys");
    std::string SKeysStr = std::string("SKeys");
    std::string PTimesStr = std::string("PTimes");
    std::string RTimesStr = std::string("RTimes");
    std::string STimesStr = std::string("STimes");
    
    for(int NodeAnimIndex = 0;
        NodeAnimIndex < NumNodeAnims;
        NodeAnimIndex++)
    {
        loaded_node_animation* Src = &Load->NodeAnims[NodeAnimIndex];
        
        std::string IndexString = std::to_string(NodeAnimIndex);
        
        Help.AddPlace(PKeysStr + IndexString, Src->PositionKeys.size(), sizeof(v3));
        Help.AddPlace(RKeysStr + IndexString, Src->RotationKeys.size(), sizeof(quat));
        Help.AddPlace(SKeysStr + IndexString, Src->ScalingKeys.size(), sizeof(v3));
        Help.AddPlace(PTimesStr + IndexString, Src->PositionTimes.size(), sizeof(f32));
        Help.AddPlace(RTimesStr + IndexString, Src->RotationTimes.size(), sizeof(f32));
        Help.AddPlace(STimesStr + IndexString, Src->ScalingTimes.size(), sizeof(f32));
    }
    
    Help.Generate();;
    
    Result.NodeAnims = (node_animation*)Help.GetPlace("NodeAnims");
    
    for(int NodeAnimIndex = 0;
        NodeAnimIndex < NumNodeAnims;
        NodeAnimIndex++)
    {
        loaded_node_animation* Src = &Load->NodeAnims[NodeAnimIndex];
        
        node_animation* NodeAnim = &Result.NodeAnims[NodeAnimIndex];
        
        NodeAnim->NumPos = Src->PositionKeys.size();
        NodeAnim->NumRot = Src->RotationKeys.size();
        NodeAnim->NumScl = Src->ScalingKeys.size();
        NodeAnim->NodeIndex = Src->NodeIndex;
        
        std::string IndexString = std::to_string(NodeAnimIndex);
        
        NodeAnim->PositionKeys = (v3*)Help.GetPlace(PKeysStr + IndexString);
        NodeAnim->RotationKeys = (quat*)Help.GetPlace(RKeysStr + IndexString);
        NodeAnim->ScalingKeys = (v3*)Help.GetPlace(SKeysStr + IndexString);
        NodeAnim->PositionTimes = (f32*)Help.GetPlace(PTimesStr + IndexString);
        NodeAnim->RotationTimes = (f32*)Help.GetPlace(RTimesStr + IndexString);
        NodeAnim->ScalingTimes = (f32*)Help.GetPlace(STimesStr + IndexString);
        
        // NOTE(Dima): Copy position values and times
        for(int PosIndex = 0; 
            PosIndex < NodeAnim->NumPos;
            PosIndex++)
        {
            NodeAnim->PositionKeys[PosIndex] = Src->PositionKeys[PosIndex];
            NodeAnim->PositionTimes[PosIndex] = Src->PositionTimes[PosIndex];
        }
        
        // NOTE(Dima): Copy rotation values and times
        for(int RotIndex = 0;
            RotIndex < NodeAnim->NumRot;
            RotIndex++)
        {
            NodeAnim->RotationKeys[RotIndex] = Src->RotationKeys[RotIndex];
            NodeAnim->RotationTimes[RotIndex] = Src->RotationTimes[RotIndex];
        }
        
        // NOTE(Dima): Copy scaling values and times
        for(int ScaIndex = 0;
            ScaIndex < NodeAnim->NumScl;
            ScaIndex++)
        {
            NodeAnim->ScalingKeys[ScaIndex] = Src->ScalingKeys[ScaIndex];
            NodeAnim->ScalingTimes[ScaIndex] = Src->ScalingTimes[ScaIndex];
        }
    }
    
    Result.Free = Help.Data;
    
    return(Result);
}

INTERNAL_FUNCTION void LoadModelAnimations(const aiScene* AssimpScene,
                                           loaded_model* Model)
{
    int NumAnims = AssimpScene->mNumAnimations;
    if(AssimpScene->HasAnimations() && (NumAnims > 0))
    {
        // NOTE(Dima): Loading animations
        Model->Animations.reserve(AssimpScene->mNumAnimations);
        
        for(int AnimIndex = 0;
            AnimIndex < AssimpScene->mNumAnimations;
            AnimIndex++)
        {
            aiAnimation* AiAnim = AssimpScene->mAnimations[AnimIndex];
            
            loaded_animation NewAnim;
            
            // NOTE(Dima): Setting animation info
            NewAnim.Name = std::string(AiAnim->mName.C_Str());
            NewAnim.DurationTicks = AiAnim->mDuration;
            NewAnim.TicksPerSecond = AiAnim->mTicksPerSecond;
            
            // NOTE(Dima): Loading node animations
            for(int ChannelIndex = 0;
                ChannelIndex < AiAnim->mNumChannels;
                ChannelIndex++)
            {
                aiNodeAnim* NodeAnim = AiAnim->mChannels[ChannelIndex];
                
                loaded_node_animation NewNodeAnim = {};
                
                NewNodeAnim.NodeIndex = Model->NodeNameToNodeIndex[std::string(NodeAnim->mNodeName.C_Str())];
                
                // NOTE(Dima): Load position keys
                for(int PositionKeyIndex = 0;
                    PositionKeyIndex < NodeAnim->mNumPositionKeys;
                    PositionKeyIndex++)
                {
                    aiVectorKey PosKey = NodeAnim->mPositionKeys[PositionKeyIndex];
                    
                    NewNodeAnim.PositionTimes.push_back(PosKey.mTime);
                    NewNodeAnim.PositionKeys.push_back(V3(PosKey.mValue.x,
                                                          PosKey.mValue.y,
                                                          PosKey.mValue.z));
                }
                
                // NOTE(Dima): Load rotation keys
                for(int RotationKeyIndex = 0;
                    RotationKeyIndex < NodeAnim->mNumRotationKeys;
                    RotationKeyIndex++)
                {
                    aiQuatKey RotKey = NodeAnim->mRotationKeys[RotationKeyIndex];
                    
                    NewNodeAnim.RotationTimes.push_back(RotKey.mTime);
                    NewNodeAnim.RotationKeys.push_back(Quaternion(RotKey.mValue.x,
                                                                  RotKey.mValue.y,
                                                                  RotKey.mValue.z,
                                                                  RotKey.mValue.w));
                }
                
                // NOTE(Dima): Load scaling keys
                for(int ScalingKeyIndex = 0;
                    ScalingKeyIndex < NodeAnim->mNumScalingKeys;
                    ScalingKeyIndex++)
                {
                    aiVectorKey ScaKey = NodeAnim->mScalingKeys[ScalingKeyIndex];
                    
                    NewNodeAnim.ScalingTimes.push_back(ScaKey.mTime);
                    NewNodeAnim.ScalingKeys.push_back(V3(ScaKey.mValue.x,
                                                         ScaKey.mValue.y,
                                                         ScaKey.mValue.z));
                }
                
                // NOTE(Dima): Pushing animation to animations array
                NewAnim.NodeAnims.push_back(NewNodeAnim);
            }
            
            Model->Animations.push_back(NewAnim);
        }
    }
}

INTERNAL_FUNCTION model ConvertToActualModel(loaded_model* Load)
{
    model Model_ = {};
    model* Model = &Model_;
    
    Model->Shared.NumNodes = Load->Nodes.size();
    Model->Shared.NumBones = Load->NumBones;
    Model->Shared.NumMeshes = Load->NumMeshes;
    Model->Shared.NumMaterials = Load->Materials.size();
    
    int AllocMaterialsCount = std::max(50, (int)Load->Materials.size());
    
    helper_byte_buffer Help = {};
    Help.AddPlace("Meshes", Model->Shared.NumMeshes, sizeof(mesh*));
    Help.AddPlace("Materials", AllocMaterialsCount, sizeof(material*));
    Help.AddPlace("Nodes", Model->Shared.NumNodes, sizeof(model_node));
    Help.AddPlace("Node_ToModel", Model->Shared.NumNodes, sizeof(m44));
    Help.AddPlace("Node_ToParent", Model->Shared.NumNodes, sizeof(m44));
    Help.AddPlace("Node_ParentIndex", Model->Shared.NumNodes, sizeof(int));
    Help.AddPlace("Bone_InvBindPose", Model->Shared.NumNodes, sizeof(m44));
    Help.AddPlace("Bone_NodeIndex", Model->Shared.NumBones, sizeof(int));
    Help.AddPlace("Bone_SkinningMatrices", Model->Shared.NumBones, sizeof(m44));
    
    Help.Generate();
    
    Model->Meshes = (mesh**)Help.GetPlace("Meshes");
    Model->Materials = (material**)Help.GetPlace("Materials");
    Model->Nodes = (model_node*)Help.GetPlace("Nodes");
    Model->Node_ToModel = (m44*)Help.GetPlace("Node_ToModel");
    Model->Node_ToParent = (m44*)Help.GetPlace("Node_ToParent");
    Model->Node_ParentIndex = (int*)Help.GetPlace("Node_ParentIndex");
    Model->Bone_InvBindPose = (m44*)Help.GetPlace("Bone_InvBindPose");
    Model->Bone_NodeIndex = (int*)Help.GetPlace("Bone_NodeIndex");
    Model->Bone_SkinningMatrices = (m44*)Help.GetPlace("Bone_SkinningMatrices");
    
    Model->Free = Help.Data;
    
    // NOTE(Dima): Setting meshes
    for(int MeshIndex = 0;
        MeshIndex < Model->Shared.NumMeshes;
        MeshIndex++)
    {
        Model->Meshes[MeshIndex] = &Load->Meshes[MeshIndex];
    }
    
    // NOTE(Dima): Setting materials
    for(int MaterialIndex = 0;
        MaterialIndex < Model->Shared.NumMaterials;
        MaterialIndex++)
    {
        Model->Materials[MaterialIndex] = Load->Materials[MaterialIndex];
    }
    
    // NOTE(Dima): Setting bones
    for(int BoneIndex = 0;
        BoneIndex < Model->Shared.NumBones;
        BoneIndex++)
    {
        Model->Bone_InvBindPose[BoneIndex] = Load->Bone_InvBindPose[BoneIndex];
        Model->Bone_NodeIndex[BoneIndex] = Load->Bone_NodeIndex[BoneIndex];
    }
    
    // NOTE(Dima): Setting nodes
    for(int NodeIndex = 0;
        NodeIndex < Model->Shared.NumNodes;
        NodeIndex++)
    {
        model_node* Node = &Model->Nodes[NodeIndex];
        loaded_model_node* Src = &Load->Nodes[NodeIndex];
        
        CopyStringsSafe(Node->Name, ArrayCount(Node->Name), (char*)Src->Name.c_str());
        Node->NumChildIndices = Src->ChildNodesIndices.size();
        Node->NumMeshIndices = Src->MeshIndices.size();
        
        Assert(Node->NumChildIndices <= ArrayCount(Node->ChildIndices));
        Assert(Node->NumMeshIndices <= ArrayCount(Node->MeshIndices));
        
        Node->NumChildIndices = FlowerMin(Node->NumChildIndices, ArrayCount(Node->ChildIndices));
        Node->NumMeshIndices = FlowerMin(Node->NumMeshIndices, ArrayCount(Node->MeshIndices));
        
        // NOTE(Dima): Copy child indices
        for(int ChildIndex = 0;
            ChildIndex < Node->NumChildIndices;
            ChildIndex++)
        {
            Node->ChildIndices[ChildIndex] = Src->ChildNodesIndices[ChildIndex];
        }
        
        // NOTE(Dima): Copy mesh indices
        for(int MeshIndex = 0;
            MeshIndex < Node->NumMeshIndices;
            MeshIndex++)
        {
            Node->MeshIndices[MeshIndex] = Src->MeshIndices[MeshIndex];
        }
        
        // NOTE(Dima): Copy other arrays
        Model->Node_ToModel[NodeIndex] = Src->ToModel;
        Model->Node_ToParent[NodeIndex] = Src->ToParent;
        Model->Node_ParentIndex[NodeIndex] = Src->ParentNodeIndex;
    }
    
    return(Model_);
}

INTERNAL_FUNCTION loaded_model LoadModelFileInternal(char* FilePath,
                                                     loading_params Params = DefaultLoadingParams(),
                                                     b32 LoadAnimations = true,
                                                     b32 LoadOnlyAnimations = false)
{
    Assimp::Importer Importer;
    Importer.SetPropertyInteger("AI_CONFIG_PP_SBP_REMOVE", 
                                aiPrimitiveType_POINT | aiPrimitiveType_LINE);
    
    const aiScene* Scene = Importer.ReadFile(FilePath,         
                                             aiProcess_CalcTangentSpace       | 
                                             aiProcess_Triangulate            |
                                             aiProcess_JoinIdenticalVertices  |
                                             aiProcess_LimitBoneWeights | 
                                             aiProcess_ImproveCacheLocality | 
                                             aiProcess_RemoveRedundantMaterials | 
                                             aiProcess_FixInfacingNormals | 
                                             aiProcess_FindDegenerates |
                                             aiProcess_SortByPType | 
                                             aiProcess_FindInvalidData | 
                                             aiProcess_OptimizeMeshes |
                                             //aiProcess_OptimizeGraph | 
                                             aiProcess_ValidateDataStructure);
    
    
#if 1    
    double UnitScaleFactor = 1.0;
    Scene->mMetaData->Get("UnitScaleFactor", UnitScaleFactor);
#endif
    
    // NOTE(Dima): Calculating transform
    m44 RootTran = AssimpToOurMatrix(Scene->mRootNode->mTransformation);
    m44 InvRootTran = InverseTransformMatrix(RootTran);
    m44 AdditionalScale = ScalingMatrix(Params.Model_DefaultScale);
    
    InvRootTran = AdditionalScale * InvRootTran;
    
    // NOTE(Dima): Applying additional rotation
    if(Params.Model_FixInvalidRotation)
    {
        m44 AdditionalRotation = RotationMatrixX(-90.0f * F_DEG2RAD);
        
        InvRootTran = AdditionalRotation * InvRootTran;
    }
    
    // NOTE(Dima): Calculate premultiply matrix
    m44 GlobalTransform = InvRootTran;
    //m44 GlobalTransform = IdentityMatrix4();
    
    // NOTE(Dima): Loading meshes
    loaded_model Loaded;
    
    AssimpProcessNode((aiScene*)Scene, 
                      Scene->mRootNode, 
                      0, GlobalTransform, 
                      &Loaded);
    
    LoadModelBones(Scene, &Loaded);
    
    if(LoadAnimations)
    {
        LoadModelAnimations(Scene, &Loaded);
    }
    
    if(!LoadOnlyAnimations)
    {
        LoadModelMeshes(Scene, &Loaded);
    }
    
    return(Loaded);
}

// NOTE(Dima): Pass LoadAnimations = true to get all animations that were in this file
model LoadModel(char* FilePath,
                loading_params Params = DefaultLoadingParams())
{
    b32 LoadOnlyAnimations = false;
    b32 LoadAnimations = false;
    
    loaded_model Loaded = LoadModelFileInternal(FilePath, Params, LoadAnimations, LoadOnlyAnimations);
    
    model Result = ConvertToActualModel(&Loaded);
    
    return(Result);
}

// NOTE(Dima): Use this function to load only 1 animation from file
struct loaded_animations
{
    animation* Animations;
    int Count;
};

loaded_animations LoadSkeletalAnimations(char* FilePath, 
                                         loading_params Params = DefaultLoadingParams())
{
    
    b32 LoadAnimations = true;
    b32 LoadOnlyAnimations = true;
    
    loaded_model Loaded = LoadModelFileInternal(FilePath, Params, 
                                                LoadAnimations, 
                                                LoadOnlyAnimations);
    
    loaded_animations Result = {};
    Result.Count = Loaded.Animations.size();
    
    if(Result.Count)
    {
        Result.Animations = (animation*)malloc(sizeof(animation) * Result.Count);
        
        for(int AnimIndex = 0;
            AnimIndex < Result.Count;
            AnimIndex++)
        {
            Result.Animations[AnimIndex] = ConvertToActualAnimation(&Loaded.Animations[AnimIndex]);
        }
    }
    
    return(Result);
}

void FreeLoadedAnimations(loaded_animations* Animations)
{
    if(Animations->Count)
    {
        
        free(Animations->Animations);
    }
    Animations->Count = 0;
}
