// Fill out your copyright notice in the Description page of Project Settings.


#include "Config/CBSaveWorldSubsystem.h"

#include "Config/CBSaveParametersComponent.h"
#include "Kismet/GameplayStatics.h"

void UCBSaveWorldSubsystem::LoadAllParameters(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
		for(AActor* It:Actors)
		{
			UCBSaveParametersComponent* SaveParametersComponent=It->FindComponentByClass<UCBSaveParametersComponent>();
			
			if(SaveParametersComponent)
			{
				SaveParametersComponent->LoadParameters();
			}
			
		}
	}
}

void UCBSaveWorldSubsystem::SaveAllParameters(UObject* WorldContextObject)
{
	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), Actors);
		for(AActor* It:Actors)
		{
			UCBSaveParametersComponent* SaveParametersComponent=It->FindComponentByClass<UCBSaveParametersComponent>();

			if(SaveParametersComponent)
			{
				SaveParametersComponent->SaveParameters();
			}
		
		}
	}
}
