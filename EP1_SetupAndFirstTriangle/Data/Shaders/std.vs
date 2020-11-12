#version 330 core

layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec3 InColor;

out vs_out
{
	vec3 Color;
} VsOut;

void main()
{
	gl_Position = vec4(InPosition, 1.0f);

	VsOut.Color = InColor; 
}