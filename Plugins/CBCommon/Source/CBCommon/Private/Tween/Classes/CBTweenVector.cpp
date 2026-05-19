// Copyright 2023 CB,  All Rights Reserved.


#include "Tween/Classes/CBTweenVector.h"

UCBTweenVector::UCBTweenVector(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
	,StartKey(0.0f)
	,ChangeKey(1.0f)
	,OriginStartKey(0.0f)
{
	
}

void UCBTweenVector::SetInitialKey(const FCBTweenVectorGetterdDelegate IntGetter,
	const FCBTweenVectorSetterdDelegate InSetter,  const FVector InEndKey, float InDuration)
{
	Duration=InDuration;
	Getter=IntGetter;
	Setter=InSetter;
	EndValue=InEndKey;
}

void UCBTweenVector::OnStartGetValue()
{
	if(Getter.IsBound())
	{
		StartValue=Getter.Execute();
	}
	OriginStartKey=StartValue;
}

void UCBTweenVector::TweenAndApplyValue(float CurrentTime)
{
	float Alpha=OnEaseFunction.Execute(CurrentTime,StartKey,ChangeKey,Duration);
	FVector Value = FMath::Lerp(StartValue, EndValue, Alpha);
	Setter.ExecuteIfBound(Value);
}

void UCBTweenVector::SetValueForIncremental()
{
	FVector DiffValue = EndValue - StartValue;
	StartValue = EndValue;
	EndValue += DiffValue;
}

void UCBTweenVector::SetOriginValueForReverse()
{
	FVector DiffValue = EndValue - StartValue;
	StartValue = OriginStartKey;
	EndValue = DiffValue+OriginStartKey;
}
