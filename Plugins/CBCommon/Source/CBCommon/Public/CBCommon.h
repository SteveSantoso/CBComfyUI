// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
DECLARE_LOG_CATEGORY_EXTERN(LogCBJson, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCBTween, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(CBJsonConfig, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCBFile, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCBPlatform, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCBProcess, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogCBAudioCapture, Log, All);
class FCBCommonModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
