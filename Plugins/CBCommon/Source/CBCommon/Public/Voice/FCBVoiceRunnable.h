// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HAL/Runnable.h"

class UCBAudioCaptureSubsystem;
/**
 * 
 */

DECLARE_DELEGATE_OneParam(FOnAudioBufferResult, TArray<float>&);

class CBCOMMON_API FCBVoiceRunnable : public FRunnable
{
public:
	FCBVoiceRunnable(FString InThreadName);
	virtual ~FCBVoiceRunnable();

	
	virtual bool Init() override;
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;

	
public:
	FString ThreadName;
	FOnAudioBufferResult OnAudioBufferResult;
protected:

	FThreadSafeBool bIsRunning;
	
	// FCriticalSection CriticalSection;

	UCBAudioCaptureSubsystem* AudioCaptureSubsystem;
};
