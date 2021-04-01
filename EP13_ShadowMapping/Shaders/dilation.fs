#version 330 core

in vec2 FragUV;
out vec3 OutColor;

uniform int Size;
uniform vec2 MinMaxThreshold;
uniform sampler2D InputTexture;

void main()
{
	vec2 PixelSize = vec2(1.0f) / textureSize(InputTexture, 0);

	float MinThreshold = MinMaxThreshold.x;
	float MaxThreshold = MinMaxThreshold.y;

	vec3 FragColor = texture2D(InputTexture, FragUV).rgb;

	float MaxGrayscale = 0.0;
	vec3 MaxColor = FragColor;

	for(int j = -Size; j <= Size; j++)
	{
		for(int i = -Size; i <= Size; i++)
		{
			bool InCircle = (i * i + j * j) <= (Size * Size);

			if(!InCircle)
			{
				continue;
			}

			vec3 Color = texture2D(InputTexture, FragUV + vec2(i, j) * PixelSize).rgb;

			float Grayscale = dot(Color, vec3(0.21, 0.72, 0.07));

			if(Grayscale > MaxGrayscale)
			{
				MaxGrayscale = Grayscale;
				MaxColor = Color;
			}
		}
	}
	
	float t = smoothstep(MinThreshold, MaxThreshold, MaxGrayscale);
	OutColor = mix(FragColor, MaxColor, t);
}