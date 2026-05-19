// Fill out your copyright notice in the Description page of Project Settings.


#include "Config/CBDeveloperSettings.h"

#include "Config/CBJsonConfig.h"

UCBDeveloperSettings::UCBDeveloperSettings(const FObjectInitializer& ObjectInitializer)
{
	FilePath=FPaths::ProjectConfigDir() / "ProjectJsonConfig"/ GetSectionName().ToString()+".json";
}

FName UCBDeveloperSettings::GetCategoryName() const
{
	return TEXT("CBPlugins");
}

void UCBDeveloperSettings::PostInitProperties()
{
	Super::PostInitProperties();

	FString Contents;
	if(!FFileHelper::LoadFileToString(Contents, *FilePath))
	{
		return;
	}
	
	FJsonConfigImport JsonConfigImport =FJsonConfigImport(Contents);
	JsonConfigImport.ReadUObject(GetClass(),this);
}
#if WITH_EDITOR
void UCBDeveloperSettings::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	if(PropertyChangedEvent.Property!=nullptr)
	{

		FString JsonOutputStr;
		
	    TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> JsonWriter = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&JsonOutputStr);
		JsonWriter->WriteObjectStart();
		FJsonConfigExporter(JsonWriter).WriteUObject(GetClass(),this);
		JsonWriter->WriteObjectEnd();
		JsonWriter->Close();
		
		FFileHelper::SaveStringToFile(JsonOutputStr, *FilePath);
	
	}
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
