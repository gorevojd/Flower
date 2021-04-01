#version 330 core

layout (location = 0) in uint InPosition;

out vs_out 
{
	flat vec3 Color;
	flat vec3 Normal;
	vec2 TexCoords;
	vec3 FragP;
} VsOut;

vec3 NormalTypes[6] = vec3[6](
	vec3(0.0f, 0.0f, 1.0f),
	vec3(0.0f, 0.0f, -1.0f),
	vec3(1.0f, 0.0f, 0.0f),
	vec3(-1.0f, 0.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, -1.0f, 0.0f)
);

uniform mat4 ViewProjection;
uniform mat4 Projection;
uniform mat4 View;
uniform float Time;

uniform vec3 ChunkAt;
uniform usamplerBuffer PerFaceData;

uniform bool HasClippingPlane;
uniform vec4 ClippingPlane;
out float gl_ClipDistance[1];

void main()
{
	//Extracting vertex data
	uint InChunkX = InPosition & 63u;
	uint InChunkZ = (InPosition >> 6u) & 63u;
	uint InChunkY = (InPosition >> 12u) & 255u;

	//Extracting per face data
	uint PerFaceEntry = texelFetch(PerFaceData, gl_VertexID / 6).r;
	
	//3 bits for normals
	uint NormalType = PerFaceEntry & 7u;
	VsOut.Normal = NormalTypes[NormalType];

	//8 bits for bitmap ID
	float DeltaUV = 1.0f / 16.0f;
	uint BitmapID = (PerFaceEntry >> 3u) & 255u;
	
	vec2 StartUV_ = vec2(float(BitmapID % 16u), float(BitmapID / 16u)) * DeltaUV;
	vec2 EndUV_ = StartUV_ + vec2(DeltaUV);

	vec2 SmallOffset = vec2(DeltaUV) * 0.01f;

	vec2 StartUV = StartUV_ + SmallOffset;
	vec2 EndUV = EndUV_ - SmallOffset;

	//Getting texture coordinates
	int InFaceID = gl_VertexID % 6;
	if ((InFaceID == 0) || (InFaceID == 3))
	{
		VsOut.TexCoords = StartUV;
	}
	else if(InFaceID == 1)
	{
		VsOut.TexCoords = vec2(EndUV.x, StartUV.y);
	}
	else if(InFaceID == 5)
	{
		VsOut.TexCoords = vec2(StartUV.x, EndUV.y);
	}
	else if((InFaceID == 2) || (InFaceID == 4))
	{
		VsOut.TexCoords = EndUV;
	}

	//Geometry is floating
	uint IsFloatingGeometry = (PerFaceEntry >> 11u) & 1u;
	

	//Calculating vertex clip space P
	vec3 VertP = ChunkAt + vec3(float(InChunkX), float(InChunkY), float(InChunkZ));

	vec3 AddOffset = vec3(0.0);	
	if(IsFloatingGeometry == 1u)
	{
		int TriangleIndex = InFaceID / 3;
		int InTriangleIndex = InFaceID % 3;

		float WaveTime = Time;
		AddOffset = vec3(
			cos(WaveTime * 0.5 + VertP.x * 2.0 + VertP.y * 2.3f + VertP.z * 1.8f),
			sin(WaveTime * 0.4 + VertP.x * 2.2 + VertP.y * 2.1f + VertP.z * 1.7f),
			cos(WaveTime * 0.45 + VertP.x * 1.85 + VertP.y * 2.25f + VertP.z * 2.1f)
		) * 0.1;
	}

	VertP += AddOffset;
	vec4 CalculatedP = vec4(VertP, 1.0);

	gl_Position = CalculatedP * ViewProjection;

	VsOut.FragP = VertP;
	VsOut.Color = vec3(1.0);
	gl_ClipDistance[0] = dot(CalculatedP, ClippingPlane);
}