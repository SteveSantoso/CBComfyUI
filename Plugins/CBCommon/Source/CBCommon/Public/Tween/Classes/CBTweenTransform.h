// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Tween/CBTweenBase.h"
#include "CBTweenTransform.generated.h"

/**
 * 
 */
DECLARE_DELEGATE_RetVal(FTransform, FCBTweenTransformGetterdDelegate);
DECLARE_DELEGATE_OneParam(FCBTweenTransformSetterdDelegate, FTransform);
/**
 * 
 */
UCLASS(NotBlueprintType)
class CBCOMMON_API UCBTweenTransform : public UCBTweenBase
{
	GENERATED_UCLASS_BODY()
	public:
	float StartKey;
	float ChangeKey;
	FTransform StartValue;
	FTransform EndValue;
	
	FTransform OriginStartKey;

	FCBTweenTransformGetterdDelegate Getter;
	FCBTweenTransformSetterdDelegate Setter;

public:
	auto SetInitialKey(const FCBTweenTransformGetterdDelegate IntGetter, const FCBTweenTransformSetterdDelegate InSetter,
					   const FTransform InEndKey, float InDuration) -> void;

protected:
	/* Begin CBTweenBase Interface. */
	virtual void OnStartGetValue() override;
	virtual void TweenAndApplyValue(float CurrentTime) override;
	virtual void SetValueForIncremental() override;
	virtual void SetOriginValueForReverse() override;
	/* End CBTweenBase Interface. */
	
};

