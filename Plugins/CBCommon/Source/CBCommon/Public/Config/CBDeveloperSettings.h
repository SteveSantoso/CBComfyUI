// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CBDeveloperSettings.generated.h"

/**
 * 用于参数项目配置，若要保存为Json，请标记宏EditorConfig
 */
UCLASS(Abstract,DefaultConfig)
class CBCOMMON_API UCBDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UCBDeveloperSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	// UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;

public:
	//Called after the C++ constructor
	virtual void PostInitProperties() override;

protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty( struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	FString FilePath;
	
};
