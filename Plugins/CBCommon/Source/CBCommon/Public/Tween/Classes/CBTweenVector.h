// Copyright 2023 CB,  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tween/CBTweenBase.h"
#include "CBTweenVector.generated.h"

DECLARE_DELEGATE_RetVal(FVector, FCBTweenVectorGetterdDelegate);
DECLARE_DELEGATE_OneParam(FCBTweenVectorSetterdDelegate, FVector);
/**
 * 
 */
UCLASS(NotBlueprintType)
class CBCOMMON_API UCBTweenVector : public UCBTweenBase
{
	GENERATED_UCLASS_BODY()

public:
	float StartKey;
	float ChangeKey;
	FVector StartValue;
	FVector EndValue;
	
	FVector OriginStartKey;

	FCBTweenVectorGetterdDelegate Getter;
	FCBTweenVectorSetterdDelegate Setter;

public:
	auto SetInitialKey(const FCBTweenVectorGetterdDelegate IntGetter, const FCBTweenVectorSetterdDelegate InSetter,
	                   const FVector InEndKey, float InDuration) -> void;

protected:
	/* Begin CBTweenBase Interface. */
	virtual void OnStartGetValue() override;
	virtual void TweenAndApplyValue(float CurrentTime) override;
	virtual void SetValueForIncremental() override;
	virtual void SetOriginValueForReverse() override;
	/* End CBTweenBase Interface. */
	
};
