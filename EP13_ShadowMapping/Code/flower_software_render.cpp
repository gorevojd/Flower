// NOTE(Dima): Src image has to be with premultiplied alpha
void RenderOneBitmapIntoAnother(image* Dst, 
                                image* Src,
                                int StartX,
                                int StartY,
                                v4 ModColor) 
{
	float OneOver255 = 1.0f / 255.0f;
    
    Assert(Dst->Format == ImageFormat_RGBA);
    
	int MaxToX = StartX + Src->Width;
	int MaxToY = StartY + Src->Height;
    
	Assert(MaxToX <= Dst->Width);
	Assert(MaxToY <= Dst->Height);
    
    int SrcX = 0;
    int SrcY = 0;
    
    u32 SrcPitch = ImageFormatPixelSizes[Src->Format];
    
    for (int Y = StartY; Y < MaxToY; Y++) 
    {
        SrcY = Y - StartY;
        
        for (int X = StartX; X < MaxToX; X++)
        {
            SrcX = X - StartX;
            
            void* From = (u8*)Src->Pixels + (SrcY * Src->Width + SrcX) * SrcPitch;
            u32* To = (u32*)Dst->Pixels + Y * Dst->Width + X;
            
            // NOTE(Dima): Extracting dst color
            u32 DstValue = *((u32*)To);
            v4 DstInitColor = UnpackRGBA(DstValue);
            
            // NOTE(Dima): Extracting src color
            v4 FromColor;
            u32 FromValue = *((u32*)From);
            if(Src->Format == ImageFormat_RGBA)
            {
                FromColor = UnpackRGBA(FromValue);
            }
            else if(Src->Format == ImageFormat_Grayscale)
            {
                FromColor = UnpackGrayscalePremultiplied(FromValue & 0xFF);
            }
            
            // NOTE(Dima): Calculate result color
            v4 ResultColor = FromColor * ModColor;
            
            //NOTE(dima): Calculating blend alpha value
            float BlendAlpha = ResultColor.a;
            
            ResultColor.x = ResultColor.x + DstInitColor.x * (1.0f - BlendAlpha);
            ResultColor.y = ResultColor.y + DstInitColor.y * (1.0f - BlendAlpha);
            ResultColor.z = ResultColor.z + DstInitColor.z * (1.0f - BlendAlpha);
            ResultColor.a = ResultColor.a + DstInitColor.a - ResultColor.a * DstInitColor.a;
            
            // NOTE(Dima): Storing
            u32 ColorValue = PackRGBA(ResultColor);
            
            *To = ColorValue;
            
            SrcX++;
        }
        
        SrcY++;
    }
}


u32 Calcualte2DGaussianBoxComponentsCount(int Radius)
{
	int Diameter = Radius + Radius + 1;
    
	u32 Result = Diameter * Diameter;
    
	return(Result);
}

void Normalize2DGaussianBox(float* Box, int Radius)
{
	//NOTE(dima): Calculate sum of all elements
	float TempSum = 0.0f;
	int Diam = Radius + Radius + 1;
	for (int i = 0; i < Diam * Diam; i++) 
    {
		TempSum += Box[i];
	}
    
	//NOTE(dima): Normalize elements
	float NormValueMul = 1.0f / TempSum;
    
	for (int i = 0; i < Diam * Diam; i++)
    {
		Box[i] *= NormValueMul;
	}
}

void Calculate2DGaussianBox(float* Box, int Radius) 
{
	int Diameter = Radius + Radius + 1;
    
	int Center = Radius;
    
	float Sigma = (float)Radius;
    
	float A = 1.0f / (2.0f * F_PI * Sigma * Sigma);
    
	float InExpDivisor = 2.0f * Sigma * Sigma;
    
	float TempSum = 0.0f;
    
	//NOTE(dima): Calculate elements
	for (int y = 0; y < Diameter; y++) {
		int PsiY = y - Center;
		for (int x = 0; x < Diameter; x++) {
			int PsiX = x - Center;
            
			float ValueToExp = -((PsiX * PsiX + PsiY * PsiY) / InExpDivisor);
            
			float Expon = Exp(ValueToExp);
            
			float ResultValue = A * Expon;
            
			Box[y * Diameter + x] = ResultValue;
			TempSum += ResultValue;
		}
	}
    
	//NOTE(dima): Normalize elements
	float NormValueMul = 1.0f / TempSum;
    
	for (int i = 0; i < Diameter * Diameter; i++) {
		Box[i] *= NormValueMul;
	}
}

static void BoxBlurApproximate(image* To,
                               image* From,
                               int BlurRadius)
{
	int BlurDiam = 1 + BlurRadius + BlurRadius;
    
    Assert(To->Format == ImageFormat_RGBA);
    Assert(From->Format == ImageFormat_RGBA);
    
	for (int Y = 0; Y < From->Height; Y++) {
		for (int X = 0; X < From->Width; X++) {
            
			u32* TargetPixel = (u32*)To->Pixels + Y * To->Width + X;
            
			v4 VertSum = {};
			int VertSumCount = 0;
			for (int kY = Y - BlurRadius; kY <= Y + BlurRadius; kY++) {
				int targetY = Clamp(kY, 0, From->Height - 1);
                
				u32* ScanPixel = (u32*)From->Pixels + targetY * From->Width + X;
				v4 UnpackedColor = UnpackRGBA(*ScanPixel);
                
				VertSum += UnpackedColor;
                
				VertSumCount++;
			}
            
            
			v4 HorzSum = {};
			int HorzSumCount = 0;
			for (int kX = X - BlurRadius; kX <= X + BlurRadius; kX++) {
				int targetX = Clamp(kX, 0, From->Width - 1);
                
				u32* ScanPixel = (u32*)From->Pixels + Y * From->Width + targetX;
				v4 UnpackedColor = UnpackRGBA(*ScanPixel);
                
				HorzSum += UnpackedColor;
                
				HorzSumCount++;
			}
            
            
			VertSum = VertSum / (float)VertSumCount;
			HorzSum = HorzSum / (float)HorzSumCount;
            
			v4 TotalSum = (VertSum + HorzSum) * 0.5f;
            
			*TargetPixel = PackRGBA(TotalSum);
		}
	}
}

void BlurBitmapApproximateGaussian(image* Dst,
                                   image* Src,
                                   image* Tmp,
                                   int BlurRadius)
{
	Assert(Dst->Width == Src->Width);
	Assert(Dst->Height == Src->Height);
    
	Assert(Tmp->Width == Src->Width);
	Assert(Tmp->Height == Src->Height);
    
    Assert(Dst->Format == ImageFormat_RGBA);
    Assert(Src->Format == ImageFormat_RGBA);
    Assert(Tmp->Format == ImageFormat_RGBA);
    
	/*
 var wIdeal = Math.sqrt((12 * sigma*sigma / n) + 1);  // Ideal averaging filter Width
 var wl = Math.floor(wIdeal);  if (wl % 2 == 0) wl--;
 var wu = wl + 2;
 var mIdeal = (12 * sigma*sigma - n*wl*wl - 4 * n*wl - 3 * n) / (-4 * wl - 4);
 var m = Math.round(mIdeal);
 // var sigmaActual = Math.sqrt( (m*wl*wl + (n-m)*wu*wu - n)/12 );
 var sizes = [];  for (var i = 0; i<n; i++) sizes.push(i<m ? wl : wu);
 */
    
	float Boxes[3];
	int n = 3;
	float nf = 3.0f;
    
	float Sigma = (float)BlurRadius;
	float WIdeal = Sqrt((12.0f * Sigma * Sigma / nf) + 1.0f);
	float wlf = floorf(WIdeal);
	int wl = (float)(wlf + 0.5f);
	if (wl & 1 == 0) {
		wl--;
	}
	int wu = wl + 2;
    
	float mIdeal = (12.0f * Sigma * Sigma - nf * float(wl) * float(wl) - 4.0f * nf * float(wl) - 3.0f * nf) / (-4.0f * (float)wl - 4.0f);
	float mf = roundf(mIdeal);
	int m = float(mf + 0.5f);
    
	for (int i = 0; i < n; i++) {
		int ToSet = wu;
		if (i < m) {
			ToSet = wl;
		}
		Boxes[i] = ToSet;
	}
    
	BoxBlurApproximate(Dst, Src, (Boxes[0] - 1) / 2);
	BoxBlurApproximate(Tmp, Dst, (Boxes[1] - 1) / 2);
	BoxBlurApproximate(Dst, Tmp, (Boxes[2] - 1) / 2);
}

void BlurBitmapExactGaussian(image* Dst,
                             image* Src,
                             int BlurRadius,
                             float* GaussianBox)
{
	Assert(Dst->Width == Src->Width);
	Assert(Dst->Height == Src->Height);
    
    Assert(Dst->Format == ImageFormat_RGBA);
    Assert(Src->Format == ImageFormat_RGBA);
    
	int BlurDiam = 1 + BlurRadius + BlurRadius;
    
	for (int Y = 0; Y < Src->Height; Y++)
    {
		for (int X = 0; X < Src->Width; X++) 
        {
			u32* TargetPixel = (u32*)Dst->Pixels + Y * Dst->Width + X;
            
			v4 SumColor = {};
			for (int kY = Y - BlurRadius; kY <= Y + BlurRadius; kY++) 
            {
				int targetY = Clamp(kY, 0, Src->Height - 1);
				int inboxY = kY - (Y - BlurRadius);
				
                for (int kX = X - BlurRadius; kX <= X + BlurRadius; kX++)
                {
					int targetX = Clamp(kX, 0, Src->Width - 1);
					int inboxX = kX - (X - BlurRadius);
                    
					u32* ScanPixel = (u32*)Src->Pixels + targetY * Src->Width + targetX;
                    
					v4 UnpackedColor = UnpackRGBA(*ScanPixel);
                    
					SumColor += UnpackedColor * GaussianBox[inboxY * BlurDiam + inboxX];
				}
			}
            
			*TargetPixel = PackRGBA(SumColor);
		}
	}
}

void InvertImageColors(image* Image)
{
    Assert(Image->Format == ImageFormat_RGBA);
    
    for(int x = 0; x < Image->Width; x++)
    {
        for(int y = 0; y < Image->Height; y++)
        {
            u32* At = (u32*)Image->Pixels + y * Image->Width + x;
            
            v4 Color = UnpackRGBA(*At);
            
            v4 ResultColor = InvertColor(Color);
            
            *At = PackRGBA(ResultColor);
        }
    }
    
    InvalidateImage(Image);
}