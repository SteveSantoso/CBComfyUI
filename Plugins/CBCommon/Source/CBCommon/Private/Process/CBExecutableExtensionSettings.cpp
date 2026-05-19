// Copyright 2023 CB,  All Rights Reserved.


#include "Process/CBExecutableExtensionSettings.h"

#include "CBCommon.h"
#include "Process/CBProcessFunctionLibrary.h"

UCBExecutableExtensionSettings::UCBExecutableExtensionSettings(const FObjectInitializer& ObjectInitializer)
{

	bCreatProcess=false;
	
	FCBProcInfo ExecutableExtensionSetting;
	ExecutableExtensionSetting.bActive=true;
	ExecutableExtensionSetting.Path=TEXT("LocalExe://websocketServer");
	ExecutableExtensionSetting.ExeName=TEXT("websocket-server.exe");
	ExecutableExtensionSetting.CommandLineArguments=FString();
	ExecutableExtensionSetting.bWindowHidden=true;
	ExecutableExtensionSetting.bLogCatch=false;

	ProcessList.Add(ExecutableExtensionSetting);

}

FName UCBExecutableExtensionSettings::GetCategoryName() const
{
	return TEXT("Message");
}

UCBExecutableExtensionSettings* UCBExecutableExtensionSettings::Get()
{
	UCBExecutableExtensionSettings* Settings = GetMutableDefault<UCBExecutableExtensionSettings>();
	return Settings;
}

void UCBExecutableExtensionSettings::PostInitProperties()
{
	Super::PostInitProperties();

	
	for(FCBProcInfo ExecutableExtensionSetting:ProcessList)
	{
		if(ExecutableExtensionSetting.bActive)
		{
			FString OutProcessID;
			FString FileExe=ExecutableExtensionSetting.Path/ExecutableExtensionSetting.ExeName;
			bool bSuccessed=UCBProcessFunctionLibrary::LoadProc(FileExe
				,ExecutableExtensionSetting.CommandLineArguments
				,ExecutableExtensionSetting.bWindowHidden
				,ExecutableExtensionSetting.bLogCatch
				,OutProcessID);

			if(false)
			{
				UE_LOG(LogCBProcess,Log,TEXT("Application is running at %s"),*FileExe);
			}
			else
			{
				UE_LOG(LogCBProcess,Warning,TEXT("No valid resoure  file found at %s"),*FileExe);
			}
		}
	}
	

}

void UCBExecutableExtensionSettings::BeginDestroy()
{
	Super::BeginDestroy();
	
	UCBProcessFunctionLibrary::UnLoadProcList();
}


