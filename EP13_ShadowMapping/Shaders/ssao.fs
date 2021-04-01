#version 330 core

in vec2 FragUV;
out float OutOcclusion;

uniform mat4 View;
uniform vec4 PerspProjCoefs;
uniform vec2 WH;

uniform sampler2D DepthTex;
uniform sampler2D NormalTex;
uniform sampler2D SSAONoiseTex;

uniform vec3 SSAOKernel[128];
uniform int SSAOKernelSamplesCount;
uniform float SSAOKernelRadius;
uniform float SSAORangeCheck;

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

vec3 GetViewSpaceP(sampler2D TextureDepth, vec2 UV)
{
	float SampleDepth = texture2D(TextureDepth, UV).r;
	float LinearDepth = GetLinearizedDepth(SampleDepth);
	vec2 ViewSpaceXY = LinearDepth * (UV * 2.0 - vec2(1.0)) / PerspProjCoefs.xy;
	vec3 Result = vec3(ViewSpaceXY, LinearDepth);

	return(Result);
}

void main()
{
	float SampleDepth = texture2D(DepthTex, FragUV).r;

	if(SampleDepth < 0.9999999)
	{	
		float LinearDepth = GetLinearizedDepth(SampleDepth);
		float ViewSpaceX = LinearDepth * ((gl_FragCoord.x * 2.0f) / WH.x - 1.0f) / PerspProjCoefs.x;
		float ViewSpaceY = LinearDepth * ((gl_FragCoord.y * 2.0f) / WH.y - 1.0f) / PerspProjCoefs.y;

		vec3 WorldN = texture2D(NormalTex, FragUV).xyz;
		vec3 ViewN = WorldN * mat3(View);
		vec3 ViewP = vec3(ViewSpaceX, ViewSpaceY, LinearDepth);

		vec2 NoiseScale = WH / vec2(4.0f);
	
		vec3 Rvec = texture2D(SSAONoiseTex, FragUV * NoiseScale).xyz;
		vec3 T = normalize(Rvec - ViewN * dot(ViewN, Rvec));
		vec3 B = cross(T, ViewN);

		mat3 TBN = transpose(mat3(T, B, ViewN));

		float Occlusion = 0.0f;
		for(int i = 0; i < SSAOKernelSamplesCount; i++)
		{
			vec3 Sample = SSAOKernel[i] * TBN;
			Sample = Sample * SSAOKernelRadius + ViewP;

			vec2 SampleProjected;
			SampleProjected.x = Sample.x * PerspProjCoefs.x;
			SampleProjected.y = Sample.y * PerspProjCoefs.y;
			SampleProjected /= Sample.z;
			SampleProjected = SampleProjected * 0.5f + vec2(0.5f);

			float SampleDepth = GetLinearizedDepth(SampleProjected);

			//float RangeCheck = abs(SampleDepth - Sample.z) < SSAORangeCheck ? 1.0f : 0.0f;
			float RangeCheck = smoothstep(0.0f, 1.0f, SSAORangeCheck / abs(SampleDepth - Sample.z));

			if(Sample.z > SampleDepth)
			{
				Occlusion += 1.0f * RangeCheck;
			}
		}
	
		OutOcclusion = 1.0 - Occlusion / SSAOKernelSamplesCount;
	}
	else
	{
		OutOcclusion = 1.0f;
	}
}