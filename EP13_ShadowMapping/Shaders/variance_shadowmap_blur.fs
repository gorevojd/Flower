#version 330 core

in vec2 FragUV;
out vec2 ResultColor;

uniform sampler2DArray TextureArray;
uniform int TextureIndex;
uniform int BlurRadius;

void main()
{
	vec2 PixelSize = vec2(1.0f) / textureSize(TextureArray, 0).xy;

#if 1
	vec2 BlurredSum = vec2(0.0);
	for(int j = -BlurRadius; j <= BlurRadius; j++)
	{
		for(int i = -BlurRadius; i <= BlurRadius; i++)
		{
			vec3 SampleUV = vec3(FragUV + vec2(i, j) * PixelSize, TextureIndex);
			vec2 SampleDepth = texture(TextureArray, SampleUV).rg;

			BlurredSum += SampleDepth;
		}
	}

	BlurredSum /= pow(BlurRadius * 2 + 1, 2);
#else
	vec2 BlurredSum = texture(TextureArray, vec3(FragUV, TextureIndex)).rg;
#endif

	ResultColor = BlurredSum;
}