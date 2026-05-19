// Copyright 2023 CB,  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CBEaseBPLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CBTweenBPLibrary.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FCBTweenFloatSetterDynamic, float, Value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCBTweenIntSetterDynamic, int32, Value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCBTweenVectorSetterDynamic,const FVector&, Value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCBTweenVector2DSetterDynamic,const FVector2D&, Value);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCBTweenTransformSetterDynamic,const FTransform&, Value);
/**
 * 
 */
UCLASS()
class CBCOMMON_API UCBTweenBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**基于float的Tween*/
	UFUNCTION(BlueprintCallable, Category = "CB|Tween", meta = (AdvancedDisplay = "Delay,EasingCategory,EasingType", WorldContext = "WorldContextObject"))
	static UCBTweenBase* FloatTo(UObject* WorldContextObject, FCBTweenFloatSetterDynamic Setter, float Start = 0.0f, float End = 1.0f, float Duration = 0.5f, float Delay = 0.0f, EEasingCategory EasingCategory=EEasingCategory::Ease_Cubic,EEasingType EasingType=EEasingType::EaseOut);

	/**基于Int的Tween，注意：返回值不是连续的int值*/
	UFUNCTION(BlueprintCallable, Category = "CB|Tween", meta = (AdvancedDisplay = "Delay,EasingCategory,EasingType", WorldContext = "WorldContextObject"))
	static UCBTweenBase* IntTo(UObject* WorldContextObject, FCBTweenIntSetterDynamic Setter, int32 Start, int32 End, float Duration = 0.5f, float Delay = 0.0f, EEasingCategory EasingCategory=EEasingCategory::Ease_Cubic,EEasingType EasingType=EEasingType::EaseOut);

	/**基于Vetcor的Tween*/
	UFUNCTION(BlueprintCallable, Category = "CB|Tween", meta = (AdvancedDisplay = "Delay,EasingCategory,EasingType", WorldContext = "WorldContextObject"))
	static UCBTweenBase* VetcorTo(UObject* WorldContextObject, FCBTweenVectorSetterDynamic Setter, FVector Start ,FVector End, float Duration = 0.5f, float Delay = 0.0f, EEasingCategory EasingCategory=EEasingCategory::Ease_Cubic,EEasingType EasingType=EEasingType::EaseOut);

	/**基于Vetcor2D的Tween*/
	UFUNCTION(BlueprintCallable, Category = "CB|Tween", meta = (AdvancedDisplay = "Delay,EasingCategory,EasingType", WorldContext = "WorldContextObject"))
	static UCBTweenBase* Vetcor2DTo(UObject* WorldContextObject, FCBTweenVector2DSetterDynamic Setter, FVector2D Start ,FVector2D End, float Duration = 0.5f, float Delay = 0.0f, EEasingCategory EasingCategory=EEasingCategory::Ease_Cubic,EEasingType EasingType=EEasingType::EaseOut);

	/**基于Rotator的Tween*/
	UFUNCTION(BlueprintCallable, Category = "CB|Tween", meta = (AdvancedDisplay = "Delay,EasingCategory,EasingType", WorldContext = "WorldContextObject"))
	static UCBTweenBase* TransformTo(UObject* WorldContextObject, FCBTweenTransformSetterDynamic Setter, FTransform Start ,FTransform End, float Duration = 0.5f, float Delay = 0.0f, EEasingCategory EasingCategory=EEasingCategory::Ease_Cubic,EEasingType EasingType=EEasingType::EaseOut);

	
	/**基于MaterialScalarParameter的Tween*/
	UFUNCTION(BlueprintCallable, Category = "CB|Tween", meta = (AdvancedDisplay = "Delay,EasingCategory,EasingType", WorldContext = "WorldContextObject"))
	static UCBTweenBase* MaterialScalarParameterTo(UObject* WorldContextObject,class UMaterialInstanceDynamic* Target, FName ParameterName,  float End = 1.0f, float Duration = 0.5f, float Delay = 0.0f, EEasingCategory EasingCategory=EEasingCategory::Ease_Cubic,EEasingType EasingType=EEasingType::EaseOut);

	/**基于MeshMaterialScalarParameter的Tween*/
	UFUNCTION(BlueprintCallable, Category = "CB|Tween", meta = (AdvancedDisplay = "Delay,EasingCategory,EasingType", WorldContext = "WorldContextObject"))
	static UCBTweenBase* MeshMaterialScalarParameterTo(class UPrimitiveComponent* Target, int32 MaterialIndex, FName ParameterName, float End, float Duration = 0.5f, float Delay = 0.0f,EEasingCategory EasingCategory=EEasingCategory::Ease_Cubic,EEasingType EasingType=EEasingType::EaseOut);
	
};
