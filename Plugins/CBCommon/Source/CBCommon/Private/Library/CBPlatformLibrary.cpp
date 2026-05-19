// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/CBPlatformLibrary.h"

#include "CBCommon.h"
#include "Interfaces/IPluginManager.h"
#include "Kismet/GameplayStatics.h"


#if PLATFORM_ANDROID
#include <AndroidPermissionFunctionLibrary.h>
#endif

const bool UCBPlatformLibrary::CheckAndroidPermission(const FString& InPermission)
{
#if PLATFORM_ANDROID
	UE_LOG(LogCBPlatform, Display, TEXT("%s: Checking android permission: %s"), *FString(__func__), *InPermission);
	if (!UAndroidPermissionFunctionLibrary::CheckPermission(InPermission))
	{
		UAndroidPermissionFunctionLibrary::AcquirePermissions({ InPermission });
		return false;
	}

#else
	UE_LOG(LogCBPlatform, Error, TEXT("%s: Platform %s is not supported"), *FString(__func__), *UGameplayStatics::GetPlatformName());
#endif

	return true;
}

const bool UCBPlatformLibrary::IsContentModuleAvailable(const FString& ModuleName)
{
	const FString QualifiedParam = QualifyModulePath(ModuleName);

	bool bOutput = false;


	
	for (const FString& Module : GetAvailableContentModules())
	{
		if (QualifyModulePath(Module).Contains(QualifiedParam, ESearchCase::IgnoreCase))
		{
			bOutput = true;
			break;
		}
	}

	return bOutput;
}

const FString UCBPlatformLibrary::QualifyModulePath(const FString& ModuleName)
{
	FString Output = ModuleName;

	if (!Output.StartsWith("/"))
	{
		Output = "/" + Output;
	}
	if (!Output.EndsWith("/"))
	{
		Output += '/';
	}

	return Output;
}

const TArray<FString> UCBPlatformLibrary::GetAvailableContentModules()
{
	TArray<FString> Output{ "Game" };

	IPluginManager& PluginManager = IPluginManager::Get();
	const TArray<TSharedRef<IPlugin>> PluginsArray = PluginManager.GetEnabledPluginsWithContent();

	for (const TSharedRef<IPlugin>& Plugin : PluginsArray)
	{
		if (Plugin->GetLoadedFrom() != EPluginLoadedFrom::Project)
		{
			continue;
		}

		Output.Add(Plugin->GetName());
	}

	return Output;
}

FString UCBPlatformLibrary::GetMachineId()
{
	TArray<uint8> Address=FPlatformMisc::GetMacAddress();

	FString Result;
	for (TArray<uint8>::TConstIterator it(Address);it;++it)
	{
		Result += FString::Printf(TEXT("%02x"),*it);
	}
	return Result;
}

bool UCBPlatformLibrary::SetGameWindow(UObject* WorldContextObject, EWindowNotify WindowNotify)
{
	TSharedPtr<SWindow> GameWindow(GEngine->GameViewport->GetWindow());
	
	if(!GameWindow)
	{
		return false;
	}
	
	switch (WindowNotify)
	{
	case EWindowNotify::Minimize :
		GameWindow->Minimize();
		break;
	case EWindowNotify::MaximizeOrRevert :
		GameWindow->IsWindowMaximized() ? GameWindow->Restore() : GameWindow->Maximize(); 
		break;
		
	case EWindowNotify::Exit :
		UKismetSystemLibrary::QuitGame(WorldContextObject,nullptr,EQuitPreference::Quit,false);
		break;
	}

	return true;
}



bool UCBPlatformLibrary::SetWindowPosition(FIntPoint Position)
{
	TSharedPtr<SWindow> Window(GEngine->GameViewport->GetWindow());
	if (Window.IsValid())
	{
		Window->MoveWindowTo(Position);
		return true;
	}

	return false;
}

bool UCBPlatformLibrary::GetWindowPosition(FIntPoint& Position)
{
	TSharedPtr<SWindow> Window(GEngine->GameViewport->GetWindow());
	if (Window.IsValid())
	{
		FDeprecateSlateVector2D SlatePosition=Window->GetPositionInScreen();
		Position=FIntPoint(SlatePosition.X,SlatePosition.Y);
		return true;
	}

	return false;
}

bool UCBPlatformLibrary::IsWindowMinimized(bool& IsWindowMinimized)
{
	TSharedPtr<SWindow> Window(GEngine->GameViewport->GetWindow());
	if (Window.IsValid())
	{
		IsWindowMinimized= Window->IsWindowMinimized();
		return true;
	}

	return false;
}

bool UCBPlatformLibrary::IsWindowMaximized(bool& IsWindowMinimized)
{
	TSharedPtr<SWindow> Window(GEngine->GameViewport->GetWindow());
	if (Window.IsValid())
	{
		IsWindowMinimized = Window->IsWindowMinimized();
		return true;
	}

	return false;
}

bool UCBPlatformLibrary::SetWindowSize(FIntPoint Size)
{
	TSharedPtr<SWindow> Window(GEngine->GameViewport->GetWindow());
	if (Window.IsValid())
	{
		Window->Resize(Size);
		return true;
	}

	return false;
}

bool UCBPlatformLibrary::GetWindowSize(FIntPoint& Size)
{
	TSharedPtr<SWindow> Window(GEngine->GameViewport->GetWindow());
	if (Window.IsValid())
	{
		FDeprecateSlateVector2D SlatePosition=Window->GetSizeInScreen();
		Size=FIntPoint(SlatePosition.X,SlatePosition.Y);
		
		return true;
	}

	return false;
}

bool UCBPlatformLibrary::SetWindowTitle(FString Title)
{
	TSharedPtr<SWindow> Window(GEngine->GameViewport->GetWindow());
	if (Window.IsValid())
	{
		Window->SetTitle(FText::FromString(Title));
		return true;
	}

	return false;
}

bool UCBPlatformLibrary::GetWindowTitle(FString& Title)
{
	TSharedPtr<SWindow> Window(GEngine->GameViewport->GetWindow());
	if (Window.IsValid())
	{
		Title = Window->GetTitle().ToString();
		return true;
	}

	return false;
}
