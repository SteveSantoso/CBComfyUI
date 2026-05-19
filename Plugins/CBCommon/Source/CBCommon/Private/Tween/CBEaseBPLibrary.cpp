// Copyright 2023 CB,  All Rights Reserved.


#include "Tween/CBEaseBPLibrary.h"

float UCBEaseBPLibrary::EasingFloat(EEasingCategory EasingCategory, EEasingType EasingType, float A, float B,
	float Alpha)
{
	return EasingfloatManual(EasingCategory, EasingType, A, B, Alpha, 1.f);
}

FVector2D UCBEaseBPLibrary::EasingVector2D(EEasingCategory EasingCategory, EEasingType EasingType, FVector2D A,
	FVector2D B, float Alpha)
{
	return EasingVector2DManual(EasingCategory, EasingType, A, B, Alpha, 1.f);
}

FVector UCBEaseBPLibrary::EasingVector(EEasingCategory EasingCategory, EEasingType EasingType, FVector A, FVector B,
                                       float Alpha)
{
	return EasingVectorManual(EasingCategory, EasingType, A, B, Alpha, 1.f);
}

FVector4 UCBEaseBPLibrary::EasingVector4(EEasingCategory EasingCategory, EEasingType EasingType, FVector4 A, FVector4 B,
	float Alpha)
{
	return EasingVector4Manual(EasingCategory, EasingType, A, B, Alpha, 1.f);
}

FRotator UCBEaseBPLibrary::EasingRotator(EEasingCategory EasingCategory, EEasingType EasingType, FRotator A, FRotator B,
                                         float Alpha)
{
	return EasingRotatorManual(EasingCategory, EasingType, A, B, Alpha, 1.f);
}

FQuat UCBEaseBPLibrary::EasingQuat(EEasingCategory EasingCategory, EEasingType EasingType, FQuat A, FQuat B,
	float Alpha)
{
	return EasingQuatManual(EasingCategory, EasingType, A, B, Alpha, 1.f);
}

FTransform UCBEaseBPLibrary::EasingtTransform(EEasingCategory EasingCategory, EEasingType EasingType, FTransform A,
	FTransform B, float Alpha)
{
	return EasingTransformManual(EasingCategory, EasingType, A, B, Alpha, 1.f);
}

FColor UCBEaseBPLibrary::EasingColor(EEasingCategory EasingCategory, EEasingType EasingType, FColor A, FColor B,
	float Alpha)
{
	return  EasingColorManual(EasingCategory, EasingType, A, B, Alpha, 1.f);
}

FLinearColor UCBEaseBPLibrary::EasingLinearColor(EEasingCategory EasingCategory, EEasingType EasingType, FLinearColor A,
	FLinearColor B, float Alpha)
{
	return EasingLinearColorManual(EasingCategory, EasingType, A, B, Alpha, 1.f);
}

float UCBEaseBPLibrary::EasingfloatManual(EEasingCategory EasingCategory, EEasingType EasingType, float A, float B,
                                          float CurrentTime, float TotalDuration)
{
	const float Diff = B - A;

	switch (EasingCategory)
	{
	case EEasingCategory::Ease_Linear:
		return Linear(CurrentTime, A, Diff, TotalDuration);
	case EEasingCategory::Ease_Sine:
		return ExecuteSineEase(EasingType, CurrentTime, A, Diff, TotalDuration);
	case EEasingCategory::Ease_Cubic:
		return ExecuteCubicEase(EasingType, CurrentTime, A, Diff, TotalDuration);
	case EEasingCategory::Ease_Quint:
		return ExecuteQuintEase(EasingType, CurrentTime, A, Diff, TotalDuration);
	case EEasingCategory::Ease_Circle:
		return ExecuteCircleEase(EasingType, CurrentTime, A, Diff, TotalDuration);
	case EEasingCategory::Ease_Elastic:
		return ExecuteElasticEase(EasingType, CurrentTime, A, Diff, TotalDuration);
	case EEasingCategory::Ease_Quad:
		return ExecuteQuadEase(EasingType, CurrentTime, A, Diff, TotalDuration);
	case EEasingCategory::Ease_Quart:
		return ExecuteQuartEase(EasingType, CurrentTime, A, Diff, TotalDuration);
	case EEasingCategory::Ease_Expo:
		return ExecuteExpoEase(EasingType, CurrentTime, A, Diff, TotalDuration);
	case EEasingCategory::Ease_Back:
		return ExecuteBackEase(EasingType, CurrentTime, A, Diff, TotalDuration);
	case EEasingCategory::Ease_Bounce:
		return ExecuteBounceEase(EasingType, CurrentTime, A, Diff, TotalDuration);
	default:
		break;
	}

	return 0;
}

FVector2D UCBEaseBPLibrary::EasingVector2DManual(EEasingCategory EasingCategory, EEasingType EasingType, FVector2D A,
	FVector2D B, float CurrentTime, float TotalDuration)
{
	FVector2D Result;
	Result.X = EasingfloatManual(EasingCategory, EasingType, A.X, B.X, CurrentTime, TotalDuration);
	Result.Y = EasingfloatManual(EasingCategory, EasingType, A.Y, B.Y, CurrentTime, TotalDuration);

	return Result;
}

FVector UCBEaseBPLibrary::EasingVectorManual(EEasingCategory EasingCategory, EEasingType EasingType, FVector A, FVector B,
                                             float CurrentTime, float TotalDuration)
{
	FVector Result;
	Result.X = EasingfloatManual(EasingCategory, EasingType, A.X, B.X, CurrentTime, TotalDuration);
	Result.Y = EasingfloatManual(EasingCategory, EasingType, A.Y, B.Y, CurrentTime, TotalDuration);
	Result.Z = EasingfloatManual(EasingCategory, EasingType, A.Z, B.Z, CurrentTime, TotalDuration);

	return Result;
}

FVector4 UCBEaseBPLibrary::EasingVector4Manual(EEasingCategory EasingCategory, EEasingType EasingType, FVector4 A,
	FVector4 B, float CurrentTime, float TotalDuration)
{
	FVector4 Result;
	Result.X = EasingfloatManual(EasingCategory, EasingType, A.X, B.X, CurrentTime, TotalDuration);
	Result.Y = EasingfloatManual(EasingCategory, EasingType, A.Y, B.Y, CurrentTime, TotalDuration);
	Result.Z = EasingfloatManual(EasingCategory, EasingType, A.Z, B.Z, CurrentTime, TotalDuration);
	Result.W = EasingfloatManual(EasingCategory, EasingType, A.W, B.W, CurrentTime, TotalDuration);
	return Result;
}

FRotator UCBEaseBPLibrary::EasingRotatorManual(EEasingCategory EasingCategory, EEasingType EasingType, FRotator A,
                                               FRotator B,float CurrentTime, float TotalDuration)
{
	FRotator Result;
	Result.Roll = EasingfloatManual(EasingCategory, EasingType, A.Roll, B.Roll, CurrentTime, TotalDuration);
	Result.Pitch = EasingfloatManual(EasingCategory, EasingType, A.Pitch, B.Pitch, CurrentTime, TotalDuration);
	Result.Yaw = EasingfloatManual(EasingCategory, EasingType, A.Yaw, B.Yaw, CurrentTime, TotalDuration);

	return Result;
}

FQuat UCBEaseBPLibrary::EasingQuatManual(EEasingCategory EasingCategory, EEasingType EasingType, FQuat A, FQuat B,
	float CurrentTime, float TotalDuration)
{
	FQuat Result;
	Result.X = EasingfloatManual(EasingCategory, EasingType, A.X, B.X, CurrentTime, TotalDuration);
	Result.Y = EasingfloatManual(EasingCategory, EasingType, A.Y, B.Y, CurrentTime, TotalDuration);
	Result.Z = EasingfloatManual(EasingCategory, EasingType, A.Z, B.Z, CurrentTime, TotalDuration);
	Result.W = EasingfloatManual(EasingCategory, EasingType, A.W, B.W, CurrentTime, TotalDuration);

	return Result;
}

FTransform UCBEaseBPLibrary::EasingTransformManual(EEasingCategory EasingCategory, EEasingType EasingType, FTransform A,
	FTransform B, float CurrentTime, float TotalDuration)
{
	FTransform Result;
	Result.SetLocation(EasingVectorManual(EasingCategory, EasingType,A.GetLocation(), B.GetLocation(), CurrentTime, TotalDuration));
	Result.SetRotation(EasingQuatManual(EasingCategory, EasingType, A.GetRotation(), B.GetRotation(), CurrentTime, TotalDuration));
	Result.SetScale3D(EasingVectorManual(EasingCategory, EasingType, A.GetScale3D(), B.GetScale3D(), CurrentTime, TotalDuration));

	return Result;
}

FColor UCBEaseBPLibrary::EasingColorManual(EEasingCategory EasingCategory, EEasingType EasingType, FColor A, FColor B,
	float CurrentTime, float TotalDuration)
{
	FColor Result;
	Result.R = (uint8)EasingfloatManual(EasingCategory, EasingType, A.R, B.R, CurrentTime, TotalDuration);
	Result.G = (uint8)EasingfloatManual(EasingCategory, EasingType, A.G, B.G, CurrentTime, TotalDuration);
	Result.B = (uint8)EasingfloatManual(EasingCategory, EasingType, A.B, B.B, CurrentTime, TotalDuration);
	Result.A = (uint8)EasingfloatManual(EasingCategory, EasingType, A.A, B.A, CurrentTime, TotalDuration);

	return Result;
}

FLinearColor UCBEaseBPLibrary::EasingLinearColorManual(EEasingCategory EasingCategory, EEasingType EasingType, FLinearColor A,
	FLinearColor B, float CurrentTime, float TotalDuration)
{
	FLinearColor Result;
	Result.R = EasingfloatManual(EasingCategory, EasingType, A.R, B.R, CurrentTime, TotalDuration);
	Result.G = EasingfloatManual(EasingCategory, EasingType, A.G, B.G, CurrentTime, TotalDuration);
	Result.B = EasingfloatManual(EasingCategory, EasingType, A.B, B.B, CurrentTime, TotalDuration);
	Result.A = EasingfloatManual(EasingCategory, EasingType, A.A, B.A, CurrentTime, TotalDuration);

	return Result;
}

float UCBEaseBPLibrary::ExecuteBackEase(EEasingType Type, float CurrentTime, float A, float Diff, float TotalDuration)
{
	switch (Type)
	{
	case EEasingType::EaseIn:
		return BackIn(CurrentTime, A, Diff, TotalDuration);
	case EEasingType::EaseOut:
		return BackOut(CurrentTime, A, Diff, TotalDuration);
	case EEasingType::EaseInOut:
		return BackInOut(CurrentTime, A, Diff, TotalDuration);
	default:
		break;
	}

	return 0;
}

float UCBEaseBPLibrary::BackIn(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	float s = 1.70158f;
	t /= d;
	float postFix = t;
	return c * (postFix)*t*((s + 1)*t - s) + b;
}

float UCBEaseBPLibrary::BackOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	float s = 1.70158f;
	t = t / d - 1;
	return c * (t*t*((s + 1)*t + s) + 1) + b;
}

float UCBEaseBPLibrary::BackInOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	float s = 1.70158f;
	float postFixS = s * (1.525f);
	if ((t /= d / 2) < 1) return c / 2 * (t*t*(((postFixS) + 1)*t - s)) + b;
	float postFix = t -= 2;
	return c / 2 * ((postFix)*t*(((postFixS) + 1)*t + s) + 2) + b;;
}

float UCBEaseBPLibrary::ExecuteBounceEase(EEasingType Type, float CurrentTime, float A, float Diff, float TotalDuration)
{
	switch (Type)
	{
	case EEasingType::EaseIn:
		return BounceIn(CurrentTime, A, Diff, TotalDuration);
	case EEasingType::EaseOut:
		return BounceOut(CurrentTime, A, Diff, TotalDuration);
	case EEasingType::EaseInOut:
		return BounceInOut(CurrentTime, A, Diff, TotalDuration);
	default:
		break;
	}

	return 0;
}

float UCBEaseBPLibrary::BounceIn(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	return c - BounceOut(d - t, 0, c, d) + b;
}

float UCBEaseBPLibrary::BounceOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d;
	if ((t /= d) < (1 / 2.75f))
	{
		return c * (7.5625f*t*t) + b;
	}
	else if (t < (2 / 2.75f))
	{
		t -= (1.5f / 2.75f);
		float postFix = t;
		return c * (7.5625f*(postFix)*t + .75f) + b;
	}
	else if (t < (2.5 / 2.75))
	{
		t -= (2.25f / 2.75f);
		float postFix = t;
		return c * (7.5625f*(postFix)*t + .9375f) + b;
	}
	else
	{
		t -= (2.625f / 2.75f);
		float postFix = t;
		return c * (7.5625f*(postFix)*t + .984375f) + b;
	}
}

float UCBEaseBPLibrary::BounceInOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	if (t < d / 2) return BounceIn(t * 2, 0, c, d) * .5f + b;
	else return BounceOut(t * 2 - d, 0, c, d) * .5f + c * .5f + b;
}

float UCBEaseBPLibrary::ExecuteCircleEase(EEasingType Type, float CurrentTime, float A, float Diff, float TotalDuration)
{
	switch (Type)
	{
	case EEasingType::EaseIn:
		return CircleIn(CurrentTime, A, Diff, TotalDuration);
		break;
		
	case EEasingType::EaseOut:
		return CircleOut(CurrentTime, A, Diff, TotalDuration);
		break;
		
	case EEasingType::EaseInOut:
		return CircleInOut(CurrentTime, A, Diff, TotalDuration);
		break;
		
	default:
		break;
	}

	return 0;
}

float UCBEaseBPLibrary::CircleIn(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d;
	return -c * (sqrt(1 - (t)*t) - 1) + b;
}

float UCBEaseBPLibrary::CircleOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t = t / d - 1;
	return c * sqrt(1 - (t)*t) + b;
}

float UCBEaseBPLibrary::CircleInOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d / 2;
	if ((t) < 1) return -c / 2 * (sqrt(1 - t * t) - 1) + b;
	t -= 2;
	return c / 2 * (sqrt(1 - t * (t)) + 1) + b;
}

float UCBEaseBPLibrary::ExecuteCubicEase(EEasingType Type, float CurrentTime, float A, float Diff, float TotalDuration)
{
	switch (Type)
	{
	case EEasingType::EaseIn:
		return CubicIn(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseOut:
		return CubicOut(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseInOut:
		return CubicInOut(CurrentTime, A, Diff, TotalDuration);
		break;
	default:
		break;
	}

	return 0;
}

float UCBEaseBPLibrary::CubicIn(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d;
	return c * (t)*t*t + b;
}

float UCBEaseBPLibrary::CubicOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t = t / d - 1.f;
	return c * ((t)*t*t + 1.f) + b;
}

float UCBEaseBPLibrary::CubicInOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d / 2;
	if ((t) < 1) return c / 2 * t*t*t + b;
	t -= 2;
	return c / 2 * ((t)*t*t + 2) + b;
}

float UCBEaseBPLibrary::ExecuteElasticEase(EEasingType Type, float CurrentTime, float A, float Diff, float TotalDuration)
{
	switch (Type)
	{
	case EEasingType::EaseIn:
		return ElasticIn(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseOut:
		return ElasticOut(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseInOut:
		return ElasticInOut(CurrentTime, A, Diff, TotalDuration);
		break;
	default:
		break;
	}

	return 0;
}

float UCBEaseBPLibrary::ElasticIn(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	if (t == 0) return b;  
	t /= d;
	if ((t) == 1) return b + c;
	float p = d * .3f;
	float a = c;
	float s = p / 4;
	t -= 1;
	float postFix = a * pow(2, 10 * (t));
	return -(postFix * sin((t*d - s)*(2 * PI) / p)) + b;
}

float UCBEaseBPLibrary::ElasticOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d;
	if (t == 0) return b;  if ((t) == 1) return b + c;
	float p = d * .3f;
	float a = c;
	float s = p / 4;
	return (a*pow(2, -10 * t) * sin((t*d - s)*(2 * PI) / p) + c + b);
}

float UCBEaseBPLibrary::ElasticInOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	if (t == 0) return b;  
	t /= d / 2;
	if ((t) == 2) return b + c;
	float p = d * (.3f*1.5f);
	float a = c;
	float s = p / 4;

	if (t < 1) {
		t -= 1;
		float postFix = a * pow(2, 10 * (t));
		return -.5f*(postFix* sin((t*d - s)*(2 * PI) / p)) + b;
	}
	t -= 1;
	float postFix = a * pow(2, -10 * (t));
	return postFix * sin((t*d - s)*(2 * PI) / p)*.5f + c + b;
}

float UCBEaseBPLibrary::ExecuteExpoEase(EEasingType Type, float CurrentTime, float A, float Diff, float TotalDuration)
{
	switch (Type)
	{
	case EEasingType::EaseIn:
		return ExpoIn(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseOut:
		return ExpoOut(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseInOut:
		return ExpoInOut(CurrentTime, A, Diff, TotalDuration);
		break;
	default:
		break;
	}

	return 0;
}

float UCBEaseBPLibrary::ExpoIn(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	return (t == 0) ? b : c * pow(2, 10 * (t / d - 1)) + b;
}

float UCBEaseBPLibrary::ExpoOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	return (t == d) ? b + c : c * (-pow(2, -10 * t / d) + 1) + b;
}

float UCBEaseBPLibrary::ExpoInOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	if (t == 0) return b;
	if (t == d) return b + c;
	t /= d / 2;
	if ((t) < 1) return c / 2 * pow(2, 10 * (t - 1)) + b;
	return c / 2 * (-pow(2, -10 * --t) + 2) + b;
}

float UCBEaseBPLibrary::ExecuteLinearEase(EEasingType Type, float CurrentTime, float A, float Diff, float TotalDuration)
{
	switch (Type)
	{
	case EEasingType::EaseIn:
		return LinearIn(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseOut:
		return LinearOut(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseInOut:
		return LinearInOut(CurrentTime, A, Diff, TotalDuration);
		break;
	default:
		break;
	}

	return 0;
}

float UCBEaseBPLibrary::LinearIn(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	return c * t / d + b;
}

float UCBEaseBPLibrary::LinearOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	return c * t / d + b;
}

float UCBEaseBPLibrary::LinearInOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	return c * t / d + b;

}

float UCBEaseBPLibrary::ExecuteQuadEase(EEasingType Type, float CurrentTime, float A, float Diff, float TotalDuration)
{
	switch (Type)
	{
	case EEasingType::EaseIn:
		return QuadIn(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseOut:
		return QuadOut(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseInOut:
		return QuadInOut(CurrentTime, A, Diff, TotalDuration);
		break;
	default:
		break;
	}

	return 0;
}

float UCBEaseBPLibrary::QuadIn(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d;
	return c * (t)*t + b;
}

float UCBEaseBPLibrary::QuadOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d;
	return -c * (t)*(t - 2) + b;
}

float UCBEaseBPLibrary::QuadInOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d / 2;
	if ((t) < 1) return ((c / 2)*(t*t)) + b;
	--t;
	return -c / 2 * (((t - 2)*(t)) - 1) + b;
}

float UCBEaseBPLibrary::ExecuteQuartEase(EEasingType Type, float CurrentTime, float A, float Diff, float TotalDuration)
{
	switch (Type)
	{
	case EEasingType::EaseIn:
		return QuartIn(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseOut:
		return QuartOut(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseInOut:
		return QuartInOut(CurrentTime, A, Diff, TotalDuration);
		break;
	default:
		break;
	}

	return 0;
}

float UCBEaseBPLibrary::QuartIn(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d;
	return c * (t)*t*t*t + b;
}

float UCBEaseBPLibrary::QuartOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t = t / d - 1;
	return -c * ((t)*t*t*t - 1) + b;
}

float UCBEaseBPLibrary::QuartInOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d / 2;
	if ((t) < 1) return c / 2 * t*t*t*t + b;
	t -= 2;
	return -c / 2 * ((t)*t*t*t - 2) + b;
}

float UCBEaseBPLibrary::ExecuteQuintEase(EEasingType Type, float CurrentTime, float A, float Diff, float TotalDuration)
{
	switch (Type)
	{
	case EEasingType::EaseIn:
		return QuintIn(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseOut:
		return QuintOut(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseInOut:
		return QuintInOut(CurrentTime, A, Diff, TotalDuration);
		break;
	default:
		break;
	}

	return 0;
}

float UCBEaseBPLibrary::QuintIn(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d;
	return c * (t)*t*t*t*t + b;
}

float UCBEaseBPLibrary::QuintOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t = t / d - 1;
	return c * ((t)*t*t*t*t + 1) + b;
}

float UCBEaseBPLibrary::QuintInOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	t /= d / 2;
	if ((t) < 1) return c / 2 * t*t*t*t*t + b;
	t -= 2;
	return c / 2 * ((t)*t*t*t*t + 2) + b;
}

float UCBEaseBPLibrary::ExecuteSineEase(EEasingType Type, float CurrentTime, float A, float Diff, float TotalDuration)
{
	switch (Type)
	{
	case EEasingType::EaseIn:
		return SineIn(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseOut:
		return SineOut(CurrentTime, A, Diff, TotalDuration);
		break;
	case EEasingType::EaseInOut:
		return SineInOut(CurrentTime, A, Diff, TotalDuration);
		break;
	default:
		break;
	}

	return 0;
}

float UCBEaseBPLibrary::SineIn(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	return -c * FMath::Cos(t / d * HALF_PI) + c + b;
}

float UCBEaseBPLibrary::SineOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	return c * sin(t / d * (PI / 2)) + b;
}

float UCBEaseBPLibrary::SineInOut(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	return -c / 2 * (cos(PI*t / d) - 1) + b;
}

float UCBEaseBPLibrary::Linear(float t, float b, float c, float d)
{
	if (d < KINDA_SMALL_NUMBER)
		return c + b;
	
	return c * t / d + b;
}
