#version 330 core

in vec2 FragUV;
out float OutOcclusion;

uniform sampler2D OcclusionTex;
uniform int BlurRadius;

float normpdf(float x, float sigma)
{
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

#define SIGMA 10.0
#define BSIGMA 0.1
#define MSIZE 15

const float kernel[MSIZE] = float[MSIZE](
	0.031225216, 0.033322271, 0.035206333, 0.036826804, 
	0.038138565, 0.039104044, 0.039695028, 0.039894000, 
	0.039695028, 0.039104044, 0.038138565, 0.036826804, 
	0.035206333, 0.033322271, 0.031225216
);

void main()
{
	vec2 PixelSize = vec2(1.0f) / textureSize(OcclusionTex, 0);
	float SumOcclusion = 0.0f;

#if 1
	for(int j = -BlurRadius; j <= BlurRadius; j++)
	{
		for(int i = -BlurRadius; i <= BlurRadius; i++)
		{
			float Occlusion = texture2D(OcclusionTex, FragUV + vec2(i, j) * PixelSize).r;

			SumOcclusion += Occlusion;
		}
	}
	SumOcclusion /= (BlurRadius * 2 + 1) * (BlurRadius * 2 + 1);
#else

	const int kSize = (MSIZE-1)/2;
	float bZ = 1.0 / normpdf(0.0, BSIGMA);
	float Z = 0.0;

	float CurAO = texture2D(OcclusionTex, FragUV).r;
	for (int i = -kSize; i <= kSize; ++i)
	{
		for (int j = -kSize; j <= kSize; ++j)
		{
			float SampleAO = texture2D(OcclusionTex, FragUV + vec2(i, j) *  PixelSize).r;
			
			float Factor = normpdf(SampleAO - CurAO, BSIGMA) * bZ * 
				kernel[kSize + j] * kernel[kSize + i];
			Z += Factor;

			SumOcclusion += Factor * SampleAO;
		}
	}

	SumOcclusion = SumOcclusion / Z;
#endif

	OutOcclusion = SumOcclusion;
}