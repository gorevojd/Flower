#version 330 core

layout (location = 0) in vec3 InPosition;
layout (location = 1) in vec2 InTexCoords;
layout (location = 2) in vec3 InNormal;
layout (location = 3) in vec3 InColor;

out vs_out
{
	vec3 Color;
	vec2 TexCoords;
	vec3 Normal;
} VsOut;

uniform mat4 Projection;
uniform mat4 View;
uniform mat4 Model;

void main()
{
	gl_Position = vec4(InPosition, 1.0f) * Model * View * Projection;

	VsOut.Color = InColor;
	VsOut.TexCoords = InTexCoords;
	VsOut.Normal = InNormal; 
}