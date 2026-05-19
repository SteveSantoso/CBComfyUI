// Copyright 2023 CB,  All Rights Reserved.


#include "Tween/Classes/CBTweenMaterialScalar.h"

#include "CBCommon.h"

UCBTweenMaterialScalar::UCBTweenMaterialScalar(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
	,StartKey(0.0f)
	,EndKey(0.0f)
	,ChangeKey(0.0f)
	,OriginStartKey(0.0f)
	,ParameterIndex(0)
{
	
}

void UCBTweenMaterialScalar::SetInitialValue(const FCBTweenMaterialScalarGetterDelegate& InGetter,
                                             const FCBTweenMaterialScalarSetterDelegate& InSetter, float InEndKey, float InDuration, int32 InParameterIndex)
{
	Duration=InDuration;
	Getter=InGetter;
	Setter=InSetter;
	EndKey=InEndKey;
	ParameterIndex=InParameterIndex;
}

void UCBTweenMaterialScalar::OnStartGetValue()
{
	if(Getter.IsBound())
	{
		if(Getter.Execute(StartKey))
		{
			ChangeKey=EndKey-StartKey;
			OriginStartKey=ChangeKey;
		}
		else
		{
			UE_LOG(LogCBTween,Warning,TEXT("[UCBTweenMaterialScalar/OnStartGetValue]Get paramter value error!"));
		}
	}
}

void UCBTweenMaterialScalar::TweenAndApplyValue(float CurrentTime)
{
	float Value=OnEaseFunction.Execute(CurrentTime,StartKey,ChangeKey,Duration);
	if(Setter.Execute(ParameterIndex,Value)==false)
	{
		UE_LOG(LogCBTween,Warning,TEXT("[UCBTweenMaterialScalar/TweenAndApplyValue]Set paramter value error!"));
	}
}

void UCBTweenMaterialScalar::SetValueForIncremental()
{
	StartKey=EndKey;
	EndKey+=ChangeKey;
}

void UCBTweenMaterialScalar::SetOriginValueForReverse()
{
	StartKey=OriginStartKey;
	EndKey=OriginStartKey+ChangeKey;
}
