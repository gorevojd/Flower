#version 330 core

in vs_out
{
	flat vec3 Color;
	flat vec3 Normal;
	vec2 TexCoords;
	vec3 FragP;
} FsIn;

layout (location = 0) out vec4 ColorSpec; 
layout (location = 1) out vec3 Normal;
layout (location = 2) out vec3 Positions;

uniform sampler2D TextureAtlas;

void main()
{
	vec4 SampleDiffuse = texture2D(TextureAtlas, FsIn.TexCoords);

	ColorSpec = vec4(SampleDiffuse.rgb, 0.1f);
	Normal = normalize(FsIn.Normal);
	Positions = FsIn.FragP;
}