// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CBPlatformLibrary.generated.h"

UENUM(BlueprintType)
enum class EWindowNotify : uint8
{
	Minimize ,
	MaximizeOrRevert,
	Exit
};


UCLASS()
class CBCOMMON_API UCBPlatformLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	
	/** 检查android平台是否已有权限，没有则添加 */
	UFUNCTION(BlueprintCallable, Category = "CBPlatform | Utils", meta = (DisplayName = "Check and Add Android Permission"))
	static const bool CheckAndroidPermission(const FString& InPermission);

	/** 检查内容模块是否可用 */
	UFUNCTION(BlueprintPure, Category = "CBPlatform | Utils")
	static const bool IsContentModuleAvailable(const FString& ModuleName);

	/** 将模块名称字符串限定为单个字符串（如 /ModulePath/）*/
	UFUNCTION(BlueprintPure, Category = "CBPlatform | Utils", meta = (DisplayName = "Qualify Module Path"))
	static const FString QualifyModulePath(const FString& ModuleName);

	/** 获取启用content的可用模块 */
	UFUNCTION(BlueprintPure, Category = "CBPlatform | Utils")
	static const TArray<FString> GetAvailableContentModules();


	/**获取电脑的物理地址*/
	UFUNCTION(BlueprintPure,Category = "CBPlatform | Utils")
	static FString GetMachineId();


	/**设置窗口模式*/
	UFUNCTION(BlueprintCallable,meta=(WorldContext="WorldContextObject"),Category="CBPlatform|WindowExtension")
	static bool SetGameWindow(UObject* WorldContextObject,EWindowNotify WindowNotify);


	/**设置窗口位置*/
	UFUNCTION(BlueprintCallable,Category="CBPlatform|WindowExtension")
	static bool SetWindowPosition(FIntPoint Position);

	/**获取窗口位置*/
	UFUNCTION(BlueprintCallable,Category="CBPlatform|WindowExtension")
	static bool GetWindowPosition(FIntPoint& Position);
	
	/**窗口是否最小化*/
	UFUNCTION(BlueprintCallable,Category="CBPlatform|WindowExtension")
	static bool IsWindowMinimized(bool& IsWindowMinimized);

	/**窗口是否最小化*/
	UFUNCTION(BlueprintCallable,Category="CBPlatform|WindowExtension")
	static bool IsWindowMaximized(bool& IsWindowMinimized);

	/**设置窗口大小*/
	UFUNCTION(BlueprintCallable,Category="CBPlatform|WindowExtension")
	static bool SetWindowSize(FIntPoint Size);

	/**获取窗口大小*/
	UFUNCTION(BlueprintCallable,Category="CBPlatform|WindowExtension")
	static bool GetWindowSize(FIntPoint& Size);
	
	/**设置窗口标题*/
	UFUNCTION(BlueprintCallable,Category="CBPlatform|WindowExtension")
	static bool SetWindowTitle(FString Title);

	/**设置窗口标题*/
	UFUNCTION(BlueprintCallable,Category="CBPlatform|WindowExtension")
	static bool GetWindowTitle(FString& Title);
};
