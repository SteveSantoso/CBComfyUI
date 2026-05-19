// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Generators/AudioGenerator.h"
#include "Subsystems/EngineSubsystem.h"
#include "CBAudioCaptureSubsystem.generated.h"

class UAudioCapture;
struct FAudioCaptureDeviceInfo;
/**
 * 
 */
UCLASS()
class CBCOMMON_API UCBAudioCaptureSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	
	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

public:
	void StartCapturingAudio();
	
	void StopCapturingAudio();
	
	bool IsCapturingAudio();

	bool GetAudioCaptureDeviceInfo(FAudioCaptureDeviceInfo& OutInfo);

	void AppendVoiceData(const TArray<float>& InVoiceData);
	
	bool GetVoiceData(TArray<float>& OutVoiceData);
	
	void ClearVoiceData();
	
	static void OnAudioGenerate(const float* InAudio, int32 NumSamples);

private:
	UPROPERTY()
	UAudioCapture* AudioCapture;

	FAudioGeneratorHandle AudioGeneratorHandle;

	FCriticalSection VoiceDataCriticalSection;
	TArray<float> VoiceData;
};
