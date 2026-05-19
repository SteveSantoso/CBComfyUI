// Copyright 2023 CB,  All Rights Reserved.


#include "Tween/CBTweenBase.h"

UCBTweenBase::UCBTweenBase(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
	,Duration(0.0f)
	,Delay(0.0f)
	,ElapseTime(0.0f)
	,NumLoopsToPlay(0)
	,CurrentLoops(0)
	,bMarkedToKill(false)
	,bMarkedPause(false)
	,bStart(false)
	,bReverse(false)
	,PlayMode(ETweenPlayMode::Type::Once)
{

	OnEaseFunction.BindStatic(&UCBEaseBPLibrary::CubicOut);
}

void UCBTweenBase::SetEase(EEasingCategory EasingCategory, EEasingType EasingType)
{
	switch (EasingCategory)
	{
	case EEasingCategory::Ease_Linear:
		OnEaseFunction.BindStatic(&UCBEaseBPLibrary::Linear);
		break;
	case EEasingCategory::Ease_Sine:
		switch (EasingType)
		{
			case EEasingType::EaseIn:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::SineIn);
				break;
			
			case EEasingType::EaseOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::SineOut);
				break;
			case EEasingType::EaseInOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::SineInOut);
				break;
		}
		break;
		
	case EEasingCategory::Ease_Cubic:
		switch (EasingType)
		{
			case EEasingType::EaseIn:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::CubicIn);
				break;
			
			case EEasingType::EaseOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::CubicOut);
				break;
			
			case EEasingType::EaseInOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::CubicInOut);
				break;
		}		
		break;
		
	case EEasingCategory::Ease_Quint:
		switch (EasingType)
		{
			case EEasingType::EaseIn:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::QuintIn);
				break;
			
			case EEasingType::EaseOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::QuintOut);
				break;
			
			case EEasingType::EaseInOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::QuintInOut);
				break;
		}		
		break;
	
	case EEasingCategory::Ease_Circle:
		switch (EasingType)
		{
			case EEasingType::EaseIn:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::CircleIn);
				break;
			
			case EEasingType::EaseOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::CircleOut);
				break;
			
			case EEasingType::EaseInOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::CircleInOut);
				break;
		}		
		break;
	
	case EEasingCategory::Ease_Elastic:
		switch (EasingType)
		{
			case EEasingType::EaseIn:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::ElasticIn);
				break;
			
			case EEasingType::EaseOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::ElasticOut);
				break;
			
			case EEasingType::EaseInOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::ElasticInOut);
				break;
		}		
		break;

	case EEasingCategory::Ease_Quad:
		switch (EasingType)
		{
			case EEasingType::EaseIn:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::QuintIn);
				break;
			
			case EEasingType::EaseOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::QuintOut);
				break;
			
			case EEasingType::EaseInOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::QuintInOut);
				break;
		}		
		break;
	
	case EEasingCategory::Ease_Quart:
		switch (EasingType)
		{
			case EEasingType::EaseIn:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::QuadIn);
				break;
			
			case EEasingType::EaseOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::QuadOut);
				break;
			
			case EEasingType::EaseInOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::QuadInOut);
				break;
		}		
		break;

	case EEasingCategory::Ease_Expo:
		switch (EasingType)
		{
			case EEasingType::EaseIn:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::ExpoIn);
				break;
			
			case EEasingType::EaseOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::ExpoOut);
				break;
			
			case EEasingType::EaseInOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::ExpoInOut);
				break;
		}		
		break;

	case EEasingCategory::Ease_Back:
		switch (EasingType)
		{
			case EEasingType::EaseIn:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::BackIn);
				break;
			
			case EEasingType::EaseOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::BackOut);
				break;
			
			case EEasingType::EaseInOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::BackInOut);
				break;
		}		
		break;

	case EEasingCategory::Ease_Bounce:
		switch (EasingType)
		{
			case EEasingType::EaseIn:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::BounceIn);
				break;
			
			case EEasingType::EaseOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::BounceOut);
				break;
			
			case EEasingType::EaseInOut:
				OnEaseFunction.BindStatic(&UCBEaseBPLibrary::BounceInOut);
			break;
		}		
		break;

	default:
		break;
	}
}

void UCBTweenBase::SetDelay(float InDelay)
{
	if (ElapseTime > 0 || bStart)
	{
		return;
	} 
	Delay = InDelay;
	
	if (Delay < 0)
	{
		Delay = 0;
	}

}

void UCBTweenBase::SetLoop(ETweenPlayMode::Type InPlayMode, int32 NomberOfLoops/** = 1*/)
{
	if(ElapseTime >0 || bStart)
	{
		return;
	}

	PlayMode=InPlayMode;
	NumLoopsToPlay=NomberOfLoops;
	
}

void UCBTweenBase::Kill(bool CallComplete)
{
	if (CallComplete)
	{
		OnCompleteEvent.ExecuteIfBound();
	}
	bMarkedToKill = true;
}

void UCBTweenBase::ForceComplete()
{
	bMarkedToKill=true;
	ElapseTime=Delay+Duration;
	TweenAndApplyValue(Duration);
	OnUpdateEvent.ExecuteIfBound(1.0f);
	OnCompleteEvent.ExecuteIfBound();
}

void UCBTweenBase::Pause()
{
	bMarkedPause=true;
}

void UCBTweenBase::Resume()
{
	bMarkedPause=false;
}

void UCBTweenBase::Restart()
{
	if(ElapseTime == 0)
	{
		return;
	}
	bMarkedPause =false;
	CurrentLoops=0;
	bReverse=false;
	SetOriginValueForReverse();
	ToNextWithElapsedTime(0.0f);
}

void UCBTweenBase::Goto(float TimePoint)
{
	TimePoint=FMath::Clamp(TimePoint,0.0f,Duration);
	CurrentLoops=0;
	bReverse=false;
	ToNextWithElapsedTime(TimePoint);
}

int32 UCBTweenBase::GetLoopCurrentCount()
{
	return CurrentLoops;
}

void UCBTweenBase::OnComplete(const FSimpleDelegate& InSignature)
{
	OnCompleteEvent=InSignature;
}

void UCBTweenBase::OnComplete(const TFunction<void()>& InFunction)
{
	if(InFunction)
	{
		OnCompleteEvent.BindLambda(InFunction);
	}
}

void UCBTweenBase::OnComplete(const FCBTweenSimpleDynamicSignature& InDynamicSignature)
{
	OnCompleteEvent.BindLambda([InDynamicSignature]
	{
		InDynamicSignature.ExecuteIfBound();
	});
}

void UCBTweenBase::OnCycleComplete(const FSimpleDelegate& InSignature)
{
	OnCycleCompleteEvent=InSignature;
}

void UCBTweenBase::OnCycleComplete(const TFunction<void()>& InFunction)
{
	if(InFunction)
	{
		OnCycleCompleteEvent.BindLambda(InFunction);
	}
}

void UCBTweenBase::OnCycleComplete(const FCBTweenSimpleDynamicSignature& InDynamicSignature)
{
	OnCycleCompleteEvent.BindLambda([InDynamicSignature]
	{
		InDynamicSignature.ExecuteIfBound();
	});
}

void UCBTweenBase::OnCycleStart(const FSimpleDelegate& InSignature)
{
	OnCycleStartEvent=InSignature;
}

void UCBTweenBase::OnCycleStart(const TFunction<void()>& InFunction)
{
	if(InFunction)
	{
		OnCycleStartEvent.BindLambda(InFunction);
	}
}

void UCBTweenBase::OnCycleStart(const FCBTweenSimpleDynamicSignature& InDynamicSignature)
{
	OnCycleStartEvent.BindLambda([InDynamicSignature]
	{
		InDynamicSignature.ExecuteIfBound();
	});
}

void UCBTweenBase::OnUpdate(const FCBTweenUpdatSignature& InSignature)
{
	OnUpdateEvent=InSignature;
}

void UCBTweenBase::OnUpdate(const TFunction<void(float)>& InFunction)
{
	if(InFunction)
	{
		OnUpdateEvent.BindLambda(InFunction);
	}
}

void UCBTweenBase::OnUpdate(const FCBTweenProgressDynamicSignature& InDynamicSignature)
{
	OnUpdateEvent.BindLambda([InDynamicSignature](float Progress)
	{
		InDynamicSignature.ExecuteIfBound(Progress);
	});
}

void UCBTweenBase::OnStart(const FSimpleDelegate& InSignature)
{
	OnStartEvent=InSignature;
}

void UCBTweenBase::OnStart(const TFunction<void()>& InFunction)
{
	if(InFunction)
	{
		OnStartEvent.BindLambda(InFunction);
	}
}

bool UCBTweenBase::ToNext(float DeltaTime)
{
	if(bMarkedToKill)
	{
		return false;
	}
	if(bMarkedPause)
	{
		return true;
	}
	return  ToNextWithElapsedTime(ElapseTime+DeltaTime);
}

bool UCBTweenBase::ToNextWithElapsedTime(float InElapseTime)
{
	ElapseTime=InElapseTime;

	//如果总的时间大于延迟，执行动画
	if(ElapseTime>Delay)
	{
		if(!bStart)
		{
			bStart=true;
			//设置重置value
			OnStartGetValue();
			OnCycleStartEvent.ExecuteIfBound();
			OnStartEvent.ExecuteIfBound();
		}

		float ElapseTimeWithoutDelay=ElapseTime-Delay;
		float CurrentTime=ElapseTimeWithoutDelay-Duration*CurrentLoops;
		if(CurrentTime>=Duration)
		{
			bool ReturnValue=true;
			CurrentLoops++;
			
			TweenAndApplyValue(bReverse ? 0:Duration);
			OnUpdateEvent.ExecuteIfBound(1.0f);
			OnCompleteEvent.ExecuteIfBound();

			if(PlayMode == ETweenPlayMode::Type::Once)
			{
				OnCompleteEvent.ExecuteIfBound();
				ReturnValue =false;
			}
			//无限循环
			else if(NumLoopsToPlay<=-1)
			{
				OnCycleStartEvent.ExecuteIfBound();
				ReturnValue =true;
			}
			else
			{
				if(CurrentLoops>=NumLoopsToPlay)
				{
					OnCompleteEvent.ExecuteIfBound();
					ReturnValue=false;
				}
				else
				{
					OnCycleStartEvent.ExecuteIfBound();
					ReturnValue=true;
				}
			}

			switch (PlayMode)
			{
			case ETweenPlayMode::Type::Reverse:
				SetValueForReverse();
				break;
			case ETweenPlayMode::Type::PingPong:
				bReverse= !bReverse;
				SetValueForPingPong();
				break;
			case ETweenPlayMode::Type::Incremental:
				SetValueForIncremental();
				break;
			}
			return ReturnValue;
		}
		else
		{
			if(bReverse)
			{
				CurrentTime=Duration-CurrentTime;
			}
			TweenAndApplyValue(CurrentTime);
			OnUpdateEvent.ExecuteIfBound(CurrentTime/Duration);
			return true;
		}
	}
	else
	{
		//等待
		return true;
	}

	
}

void UCBTweenBase::OnStart(const FCBTweenSimpleDynamicSignature& InDynamicSignature)
{
	OnStartEvent.BindLambda([InDynamicSignature]
	{
		InDynamicSignature.ExecuteIfBound();
	});
}
