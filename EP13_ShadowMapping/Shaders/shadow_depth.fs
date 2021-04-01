#version 330 core

/*
in  vs_out
{
	vec3 Color;
	vec2 TexCoords;
	vec3 Normal;
	vec3 FragP;
} FsIn;
*/

out vec2 OutColor;

void main()
{
	float SampleDepth = gl_FragCoord.z;

	float dx = dFdx(SampleDepth);
	float dy = dFdy(SampleDepth);

	OutColor.r = SampleDepth;
	OutColor.g = SampleDepth * SampleDepth + 0.25 * (dx * dx + dy * dy);
}