// Copyright 2023 CB,  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CBEaseBPLibrary.generated.h"


UENUM(BlueprintType)
enum class EEasingCategory:uint8
{
	Ease_Linear		UMETA(DisplayName = "Linear"),
	Ease_Back		UMETA(DisplayName = "Back"),
	Ease_Bounce		UMETA(DisplayName = "Bounce"),
	Ease_Circle		UMETA(DisplayName = "Circle"),
	Ease_Cubic		UMETA(DisplayName = "Cubic"),
	Ease_Elastic	UMETA(DisplayName = "Elastic"),
	Ease_Expo		UMETA(DisplayName = "Expo"),
	Ease_Sine		UMETA(DisplayName = "Sine"),
	Ease_Quad		UMETA(DisplayName = "Quad"),
	Ease_Quart		UMETA(DisplayName = "Quart"),
	Ease_Quint		UMETA(DisplayName = "Quint"),
};

UENUM(BlueprintType)
enum class EEasingType:uint8
{
	EaseIn		UMETA(DisplayName = "In"),
	EaseOut		UMETA(DisplayName = "Out"),
	EaseInOut	UMETA(DisplayName = "InOut"),
};

/**
 * 
 */
UCLASS()
class CBCOMMON_API UCBEaseBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Float)",BlueprintInternalUseOnly = "true"), Category = "CB|Interpolation")
	static float EasingFloat(EEasingCategory EasingCategory,EEasingType EasingType, float A, float B, float Alpha);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Vector2D)",BlueprintInternalUseOnly = "true"), Category = "CB|Interpolation")
	static FVector2D EasingVector2D(EEasingCategory EasingCategory,EEasingType EasingType, FVector2D A, FVector2D B, float Alpha);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Vector)",BlueprintInternalUseOnly = "true"), Category = "CB|Interpolation")
	static FVector EasingVector(EEasingCategory EasingCategory,EEasingType EasingType, FVector A, FVector B, float Alpha);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Vector4)",BlueprintInternalUseOnly = "true"), Category = "CB|Interpolation")
	static FVector4 EasingVector4(EEasingCategory EasingCategory,EEasingType EasingType, FVector4 A, FVector4 B, float Alpha);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Rotator)",BlueprintInternalUseOnly = "true"), Category = "CB|Interpolation")
	static FRotator EasingRotator(EEasingCategory EasingCategory,EEasingType EasingType, FRotator A, FRotator B, float Alpha);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Quat)",BlueprintInternalUseOnly = "true"), Category = "CB|Interpolation")
	static FQuat EasingQuat(EEasingCategory EasingCategory,EEasingType EasingType, FQuat A, FQuat B, float Alpha);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Transform)",BlueprintInternalUseOnly = "true"), Category = "CB|Interpolation")
	static FTransform EasingtTransform(EEasingCategory EasingCategory,EEasingType EasingType, FTransform A, FTransform B, float Alpha);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Color)",BlueprintInternalUseOnly = "true"), Category = "CB|Interpolation")
	static FColor EasingColor(EEasingCategory EasingCategory,EEasingType EasingType, FColor A, FColor B, float Alpha);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (LinearColor)",BlueprintInternalUseOnly = "true"), Category = "CB|Interpolation")
	static FLinearColor EasingLinearColor(EEasingCategory EasingCategory,EEasingType EasingType, FLinearColor A, FLinearColor B, float Alpha);

	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (float) | Manual",BlueprintInternalUseOnly = "true"), Category = "CB|Interpolation")
	static float EasingfloatManual(EEasingCategory EasingCategory,EEasingType EasingType, float A, float B, float CurrentTime, float TotalDuration);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Vector2D) | Manual"), Category = "CB|Interpolation")
	static FVector2D EasingVector2DManual(EEasingCategory EasingCategory,EEasingType EasingType, FVector2D A, FVector2D B,float CurrentTime, float TotalDuration);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Vector) | Manual"), Category = "CB|Interpolation")
	static FVector EasingVectorManual(EEasingCategory EasingCategory,EEasingType EasingType, FVector A, FVector B,float CurrentTime, float TotalDuration);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Vector4) | Manual"), Category = "CB|Interpolation")
	static FVector4 EasingVector4Manual(EEasingCategory EasingCategory,EEasingType EasingType, FVector4 A, FVector4 B,float CurrentTime, float TotalDuration);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Rotator) | Manual"), Category = "CB|Interpolation")
	static FRotator EasingRotatorManual(EEasingCategory EasingCategory,EEasingType EasingType, FRotator A, FRotator B,float CurrentTime, float TotalDuration);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (FQuat) | Manual"), Category = "CB|Interpolation")
	static FQuat EasingQuatManual(EEasingCategory EasingCategory, EEasingType EasingType, FQuat A, FQuat B, float CurrentTime, float TotalDuration);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Transform) | Manual"), Category = "CB|Interpolation")
	static FTransform EasingTransformManual(EEasingCategory EasingCategory, EEasingType EasingType, FTransform A, FTransform B, float CurrentTime, float TotalDuration);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (Color) | Manual"), Category = "CB|Interpolation")
	static FColor EasingColorManual(EEasingCategory EasingCategory, EEasingType EasingType, FColor A, FColor B, float CurrentTime, float TotalDuration);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ease (LinearColor) | Manual"), Category = "CB|Interpolation")
	static FLinearColor EasingLinearColorManual(EEasingCategory EasingCategory, EEasingType EasingType, FLinearColor A, FLinearColor B, float CurrentTime, float TotalDuration);
public:
	// Back Ease
	static float ExecuteBackEase(EEasingType Type, float CurrentTime, float Start, float Diff, float TotalDuration);
	static float BackIn(float t, float b, float c, float d);
	static float BackOut(float t, float b, float c, float d);
	static float BackInOut(float t, float b, float c, float d);

	// Bounce Ease
	static float ExecuteBounceEase(EEasingType Type, float CurrentTime, float Start, float Diff, float TotalDuration);
	static float BounceIn(float t, float b, float c, float d);
	static float BounceOut(float t, float b, float c, float d);
	static float BounceInOut(float t, float b, float c, float d);

	// Circle Ease
	static float ExecuteCircleEase(EEasingType Type, float CurrentTime, float Start, float Diff, float TotalDuration);
	static float CircleIn(float t, float b, float c, float d);
	static float CircleOut(float t, float b, float c, float d);
	static float CircleInOut(float t, float b, float c, float d);

	// Cubic Ease
	static float ExecuteCubicEase(EEasingType Type, float CurrentTime, float Start, float Diff, float TotalDuration);
	static float CubicIn(float t, float b, float c, float d);
	static float CubicOut(float t, float b, float c, float d);
	static float CubicInOut(float t, float b, float c, float d);

	// Elastic Ease
	static float ExecuteElasticEase(EEasingType Type, float CurrentTime, float Start, float Diff, float TotalDuration);
	static float ElasticIn(float t, float b, float c, float d);
	static float ElasticOut(float t, float b, float c, float d);
	static float ElasticInOut(float t, float b, float c, float d);

	// Expo Ease
	static float ExecuteExpoEase(EEasingType Type, float CurrentTime, float Start, float Diff, float TotalDuration);
	static float ExpoIn(float t, float b, float c, float d);
	static float ExpoOut(float t, float b, float c, float d);
	static float ExpoInOut(float t, float b, float c, float d);

	// Linear Ease
	static float ExecuteLinearEase(EEasingType Type, float CurrentTime, float Start, float Diff, float TotalDuration);
	static float LinearIn(float t, float b, float c, float d);
	static float LinearOut(float t, float b, float c, float d);
	static float LinearInOut(float t, float b, float c, float d);

	// Quad Ease
	static float ExecuteQuadEase(EEasingType Type, float CurrentTime, float Start, float Diff, float TotalDuration);
	static float QuadIn(float t, float b, float c, float d);
	static float QuadOut(float t, float b, float c, float d);
	static float QuadInOut(float t, float b, float c, float d);

	// Quart Ease
	static float ExecuteQuartEase(EEasingType Type, float CurrentTime, float Start, float Diff, float TotalDuration);
	static float QuartIn(float t, float b, float c, float d);
	static float QuartOut(float t, float b, float c, float d);
	static float QuartInOut(float t, float b, float c, float d);

	// Quint Ease
	static float ExecuteQuintEase(EEasingType Type, float CurrentTime, float Start, float Diff, float totalDuration);
	static float QuintIn(float t, float b, float c, float d);
	static float QuintOut(float t, float b, float c, float d);
	static float QuintInOut(float t, float b, float c, float d);

	// Sine Ease
	static float ExecuteSineEase(EEasingType Type, float CurrentTime, float Start, float Diff, float TotalDuration);
	static float SineIn(float t, float b, float c, float d);
	static float SineOut(float t, float b, float c, float d);
	static float SineInOut(float t, float b, float c, float d);

	static float Linear(float t, float b, float c, float d);
};
