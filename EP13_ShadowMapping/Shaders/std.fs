#version 330 core

in vs_out
{
	vec3 Color;
	vec2 TexCoords;
	vec3 Normal;
	vec3 FragP;
} FsIn;

layout (location = 0) out vec4 ColorSpec; 
layout (location = 1) out vec3 Normal;
layout (location = 2) out vec3 Positions;

uniform bool MaterialMissing;
uniform bool HasDiffuse;
uniform sampler2D TexDiffuse;
uniform vec3 MultColor;

void main()
{
	vec4 SampleDiffuse = vec4(MultColor * FsIn.Color, 1.0f);
	
	if(!MaterialMissing)
	{
		SampleDiffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		if(HasDiffuse)
		{
			SampleDiffuse = texture2D(TexDiffuse, FsIn.TexCoords);
		}

		SampleDiffuse *= vec4(MultColor * FsIn.Color, 1.0f);
	}
	
	ColorSpec = vec4(SampleDiffuse.rgb, 0.1f);
	Positions = FsIn.FragP;
	Normal = normalize(FsIn.Normal);
}