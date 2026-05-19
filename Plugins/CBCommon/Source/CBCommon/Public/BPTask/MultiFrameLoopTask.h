// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CBBPTaskBase.h"
#include "CBBPTaskTypes.h"

#include "MultiFrameLoopTask.generated.h"

/**
 * 
 */

class UHierarchicalInstancedStaticMeshComponent;

DECLARE_MULTICAST_DELEGATE(FLoopTaskDelegate);

UCLASS(Blueprintable, hidecategories = (Object), meta = (DontUseGenericSpawnObject = "True"))
class CBCOMMON_API UMultiFrameLoopTask : public UCBBPTaskBase
{
	GENERATED_BODY()

	friend class FMultiFrameLoopTaskAction;

public:
	UMultiFrameLoopTask();

	virtual bool Start() override;

	/**检查是否在进程中. */
	virtual bool IsRunning() override;

protected:
	virtual void Tick(float DeltaTime) override;


public:
	int32 Size;
	
	int32 IterationsPerTick;
	
	float Delay;

public:
	FLoopTaskDelegate TaskDelegate;

protected:
	int32 CurrentIndex;

	bool bStarted;

	float TimeRemaining;
	
};

UCLASS(Blueprintable, hidecategories = (Object), meta = (DontUseGenericSpawnObject = "True"))
class CBCOMMON_API USpawnInstanceTask : public UMultiFrameLoopTask
{
	GENERATED_BODY()

	friend class FSpawnInstansTaskAction;

public:
	USpawnInstanceTask();

protected:
	virtual void Tick(float DeltaTime) override;

public:		
	TArray<FTransform> Transforms;

	UHierarchicalInstancedStaticMeshComponent* HISM;
};


class CBCOMMON_API FMultiFrameLoopTaskAction :public FSingleTaskActionBase
{
private:
	int32& CurrentIndex;
	int32 Size;
	ECBTaskBranchesWithBody& Branches;
	bool bStarted;

public:
	FMultiFrameLoopTaskAction(UObject* InObject, ECBTaskBranchesWithBody& InBranches, const FLatentActionInfo& LatentInfo, TSubclassOf<class UMultiFrameLoopTask> TaskClass, int32& InCurrentIndex, const int32 InSize, const int32 InIterationsPerTick, const float InDelay, UMultiFrameLoopTask*& OutTask)
		: FSingleTaskActionBase(InObject, LatentInfo, TaskClass)
		, CurrentIndex(InCurrentIndex)
		, Size(InSize)
		, Branches(InBranches)
		, bStarted(false)
	{
		OutTask = Cast<UMultiFrameLoopTask>(Task);

		if (OutTask)
		{
			Branches = ECBTaskBranchesWithBody::OnStart;
			OutTask->BodyFunction();
			OutTask->Size = InSize;
			OutTask->IterationsPerTick = InIterationsPerTick;
			OutTask->Delay = InDelay;
			bStarted = Task->Start();
		}
		else
		{
			return;
		}

		if (bStarted)
		{
			OutTask->TaskDelegate.AddLambda([OutTask, &InBranches, &InCurrentIndex]
			{
				InBranches = ECBTaskBranchesWithBody::OnTaskBody;
				InCurrentIndex = OutTask->CurrentIndex % OutTask->Size;
				OutTask->BodyFunction();
			});
		}
	}


	virtual ~FMultiFrameLoopTaskAction()
	{
		if (Task != nullptr && Task->IsValidLowLevel() && !IsValid(Task))
		{
			UMultiFrameLoopTask* LocalTask = Cast<UMultiFrameLoopTask>(Task);
			if (LocalTask)
			{
				LocalTask->TaskDelegate.RemoveAll(this);
			}
		}
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		if (bStarted)
		{
			if (!IsCanceled())
			{
				if (!IsRunning())
				{
					Branches = ECBTaskBranchesWithBody::OnCompleted;
					Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
				}
			}
			else
			{
				Branches = ECBTaskBranchesWithBody::OnCanceled;
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			}
		}
		else
		{
			Branches = ECBTaskBranchesWithBody::OnCompleted;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}


};

class CBCOMMON_API FSpawnInstansTaskAction :public FSingleTaskActionBase
{
private:
	int32& CurrentIndex;
	UHierarchicalInstancedStaticMeshComponent* HISM;
	TArray<FTransform> Transforms;
	ECBTaskBranchesWithBody& Branches;
	bool bStarted;

public:
	FSpawnInstansTaskAction(UObject* InObject, ECBTaskBranchesWithBody& InBranches, const FLatentActionInfo& LatentInfo, TSubclassOf<class USpawnInstanceTask> TaskClass, int32& InCurrentIndex, UHierarchicalInstancedStaticMeshComponent* InHISM, TArray<FTransform> InTransforms, const int32 InIterationsPerTick, const float InDelay, USpawnInstanceTask*& OutTask)
		:FSingleTaskActionBase(InObject, LatentInfo, TaskClass)
		, CurrentIndex(InCurrentIndex)
		, HISM(InHISM)
		, Transforms(InTransforms)
		, Branches(InBranches)
		, bStarted(false)
	{
		OutTask = Cast<USpawnInstanceTask>(Task);

		if (OutTask)
		{
			Branches = ECBTaskBranchesWithBody::OnStart;
			OutTask->BodyFunction();
			OutTask->HISM = InHISM;
			OutTask->Size = InTransforms.Num();
			OutTask->Transforms = InTransforms;
			OutTask->IterationsPerTick = InIterationsPerTick;
			OutTask->Delay = InDelay;
			bStarted = Task->Start();
		}
		else
		{
			return;
		}

		if (bStarted)
		{
			OutTask->TaskDelegate.AddLambda([OutTask, &InBranches, &InCurrentIndex]
			{
				InBranches = ECBTaskBranchesWithBody::OnTaskBody;
				InCurrentIndex = OutTask->CurrentIndex % OutTask->Size;
				OutTask->BodyFunction();
			});
		}
	}

	virtual ~FSpawnInstansTaskAction()
	{
		if (Task != nullptr && Task->IsValidLowLevel() && !IsValid(Task))
		{
			USpawnInstanceTask* LocalTask = Cast<USpawnInstanceTask>(Task);
			if (LocalTask)
			{
				LocalTask->TaskDelegate.RemoveAll(this);
			}
		}
	}

	virtual void UpdateOperation(FLatentResponse& Response) override
	{
		if (bStarted)
		{
			if (!IsCanceled())
			{
				if (!IsRunning())
				{
					Branches = ECBTaskBranchesWithBody::OnCompleted;
					Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
				}
			}
			else
			{
				Branches = ECBTaskBranchesWithBody::OnCanceled;
				Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
			}
		}
		else
		{
			Branches = ECBTaskBranchesWithBody::OnCompleted;
			Response.FinishAndTriggerIf(true, ExecutionFunction, OutputLink, CallbackTarget);
		}
	}

};
