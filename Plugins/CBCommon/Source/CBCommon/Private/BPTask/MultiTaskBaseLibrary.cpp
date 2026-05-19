// Fill out your copyright notice in the Description page of Project Settings.


#include "BPTask/MultiTaskBaseLibrary.h"
#if WITH_EDITOR
#include "Editor.h"
#endif

int32 UMultiTaskBaseLibrary::TaskIndex = 0;
TArray<UObject*>  UMultiTaskBaseLibrary::RootObjects = {};
int32 UMultiTaskBaseLibrary::MutexIndex = 0;
int32 UMultiTaskBaseLibrary::ThreadPoolIndex = 0;

#if WITH_EDITOR
FDelegateHandle UMultiTaskBaseLibrary::EndPIEHandle = FDelegateHandle();
#else
FDelegateHandle UMultiTaskBaseLibrary::PreExitHandle = FDelegateHandle();
#endif

UMultiTaskBaseLibrary::UMultiTaskBaseLibrary()
{
#if WITH_EDITOR
	EndPIEHandle = FEditorDelegates::PrePIEEnded.AddStatic(&UMultiTaskBaseLibrary::OnEndPIE);
#else
	PreExitHandle = FCoreDelegates::OnPreExit.AddStatic(&UMultiTaskBaseLibrary::OnPreExit);
#endif
}

UMultiTaskBaseLibrary::~UMultiTaskBaseLibrary()
{
#if WITH_EDITOR
	FEditorDelegates::PrePIEEnded.Remove(EndPIEHandle);
#else
	FCoreDelegates::OnPreExit.Remove(PreExitHandle);
#endif
	TaskIndex = 0;
	MutexIndex = 0;
	ThreadPoolIndex = 0;
}

void UMultiTaskBaseLibrary::SetMaximumLoopIterations(int32 MaximumLoopIterations)
{
#if DO_BLUEPRINT_GUARD
	FBlueprintCoreDelegates::SetScriptMaximumLoopIterations(MaximumLoopIterations);
#endif
}

void UMultiTaskBaseLibrary::ResetRunaway()
{
#if DO_BLUEPRINT_GUARD
	FBlueprintContextTracker& BlueprintContextTracker = FBlueprintContextTracker::Get();
	BlueprintContextTracker.ResetRunaway();
#endif
}

UObject* UMultiTaskBaseLibrary::GetContextWorld(UObject* WorldContextObject)
{
	return GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull);
}

void UMultiTaskBaseLibrary::AddToRoot(UObject* Object)
{
	if (Object)
	{
		if (!Object->IsRooted())
		{
			if (!RootObjects.Contains(Object))
			{
				RootObjects.Add(Object);
				Object->AddToRoot();
			}
		}
	}
}

void UMultiTaskBaseLibrary::RemoveFromRoot(UObject* Object)
{
	if (Object)
	{
		if (Object->IsRooted())
		{
			if (RootObjects.Contains(Object))
			{
				RootObjects.Remove(Object);
				Object->RemoveFromRoot();
			}
		}
	}
}

void UMultiTaskBaseLibrary::Cancel(UCBBPTaskBase* Task)
{
	if (Task != nullptr && Task->IsValidLowLevel() && !Task->IsUnreachable())
	{
		Task->Cancel();
	}
}

bool UMultiTaskBaseLibrary::IsRunning(UCBBPTaskBase* Task)
{
	if (Task != nullptr && Task->IsValidLowLevel() && !Task->IsUnreachable())
	{
		return Task->IsRunning();
	}
	return false;
}

bool UMultiTaskBaseLibrary::IsCanceled(UCBBPTaskBase* Task)
{
	if (Task != nullptr && Task->IsValidLowLevel() && !Task->IsUnreachable())
	{
		return Task->IsCanceled();
	}
	return true;
}

void UMultiTaskBaseLibrary::OnEndPIE(const bool bIsSimulating)
{
	for (auto Object : RootObjects)
	{
		if (Object != nullptr && Object->IsValidLowLevel() && !Object->IsUnreachable())
		{
			if (Object->IsRooted())
			{
				if (RootObjects.Contains(Object))
				{
					Object->RemoveFromRoot();
				}
			}
		}
	}
	RootObjects.Empty();
}

void UMultiTaskBaseLibrary::OnPreExit()
{
	for (auto Object : RootObjects)
	{
		if (Object)
		{
			if (Object->IsRooted())
			{
				if (RootObjects.Contains(Object))
				{
					Object->RemoveFromRoot();
				}
			}
		}
	}
	RootObjects.Empty();
}
