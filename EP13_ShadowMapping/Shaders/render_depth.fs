#version 330 core

in vec2 FragUV;
out vec3 Color;

uniform bool SamplerIsArrayTexture; 
uniform sampler2DArray DepthTextureArray;
uniform int DepthLayerIndex;
uniform bool IsSecondDepth;

void main()
{
	float DepthValue = 0.0f;
	if(SamplerIsArrayTexture)
	{
		vec2 Sampled = texture(DepthTextureArray, vec3(FragUV, DepthLayerIndex)).rg;
		DepthValue = Sampled.r;
		if(IsSecondDepth)
		{
			DepthValue = Sampled.g;
		}
	}

	Color = vec3(DepthValue);
}