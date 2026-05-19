// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CBSaveWorldSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class CBCOMMON_API UCBSaveWorldSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/**加载所有参数*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	void LoadAllParameters(UObject* WorldContextObject);

	/**保存所有参数*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	void SaveAllParameters(UObject* WorldContextObject);
};
