#version 330 core

in vec2 TexCoords;
in vec4 Color;

// Is glyph used for batch rendering
flat in int IsRect;

// Is image used for non-batch rendering
uniform bool IsImage;
uniform sampler2D Image;

out vec4 OutColor;

void main()
{
	vec4 SampleColor = vec4(1.0f);
	if(IsImage && (IsRect != 0))
	{
		SampleColor = texture2D(Image, TexCoords);

		if(SampleColor.a < 0.05f)
		{
			discard;
		}
	}
	
	vec4 ResultColor = SampleColor * Color;

	OutColor = ResultColor;
}