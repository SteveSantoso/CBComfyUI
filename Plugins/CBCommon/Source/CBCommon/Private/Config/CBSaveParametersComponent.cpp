// Fill out your copyright notice in the Description page of Project Settings.


#include "Config/CBSaveParametersComponent.h"

#include "Config/CBJsonConfig.h"

UCBSaveParametersComponent::UCBSaveParametersComponent(const FObjectInitializer& ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

#if WITH_EDITOR
	FCoreUObjectDelegates::OnObjectPropertyChanged.AddUObject(this, &UCBSaveParametersComponent::OnActorEditChangeProperty);
#endif 
	if(GetOwner())
	{
		FilePath=FPaths::ProjectConfigDir() / "ActorJsonConfig"/GetOwner()->GetName()+".json";
	}
	
}

void UCBSaveParametersComponent::SaveParameters()
{
	AActor* Owner=GetOwner();
	if(!Owner)
	{
		return;
	}

	if(HasAnyFlags(RF_ClassDefaultObject | RF_Transient))
	{
		return;
	}

	if(GetOwner()->HasAnyFlags(RF_Transient | RF_ClassDefaultObject))
	{
		return;
	}
	
	FString JsonOutputStr;
		
	TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonOutputStr);
	JsonWriter->WriteObjectStart();
	JsonWriter->WriteObjectStart(Owner->GetName());
	FJsonConfigExporter(JsonWriter).WriteUObject(Owner->GetClass(),Owner);
	JsonWriter->WriteObjectEnd();

	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);

	for(UActorComponent* Component:Components)
	{
		bool bAnyWritten=false;
		for (TFieldIterator<FProperty> It(Component->GetClass()); It; ++It)
		{
			const FProperty* Property = *It;
		
			if (!Property->HasAnyPropertyFlags(CPF_Config ) ||
				Property->GetName()==TEXT("DefaultUpdateOverlapsMethodDuringLevelStreaming") ||
				Property->GetName()==TEXT("bReplicateUsingRegisteredSubObjectList") ||
				Property->GetName()==TEXT("bCanEverAffectNavigation"))
			{
				continue;
			}

			bAnyWritten = true;
			break;
		
		}

		if(bAnyWritten)
		{
			JsonWriter->WriteObjectStart(Component->GetName());
			FJsonConfigExporter(JsonWriter).WriteUObject(Component->GetClass(),Component);
			JsonWriter->WriteObjectEnd();
		}

	}
	
	JsonWriter->WriteObjectEnd();
	JsonWriter->Close();
		
	FFileHelper::SaveStringToFile(JsonOutputStr, *FilePath);
	
}

void UCBSaveParametersComponent::LoadParameters()
{
	AActor* Owner=GetOwner();
	if(!Owner)
	{
		return;
	}
	
	if(HasAnyFlags(RF_ClassDefaultObject | RF_Transient))
	{
		return;
	}

	if(Owner->HasAnyFlags(RF_Transient | RF_ClassDefaultObject))
	{
		return;
	}
	
	FString Contents;
	if(!FFileHelper::LoadFileToString(Contents, *FilePath))
	{
		return;
	}

	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<FJsonStringReader> JsonReader = FJsonStringReader::Create(FString(Contents));
	if (!FJsonSerializer::Deserialize(JsonReader.Get(), JsonObject))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON string"));
	}

	const TSharedPtr<FJsonObject>* ActorJsonObject;
	if(JsonObject->TryGetObjectField(GetOwner()->GetName(),ActorJsonObject))
	{
		TSharedPtr<FJsonConfigImport> JsonConfigImport =MakeShareable(new  FJsonConfigImport(*ActorJsonObject));
		JsonConfigImport->ReadUObject(GetOwner()->GetClass(),GetOwner());
	}

	TArray<UActorComponent*> Components;
	Owner->GetComponents(Components);

	for(UActorComponent* Component:Components)
	{
		const TSharedPtr<FJsonObject>* ComponentJsonObject;
		if(JsonObject->TryGetObjectField(Component->GetName(),ComponentJsonObject))
		{
			TSharedPtr<FJsonConfigImport> JsonConfigImport =MakeShareable(new  FJsonConfigImport(*ComponentJsonObject));
			JsonConfigImport->ReadUObject(Component->GetClass(),Component);
		}
	}
}

void UCBSaveParametersComponent::BeginPlay() 
{
	
	LoadParameters();
	
	Super::BeginPlay();
}


#if WITH_EDITOR
void UCBSaveParametersComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UCBSaveParametersComponent::OnActorEditChangeProperty(UObject* Actor, FPropertyChangedEvent& PropertyChangedEvent)
{
	if( PropertyChangedEvent.Property != nullptr)
	{
		SaveParameters();
	}
	
}
#endif


