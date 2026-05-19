// Fill out your copyright notice in the Description page of Project Settings.


#include "BPTask/MultiFrameTaskLibrary.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

void UMultiFrameTaskLibrary::DoLoopTask(UObject* WorldContextObject, ECBTaskBranchesWithBody& Out, FLatentActionInfo LatentInfo, int32& Index, UMultiFrameLoopTask*& Task, int32 Size, int32 IterationsPerTick, float Delay)
{
	if (Size<0 || IterationsPerTick <= 0 || nullptr == WorldContextObject|| Delay < 0.0f)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoopTask: Someting problems, please check."), ELogVerbosity::Error);
		return;
	}


	TSubclassOf<UMultiFrameLoopTask> Class = LoadClass<UMultiFrameLoopTask>(nullptr, TEXT("Class'/Script/CBCommon.MultiFrameLoopTask'"));

	if (nullptr == Class)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoopTask: Invalid Class. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FMultiFrameLoopTaskAction* Action = LatentActionManager.FindExistingAction<FMultiFrameLoopTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && !Action->IsCanceled())
		{
			FFrame::KismetExecutionMessage(TEXT("DoLoopTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else
		{
			Action = new FMultiFrameLoopTaskAction(WorldContextObject, Out, LatentInfo, Class, Index, Size, IterationsPerTick, Delay, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}

void UMultiFrameTaskLibrary::DoSpawnInstancesTask(UObject* WorldContextObject, ECBTaskBranchesWithBody& Out, FLatentActionInfo LatentInfo, int32& Index, USpawnInstanceTask*& Task, UHierarchicalInstancedStaticMeshComponent* HISM, TArray<FTransform> Transforms, int32 IterationsPerTick, float Delay)
{
	if (!HISM->IsValidLowLevel()|| Transforms.Num()<=0|| IterationsPerTick <= 0 || nullptr == WorldContextObject|| Delay < 0.0f)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoopTask: Someting problems, please check."), ELogVerbosity::Error);
		return;
	}

	TSubclassOf<USpawnInstanceTask> Class = LoadClass<USpawnInstanceTask>(nullptr, TEXT("Class'/Script/CBCommon.SpawnInstanceTask'"));

	if (nullptr == Class)
	{
		FFrame::KismetExecutionMessage(TEXT("DoLoopTask: Invalid Class. Cannot execute."), ELogVerbosity::Error);
		return;
	}

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		FLatentActionManager& LatentActionManager = World->GetLatentActionManager();
		FSpawnInstansTaskAction* Action = LatentActionManager.FindExistingAction<FSpawnInstansTaskAction>(LatentInfo.CallbackTarget, LatentInfo.UUID);

		if (Action && !Action->IsCanceled())
		{
			FFrame::KismetExecutionMessage(TEXT("DoLoopTask: This node is already running."), ELogVerbosity::Error);
			return;
		}
		else
		{
			Action = new FSpawnInstansTaskAction(WorldContextObject, Out, LatentInfo, Class, Index, HISM,Transforms,IterationsPerTick, Delay, Task);
			LatentActionManager.AddNewAction(LatentInfo.CallbackTarget, LatentInfo.UUID, Action);
		}
	}
}
