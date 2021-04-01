#version 330 core

in vec2 FragUV;
out vec4 Color;

uniform sampler2D DepthTex;
uniform sampler2D NormalTex;
uniform sampler2D ColorSpecTex;
uniform sampler2D SSAOTex;
uniform sampler2D PositionsTex;
uniform sampler2D SkyTexture;
uniform vec4 PerspProjCoefs;
uniform vec2 WH;

uniform vec3 CameraP;
uniform float AmbientPercentage;
uniform float ShadowStrength;

uniform vec3 DirectionalLightDirection;
uniform vec3 DirectionalLightColor;
uniform bool CalculateDirLightShadow;
uniform sampler2DArray LightDepthTex;
uniform float CascadeDistances[8];
uniform mat4 CascadeLightProjections[8];
uniform int CascadeCount;

uniform vec2 PoissonSamples[12];
uniform sampler2D PoissonSamplesRotations;
uniform bool ShouldRotateSamples;

uniform bool SSAOEnabled;
uniform float SSAOContribution;

vec3 CalcDirLit(vec3 FragSampleC, vec3 FragN)
{
	float DiffCoef = max(dot(FragN, -DirectionalLightDirection), 0.0f);
	vec3 Result = DiffCoef * DirectionalLightColor * FragSampleC;

	return(Result); 
}

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

float LinStep(float Value, float Min, float Max)
{
	float Result = clamp((Value - Min) / (Max - Min), 0, 1);

	return(Result);
}

float ReduceLightBleeding(float PMax, float Amount)
{
	float Result = LinStep(PMax, Amount, 1);

	return(Result);
}

float ShadowCalculation(vec3 WorldP, vec3 WorldN, float LinearDepth)
{
	float Shadow = 0.0;
	
	vec3 ShadowTexSize = textureSize(LightDepthTex, 0);

	int SampleLayerIndex = 0;
	for(int CascadeIndex = 0;
		CascadeIndex < CascadeCount;
		CascadeIndex++)
	{
		if(LinearDepth < CascadeDistances[CascadeIndex])
		{		
			SampleLayerIndex = CascadeIndex;
			break;
		}
	}

	vec4 FragLitProjected = vec4(WorldP, 1.0) * CascadeLightProjections[SampleLayerIndex];
	FragLitProjected /= FragLitProjected.w;
	FragLitProjected.xyz = FragLitProjected.xyz * 0.5 + 0.5;

	float FragDepth = FragLitProjected.z;

	float Bias = max(0.0015, 0.01 * (1.0 - dot(WorldN, -DirectionalLightDirection)));	

	vec2 TexelSize = vec2(1.0) / ShadowTexSize.xy;		
	vec3 SampleDepthUV = vec3(FragLitProjected.xy, SampleLayerIndex);

	float M1 = texture(LightDepthTex, SampleDepthUV).r;
	float M2 = texture(LightDepthTex, SampleDepthUV).g;
	float ClosestDepth = M1;

	float p = step(FragDepth, M1);
	float Nu = M1;
	float SigmaSquared = max(M2 - M1 * M1, 0.0000002);
	float Temp = FragDepth - Nu;
	float PMaxT = SigmaSquared / (SigmaSquared + Temp * Temp);
	PMaxT = clamp(max(p, PMaxT), 0, 1);
	PMaxT = ReduceLightBleeding(PMaxT, 0.2);

	Shadow = 1.0 - PMaxT;

#if 0
	if(FragDepth - Bias > ClosestDepth)
	{
		Shadow = 1.0f;
	}
#endif

#if 0
#if 0
	for(int j = -1; j <= 1; j++)
	{
		for(int i = -1; i <= 1; i++)
		{
			vec2 SampleOffset = vec2(i, j) * TexelSize;

			vec3 NewSampleCoord = vec3(FragLitProjected.xy + SampleOffset, SampleLayerIndex);
			float pcfDepth = texture(LightDepthTex, NewSampleCoord).r;
			if(FragDepth - Bias > pcfDepth)
			{
				Shadow += 1.0 * ShadowStrength;
			}
		}
	}
	Shadow /= 9.0;
#else
	vec2 DimScaleRotations = ShadowTexSize.xy / vec2(4.0);

	for(int i = 0; i < PoissonSamples.length(); i++)
	{
		// Reading random rotation vector
		vec2 SampleRandRotationUV = FragLitProjected.xy * DimScaleRotations;

		vec2 SampleOffset = PoissonSamples[i] * TexelSize;

		// Applying random rotation vector
		if(ShouldRotateSamples)
		{
			float RandomRotation = texture2D(PoissonSamplesRotations, SampleRandRotationUV).r;
			float s = sin(RandomRotation);
			float c = cos(RandomRotation);

			SampleOffset.x = dot(SampleOffset, vec2(c, s));
			SampleOffset.y = dot(SampleOffset, vec2(-s, c));
		}

		// Using rotated offset to fetch shadow sample
		vec3 NewSampleCoord = vec3(FragLitProjected.xy + SampleOffset, SampleLayerIndex);
		float pcfDepth = texture(LightDepthTex, NewSampleCoord).r;
		if(FragDepth - Bias > pcfDepth)
		{
			Shadow += 1.0 * ShadowStrength;
		}
	}
	Shadow /= 12.0f;
#endif
#endif

	return(Shadow);
}

void main()
{
	
	vec2 PixelDelta = vec2(1.0) / WH;

	vec4 SampleColorSpec = texture2D(ColorSpecTex, FragUV);
	vec3 WorldP = texture2D(PositionsTex, FragUV).xyz;
	vec3 WorldN = texture2D(NormalTex, FragUV).xyz;
	float SampleDepth = texture2D(DepthTex, FragUV).r;
	float LinearDepth = GetLinearizedDepth(SampleDepth);

	float TotalShadow = 0.0f;
	if(CalculateDirLightShadow)
	{
		TotalShadow = ShadowCalculation(WorldP, WorldN, LinearDepth);
	}
	
	if(SSAOEnabled)
	{
		float SampleOcclusion = (1.0 - texture2D(SSAOTex, FragUV).r) * SSAOContribution;

		TotalShadow += SampleOcclusion;
	}

	TotalShadow = clamp(1.0 - TotalShadow, 0, 1);

	vec3 ResultColor = vec3(0.0);
	ResultColor += SampleColorSpec.rgb * AmbientPercentage;
	ResultColor += CalcDirLit(SampleColorSpec.rgb, WorldN) * TotalShadow;
	
	vec3 SkyColor = texture2D(SkyTexture, FragUV).rgb;
	if(SampleDepth > 0.999999)
	{
		ResultColor = SkyColor;
	}
	
	Color = vec4(ResultColor, 1.0);
	
	//Color = vec4(vec3(texture2D(SSAOTex, FragUV).r), 1.0f);
	//Color = vec4(vec3(GetLinearizedDepth(FragUV) / 1500.0f), 1.0f);
	//Color = vec4(SamplePosition, 1.0f);
	//Color = vec4(SampleColorSpec.rgb, 1.0f);
}
