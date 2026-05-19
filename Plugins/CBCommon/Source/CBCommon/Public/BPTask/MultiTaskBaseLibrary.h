// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BPTask/CBBPTaskBase.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MultiTaskBaseLibrary.generated.h"

/**
 * 
 */
UCLASS()
class CBCOMMON_API UMultiTaskBaseLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UMultiTaskBaseLibrary();
	~UMultiTaskBaseLibrary();

public:
	/**设置最大循环迭代值.*/
	UFUNCTION(BlueprintCallable, Category = "CBBPTask|Utilities")
	static void SetMaximumLoopIterations(int32 MaximumLoopIterations);

	/**重置失控loop检测以防止破坏loop.*/
	UFUNCTION(BlueprintCallable, Category = "CBBPTask|Utilities")
	static void ResetRunaway();

	/**Get World*/
	UFUNCTION(BlueprintPure, Category = "CBBPTask|Utilities")
	static UObject* GetContextWorld(UObject* WorldContextObject);

	/**
	* 将对象添加到 Root Set 以防止它被 GC 收集。
	* 当不需要对象时，将其从 Root Set删除。
	*/
	UFUNCTION(BlueprintCallable, Category = "CBBPTask|Utilities")
	static void AddToRoot(UObject* Object);

	/**从 Root Set删除。*/
	UFUNCTION(BlueprintCallable, Category = "CBBPTask|Utilities")
	static void RemoveFromRoot(UObject* Object);

	/**取消Task*/
	UFUNCTION(BlueprintCallable, Category = "CBBPTask|Task")
	static void Cancel(UCBBPTaskBase* Task);

	/**检查Task是否在进程中*/
	UFUNCTION(BlueprintPure, Category = "CBBPTask|Task")
	static bool IsRunning(UCBBPTaskBase* Task);

	/**检查Task是否被取消*/
	UFUNCTION(BlueprintPure, Category = "CBBPTask|Task")
	static bool IsCanceled(UCBBPTaskBase* Task);

public:
	static TArray<UObject*> RootObjects;
	static int32 TaskIndex;
	static int32 MutexIndex;
	static int32 ThreadPoolIndex;

#if WITH_EDITOR
	static FDelegateHandle EndPIEHandle;
#else
	static FDelegateHandle PreExitHandle;
#endif

private:
	static void OnEndPIE(const bool bIsSimulating);
	static void OnPreExit();
};
