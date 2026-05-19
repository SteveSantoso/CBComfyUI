// Copyright Epic Games, Inc. All Rights Reserved.

#include "CBCommon.h"

DEFINE_LOG_CATEGORY(LogCBJson);
DEFINE_LOG_CATEGORY(LogCBTween);
DEFINE_LOG_CATEGORY(CBJsonConfig);
DEFINE_LOG_CATEGORY(LogCBFile);
DEFINE_LOG_CATEGORY(LogCBProcess);
DEFINE_LOG_CATEGORY(LogCBPlatform);
DEFINE_LOG_CATEGORY(LogCBAudioCapture);

#define LOCTEXT_NAMESPACE "FCBCommonModule"

void FCBCommonModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FCBCommonModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCBCommonModule, CBCommon)