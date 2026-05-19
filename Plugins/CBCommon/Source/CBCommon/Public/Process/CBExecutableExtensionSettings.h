// Copyright 2023 CB,  All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Config/CBDeveloperSettings.h"
#include "CBExecutableExtensionSettings.generated.h"




/**创建进行所需参数*/
USTRUCT(BlueprintType)
struct FCBProcInfo
{
	GENERATED_BODY()

	/**是否自启动*/
	UPROPERTY(BlueprintReadOnly, Config,EditAnywhere, Category = "CBCmmon|Process")
	bool bActive;

	/**程序路径*/
	UPROPERTY(BlueprintReadOnly, Config,EditAnywhere, Category = "CBCmmon|Process")
	FString Path;

	/**程序名*/
	UPROPERTY(BlueprintReadOnly, Config,EditAnywhere, Category = "CBCmmon|Process")
	FString ExeName;

	/**程序启动参数*/
	UPROPERTY(BlueprintReadOnly, Config,EditAnywhere, Category = "CBCmmon|Process")
	FString CommandLineArguments;
	
	/**是否隐藏界面*/
	UPROPERTY(BlueprintReadOnly, Config,EditAnywhere, Category = "CBCmmon|Process")
	bool bWindowHidden;

	/**是否抓取信息*/
	UPROPERTY(BlueprintReadOnly, Config,EditAnywhere, Category = "CBCmmon|Process")
	bool bLogCatch;
	
	FCBProcInfo()
		:bActive(false)
		,Path(TEXT("LocalExe://"))
		,ExeName(FString())
		,CommandLineArguments(FString())
		,bWindowHidden(true)
		,bLogCatch(false)
	{
	}
};

/**
 *用于启动进程和第三方程序配置
 */
UCLASS(Config = "CB")
class CBCOMMON_API UCBExecutableExtensionSettings : public UCBDeveloperSettings
{
	GENERATED_BODY()
public:
	UCBExecutableExtensionSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// UDeveloperSettings Interface
	virtual FName GetCategoryName() const override;
public:
	UFUNCTION(BlueprintPure, Category = "CB|Process",DisplayName = "GetExecutableExtensionSettings")
	static UCBExecutableExtensionSettings* Get();

public:
	UPROPERTY(BlueprintReadOnly, Config,EditAnywhere, Category = "CB|Process")
	TArray<FCBProcInfo> ProcessList;

public:
	//Called after the C++ constructor
	virtual void PostInitProperties() override;
	virtual void BeginDestroy() override;

private:
	bool bCreatProcess;
};
