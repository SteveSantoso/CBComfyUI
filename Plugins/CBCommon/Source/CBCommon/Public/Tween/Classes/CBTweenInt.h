// Copyright 2023 CB,  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tween/CBTweenBase.h"
#include "CBTweenInt.generated.h"
DECLARE_DELEGATE_RetVal(float, FCBTweenIntGetterdDelegate);
DECLARE_DELEGATE_OneParam(FCBTweenIntSetterdDelegate, float);
/**
 * 
 */
UCLASS(NotBlueprintType)
class CBCOMMON_API UCBTweenInt : public UCBTweenBase
{
	GENERATED_UCLASS_BODY()

public:
	float StartKey;
	int32 StartValue;
	int32 EndValue;
	float ChangeKey;

	int32 OriginStartKey;

	FCBTweenIntGetterdDelegate Getter;
	FCBTweenIntSetterdDelegate Setter;

public:
	void SetInitialKey(const FCBTweenIntGetterdDelegate IntGetter,const FCBTweenIntSetterdDelegate InSetter,int32 InEndKey,float InDuration);

protected:
	/* Begin CBTweenBase Interface. */
	virtual void OnStartGetValue() override;
	virtual void TweenAndApplyValue(float CurrentTime) override;
	virtual void SetValueForIncremental() override;
	virtual void SetOriginValueForReverse() override;
	/* End CBTweenBase Interface. */
};
