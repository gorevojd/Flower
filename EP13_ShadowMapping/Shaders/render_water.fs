#version 330 core

in vec2 FragUV;
out vec3 Color;

uniform sampler2D SceneRender;
uniform sampler2D ScenePositionsTex;
uniform sampler2D SceneDepthTex;
uniform sampler2D ReflectionRender;

uniform vec4 ClipPlane;
uniform vec3 CameraP;
uniform vec4 PerspProjCoefs;

uniform vec3 WaterColor;

const float MinWaterReflection = 0.05;
const float MaxWaterReflection = 0.94;

const float MinVisibleDepth = 5.0;
const float MaxVisibleDepth = 25.0;

const vec3 WaterDepthColor = vec3(0.3, 0.6, 1.0);

float GetLinearizedDepth(float SampleDepth)
{
	float Zndc = SampleDepth * 2.0 - 1.0;
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

vec3 RayPlaneIntersect(vec3 O, vec3 D, vec4 Plane)
{
	float t = -(Plane.w + dot(O, Plane.xyz)) / dot(D, Plane.xyz);

	vec3 Result = O + D * t;

	return(Result);
}

float RayPlaneIntersectT(vec3 O, vec3 D, vec4 Plane)
{
	float t = -(Plane.w + dot(O, Plane.xyz)) / dot(D, Plane.xyz);

	return(t);
}

void main()
{
	vec3 WorldP = texture2D(ScenePositionsTex, FragUV).xyz;
	vec3 ToCamera = CameraP - WorldP;

	vec3 SceneC = texture2D(SceneRender, FragUV).xyz;
	vec3 ReflectC = texture2D(ReflectionRender, FragUV).xyz;
	vec3 ResultColor = SceneC;

	vec3 ViewDirNorm = normalize(-ToCamera);

	if(texture2D(SceneDepthTex, FragUV).r < 0.99999999)
	{
		if(dot(vec4(CameraP, 1.0), ClipPlane) > 0)
		{
			float PlaneTest = dot(vec4(WorldP, 1.0), ClipPlane);
			if(PlaneTest < 0)
			{
				vec3 OnWater = RayPlaneIntersect(CameraP, ViewDirNorm, ClipPlane);
				vec3 OnWaterToCamera = CameraP - OnWater;
				float InWaterDist = length(ToCamera) - length(OnWaterToCamera);

				vec3 UnderwaterColor = SceneC * WaterColor;
				
				float tColor = smoothstep(MinVisibleDepth, MaxVisibleDepth, InWaterDist);
				UnderwaterColor = mix(UnderwaterColor, WaterDepthColor, tColor);

				float FresnelEffect = 1.0 - clamp(dot(-ViewDirNorm, ClipPlane.xyz), 0, 1);
				float Reflectivity = mix(MinWaterReflection, MaxWaterReflection, FresnelEffect);

				vec3 ReflectedColor = Reflectivity * ReflectC;
				vec3 DiffuseColor = (1.0 - Reflectivity) * UnderwaterColor;

				ResultColor = ReflectedColor + DiffuseColor;
			}
		}
		else
		{
			ResultColor = SceneC * WaterDepthColor;
		}
	}

	Color = ResultColor;
	//Color = vec3(1.0, 0.0, 0.0);
}