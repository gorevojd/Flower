#include <vector>
#include <string>
#include <map>
#include <algorithm>

struct helper_byte_buffer
{
    void* Data;
    mi DataSize;
    
    std::unordered_map<std::string, mi> NameToOffset;
    std::unordered_map<std::string, int> NameToCount;
    
    helper_byte_buffer()
    {
        Data = 0;
        DataSize = 0;
    }
    
    u32 AddPlace(std::string Name, 
                 int NumElements, 
                 mi SizeOfElement)
    {
        u32 Offset = 0;
        
        NameToCount.insert(std::pair<std::string, int>(Name, NumElements));
        
        if(NumElements > 0)
        {
            Offset = DataSize;
            
            NameToOffset.insert(std::pair<std::string, mi>(Name, Offset));
            
            DataSize += NumElements * SizeOfElement;
        }
        
        return(Offset);
    }
    
    void Generate()
    {
        Data = malloc(DataSize);
    }
    
    void Free()
    {
        if(Data != 0)
        {
            free(Data);
        }
        Data = 0;
    }
    
    void* GetPlace(std::string GetName)
    {
        void* Result = 0;
        
        Assert(NameToCount.find(GetName) != NameToCount.end());
        
        if(NameToCount[GetName] > 0)
        {
            Assert(NameToOffset.find(GetName) != NameToOffset.end());
            
            Result = (void*)((u8*)Data + NameToOffset[GetName]);
        }
        
        return(Result);
    }
};


struct helper_mesh
{
    std::string Name;
    
    std::vector<v3> Vertices;
    std::vector<v2> TexCoords;
    std::vector<v3> Normals;
    std::vector<v3> Colors;
    std::vector<u32> Indices;
    std::vector<v4> BoneWeights;
    std::vector<u32> BoneIndices;
    
    b32 IsSkinned;
};

INTERNAL_FUNCTION helper_mesh CombineHelperMeshes(const helper_mesh* A, const helper_mesh* B)
{
    helper_mesh Result = {};
    
    Result.IsSkinned = A->IsSkinned;
    Result.Name = A->Name;
    
    // NOTE(Dima): Copy positions
    std::copy(A->Vertices.begin(), A->Vertices.end(), std::back_inserter(Result.Vertices));
    std::copy(B->Vertices.begin(), B->Vertices.end(), std::back_inserter(Result.Vertices));
    
    // NOTE(Dima):  copy tex coords
    std::copy(A->TexCoords.begin(), A->TexCoords.end(), std::back_inserter(Result.TexCoords));
    std::copy(B->TexCoords.begin(), B->TexCoords.end(), std::back_inserter(Result.TexCoords));
    
    // NOTE(Dima): Copy colors
    std::copy(A->Colors.begin(), A->Colors.end(), std::back_inserter(Result.Colors));
    std::copy(B->Colors.begin(), B->Colors.end(), std::back_inserter(Result.Colors));
    
    // NOTE(Dima): Copy normals
    std::copy(A->Normals.begin(), A->Normals.end(), std::back_inserter(Result.Normals));
    std::copy(B->Normals.begin(), B->Normals.end(), std::back_inserter(Result.Normals));
    
    // NOTE(Dima): Copy bone weights
    std::copy(A->BoneWeights.begin(), A->BoneWeights.end(), std::back_inserter(Result.BoneWeights));
    std::copy(B->BoneWeights.begin(), B->BoneWeights.end(), std::back_inserter(Result.BoneWeights));
    
    // NOTE(Dima): Bone indices
    std::copy(A->BoneIndices.begin(), A->BoneIndices.end(), std::back_inserter(Result.BoneIndices));
    std::copy(B->BoneIndices.begin(), B->BoneIndices.end(), std::back_inserter(Result.BoneIndices));
    
    // NOTE(Dima): Copy ids and recalculate ids from second array
    std::copy(A->Indices.begin(), A->Indices.end(), std::back_inserter(Result.Indices));
    for(int IndexIndex = 0;
        IndexIndex < B->Indices.size();
        IndexIndex++)
    {
        Result.Indices.push_back(B->Indices[IndexIndex] + A->Vertices.size());
    }
    
    return(Result);
}

INTERNAL_FUNCTION image AllocateImageInternal(u32 Width, u32 Height, void* PixData)
{
    image Result = {};
    
	Result.Width = Width;
	Result.Height = Height;
    
	Result.WidthOverHeight = (float)Width / (float)Height;
    
	Result.Pixels = PixData;
    
	return(Result);
}
