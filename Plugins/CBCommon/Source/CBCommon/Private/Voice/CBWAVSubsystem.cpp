// Fill out your copyright notice in the Description page of Project Settings.


#include "Voice/CBWAVSubsystem.h"
#include "Components/AudioComponent.h"
#include "CBCommon.h"

void UCBWAVSubsystem::SetAudioComponent(UAudioComponent* InAudioComponent)
{
	if(InAudioComponent)
	{
		AudioComponent=InAudioComponent;
		AudioComponent->OnAudioFinishedNative.AddUObject(this,&UCBWAVSubsystem::PlayAudioFinish);
	}

}

void UCBWAVSubsystem::InsertSoundWaveQueue( USoundWave* const SoundWave)
{
	if(SoundWave)
	{
		SoundWaveArray.Add(SoundWave);
		SoundWave->AddToRoot();
	}
}

void UCBWAVSubsystem::InitializeQueueExecution()
{
	if(IsValidAudioComponent() &&SoundWaveArray[0])
	{
		AudioComponent->SetSound(SoundWaveArray[0]);
		AudioComponent->Play();
		SoundWaveArray[0]->RemoveFromRoot();
		SoundWaveArray.RemoveAt(0);
	}
}

void UCBWAVSubsystem::StopSoundWaveQueueExecution()
{
	for(USoundWave* Wave:SoundWaveArray)
	{
		Wave->RemoveFromRoot();
	}
	
	SoundWaveArray.Empty();
	if(AudioComponent->IsPlaying())
	{
		AudioComponent->Stop();
	}
	
}


bool UCBWAVSubsystem::IsValidAudioComponent() 
{
	return AudioComponent->IsValidLowLevel();
}

void UCBWAVSubsystem::PlayAudioFinish(UAudioComponent* InAudioComponent)
{
	if(SoundWaveArray.IsValidIndex(0)&&SoundWaveArray[0])
	{
		InAudioComponent->SetSound(SoundWaveArray[0]);
		InAudioComponent->Play();
		SoundWaveArray[0]->RemoveFromRoot();
		SoundWaveArray.RemoveAt(0);
	}
	else
	{
		StopSoundWaveQueueExecution();
	}
}



