// Copyright 2023 CB,  All Rights Reserved.


#include "Tween/Classes/CBTweenVector2D.h"

UCBTweenVector2D::UCBTweenVector2D(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
	,StartKey(0.0f)
	,ChangeKey(1.0f)
	,OriginStartKey(0.0f)
{
	
}

void UCBTweenVector2D::SetInitialKey(const FCBTweenVector2DGetterdDelegate IntGetter,
	const FCBTweenVector2DSetterdDelegate InSetter,  const FVector2D InEndKey, float InDuration)
{
	Duration=InDuration;
	Getter=IntGetter;
	Setter=InSetter;
	EndValue=InEndKey;
}

void UCBTweenVector2D::OnStartGetValue()
{
	if(Getter.IsBound())
	{
		StartValue=Getter.Execute();
	}
	OriginStartKey=StartValue;
}

void UCBTweenVector2D::TweenAndApplyValue(float CurrentTime)
{
	float Alpha=OnEaseFunction.Execute(CurrentTime,StartKey,ChangeKey,Duration);
	auto Value = FMath::Lerp(StartValue, EndValue, Alpha);
	Setter.ExecuteIfBound(Value);
}

void UCBTweenVector2D::SetValueForIncremental()
{
	FVector2D DiffValue = EndValue - StartValue;
	StartValue = EndValue;
	EndValue += DiffValue;
}

void UCBTweenVector2D::SetOriginValueForReverse()
{
	FVector2D DiffValue = EndValue - StartValue;
	StartValue = OriginStartKey;
	EndValue = DiffValue+OriginStartKey;
}
