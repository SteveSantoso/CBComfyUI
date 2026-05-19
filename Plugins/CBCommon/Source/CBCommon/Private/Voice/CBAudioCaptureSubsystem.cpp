// Fill out your copyright notice in the Description page of Project Settings.


#include "Voice/CBAudioCaptureSubsystem.h"
#include "Generators/AudioGenerator.h"
#include "AudioCapture.h"
#include "CBCommon.h"

void UCBAudioCaptureSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogCBAudioCapture, Display, TEXT("%s: CBAudioCapture Engine Subsystem initialized."), *FString(__func__));
}

void UCBAudioCaptureSubsystem::Deinitialize()
{
	UE_LOG(LogCBAudioCapture, Display, TEXT("%s: CBAudioCapture Engine Subsystem deinitialized."), *FString(__func__));
	StopCapturingAudio();
	
	Super::Deinitialize();
}

void UCBAudioCaptureSubsystem::StartCapturingAudio()
{
	if (!AudioCapture)
	{
		StopCapturingAudio();
	}

	AudioCapture = UAudioCaptureFunctionLibrary::CreateAudioCapture();

	if(const UCBAudioCaptureSubsystem* const Subsystem = GEngine->GetEngineSubsystem<UCBAudioCaptureSubsystem>())
	{
		AudioGeneratorHandle = AudioCapture->AddGeneratorDelegate(&UCBAudioCaptureSubsystem::OnAudioGenerate);
	}
	

	if (AudioCapture)
	{
		ClearVoiceData();

		AudioCapture->StartCapturingAudio();
	}
}

void UCBAudioCaptureSubsystem::StopCapturingAudio()
{
	if (AudioCapture)
	{
		AudioCapture->StopCapturingAudio();
		AudioCapture->RemoveGeneratorDelegate(AudioGeneratorHandle);
		AudioCapture = nullptr;
	}

	ClearVoiceData();
}

bool UCBAudioCaptureSubsystem::IsCapturingAudio()
{
	if (AudioCapture)
	{
		return AudioCapture->IsCapturingAudio();
	}

	return false;
}

bool UCBAudioCaptureSubsystem::GetAudioCaptureDeviceInfo(FAudioCaptureDeviceInfo& OutInfo)
{
	if (AudioCapture)
	{
		return AudioCapture->GetAudioCaptureDeviceInfo(OutInfo);
	}

	return false;
}

void UCBAudioCaptureSubsystem::AppendVoiceData(const TArray<float>& InVoiceData)
{
	{
		FScopeLock VoiceDataScopeLock(&VoiceDataCriticalSection);
		VoiceData.Append(InVoiceData);
	}
}

bool UCBAudioCaptureSubsystem::GetVoiceData(TArray<float>& OutVoiceData)
{
	OutVoiceData.Empty();

	if (VoiceData.Num() > 1024)
	{
		OutVoiceData.Append(VoiceData.GetData(), 1024);
		
		{
			FScopeLock VoiceDataScopeLock(&VoiceDataCriticalSection);
			VoiceData.RemoveAt(0, 1024);
		}
		
		return true;
	}
	
	return false;
}

void UCBAudioCaptureSubsystem::ClearVoiceData()
{
	{
		FScopeLock VoiceDataScopeLock(&VoiceDataCriticalSection);
		VoiceData.Empty();
	}
}

void UCBAudioCaptureSubsystem::OnAudioGenerate(const float* InAudio, int32 NumSamples)
{
	static int32 IndexSend = 0;
	TArray<float> VoiceDataToApeend;
	int32 VoiceIndex = 0;

	if (NumSamples == 2048)
	{

		if (IndexSend == 0)
		{
			IndexSend++;
			for (int32 VoiceNum = 0; VoiceNum < 341; VoiceNum++)
			{
				VoiceDataToApeend.Add(InAudio[VoiceIndex]);
				VoiceIndex += 6;
			}

		}
		else if (IndexSend == 1)
		{
			IndexSend++;

			for (int32 VoiceNum = 341; VoiceNum < 682; VoiceNum++)
			{
				VoiceDataToApeend.Add(InAudio[VoiceIndex]);

				VoiceIndex += 6;
			}
		}
		else if (IndexSend == 2)
		{

			IndexSend = 0;

			for (int32 VoiceNum = 682; VoiceNum < 1024; VoiceNum++)
			{
				VoiceDataToApeend.Add(InAudio[VoiceIndex]);
				VoiceIndex += 6;
			}
		}
		

	}
	else if (NumSamples == 960)
	{
		for (int32 VoiceNum = 0; VoiceNum < 160; VoiceNum++)
		{
			VoiceDataToApeend.Add(InAudio[VoiceIndex]);
			VoiceIndex += 6;
		}
	}
	else if (NumSamples == 480)
	{
		for (int32 VoiceNum = 0; VoiceNum < 160; VoiceNum++)
		{
			VoiceDataToApeend.Add(InAudio[VoiceIndex]);
			VoiceIndex += 3;
		}
	}
	else if (NumSamples == 320)
	{
		for (int32 VoiceNum = 0; VoiceNum < 160; VoiceNum++)
		{
			VoiceDataToApeend.Add(InAudio[VoiceIndex]);
			VoiceIndex += 2;
		}

	}
	else if (NumSamples == 160)
	{
		for (int32 VoiceNum = 0; VoiceNum < 160; VoiceNum++)
		{
			VoiceDataToApeend.Add(InAudio[VoiceIndex]);
			VoiceIndex += 1;
		}

	}

	if( UCBAudioCaptureSubsystem*  Subsystem = GEngine->GetEngineSubsystem<UCBAudioCaptureSubsystem>())
	{
		Subsystem->AppendVoiceData(VoiceDataToApeend);
	}
	
}
