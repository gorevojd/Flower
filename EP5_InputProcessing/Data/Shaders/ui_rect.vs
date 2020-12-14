#version 330 core

layout (location = 0) in vec2 InPosition;
layout (location = 1) in vec2 InTexCoords;

out vec2 TexCoords;
out vec4 Color;
flat out int IsRect;

uniform bool Is3D;
uniform bool IsBatch;
uniform vec4 MultColor;
uniform usamplerBuffer RectsColors;
uniform usamplerBuffer RectsTypes;

uniform samplerBuffer RectsTransforms;
uniform usamplerBuffer RectsIndicesToTransforms;

uniform int RectOrthoMatrixIndex;
uniform int RectPerspMatrixIndex;

mat4 FetchMatrixByIndex(int Index)
{
	int FetchIndexBase = Index * 4;

	vec4 Row0 = texelFetch(RectsTransforms, FetchIndexBase + 0);
	vec4 Row1 = texelFetch(RectsTransforms, FetchIndexBase + 1);
	vec4 Row2 = texelFetch(RectsTransforms, FetchIndexBase + 2);
	vec4 Row3 = texelFetch(RectsTransforms, FetchIndexBase + 3);

	//NOTE(Dima): Calculating glyph model matrix based on fetched data
	mat4 Result = transpose(mat4(Row0, Row1, Row2, Row3));

	return(Result);
}

void main()
{
	int PerRectIndex = gl_VertexID / 4;

	//NOTE(Dima): Fetching glyph model matrix from RectsTransforms buffer by first fetching it's index 
	uint RectIndexToTransform = texelFetch(RectsIndicesToTransforms, PerRectIndex).r;

	mat4 RectModel = FetchMatrixByIndex(int(RectIndexToTransform));
	mat4 Projection = mat4(1.0f);

	//NOTE(dima): Defining additional offset for 3d glyphs
	float ZBias = 0.0f;

	int ProjMatrixFetchIndex = RectOrthoMatrixIndex;	
	if(Is3D)
	{
		ProjMatrixFetchIndex = RectPerspMatrixIndex;
		float ZBiasRange = 0.0001f;
		ZBias = (PerRectIndex & 1) == 0 ? -ZBiasRange : ZBiasRange;
	}
	Projection = FetchMatrixByIndex(ProjMatrixFetchIndex);

	//NOTE(Dima): Calculating Clip-Space position
	gl_Position = vec4(InPosition, ZBias, 1.0f) * RectModel * Projection;

	//NOTE(Dima): Extracting color
	vec4 ExtractedColor = MultColor;
	if(IsBatch)
	{
		uint InColor = texelFetch(RectsColors, PerRectIndex).r;

		float OneOver255 = 1.0f / 255.0f;
		float InColorR = float((InColor >> 0u) & 255u) * OneOver255;
		float InColorG = float((InColor >> 8u) & 255u) * OneOver255;
		float InColorB = float((InColor >> 16u) & 255u) * OneOver255;
		float InColorA = float((InColor >> 24u) & 255u) * OneOver255;

		ExtractedColor = vec4(InColorR, InColorG, InColorB, InColorA);
	}
	
	//NOTE(dima): Fetching glyph type
	uint RectType = texelFetch(RectsTypes, PerRectIndex).r;

	//NOTE(Dima): Types of glyph(Rect_Rect = 0 or Rect_Rect3D = 2) when taking & with 1 will give 0
	IsRect = int(RectType == 0u);

	Color = ExtractedColor;
	TexCoords = InTexCoords;
}


























