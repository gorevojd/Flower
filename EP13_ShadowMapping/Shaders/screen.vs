#version 330 core

layout (location = 0) in vec4 InPosUV;

out vec2 FragUV;

void main()
{
	FragUV  = InPosUV.zw;

	gl_Position = vec4(InPosUV.xy, 0.0f, 1.0f);
}