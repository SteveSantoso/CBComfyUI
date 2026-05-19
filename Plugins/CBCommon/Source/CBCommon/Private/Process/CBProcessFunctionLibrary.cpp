// Fill out your copyright notice in the Description page of Project Settings.


#include "Process/CBProcessFunctionLibrary.h"

#include "CBCommon.h"
#include "Kismet/KismetStringLibrary.h"
#include "Library/CBFileHeplerBPLibrary.h"
#include "Misc/MonitoredProcess.h"

TMap<FString, TSharedPtr<FMonitoredProcess>> UCBProcessFunctionLibrary::CurrentProcRegistry;

bool UCBProcessFunctionLibrary::LoadProc(FString Path, FString CommandLineArguments, bool bHiddenWindow,
                                         bool bLogCatch, FString& OutProcessID)
{
	const FString PathPrefix="LocalExe://";
	const FString PathPrefixLocalSaved="LocalSaved://";
	if((UKismetStringLibrary::StartsWith(Path,PathPrefix,ESearchCase::IgnoreCase)))
	{
		FString File=UKismetStringLibrary::RightChop(Path,PathPrefix.Len());
		Path=FPaths::Combine(UCBFileHeplerBPLibrary::GetExtrasExeDir(TEXT("CBCommon"))/File);
		FPaths::NormalizeFilename(Path);
	}else if((UKismetStringLibrary::StartsWith(Path,PathPrefixLocalSaved,ESearchCase::IgnoreCase)))
	{
		FString File=UKismetStringLibrary::RightChop(Path,PathPrefixLocalSaved.Len());
		Path=FPaths::Combine(FPaths::ProjectSavedDir()/File);
		FPaths::NormalizeFilename(Path);
	}
	FString ProcItWorkingDir = FPaths::GetPath(Path);
	
	
	TSharedPtr<FMonitoredProcess> CurrentProc = MakeShareable(new FMonitoredProcess(Path, CommandLineArguments, ProcItWorkingDir, bHiddenWindow, bLogCatch));
	FString CurrentProcLaunchInfo = FString::Printf(TEXT("ProcPath:%s Parms:%s WorkdingDir:%s"), *Path, *CommandLineArguments, *ProcItWorkingDir);
	UE_LOG(LogCBProcess, Verbose, TEXT("Launching: %s"), *CurrentProcLaunchInfo);

	if (bLogCatch)
	{
		CurrentProc->OnOutput().BindStatic(&UCBProcessFunctionLibrary::OutputProcLog, Path);
	}
	
	FPlatformProcess::Sleep(0.1f);

	if (!CurrentProc.IsValid() || !CurrentProc->Launch())
	{
		FString ErrorMsg = FString::Printf(TEXT("Failed To Launch: %s "), *CurrentProcLaunchInfo);
		UE_LOG(LogCBProcess, Error, TEXT("%s"), *ErrorMsg);
		return false;
	}

	OutProcessID = FGuid::NewGuid().ToString();

	CurrentProcRegistry.Add(OutProcessID, CurrentProc);
	return true;
}

bool UCBProcessFunctionLibrary::UnLoadProc(FString ProcessID)
{
	if (ProcessID.IsEmpty())
	{
		return false;
	}

	TSharedPtr<FMonitoredProcess> TargetProc = *CurrentProcRegistry.Find(ProcessID);
	if (!TargetProc.IsValid() || !TargetProc->Update())
	{
		return false;
	}

	TargetProc->Cancel(true);
	CurrentProcRegistry.Remove(ProcessID);
	return true;
}

void UCBProcessFunctionLibrary::UnLoadProcList()
{
	TArray<TSharedPtr<FMonitoredProcess>> CurrentProcList;
	CurrentProcRegistry.GenerateValueArray(CurrentProcList);
	for (const TSharedPtr<FMonitoredProcess>& ProcIt : CurrentProcList)
	{
		if (ProcIt.IsValid() && ProcIt->Update())
		{
			ProcIt->Cancel(true);
		}
	}

	CurrentProcList.Empty();
	CurrentProcRegistry.Empty();
}

void UCBProcessFunctionLibrary::OutputProcLog(FString Log, FString ProcPath)
{
	FString LogMsg = FString::Printf(TEXT("%s: %s"), *FPaths::GetCleanFilename(ProcPath), *Log);
	UE_LOG(LogCBProcess, Log, TEXT("%s"), *LogMsg);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, FString::Printf(TEXT("CBProcess:Screen: %s"), *LogMsg));
	}
}