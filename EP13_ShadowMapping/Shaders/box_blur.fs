#version 330 core

in vec2 FragUV;
out vec3 OutColor;

uniform sampler2D ToBlurTex;
uniform int BlurRadius;

void main()
{
	vec2 PixelSize = vec2(1.0f) / textureSize(ToBlurTex, 0);

	vec3 SumColor = vec3(0.0);
	for(int j = -BlurRadius; j <= BlurRadius; j++)
	{
		for(int i = -BlurRadius; i <= BlurRadius; i++)
		{
			vec3 Color = texture2D(ToBlurTex, FragUV + vec2(i, j) * PixelSize).rgb;

			SumColor += Color;
		}
	}

	OutColor = SumColor / pow(BlurRadius * 2 + 1, 2);
}