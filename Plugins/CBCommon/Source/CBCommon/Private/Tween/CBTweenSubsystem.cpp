// Copyright 2023 CB,  All Rights Reserved.


#include "Tween/CBTweenSubsystem.h"

#include "Kismet/GameplayStatics.h"
#include "Tween/CBTweenBase.h"
DECLARE_CYCLE_STAT(TEXT("CBTween Update"), STAT_Update, STATGROUP_CBTween);

void UCBTweenSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UCBTweenSubsystem::Deinitialize()
{
	Super::Deinitialize();
	TweenerList.Empty();
}

bool UCBTweenSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return true;
}

void UCBTweenSubsystem::Tick(float DeltaTime)
{
	if (TickPaused == false)
	{
		OnTick(DeltaTime);
	}
}

ETickableTickType UCBTweenSubsystem::GetTickableTickType() const
{
	return ETickableTickType::Conditional;
}

bool UCBTweenSubsystem::IsTickable() const
{
	return !HasAnyFlags(RF_ClassDefaultObject);
}

TStatId UCBTweenSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UCBTweenSubsystem, STATGROUP_Tickables);
}

UWorld* UCBTweenSubsystem::GetTickableGameObjectWorld() const
{
	return FTickableGameObject::GetTickableGameObjectWorld();
}

UCBTweenSubsystem* UCBTweenSubsystem::GetTweenSubsystee(UObject* WorldContextObject)
{
	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	return GameInstance->GetSubsystem<UCBTweenSubsystem>();
}

void UCBTweenSubsystem::CustomTick(float DeltaTime)
{
	OnTick(DeltaTime);
}

void UCBTweenSubsystem::DisableTick()
{
	TickPaused = true;
}

void UCBTweenSubsystem::EnableTick()
{
	TickPaused = false;
}

void UCBTweenSubsystem::KillAllTweens(bool bCallComplete)
{
	for(UCBTweenBase *Item:TweenerList)
	{
		if(IsValid(Item))
		{
			Item->Kill(bCallComplete);
		}
	}
}

UCBTweenBase* UCBTweenSubsystem::To(const FCBTweenFloatGetterdDelegate& Getter,
	const FCBTweenFloatSetterdDelegate& Setter, float EndValue, float Duration)
{
	UCBTweenFloat* TweenFloat=NewObject<UCBTweenFloat>();
	TweenFloat->SetInitialKey(Getter,Setter,EndValue,Duration);
	TweenerList.Add(TweenFloat);

	return TweenFloat;
}

UCBTweenBase* UCBTweenSubsystem::To(const FCBTweenVectorGetterdDelegate& Getter,
	const FCBTweenVectorSetterdDelegate& Setter, FVector EndValue, float Duration)
{
	UCBTweenVector* TweenVector=NewObject<UCBTweenVector>();
	TweenVector->SetInitialKey(Getter,Setter,EndValue,Duration);
	TweenerList.Add(TweenVector);

	return TweenVector;
}

UCBTweenBase* UCBTweenSubsystem::To(const FCBTweenVector2DGetterdDelegate& Getter,
	const FCBTweenVector2DSetterdDelegate& Setter, FVector2D EndValue, float Duration)
{
	UCBTweenVector2D* TweenVector=NewObject<UCBTweenVector2D>();
	TweenVector->SetInitialKey(Getter,Setter,EndValue,Duration);
	TweenerList.Add(TweenVector);

	return TweenVector;
}

UCBTweenBase* UCBTweenSubsystem::To(const FCBTweenTransformGetterdDelegate& Getter,
	const FCBTweenTransformSetterdDelegate& Setter, FTransform EndValue, float Duration)
{
	UCBTweenTransform* TweenTransform=NewObject<UCBTweenTransform>();
	TweenTransform->SetInitialKey(Getter,Setter,EndValue,Duration);
	TweenerList.Add(TweenTransform);

	return TweenTransform;
}

UCBTweenBase* UCBTweenSubsystem::To(const FCBTweenIntGetterdDelegate& Getter, const FCBTweenIntSetterdDelegate& Setter,
                                    int32 EndValue, float Duration)
{
	UCBTweenInt* TweenInt=NewObject<UCBTweenInt>();
	TweenInt->SetInitialKey(Getter,Setter,EndValue,Duration);
	TweenerList.Add(TweenInt);

	return TweenInt;
}

UCBTweenBase* UCBTweenSubsystem::To(const FCBTweenMaterialScalarGetterDelegate& Getter,
                                    const FCBTweenMaterialScalarSetterDelegate& Setter, float EndValue, float Duration, int32 ParameterIndex)
{
	UCBTweenMaterialScalar* TweenMaterialScalar=NewObject<UCBTweenMaterialScalar>();
	TweenMaterialScalar->SetInitialValue(Getter,Setter,EndValue,Duration,ParameterIndex);
	TweenerList.Add(TweenMaterialScalar);

	return TweenMaterialScalar;
}

void UCBTweenSubsystem::OnTick(float DeltaTime)
{
	SCOPE_CYCLE_COUNTER(STAT_Update);
	
	auto Count = TweenerList.Num();
	for (int32 i = 0; i < Count; i++)
	{
		UCBTweenBase* Tweener = TweenerList[i];
		if (!IsValid(Tweener))
		{
			TweenerList.RemoveAt(i);
			i--;
			Count--;
		}
		else
		{
			if (Tweener->ToNext(DeltaTime) == false)
			{
				TweenerList.RemoveAt(i);
				Tweener->ConditionalBeginDestroy();
				i--;
				Count--;
			}
		}
	}
	if (OnMulticastUpdateEvent.IsBound())
	{
		OnMulticastUpdateEvent.Broadcast(DeltaTime);
	}
	
}
