// Fill out your copyright notice in the Description page of Project Settings.


#include "Voice/FCBVoiceRunnable.h"

#include "Voice/CBAudioCaptureSubsystem.h"


FCBVoiceRunnable::FCBVoiceRunnable(FString InThreadName)
	:ThreadName(InThreadName)
{
}

FCBVoiceRunnable::~FCBVoiceRunnable()
{
}

bool FCBVoiceRunnable::Init()
{
	if(UCBAudioCaptureSubsystem* Subsystem = GEngine->GetEngineSubsystem<UCBAudioCaptureSubsystem>())
	{
		AudioCaptureSubsystem = Subsystem;
		return true;
	}

	return false;
}

uint32 FCBVoiceRunnable::Run()
{
	bIsRunning = true;
	
	while (true)
	{
		FPlatformProcess::Sleep(0.04);

		TArray<float> VoiceData;


		bool bRightVoice = AudioCaptureSubsystem->GetVoiceData(VoiceData);


		// FScopeLock RunnableLcoak(&CriticalSection);

		if (!bIsRunning)
		{
			break;
		}

		if (bRightVoice)
		{
			OnAudioBufferResult.ExecuteIfBound(VoiceData);
		}


	}
	return 0;
}

void FCBVoiceRunnable::Stop()
{
	bIsRunning = false;
	AudioCaptureSubsystem->ClearVoiceData();
}

void FCBVoiceRunnable::Exit()
{
	AudioCaptureSubsystem->ClearVoiceData();
}
