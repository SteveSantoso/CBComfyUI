// Copyright 2023 CB,  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tween/CBTweenBase.h"
#include "CBTweenFloat.generated.h"


DECLARE_DELEGATE_RetVal(float, FCBTweenFloatGetterdDelegate);
DECLARE_DELEGATE_OneParam(FCBTweenFloatSetterdDelegate, float);

/**
 * 
 */
UCLASS(NotBlueprintType)
class CBCOMMON_API UCBTweenFloat : public UCBTweenBase
{
	GENERATED_UCLASS_BODY()

public:
	float StartKey;
	float ChangeKey;
	float EndKey;

	float OriginStartKey;

	FCBTweenFloatGetterdDelegate Getter;
	FCBTweenFloatSetterdDelegate Setter;

public:
	void SetInitialKey(const FCBTweenFloatGetterdDelegate IntGetter,const FCBTweenFloatSetterdDelegate InSetter,float InEndKey,float InDuration);

protected:
	/* Begin CBTweenBase Interface. */
	virtual void OnStartGetValue() override;
	virtual void TweenAndApplyValue(float CurrentTime) override;
	virtual void SetValueForIncremental() override;
	virtual void SetOriginValueForReverse() override;
	/* End CBTweenBase Interface. */
};


