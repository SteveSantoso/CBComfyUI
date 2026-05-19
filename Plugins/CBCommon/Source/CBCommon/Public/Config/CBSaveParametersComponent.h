// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CBSaveParametersComponent.generated.h"


UCLASS( ClassGroup=(CBConfig), meta=(BlueprintSpawnableComponent) )
class CBCOMMON_API UCBSaveParametersComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCBSaveParametersComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
public:
    void SaveParameters();
	void LoadParameters();
public:
	
	//~ Begin UObject Interface.
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void OnActorEditChangeProperty(UObject* Actor, struct FPropertyChangedEvent& PropertyChangedEvent);
#endif
	//~ End UObject Interface.

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
private:
	//用于保存json路径
	FString FilePath;
		
};
