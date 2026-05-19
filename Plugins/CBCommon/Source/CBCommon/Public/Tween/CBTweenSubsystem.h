// Copyright 2023 CB,  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Classes/CBTweenFloat.h"
#include "Classes/CBTweenInt.h"
#include "Classes/CBTweenMaterialScalar.h"
#include "Classes/CBTweenTransform.h"
#include "Classes/CBTweenVector.h"
#include "Classes/CBTweenVector2D.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CBTweenSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FCBTweenUpdateMulticastSignature, float);

DECLARE_STATS_GROUP(TEXT("CBTween"), STATGROUP_CBTween, STATCAT_Advanced);
/**
 * 
 */
UCLASS()
class CBCOMMON_API UCBTweenSubsystem : public UGameInstanceSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
public:
	//~USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End of USubsystem interface

	//~FTickableObjectBase interface
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	virtual UWorld* GetTickableGameObjectWorld() const override;
	//~End of FTickableObjectBase interface

public:
	
	UFUNCTION(BlueprintPure, Category = LTween, meta = (WorldContext = "WorldContextObject"))
	static UCBTweenSubsystem* GetTweenSubsystee(UObject* WorldContextObject);
	
	/** 使用“CustomTick”而不是 UE4 的默认 Tick 来控制补间动画。 调用“DisableTick”函数禁用UE4默认的Tick函数，再调用这个CustomTick函数*/
	UFUNCTION(BlueprintCallable, Category = "CB|Tween")
	void CustomTick(float DeltaTime);
	
	/**
	* 禁用默认的 Tick 功能，这样你就可以暂停所有补间或使用 CustomTick 来做你自己的 tick 并使用你自己的 DeltaTime。
	* 这只会暂停当前 UCBTweenSubsystem 实例的 tick，因此在加载新关卡后，默认 Tick 将再次工作，如果要禁用 tick，则需要再次调用 DisableTick。
	*/
	UFUNCTION(BlueprintCallable, Category = "CB|Tween")
	void DisableTick();
	
	/** 启用默认勾选（如果禁用）*/
	UFUNCTION(BlueprintCallable, Category = "CB|Tween")
	void EnableTick();

	/** 杀死 All Tweens */
	UFUNCTION(BlueprintCallable, Category = "CB|Tween")
	void KillAllTweens(bool bCallComplete = false);

public:
	UCBTweenBase* To(const FCBTweenIntGetterdDelegate& Getter, const FCBTweenIntSetterdDelegate& Setter, int32 EndValue, float Duration);
	UCBTweenBase* To(const FCBTweenFloatGetterdDelegate& Getter, const FCBTweenFloatSetterdDelegate& Setter, float EndValue, float Duration);
	UCBTweenBase* To(const FCBTweenVectorGetterdDelegate& Getter, const FCBTweenVectorSetterdDelegate& Setter, FVector EndValue, float Duration);
	UCBTweenBase* To(const FCBTweenVector2DGetterdDelegate& Getter, const FCBTweenVector2DSetterdDelegate& Setter, FVector2D EndValue, float Duration);
	UCBTweenBase* To(const FCBTweenTransformGetterdDelegate& Getter, const FCBTweenTransformSetterdDelegate& Setter, FTransform EndValue, float Duration);
	UCBTweenBase* To(const FCBTweenMaterialScalarGetterDelegate& Getter, const FCBTweenMaterialScalarSetterDelegate& Setter,float EndValue, float Duration, int32 ParameterIndex);
	
private:
	UPROPERTY(VisibleAnywhere, Category="CB|Tween")
	TArray<UCBTweenBase*> TweenerList;

private:
	FCBTweenUpdateMulticastSignature OnMulticastUpdateEvent;
	
	bool TickPaused;
	
private:
	void OnTick(float DeltaTime);
	
};
