// Copyright 2023 CB,  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Tween/CBTweenBase.h"
#include "CBTweenMaterialScalar.generated.h"

/**
 * 
 */
DECLARE_DELEGATE_RetVal_OneParam(bool, FCBTweenMaterialScalarGetterDelegate, float&);
DECLARE_DELEGATE_RetVal_TwoParams(bool, FCBTweenMaterialScalarSetterDelegate, int32, float);

UCLASS(NotBlueprintType)
class CBCOMMON_API UCBTweenMaterialScalar : public UCBTweenBase
{
	GENERATED_UCLASS_BODY()

public:
	float StartKey;
	float EndKey;
	float ChangeKey;
	float OriginStartKey;
	int32 ParameterIndex;
	
	FCBTweenMaterialScalarGetterDelegate Getter;
	FCBTweenMaterialScalarSetterDelegate Setter;
public:
	void SetInitialValue(const FCBTweenMaterialScalarGetterDelegate& InGetter, const FCBTweenMaterialScalarSetterDelegate& InSetter, float InEndKey, float InDuration, int32 InParameterIndex);

protected:
	/* Begin CBTweenBase Interface. */
	virtual void OnStartGetValue() override;
	virtual void TweenAndApplyValue(float CurrentTime) override;
	virtual void SetValueForIncremental() override;
	virtual void SetOriginValueForReverse() override;
	/* End CBTweenBase Interface. */
};
