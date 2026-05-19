// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "CBWAVSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class CBCOMMON_API UCBWAVSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CBCommon | Voice")
	void SetAudioComponent(class UAudioComponent* AudioComponent);
	
	UFUNCTION(BlueprintCallable, Category = "CBCommon | Voice")
	void InsertSoundWaveQueue( class USoundWave* const SoundWave) ;

	UFUNCTION(BlueprintCallable, Category = "CBCommon | Voice")
	void InitializeQueueExecution() ;

	UFUNCTION(BlueprintCallable, Category = "CBCommon | Voice")
	void StopSoundWaveQueueExecution();
	
private:
	bool IsValidAudioComponent();
	void PlayAudioFinish(UAudioComponent* InAudioComponent);
private:
	 TArray<USoundWave*> SoundWaveArray;
	
	 UAudioComponent* AudioComponent=nullptr;
};
