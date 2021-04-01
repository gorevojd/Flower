#version 330 core

in vec3 SampleDir;
layout (location = 0) out vec3 OutColor;

uniform int SkyType;
uniform samplerCube Sky;
uniform vec3 SkyColor;

void main()
{
	vec3 ResultColor;

	if(SkyType == 0)
	{
		ResultColor = SkyColor;
	}
	else if(SkyType == 1)
	{
		ResultColor = SkyColor;
	}
	else if(SkyType == 2)
	{
		ResultColor = texture(Sky, SampleDir).rgb;
	}

	OutColor = ResultColor;
}