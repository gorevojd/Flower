#version 330 core

in vec2 TexCoords;
in vec4 Color;
flat in int IsTextured;

out vec4 OutColor;

// Is image used for non-batch rendering
uniform bool IsImage;
uniform sampler2D Image;

void main()
{
	vec4 SampleColor = vec4(1.0f);
	if(IsImage && (IsTextured != 0))
	{
		SampleColor = texture2D(Image, TexCoords);
	}

	OutColor = SampleColor * Color;
}