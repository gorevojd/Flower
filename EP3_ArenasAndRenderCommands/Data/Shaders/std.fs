#version 330 core

in vs_out
{
	vec3 Color;
	vec2 TexCoords;
	vec3 Normal;
} FsIn;

uniform sampler2D TexDiffuse;
uniform vec3 MultColor;

out vec4 OutColor;

void main()
{
	vec4 SampleDiffuse = texture2D(TexDiffuse, FsIn.TexCoords);

	vec3 PixelColor = SampleDiffuse.rgb * FsIn.Color * MultColor;

	OutColor = vec4(PixelColor, 1.0f);
}