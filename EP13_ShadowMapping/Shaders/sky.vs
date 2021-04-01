#version 330 core

layout (location = 0) in vec3 InPosition;
out vec3 SampleDir;

uniform mat4 Projection;
uniform mat4 View;

void main()
{
	vec4 Pos = vec4(InPosition, 1.0f) * View * Projection;
	Pos = Pos.xyww;
	Pos.z -= 0.000001;
	gl_Position = Pos;

	SampleDir = InPosition;
}