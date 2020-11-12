#version 330 core

in vs_out
{
	vec3 Color;
} FsIn;

out vec4 OutColor;

void main()
{
	OutColor = vec4(FsIn.Color, 1.0f);
}