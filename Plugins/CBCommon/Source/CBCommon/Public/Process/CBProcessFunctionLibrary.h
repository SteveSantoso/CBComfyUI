// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CBProcessFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class CBCOMMON_API UCBProcessFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()


public:
	static bool LoadProc(FString Path,FString CommandLineArguments,bool bHiddenWindow,bool bLogCatch,FString& OutProcessID);
	static bool UnLoadProc(FString ProcessID);
	static void UnLoadProcList();

private:
	static TMap<FString, TSharedPtr< class FMonitoredProcess>> CurrentProcRegistry;

	static void OutputProcLog(FString Log,FString ProcPath);
	
};
