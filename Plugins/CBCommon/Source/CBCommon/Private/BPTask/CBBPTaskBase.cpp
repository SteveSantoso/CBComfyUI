// Fill out your copyright notice in the Description page of Project Settings.


#include "BPTask/CBBPTaskBase.h"
#include "BPTask/MultiTaskBaseLibrary.h"
#include "Async/Async.h"

UCBBPTaskBase::UCBBPTaskBase()
	:bIsTickable(false)
	, bIsTickableInEditor(false)
	, bIsTickableWhenPaused(false)
	, bCanceled(false)
{

}

void UCBBPTaskBase::OnCancel_Implementation()
{
}

void UCBBPTaskBase::OnComplete_Implementation()
{
}

bool UCBBPTaskBase::Start()
{
	unimplemented();

	return false;
}

void UCBBPTaskBase::Cancel()
{
	if (IsRunning() && !IsCanceled())
	{
		bCanceled = true;

		UCBBPTaskBase* Worker = this;

		AsyncTask(ENamedThreads::GameThread, [Worker]
		{
			if (Worker != nullptr && Worker->IsValidLowLevel() && !IsValid(Worker))
			{
				Worker->OnCancel();
				if (Worker->OnCancelDelegate.IsBound())
				{
					Worker->OnCancelDelegate.Broadcast();
				}
			}

		});
	}
}

bool UCBBPTaskBase::IsRunning()
{
	unimplemented();
	return false;
}

bool UCBBPTaskBase::IsCanceled()
{
	return bCanceled;
}

void UCBBPTaskBase::Tick(float DeltaTime)
{
}

bool UCBBPTaskBase::IsTickable() const
{
	return bIsTickable;
}

bool UCBBPTaskBase::IsTickableInEditor() const
{
	return bIsTickableInEditor;
}

bool UCBBPTaskBase::IsTickableWhenPaused() const
{
	return bIsTickableWhenPaused;
}

UWorld* UCBBPTaskBase::GetTickableGameObjectWorld() const
{
	return GetWorld();
}

UWorld* UCBBPTaskBase::GetWorld() const
{
	if (HasAllFlags(RF_ClassDefaultObject))
	{
		return nullptr;
	}

	if (IsValid(GetOuter()))
	{
		return GetOuter()->GetWorld();
	}
	return nullptr;
}

TStatId UCBBPTaskBase::GetStatId() const
{
	return TStatId();
}

FSingleTaskActionBase::FSingleTaskActionBase(UObject* InObject, const FLatentActionInfo& LatentInfo, TSubclassOf<class UCBBPTaskBase> TaskClass)
	:Object(InObject)
	, ExecutionFunction(LatentInfo.ExecutionFunction)
	, OutputLink(LatentInfo.Linkage)
	, CallbackTarget(LatentInfo.CallbackTarget)
{
	UMultiTaskBaseLibrary::TaskIndex++;
	const FString Name = "CBMultiTask" + FString::FromInt(UMultiTaskBaseLibrary::TaskIndex);

	Task = NewObject<UCBBPTaskBase>(InObject, TaskClass, FName(*Name), RF_Transient);
	UMultiTaskBaseLibrary::AddToRoot(Task);

	if (UObject* CallbackObject = CallbackTarget.Get())
	{
		if (UFunction* Function = CallbackObject->FindFunction(ExecutionFunction))
		{
			checkf(CallbackObject && Function, TEXT("Something went horibly wrong."));

			int32 LocalOutputLink = OutputLink;
			Task->BodyFunction = [CallbackObject, Function, LocalOutputLink]
			{
				int32 FinalOutputLink = LocalOutputLink;
				if (CallbackObject && Function)
				{
					CallbackObject->ProcessEvent(Function, &(FinalOutputLink));
				}
			};
		}
	}
}


FSingleTaskActionBase::~FSingleTaskActionBase()
{
	if (Task != nullptr && Task->IsValidLowLevel() && !IsValid(Task))
	{
		Task->Cancel();
		UMultiTaskBaseLibrary::RemoveFromRoot(Task);
	}
}

bool FSingleTaskActionBase::IsRunning()
{
	if (Task != nullptr && Task->IsValidLowLevel() && !IsValid(Task))
	{
		return Task->IsRunning();
	}
	return false;
}

bool FSingleTaskActionBase::IsCanceled()
{
	if (Task != nullptr && Task->IsValidLowLevel() && !IsValid(Task))
	{
		return Task->IsCanceled();
	}
	return true;
}
