#version 330 core

layout (location = 0) in vec2 InPosition;
layout (location = 1) in vec2 InTexCoords;

out vec2 TexCoords;
out vec4 Color;
flat out int IsGlyph;

uniform bool Is3D;
uniform bool IsBatch;
uniform vec4 MultColor;
uniform usamplerBuffer GlyphsColors;
uniform usamplerBuffer GlyphsTypes;

uniform samplerBuffer GlyphsTransforms;
uniform usamplerBuffer GlyphsIndicesToTransforms;

uniform int GlyphDefaultProjectionMatrixIndex;
uniform int GlyphOrthoMatrixIndex;
uniform int GlyphPerspMatrixIndex;

mat4 FetchMatrixByIndex(int Index)
{
	int FetchIndexBase = Index * 4;

	vec4 Row0 = texelFetch(GlyphsTransforms, FetchIndexBase + 0);
	vec4 Row1 = texelFetch(GlyphsTransforms, FetchIndexBase + 1);
	vec4 Row2 = texelFetch(GlyphsTransforms, FetchIndexBase + 2);
	vec4 Row3 = texelFetch(GlyphsTransforms, FetchIndexBase + 3);

	//NOTE(Dima): Calculating glyph model matrix based on fetched data
	mat4 Result = transpose(mat4(Row0, Row1, Row2, Row3));

	return(Result);
}

void main()
{
	int PerGlyphIndex = gl_VertexID / 4;

	//NOTE(Dima): Fetching glyph model matrix from GlyphsTransforms buffer by first fetching it's index 
	uint GlyphIndexToTransform = texelFetch(GlyphsIndicesToTransforms, PerGlyphIndex).r;

	mat4 GlyphModel = FetchMatrixByIndex(int(GlyphIndexToTransform));
	mat4 Projection = mat4(1.0f);

	//NOTE(dima): Defining additional offset for 3d glyphs
	float ZBias = 0.0f;

	int ProjMatrixFetchIndex = GlyphOrthoMatrixIndex;	
	if(Is3D)
	{
		ProjMatrixFetchIndex = GlyphPerspMatrixIndex;
		float ZBiasRange = 0.0001f;
		ZBias = (PerGlyphIndex & 1) == 0 ? -ZBiasRange : ZBiasRange;
	}
	Projection = FetchMatrixByIndex(ProjMatrixFetchIndex);

	//NOTE(Dima): Calculating Clip-Space position
	gl_Position = vec4(InPosition, ZBias, 1.0f) * GlyphModel * Projection;

	//NOTE(Dima): Extracting color
	vec4 ExtractedColor = MultColor;
	if(IsBatch)
	{
		uint InColor = texelFetch(GlyphsColors, PerGlyphIndex).r;

		float OneOver255 = 1.0f / 255.0f;
		float InColorR = float((InColor >> 0u) & 255u) * OneOver255;
		float InColorG = float((InColor >> 8u) & 255u) * OneOver255;
		float InColorB = float((InColor >> 16u) & 255u) * OneOver255;
		float InColorA = float((InColor >> 24u) & 255u) * OneOver255;

		ExtractedColor = vec4(InColorR, InColorG, InColorB, InColorA);
	}
	
	//NOTE(dima): Fetching glyph type
	uint GlyphType = texelFetch(GlyphsTypes, PerGlyphIndex).r;

	//NOTE(Dima): Types of glyph(Glyph_Glyph = 0 or Glyph_Glyph3D = 2) when taking & with 1 will give 0
	IsGlyph = int(GlyphType == 0u);

	Color = ExtractedColor;
	TexCoords = InTexCoords;
}


























