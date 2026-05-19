// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LatentActions.h"
#include "CBBPTaskBase.generated.h"

/**
 * 
 */

DECLARE_MULTICAST_DELEGATE(FCBBPTaskOnCancelDelegate);

UCLASS(HideDropdown, BlueprintType, HideCategories = (Object), meta = (DontUseGenericSpawnObject = "true"))
class CBCOMMON_API UCBBPTaskBase : public UObject, public FTickableGameObject
{
	GENERATED_BODY()

public:
	UCBBPTaskBase();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TVTaskTick")
	bool bIsTickable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TVTaskTick")
	bool bIsTickableInEditor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TVTaskTick")
	bool bIsTickableWhenPaused;

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TVTaskEvents")
	void OnComplete();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TVTaskEvents")
	void OnCancel();

public:
	FCBBPTaskOnCancelDelegate OnCancelDelegate;

	TFunction<void()>BodyFunction;

public:
	/**
	 *开始Task
	 *@param return fasle如果Task开始运行
	 */
	virtual bool Start();

	/*
	 * 取消Task
	 */
	virtual  void Cancel();

	/*
	 *检查Task是否正在进行
	 */
	virtual bool IsRunning();

	/*
	 * 检查Task是否取消
	 */
	virtual bool IsCanceled();

public:
	virtual void Tick(float DeltaTime) override;

	virtual bool IsTickable() const override;

	virtual bool IsTickableInEditor() const override;

	virtual bool IsTickableWhenPaused() const override;

	virtual UWorld* GetTickableGameObjectWorld() const override;

	virtual UWorld* GetWorld() const override;

private:
	virtual TStatId GetStatId() const override;

protected:
	FThreadSafeBool bCanceled;
};


class CBCOMMON_API FSingleTaskActionBase :public FPendingLatentAction
{
public:
	FSingleTaskActionBase(UObject* InObject, const FLatentActionInfo& LatentInfo, TSubclassOf<class UCBBPTaskBase> TaskClass);

	virtual ~FSingleTaskActionBase() override;

	virtual bool IsRunning();

	virtual bool IsCanceled();

protected:
	UObject* Object;
	FName ExecutionFunction;
	int32 OutputLink;
	FWeakObjectPtr CallbackTarget;
	UCBBPTaskBase* Task = nullptr;
};
