// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


class CBCOMMON_API FJsonConfigImport
{
public:
	FJsonConfigImport();
	FJsonConfigImport(const FString& InJSONData);
	FJsonConfigImport(TSharedPtr<FJsonObject> JsonObject);
	~FJsonConfigImport();
	
public:
	bool ReadContainerEntry(const TSharedRef<FJsonValue>& InParsedPropertyValue,const int32 InArrayEntryIndex, FProperty* InProperty, void* InPropertyData);
	bool ReadStruct(const TSharedRef<FJsonObject>& InParsedObject, UScriptStruct* InStruct, void* InStructData);
	bool ReadStructEntry(const TSharedRef<FJsonValue>& InParsedPropertyValue, const void* InRowData, FProperty* InProperty, void* InPropertyData);
	bool ReadValue(const TSharedPtr<FJsonValue>& InParsedPropertyValue, const FProperty* InProperty, void* InPropertyData);
	
	void ReadUObject(const UClass* Class, UObject* Instance);

private:

	TSharedPtr<FJsonObject> JsonObject;
};


class CBCOMMON_API FJsonConfigExporter
{
public:
	typedef TJsonWriter<> FConfigJsonWriter;

	FJsonConfigExporter(TSharedRef<FConfigJsonWriter> InJsonWriter);
	
	bool WriteStruct(const UScriptStruct* InStruct, const void* InStructData);
	bool WriteStructEntry(const void* InRowData, const FProperty* InProperty, const void* InPropertyData);
	bool WriteContainerEntry(const FProperty* InProperty, const void* InPropertyData, const FString* InIdentifier = nullptr);
	
	void WriteUObject(const UClass* Class, const UObject* Instance);
	
protected:
	TSharedRef<FConfigJsonWriter> JsonWriter;

private:
	TSharedPtr<FJsonObject> JsonObject;
};


