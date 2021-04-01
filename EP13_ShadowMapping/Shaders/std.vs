#version 330 core

layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec2 InTexCoords;
layout (location = 2) in vec3 InNormal;
layout (location = 3) in uint InColor;
layout (location = 4) in vec4 InWeights;
layout (location = 5) in uint InBoneIDs;

layout (location = 6) in vec4 InInstanceModelTran1;
layout (location = 7) in vec4 InInstanceModelTran2;
layout (location = 8) in vec4 InInstanceModelTran3;
layout (location = 9) in vec4 InInstanceModelTran4;

out vs_out
{
	vec3 Color;
	vec2 TexCoords;
	vec3 Normal;
	vec3 FragP;
} VsOut;

uniform mat4 ViewProjection;
uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;
uniform bool UseInstancing;

uniform samplerBuffer SkinningMatrices;
uniform int SkinningMatricesCount;
uniform bool MeshIsSkinned;

uniform bool HasClippingPlane;
uniform vec4 ClippingPlane;
out float gl_ClipDistance[1];

mat4 ExtractMatrixForBoneID(int BoneID)
{
	int InstanceOffset = gl_InstanceID * SkinningMatricesCount * 4;
	vec4 SkinMatrixRow0 = texelFetch(SkinningMatrices, InstanceOffset + BoneID * 4 + 0);
	vec4 SkinMatrixRow1 = texelFetch(SkinningMatrices, InstanceOffset + BoneID * 4 + 1);
	vec4 SkinMatrixRow2 = texelFetch(SkinningMatrices, InstanceOffset + BoneID * 4 + 2);
	vec4 SkinMatrixRow3 = texelFetch(SkinningMatrices, InstanceOffset + BoneID * 4 + 3);
	
	mat4 SkinMatrix = mat4(SkinMatrixRow0,
							SkinMatrixRow1,
							SkinMatrixRow2,
							SkinMatrixRow3);

	mat4 Result = transpose(SkinMatrix);

	return(Result);
}

void main()
{
	vec4 CalculatedP = vec4(InPosition, 1.0f);
	vec3 CalculatedN = InNormal;

	if(MeshIsSkinned)
	{
		int SkinMatrixIndex0 = int(InBoneIDs & 255u);
		int SkinMatrixIndex1 = int((InBoneIDs >> 8u) & 255u);
		int SkinMatrixIndex2 = int((InBoneIDs >> 16u) & 255u);
		int SkinMatrixIndex3 = int((InBoneIDs >> 24u) & 255u);
		
		mat4 Skin0 = ExtractMatrixForBoneID(SkinMatrixIndex0);
		mat4 Skin1 = ExtractMatrixForBoneID(SkinMatrixIndex1);
		mat4 Skin2 = ExtractMatrixForBoneID(SkinMatrixIndex2);
		mat4 Skin3 = ExtractMatrixForBoneID(SkinMatrixIndex3);

		mat4 ResultTransform = Skin0 * InWeights.x + 
								Skin1 * InWeights.y +
								Skin2 * InWeights.z +
								Skin3 * InWeights.w;

		CalculatedP = CalculatedP * ResultTransform;
		CalculatedN = CalculatedN * mat3(ResultTransform);
	}

	//NOTE(Dima): Instansing processing
	mat4 UseModel = Model;
	if(UseInstancing)
	{
		UseModel = transpose(mat4(InInstanceModelTran1,
								InInstanceModelTran2,
								InInstanceModelTran3,
								InInstanceModelTran4));
	}

	vec4 WorldSpaceVertex = CalculatedP * UseModel;
	vec4 ViewSpaceVertex = WorldSpaceVertex * View;
	gl_Position = WorldSpaceVertex * ViewProjection;

	vec3 ExtractedColor;
	float OneOver255 = 0.00392156862f;
	ExtractedColor.r = float(InColor & 255u) * OneOver255;
	ExtractedColor.g = float((InColor >> 8u) & 255u) * OneOver255;
	ExtractedColor.b = float((InColor >> 16u) & 255u) * OneOver255;

	VsOut.Color = ExtractedColor;
	VsOut.TexCoords = vec2(InTexCoords.x, 1.0f - InTexCoords.y);
	VsOut.Normal = CalculatedN * mat3(transpose(inverse(Model))); 
	VsOut.FragP = WorldSpaceVertex.xyz;
	gl_ClipDistance[0] = dot(CalculatedP, ClippingPlane);
}