// Copyright 2023 CB,  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tween/CBTweenBase.h"
#include "CBTweenVector2D.generated.h"

DECLARE_DELEGATE_RetVal(FVector2D, FCBTweenVector2DGetterdDelegate);
DECLARE_DELEGATE_OneParam(FCBTweenVector2DSetterdDelegate, FVector2D);
/**
 * 
 */
UCLASS()
class CBCOMMON_API UCBTweenVector2D : public UCBTweenBase
{
	GENERATED_UCLASS_BODY()
public:
	float StartKey;
	float ChangeKey;
	FVector2D StartValue;
	FVector2D EndValue;
	
	FVector2D OriginStartKey;

	FCBTweenVector2DGetterdDelegate Getter;
	FCBTweenVector2DSetterdDelegate Setter;

public:
	auto SetInitialKey(const FCBTweenVector2DGetterdDelegate IntGetter, const FCBTweenVector2DSetterdDelegate InSetter,
					   const FVector2D InEndKey, float InDuration) -> void;

protected:
	/* Begin CBTweenBase Interface. */
	virtual void OnStartGetValue() override;
	virtual void TweenAndApplyValue(float CurrentTime) override;
	virtual void SetValueForIncremental() override;
	virtual void SetOriginValueForReverse() override;
	/* End CBTweenBase Interface. */
	
};
