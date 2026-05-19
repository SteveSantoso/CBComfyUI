// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CBBPTaskTypes.h"
#include "BPTask/MultiFrameLoopTask.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MultiFrameTaskLibrary.generated.h"

/**
 * 
 */
class UHierarchicalInstancedStaticMeshComponent;

UCLASS()
class CBCOMMON_API UMultiFrameTaskLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "CBTask|Multi-Frame", Meta = (DisplayName = "Do Loop Task", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DeterminesOutputType = "Class", DynamicOutputParam = "Task"))
	static void DoLoopTask(UObject* WorldContextObject, ECBTaskBranchesWithBody& Out, FLatentActionInfo LatentInfo, int32& Index, UMultiFrameLoopTask*& Task, int32 Size = 1, int32 IterationsPerTick = 1, float Delay = 0.0f);


	UFUNCTION(BlueprintCallable, Category = "CBTask|Multi-Frame", Meta = (DisplayName = "Do Spawn Instances Task", Latent, LatentInfo = "LatentInfo", ExpandEnumAsExecs = "Out", HidePin = "WorldContextObject", DefaultToSelf = "WorldContextObject", DeterminesOutputType = "Class", DynamicOutputParam = "Task"))
	static void DoSpawnInstancesTask(UObject* WorldContextObject, ECBTaskBranchesWithBody& Out, FLatentActionInfo LatentInfo, int32& Index, USpawnInstanceTask*& Task, UHierarchicalInstancedStaticMeshComponent* HISM, TArray<FTransform> Transforms, int32 IterationsPerTick = 1, float Delay = 0.0f);
};
