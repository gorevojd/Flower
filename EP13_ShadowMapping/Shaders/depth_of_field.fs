#version 330 core

in vec2 FragUV;
out vec3 OutColor;

uniform float MinDistance;
uniform float MaxDistance;
uniform float FocusZ;
uniform vec2 WH;
uniform vec4 PerspProjCoefs;

uniform sampler2D FocusTex;
uniform sampler2D OutOfFocusTex;
uniform sampler2D DepthTex;

float GetLinearizedDepth(vec2 UV)
{
	float SampleDepth = texture2D(DepthTex, UV).r;
	float Zndc = SampleDepth * 2.0f - 1.0f;
	float LinearDepth = PerspProjCoefs.w / (Zndc - PerspProjCoefs.z);

	return(LinearDepth);
}

float GetLinearizedDepth(float SampleDepth)
{
	float Zndc = SampleDepth * 2.0f - 1.0f;
	float LinearDepth = PerspProjCoefs.w / (Zndc - PerspProjCoefs.z);

	return(LinearDepth);
}

void main()
{
	float SampleDepth = texture2D(DepthTex, FragUV).r;
	vec3 FocusColor = texture2D(FocusTex, FragUV).rgb;
	
	vec3 ResultColor;
	if(SampleDepth > 0.999999)
	{
		ResultColor = FocusColor;
	}
	else
	{
		vec3 OutOfFocusColor = texture2D(OutOfFocusTex, FragUV).rgb;

		float LinearDepth = GetLinearizedDepth(SampleDepth);
		float ViewSpaceX = LinearDepth * ((gl_FragCoord.x * 2.0f) / WH.x - 1.0f) / PerspProjCoefs.x;
		float ViewSpaceY = LinearDepth * ((gl_FragCoord.y * 2.0f) / WH.y - 1.0f) / PerspProjCoefs.y;

		vec3 ViewP = vec3(ViewSpaceX, ViewSpaceY, LinearDepth);
		vec3 FocusPoint = vec3(0.0f, 0.0f, FocusZ);

		float Blur = smoothstep(MinDistance, MaxDistance, distance(ViewP, FocusPoint));
	
		ResultColor = mix(FocusColor, OutOfFocusColor, Blur);	
	}
	
	OutColor = ResultColor;
}
