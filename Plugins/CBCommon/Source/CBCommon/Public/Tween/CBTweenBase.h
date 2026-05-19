// Copyright 2023 CB,  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "UObject/NoExportTypes.h"
#include "CBEaseBPLibrary.h"
#include "CBTweenBase.generated.h"



DECLARE_DELEGATE_RetVal_FourParams(float, FCBTweenSignature, float, float, float, float);
DECLARE_DELEGATE_OneParam(FCBTweenUpdatSignature, float);

DECLARE_DYNAMIC_DELEGATE(FCBTweenSimpleDynamicSignature);
DECLARE_DYNAMIC_DELEGATE_OneParam(FCBTweenProgressDynamicSignature, float, InProgress);
/** 描述动画的循环方式 */
UENUM(BlueprintType)
namespace ETweenPlayMode
{
	enum Type
	{
		/** 只播放一次，不做循环(A-B) */
		Once,
		/** 从头开始循环播放（A-B,A-B...） */
		Reverse,
		/** 往复循环播放 （A-B,B-A...）*/
		PingPong,
		/**每个循环周期结束时连续递增（A,B,B,B+(A-B)...）*/
		Incremental
	};
}


UCLASS(BlueprintType, Abstract)
class CBCOMMON_API UCBTweenBase : public UObject
{
	GENERATED_UCLASS_BODY()

public:
	/** 动画完成一次后调用 */
	FSimpleDelegate OnCompleteEvent;
	/** 如果使用循环，这将在每次循环开始时调用 */
	FSimpleDelegate OnCycleStartEvent;
	/** 如果使用循环，这将在每个循环中的补间完成后每次调用 */
	FSimpleDelegate OnCycleCompleteEvent;
	/** 动画开始后调用每一帧 */
	FCBTweenUpdatSignature OnUpdateEvent;
	/** 动画开始时调用一次 */
	FSimpleDelegate OnStartEvent;
	
public:
	
	/**设置动画曲线类型，若动画已经开始则无效*/
	UFUNCTION(BlueprintCallable,Category="Visual|Tween")
	virtual void SetEase(EEasingCategory EasingCategory,EEasingType EasingType);

	/**设置播放延迟，若动画以开始则无效*/
	UFUNCTION(BlueprintCallable,Category="Visual|Tween")
	virtual void SetDelay(float InDelay);

	/**
	 * 设置循环播放
	 * @param InPlayMode ETweenPlayMode::Type=Once时候只循环一次
	 * @param NomberOfLoops 
	 */
	UFUNCTION(BlueprintCallable,Category="Visual|Tween")
	virtual void SetLoop(ETweenPlayMode::Type InPlayMode, int32 NomberOfLoops = 1);

	/** 强制停止这个动画。 如果 CallComplete = true，将在停止后调用 OnComplete*/
	UFUNCTION(BlueprintCallable,Category="Visual|Tween")
	virtual void Kill(bool CallComplete = false);

	/** 帧强制停止此动画，将值value为结束，调用OnComplete。*/
	UFUNCTION(BlueprintCallable, Category = "Visual|Tween")
	virtual void ForceComplete();

	/** 暂停动画. */
	UFUNCTION(BlueprintCallable, Category = "Visual|Tween")
	virtual void Pause();

	/**继续播放动画，若暂停时*/
	UFUNCTION(BlueprintCallable, Category = "Visual|Tween")
	virtual void Resume();

	/**
	* 重启动画。
	* 如果未启动 Tween，则无效。
	*/
	UFUNCTION(BlueprintCallable, Category = "Visual|Tween")
	virtual void Restart();

	/**
	* 及时将时刻发送到给定的位置。
	* @param TimePoint 到达的时间位置（如果高于整个Tween持续时间，直接到达终点）。
	*/
	UFUNCTION(BlueprintCallable, Category = "Visual|Tween")
	virtual void Goto(float TimePoint);

	/**获取当前循环次数*/
	UFUNCTION(BlueprintCallable,Category="Visual|Tween")
	int32 GetLoopCurrentCount();

public:
	/**动画完成时执行*/
	UFUNCTION(BlueprintCallable, Category = "Visual|Tween")
	void OnComplete(const FCBTweenSimpleDynamicSignature& InDynamicSignature);

	/**如果使用循环，这将在每个循环周期完成后每次调用*/
	UFUNCTION(BlueprintCallable, Category = "Visual|Tween")
	void OnCycleComplete(const FCBTweenSimpleDynamicSignature& InDynamicSignature);
	
	/**如果使用循环，这将在每个循环周期开始每次调用*/
	UFUNCTION(BlueprintCallable, Category = "Visual|Tween")
	void OnCycleStart(const FCBTweenSimpleDynamicSignature& InDynamicSignature);
	
	/**如果正在播放动画，则执行每一帧*/
	UFUNCTION(BlueprintCallable, Category = "Visual|Tween")
	void OnUpdate(const FCBTweenProgressDynamicSignature& InDynamicSignature);
	
	/**动画开始时执行*/
	UFUNCTION(BlueprintCallable, Category = "Visual|Tween")
	void OnStart(const FCBTweenSimpleDynamicSignature& InDynamicSignature);



public:
	void OnComplete(const FSimpleDelegate& InSignature);
	void OnComplete(const TFunction<void()>& InFunction);

	void OnCycleComplete(const FSimpleDelegate& InSignature);
	void OnCycleComplete(const TFunction<void()>& InFunction);
	

	void OnCycleStart(const FSimpleDelegate& InSignature);
	void OnCycleStart(const TFunction<void()>& InFunction);
	

	void OnUpdate(const FCBTweenUpdatSignature& InSignature);
	void OnUpdate(const TFunction<void(float)>& InFunction);
	

	void OnStart(const FSimpleDelegate& InSignature);
	void OnStart(const TFunction<void()>& InFunction);

public:
	/**
	* @return false：tween 已经完成，需要被杀死。 true：仍在执行。
	*/
	virtual bool ToNext(float DeltaTime);
	
	virtual bool ToNextWithElapsedTime(float InElapseTime);

protected:
	/** 开始时获得取值 子类必须override它 */
	virtual void OnStartGetValue() PURE_VIRTUAL(UCBTweenBase::OnStartGetValue, );
	/** 执行时设置值。 子类必须override它*/
	virtual void TweenAndApplyValue(float CurrentTime) PURE_VIRTUAL(UCBTweenBase::TweenAndApplyValue, );
	/** 如果循环类型为增量，则设置起始值和结束值 */
	virtual void SetValueForIncremental() PURE_VIRTUAL(UCBTweenBase::SetValueForIncremental, );
	/** 在动画不想重新开始之前设置开始和结束值 */
	virtual void SetOriginValueForReverse() PURE_VIRTUAL(UCBTweenBase::SetOriginValueForReverse, );
	virtual void SetValueForPingPong() {};
	virtual void SetValueForReverse() {};

protected:
	
	/**一次动画时长*/
	float Duration;
	/**动画开始前的延迟*/
	float Delay;
	/**总运行时间，包括延迟*/
	float ElapseTime;

	/**循环播放的最大粗疏*/
	float NumLoopsToPlay;
	/**当前循环次数*/
	float CurrentLoops;
	
	/** 将这个 tween 标记为 kill，这样当下一次更新到来时，这个 tween 就会被杀死*/
	bool bMarkedToKill;
	/** 将此标记为暂停，它将保持暂停直到调用 Resume() */
	bool bMarkedPause;
	/**是否开始播放*/
	bool bStart ;
	/**是否播放反向动画*/
	bool bReverse ;
	
	ETweenPlayMode::Type PlayMode;

	/**easefunction调用*/
	FCBTweenSignature OnEaseFunction;

	
};
