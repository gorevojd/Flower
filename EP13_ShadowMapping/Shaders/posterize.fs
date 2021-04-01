#version 330 core

in vec2 FragUV;
out vec3 OutColor;

uniform int Levels;
uniform sampler2D InputTexture;

void main()
{
	vec3 FragColor = texture2D(InputTexture, FragUV).rgb;

	float Grayscale = max(FragColor.r, max(FragColor.g, FragColor.b));

	float Lower = floor(Grayscale * Levels) / Levels;
	float LowerDiff = abs(Grayscale - Lower);

	float Upper = ceil(Grayscale * Levels) / Levels;
	float UpperDiff = abs(Grayscale - Upper);

	float Level = Upper;
	if(LowerDiff <= UpperDiff)
	{
		Level = Lower;
	}

	float Adjustment = Level / Grayscale;
	
	OutColor = FragColor * Adjustment;
}