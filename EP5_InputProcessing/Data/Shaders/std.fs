#version 330 core

in vs_out
{
	vec3 Color;
	vec2 TexCoords;
	vec3 Normal;
} FsIn;

uniform bool MaterialMissing;
uniform bool HasDiffuse;
uniform sampler2D TexDiffuse;

uniform vec3 MultColor;

out vec4 OutColor;

void main()
{
	vec4 SampleDiffuse = vec4(FsIn.Color, 1.0f);
	
	if(!MaterialMissing)
	{
		SampleDiffuse = vec4(1.0f, 1.0f, 1.0f, 1.0f);
		if(HasDiffuse)
		{
			SampleDiffuse = texture2D(TexDiffuse, FsIn.TexCoords);
		}

		SampleDiffuse *= vec4(MultColor, 1.0f);
	}
	
	vec3 PixelColor = SampleDiffuse.rgb;

	OutColor = vec4(PixelColor, 1.0f);
}