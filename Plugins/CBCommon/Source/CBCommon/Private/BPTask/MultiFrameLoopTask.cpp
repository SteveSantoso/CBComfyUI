// Fill out your copyright notice in the Description page of Project Settings.


#include "BPTask/MultiFrameLoopTask.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"

UMultiFrameLoopTask::UMultiFrameLoopTask()
{
	Size = 1;
	IterationsPerTick = 1;
	Delay = 0.0f;

	CurrentIndex = 0;
	bStarted = false;
	TimeRemaining = 0.0f;

	bIsTickable = true;
}

bool UMultiFrameLoopTask::Start()
{
	if (Size > 0 && IterationsPerTick >= 1 && Delay >= 0.0f)
	{
		TimeRemaining = 0.0f;
		bStarted = true;
		return true;
	}
	return false;
}

bool UMultiFrameLoopTask::IsRunning()
{
	if (!IsCanceled() && bStarted)
	{
		return CurrentIndex < Size;
	}
	return false;
}

void UMultiFrameLoopTask::Tick(float DeltaTime)
{
	if (bStarted && !IsCanceled())
	{
		TimeRemaining -= DeltaTime;
		if (TimeRemaining <= 0.0f)
		{
			TimeRemaining = Delay;
			for (int32 CurrentIt = 0; CurrentIndex < Size && CurrentIt < IterationsPerTick; CurrentIt++, CurrentIndex++)
			{
				if (IsCanceled())
				{
					break;
				}
				else {
					if (TaskDelegate.IsBound())
					{
						TaskDelegate.Broadcast();
					}
				}
			}
		}
	}
}

//void UMultiFrameLoopTask::TaskBody_Implementation(int32 X)
//{
//}

USpawnInstanceTask::USpawnInstanceTask()
{
	bIsTickable = true;
}



void USpawnInstanceTask::Tick(float DeltaTime)
{
	if (bStarted && !IsCanceled())
	{
		TimeRemaining -= DeltaTime;
		if (TimeRemaining <= 0.0f)
		{
			TimeRemaining = Delay;
			for (int32 CurrentIt = 0; CurrentIndex < Size && CurrentIt < IterationsPerTick; CurrentIt++, CurrentIndex++)
			{
				if (IsCanceled())
				{
					break;
				}
				else {

					const int32 CurrentX = CurrentIndex % Size;
					HISM->AddInstance(Transforms[CurrentX]);

					if (TaskDelegate.IsBound())
					{
						TaskDelegate.Broadcast();
					}
				}
			}
		}
	}
}
