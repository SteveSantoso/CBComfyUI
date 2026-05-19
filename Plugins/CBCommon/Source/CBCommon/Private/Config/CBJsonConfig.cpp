// Fill out your copyright notice in the Description page of Project Settings.


#include "Config/CBJsonConfig.h"

#include "JsonObjectConverter.h"

namespace
{
	const TCHAR* JSONTypeToString(const EJson InType)
	{
		switch(InType)
		{
		case EJson::None:
			return TEXT("None");
		case EJson::Null:
			return TEXT("Null");
		case EJson::String:
			return TEXT("String");
		case EJson::Number:
			return TEXT("Number");
		case EJson::Boolean:
			return TEXT("Boolean");
		case EJson::Array:
			return TEXT("Array");
		case EJson::Object:
			return TEXT("Object");
		default:
			return TEXT("Unknown");
		}
	}
	
	void WriteJSONObjectStartWithOptionalIdentifier(FJsonConfigExporter::FConfigJsonWriter& InJsonWriter, const FString* InIdentifier)
	{
		if (InIdentifier)
		{
			InJsonWriter.WriteObjectStart(*InIdentifier);
		}
		else
		{
			InJsonWriter.WriteObjectStart();
		}
	}

	template < typename ValueType>
	void WriteJSONValueWithOptionalIdentifier(FJsonConfigExporter::FConfigJsonWriter& InJsonWriter, const FString* InIdentifier,const ValueType InValue)
	{
		if (InIdentifier)
		{
			InJsonWriter.WriteValue(*InIdentifier, InValue);
		}
		else
		{
			InJsonWriter.WriteValue(InValue);
		}
	}
}

FJsonConfigImport::FJsonConfigImport()
{
	JsonObject=MakeShared<FJsonObject>();
}

FJsonConfigImport::FJsonConfigImport(const FString& InJSONData)
{
	TSharedRef<FJsonStringReader> JsonReader = FJsonStringReader::Create(FString(InJSONData));
	if (!FJsonSerializer::Deserialize(JsonReader.Get(), JsonObject))
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to deserialize JSON string"));
	}
}

FJsonConfigImport::FJsonConfigImport(TSharedPtr<FJsonObject> InJsonObject)
{
	JsonObject=InJsonObject;
}

FJsonConfigImport::~FJsonConfigImport()
{
}

bool FJsonConfigImport::ReadContainerEntry(const TSharedRef<FJsonValue>& InParsedPropertyValue,
                                           const int32 InArrayEntryIndex, FProperty* InProperty, void* InPropertyData)
{
	
	if (FEnumProperty* EnumProp = CastField<FEnumProperty>(InProperty))
	{
		FString EnumValue;
		if (InParsedPropertyValue->TryGetString(EnumValue))
		{
			FString Error = DataTableUtils::AssignStringToPropertyDirect(EnumValue, InProperty, (uint8*)InPropertyData);
			if (!Error.IsEmpty())
			{
				return false;
			}
		}
		else
		{
			int64 PropertyValue = 0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				return false;
			}

			EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(InPropertyData, PropertyValue);
		}
	}
	else if (FNumericProperty *NumProp = CastField<FNumericProperty>(InProperty))
	{
		FString EnumValue;
		if (NumProp->IsEnum() && InParsedPropertyValue->TryGetString(EnumValue))
		{
			FString Error = DataTableUtils::AssignStringToPropertyDirect(EnumValue, InProperty, (uint8*)InPropertyData);
			if (!Error.IsEmpty())
			{
				return false;
			}
		}
		else if(NumProp->IsInteger())
		{
			int64 PropertyValue = 0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				return false;
			}

			NumProp->SetIntPropertyValue(InPropertyData, PropertyValue);
		}
		else
		{
			double PropertyValue = 0.0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				return false;
			}

			NumProp->SetFloatingPointPropertyValue(InPropertyData, PropertyValue);
		}
	}
	else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(InProperty))
	{
		bool PropertyValue = false;
		if (!InParsedPropertyValue->TryGetBool(PropertyValue))
		{
			return false;
		}

		BoolProp->SetPropertyValue(InPropertyData, PropertyValue);
	}
	else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(InProperty))
	{
		// Cannot nest arrays
		return false;
	}
	else if (FSetProperty* SetProp = CastField<FSetProperty>(InProperty))
	{
		// Cannot nest sets
		return false;
	}
	else if (FMapProperty* MapProp = CastField<FMapProperty>(InProperty))
	{
		// Cannot nest maps
		return false;
	}
	else if (FStructProperty* StructProp = CastField<FStructProperty>(InProperty))
	{
		const TSharedPtr<FJsonObject>* PropertyValue = nullptr;
		if (InParsedPropertyValue->TryGetObject(PropertyValue))
		{
			return ReadStruct(PropertyValue->ToSharedRef(), StructProp->Struct,InPropertyData);
			
		}
		
		else
		{
			// If the JSON does not contain a JSON object for this struct, we try to use the backwards-compatible string deserialization, same as the "else" block below
			FString PropertyValueString;
			if (!InParsedPropertyValue->TryGetString(PropertyValueString))
			{
				return false;
			}

			const FString Error = DataTableUtils::AssignStringToPropertyDirect(PropertyValueString, InProperty, (uint8*)InPropertyData);
			if (Error.Len() > 0)
			{
				return false;
			}

			return true;
		}
	}
	else
	{
		FString PropertyValue;
		if (!InParsedPropertyValue->TryGetString(PropertyValue))
		{
			return false;
		}

		const FString Error = DataTableUtils::AssignStringToPropertyDirect(PropertyValue, InProperty, (uint8*)InPropertyData);
		if(Error.Len() > 0)
		{
			return false;
		}
	}

	return true;
}

bool FJsonConfigImport::ReadStruct(const TSharedRef<FJsonObject>& InParsedObject, UScriptStruct* InStruct,
	void* InStructData)
{
	TArray<FString> TempPropertyImportNames;
	for (TFieldIterator<FProperty> It(InStruct); It; ++It)
	{
		FProperty* BaseProp = *It;
		check(BaseProp);
		TSharedPtr<FJsonValue> ParsedPropertyValue;
		DataTableUtils::GetPropertyImportNames(BaseProp, TempPropertyImportNames);
		for (const FString& PropertyName : TempPropertyImportNames)
		{
			ParsedPropertyValue = InParsedObject->TryGetField(PropertyName);
			if (ParsedPropertyValue.IsValid())
			{
				break;
			}
		}

		if (!ParsedPropertyValue.IsValid())
		{
#if WITH_EDITOR
			// If the structure has specified the property as optional for import (gameplay code likely doing a custom fix-up or parse of that property),
			// then avoid warning about it
			static const FName DataTableImportOptionalMetadataKey(TEXT("DataTableImportOptional"));
			if (BaseProp->HasMetaData(DataTableImportOptionalMetadataKey))
			{
				continue;
			}
#endif // WITH_EDITOR
			
			continue;
		}

		if (BaseProp->ArrayDim == 1)
		{
			void* Data = BaseProp->ContainerPtrToValuePtr<void>(InStructData, 0);
			ReadStructEntry(ParsedPropertyValue.ToSharedRef(), InStructData, BaseProp, Data);
		}
		else
		{
			const TCHAR* const ParsedPropertyType = JSONTypeToString(ParsedPropertyValue->Type);

			const TArray< TSharedPtr<FJsonValue> >* PropertyValuesPtr;
			if (!ParsedPropertyValue->TryGetArray(PropertyValuesPtr))
			{
				return false;
			}

			if (BaseProp->ArrayDim != PropertyValuesPtr->Num())
			{
				return false;
			}

			for (int32 ArrayEntryIndex = 0; ArrayEntryIndex < BaseProp->ArrayDim; ++ArrayEntryIndex)
			{
				if (PropertyValuesPtr->IsValidIndex(ArrayEntryIndex))
				{
					void* Data = BaseProp->ContainerPtrToValuePtr<void>(InStructData, ArrayEntryIndex);
					const TSharedPtr<FJsonValue>& PropertyValueEntry = (*PropertyValuesPtr)[ArrayEntryIndex];
					ReadContainerEntry(PropertyValueEntry.ToSharedRef(), ArrayEntryIndex, BaseProp, Data);
				}
			}
		}
	}

	return true;
}

bool FJsonConfigImport::ReadStructEntry(const TSharedRef<FJsonValue>& InParsedPropertyValue,
	const void* InRowData, FProperty* InProperty, void* InPropertyData)
{

	if (FEnumProperty* EnumProp = CastField<FEnumProperty>(InProperty))
	{
		FString EnumValue;
		if (InParsedPropertyValue->TryGetString(EnumValue))
		{
			FString Error = DataTableUtils::AssignStringToProperty(EnumValue, InProperty, (uint8*)InRowData);
			if (!Error.IsEmpty())
			{
				return false;
			}
		}
		else
		{
			int64 PropertyValue = 0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				return false;
			}

			EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(InPropertyData, PropertyValue);
		}
	}
	else if (FNumericProperty *NumProp = CastField<FNumericProperty>(InProperty))
	{
		FString EnumValue;
		if (NumProp->IsEnum() && InParsedPropertyValue->TryGetString(EnumValue))
		{
			FString Error = DataTableUtils::AssignStringToProperty(EnumValue, InProperty, (uint8*)InRowData);
			if (!Error.IsEmpty())
			{
				return false;
			}
		}
		else if (NumProp->IsInteger())
		{
			int64 PropertyValue = 0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				return false;
			}

			NumProp->SetIntPropertyValue(InPropertyData, PropertyValue);
		}
		else
		{
			double PropertyValue = 0.0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				return false;
			}

			NumProp->SetFloatingPointPropertyValue(InPropertyData, PropertyValue);
		}
	}
	else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(InProperty))
	{
		bool PropertyValue = false;
		if (!InParsedPropertyValue->TryGetBool(PropertyValue))
		{
			return false;
		}

		BoolProp->SetPropertyValue(InPropertyData, PropertyValue);
	}
	else if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(InProperty))
	{
		const TArray< TSharedPtr<FJsonValue> >* PropertyValuesPtr;
		if (!InParsedPropertyValue->TryGetArray(PropertyValuesPtr))
		{
			return false;
		}

		FScriptArrayHelper ArrayHelper(ArrayProp, InPropertyData);
		ArrayHelper.EmptyValues();
		for (const TSharedPtr<FJsonValue>& PropertyValueEntry : *PropertyValuesPtr)
		{
			const int32 NewEntryIndex = ArrayHelper.AddValue();
			uint8* ArrayEntryData = ArrayHelper.GetRawPtr(NewEntryIndex);
			ReadContainerEntry(PropertyValueEntry.ToSharedRef(), NewEntryIndex, ArrayProp->Inner, ArrayEntryData);
		}
	}
	else if (FSetProperty* SetProp = CastField<FSetProperty>(InProperty))
	{
		const TArray< TSharedPtr<FJsonValue> >* PropertyValuesPtr;
		if (!InParsedPropertyValue->TryGetArray(PropertyValuesPtr))
		{
			return false;
		}

		FScriptSetHelper SetHelper(SetProp, InPropertyData);
		SetHelper.EmptyElements();
		for (const TSharedPtr<FJsonValue>& PropertyValueEntry : *PropertyValuesPtr)
		{
			const int32 NewEntryIndex = SetHelper.AddDefaultValue_Invalid_NeedsRehash();
			uint8* SetEntryData = SetHelper.GetElementPtr(NewEntryIndex);
			ReadContainerEntry(PropertyValueEntry.ToSharedRef(), NewEntryIndex, SetHelper.GetElementProperty(), SetEntryData);
		}
		SetHelper.Rehash();
	}
	else if (FMapProperty* MapProp = CastField<FMapProperty>(InProperty))
	{
		const TSharedPtr<FJsonObject>* PropertyValue;
		if (!InParsedPropertyValue->TryGetObject(PropertyValue))
		{
			return false;
		}

		FScriptMapHelper MapHelper(MapProp, InPropertyData);
		MapHelper.EmptyValues();
		for (const auto& PropertyValuePair : (*PropertyValue)->Values)
		{
			const int32 NewEntryIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
			uint8* MapKeyData = MapHelper.GetKeyPtr(NewEntryIndex);
			uint8* MapValueData = MapHelper.GetValuePtr(NewEntryIndex);

			// JSON object keys are always strings
			const FString KeyError = DataTableUtils::AssignStringToPropertyDirect(PropertyValuePair.Key, MapHelper.GetKeyProperty(), MapKeyData);
			if (KeyError.Len() > 0)
			{
				MapHelper.RemoveAt(NewEntryIndex);
				return false;
			}

			if (!ReadContainerEntry(PropertyValuePair.Value.ToSharedRef(), NewEntryIndex, MapHelper.GetValueProperty(), MapValueData))
			{
				MapHelper.RemoveAt(NewEntryIndex);
				return false;
			}
		}
		MapHelper.Rehash();
	}
	else if (FStructProperty* StructProp = CastField<FStructProperty>(InProperty))
	{
		const TSharedPtr<FJsonObject>* PropertyValue = nullptr;
		if (InParsedPropertyValue->TryGetObject(PropertyValue))
		{
			return ReadStruct(PropertyValue->ToSharedRef(), StructProp->Struct, InPropertyData);
		}
		else
		{
			// If the JSON does not contain a JSON object for this struct, we try to use the backwards-compatible string deserialization, same as the "else" block below
			FString PropertyValueString;
			if (!InParsedPropertyValue->TryGetString(PropertyValueString))
			{
				return false;
			}

			return true;
		}
	}
	else
	{
		FString PropertyValue;
		if (!InParsedPropertyValue->TryGetString(PropertyValue))
		{
			return false;
		}
		const FString Error = DataTableUtils::AssignStringToProperty(PropertyValue, InProperty, (uint8*)InRowData);
		if(Error.Len() > 0)
		{
			return false;
		}
	}

	return true;
}

bool FJsonConfigImport::ReadValue(const TSharedPtr<FJsonValue>& InParsedPropertyValue, const  FProperty* InProperty,
	void* InPropertyData)
{
	if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(InProperty))
	{
		int64 PropertyValue = 0;
		if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
		{
			return false;
		}
		EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(InPropertyData, PropertyValue);
	}
	else if (const FNumericProperty* NumProp = CastField<FNumericProperty>(InProperty))
	{
		if (NumProp->IsInteger())
		{
			int64 PropertyValue = 0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				return false;
			}
			NumProp->SetIntPropertyValue(InPropertyData, PropertyValue);
		}else
		{
			double PropertyValue = 0.0;
			if (!InParsedPropertyValue->TryGetNumber(PropertyValue))
			{
				return false;
			}

			NumProp->SetFloatingPointPropertyValue(InPropertyData, PropertyValue);
		}
	}else if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(InProperty))
	{
		bool PropertyValue = false;
		if (!InParsedPropertyValue->TryGetBool(PropertyValue))
		{
			return false;
		}

		BoolProp->SetPropertyValue(InPropertyData, PropertyValue);
	}else if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(InProperty))
	{
		const TArray< TSharedPtr<FJsonValue> >* PropertyValuesPtr;
		if (!InParsedPropertyValue->TryGetArray(PropertyValuesPtr))
		{
			return false;
		}

		FScriptArrayHelper ArrayHelper(ArrayProp, InPropertyData);
		ArrayHelper.EmptyValues();
		for (const TSharedPtr<FJsonValue>& PropertyValueEntry : *PropertyValuesPtr)
		{
			const int32 NewEntryIndex = ArrayHelper.AddValue();
			uint8* ArrayEntryData = ArrayHelper.GetRawPtr(NewEntryIndex);
			ReadContainerEntry(PropertyValueEntry.ToSharedRef(), NewEntryIndex, ArrayProp->Inner, ArrayEntryData);
		}
	}
	else if (const FSetProperty* SetProp = CastField<FSetProperty>(InProperty))
	{
		const TArray< TSharedPtr<FJsonValue> >* PropertyValuesPtr;
		if (!InParsedPropertyValue->TryGetArray(PropertyValuesPtr))
		{
			return false;
		}

		FScriptSetHelper SetHelper(SetProp, InPropertyData);
		SetHelper.EmptyElements();
		for (const TSharedPtr<FJsonValue>& PropertyValueEntry : *PropertyValuesPtr)
		{
			const int32 NewEntryIndex = SetHelper.AddDefaultValue_Invalid_NeedsRehash();
			uint8* SetEntryData = SetHelper.GetElementPtr(NewEntryIndex);
			ReadContainerEntry(PropertyValueEntry.ToSharedRef(), NewEntryIndex, SetHelper.GetElementProperty(), SetEntryData);
		}
		SetHelper.Rehash();
	}
	else if (const FMapProperty* MapProp = CastField<FMapProperty>(InProperty))
	{
		const TSharedPtr<FJsonObject>* PropertyValue;
		if (!InParsedPropertyValue->TryGetObject(PropertyValue))
		{
			return false;
		}

		FScriptMapHelper MapHelper(MapProp, InPropertyData);
		MapHelper.EmptyValues();
		for (const auto& PropertyValuePair : (*PropertyValue)->Values)
		{
			const int32 NewEntryIndex = MapHelper.AddDefaultValue_Invalid_NeedsRehash();
			uint8* MapKeyData = MapHelper.GetKeyPtr(NewEntryIndex);
			uint8* MapValueData = MapHelper.GetValuePtr(NewEntryIndex);

			// JSON object keys are always strings
			const FString KeyError = DataTableUtils::AssignStringToPropertyDirect(PropertyValuePair.Key, MapHelper.GetKeyProperty(), MapKeyData);
			if (KeyError.Len() > 0)
			{
				MapHelper.RemoveAt(NewEntryIndex);
				return false;
			}

			if (!ReadContainerEntry(PropertyValuePair.Value.ToSharedRef(), NewEntryIndex, MapHelper.GetValueProperty(), MapValueData))
			{
				MapHelper.RemoveAt(NewEntryIndex);
				return false;
			}
		}
		MapHelper.Rehash();
	}
	else if (const FStructProperty* StructProp = CastField<FStructProperty>(InProperty))
	{
		const TSharedPtr<FJsonObject>* PropertyValue = nullptr;
		if (InParsedPropertyValue->TryGetObject(PropertyValue))
		{
			return ReadStruct(PropertyValue->ToSharedRef(), StructProp->Struct, InPropertyData);
		}
		else
		{
			// If the JSON does not contain a JSON object for this struct, we try to use the backwards-compatible string deserialization, same as the "else" block below
			FString PropertyValueString;
			if (!InParsedPropertyValue->TryGetString(PropertyValueString))
			{
				return false;
			}

			return true;
		}
	}
	else if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(InProperty))
	{
		FString PathString;
		if (InParsedPropertyValue->TryGetString(PathString))
		{
			InProperty->ImportText_Direct(*PathString, InPropertyData, ObjectProperty->GetOwner<UObject>(), 0);
		}
		return false;
	}
	else
	{
		FString PropertyValue;
		if(!InParsedPropertyValue->TryGetString(PropertyValue))
		{
			return false;
		}
		
		FString Error = DataTableUtils::AssignStringToPropertyDirect(PropertyValue, InProperty, (uint8*)InPropertyData);
		if (!Error.IsEmpty())
		{
			return false;
		}

	}
	return true;
	
}


void FJsonConfigImport::ReadUObject( const UClass* Class, UObject* OutValue) 
{

	if(!JsonObject)
	{
		return;
	}
	
	for (TFieldIterator<FProperty> It(Class); It; ++It)
	{
		FProperty* Property = *It;
		
	
		if (!Property->HasAnyPropertyFlags(CPF_Config ) || Property->GetName()==TEXT("DefaultUpdateOverlapsMethodDuringLevelStreaming") ||Property->GetName()==TEXT("bReplicateUsingRegisteredSubObjectList"))
		{
			continue;
		}

		void* DataPtr = Property->ContainerPtrToValuePtr<void>(OutValue);
		
		 TSharedPtr<FJsonValue> Value = JsonObject->TryGetField(FJsonObjectConverter::StandardizeCase(Property->GetAuthoredName()));
		if (Value.IsValid())
		{
			 ReadValue(Value, Property, DataPtr);
		}
	}
}


FJsonConfigExporter::FJsonConfigExporter(TSharedRef<FConfigJsonWriter> InJsonWriter)
	:JsonWriter(InJsonWriter)
{
}

bool FJsonConfigExporter::WriteStruct(const UScriptStruct* InStruct, const void* InStructData)
{
	for (TFieldIterator<const FProperty> It(InStruct); It; ++It)
	{
		const FProperty* BaseProp = *It;
		check(BaseProp);

		const FString Identifier = BaseProp->GetAuthoredName();
 
		if (BaseProp->ArrayDim == 1)
		{
			const void* Data = BaseProp->ContainerPtrToValuePtr<void>(InStructData, 0);
			WriteStructEntry(InStructData, BaseProp, Data);
		}
		else
		{
			JsonWriter->WriteArrayStart(Identifier);

			for (int32 ArrayEntryIndex = 0; ArrayEntryIndex < BaseProp->ArrayDim; ++ArrayEntryIndex)
			{
				const void* Data = BaseProp->ContainerPtrToValuePtr<void>(InStructData, ArrayEntryIndex);
				WriteContainerEntry(BaseProp, Data);
			}

			JsonWriter->WriteArrayEnd();
		}
	}

	return true;
}

bool FJsonConfigExporter::WriteStructEntry(const void* InRowData, const FProperty* InProperty,
	const void* InPropertyData)
{
	const FString Identifier =  InProperty->GetAuthoredName();

	if (const FEnumProperty* EnumProp = CastField<const FEnumProperty>(InProperty))
	{
		const FString PropertyValue = DataTableUtils::GetPropertyValueAsString(EnumProp, (uint8*)InRowData, EDataTableExportFlags::UseJsonObjectsForStructs);
		JsonWriter->WriteValue(Identifier, PropertyValue);
	}
	else if (const FNumericProperty *NumProp = CastField<const FNumericProperty>(InProperty))
	{
		if (NumProp->IsEnum())
		{
			const FString PropertyValue = DataTableUtils::GetPropertyValueAsString(InProperty, (uint8*)InRowData,  EDataTableExportFlags::UseJsonObjectsForStructs);
			JsonWriter->WriteValue(Identifier, PropertyValue);
		}
		else if (NumProp->IsInteger())
		{
			const int64 PropertyValue = NumProp->GetSignedIntPropertyValue(InPropertyData);
			JsonWriter->WriteValue(Identifier, PropertyValue);
		}
		else if (NumProp->IsA(FFloatProperty::StaticClass()))
		{
			const float PropertyValue = (float)NumProp->GetFloatingPointPropertyValue(InPropertyData);
			JsonWriter->WriteValue(Identifier, PropertyValue);
		}
		else
		{
			const double PropertyValue = NumProp->GetFloatingPointPropertyValue(InPropertyData);
			JsonWriter->WriteValue(Identifier, PropertyValue);
		}
	}
	else if (const FBoolProperty* BoolProp = CastField<const FBoolProperty>(InProperty))
	{
		const bool PropertyValue = BoolProp->GetPropertyValue(InPropertyData);
		JsonWriter->WriteValue(Identifier, PropertyValue);
	}
	else if (const FArrayProperty* ArrayProp = CastField<const FArrayProperty>(InProperty))
	{
		JsonWriter->WriteArrayStart(Identifier);

		FScriptArrayHelper ArrayHelper(ArrayProp, InPropertyData);
		for (int32 ArrayEntryIndex = 0; ArrayEntryIndex < ArrayHelper.Num(); ++ArrayEntryIndex)
		{
			const uint8* ArrayEntryData = ArrayHelper.GetRawPtr(ArrayEntryIndex);
			WriteContainerEntry(ArrayProp->Inner, ArrayEntryData);
		}

		JsonWriter->WriteArrayEnd();
	}
	else if (const FSetProperty* SetProp = CastField<const FSetProperty>(InProperty))
	{
		JsonWriter->WriteArrayStart(Identifier);

		FScriptSetHelper SetHelper(SetProp, InPropertyData);
		for (int32 SetSparseIndex = 0; SetSparseIndex < SetHelper.GetMaxIndex(); ++SetSparseIndex)
		{
			if (SetHelper.IsValidIndex(SetSparseIndex))
			{
				const uint8* SetEntryData = SetHelper.GetElementPtr(SetSparseIndex);
				WriteContainerEntry(SetHelper.GetElementProperty(), SetEntryData);
			}
		}

		JsonWriter->WriteArrayEnd();
	}
	else if (const FMapProperty* MapProp = CastField<const FMapProperty>(InProperty))
	{
		JsonWriter->WriteObjectStart(Identifier);

		FScriptMapHelper MapHelper(MapProp, InPropertyData);
		for (int32 MapSparseIndex = 0; MapSparseIndex < MapHelper.GetMaxIndex(); ++MapSparseIndex)
		{
			if (MapHelper.IsValidIndex(MapSparseIndex))
			{
				const uint8* MapKeyData = MapHelper.GetKeyPtr(MapSparseIndex);
				const uint8* MapValueData = MapHelper.GetValuePtr(MapSparseIndex);

				// JSON object keys must always be strings
				const FString KeyValue = DataTableUtils::GetPropertyValueAsStringDirect(MapHelper.GetKeyProperty(), (uint8*)MapKeyData,  EDataTableExportFlags::UseJsonObjectsForStructs);
				WriteContainerEntry(MapHelper.GetValueProperty(), MapValueData, &KeyValue);
			}
		}

		JsonWriter->WriteObjectEnd();
	}
	else if (const FStructProperty* StructProp = CastField<const FStructProperty>(InProperty))
	{

		JsonWriter->WriteObjectStart(Identifier);
		WriteStruct(StructProp->Struct, InPropertyData);
		JsonWriter->WriteObjectEnd();

	}
	else if(const FNameProperty* NameProperty = CastField<FNameProperty>(InProperty))
	{
		FName* PropertyValue = (FName*) InPropertyData;
		JsonWriter->WriteValue(Identifier, *PropertyValue->ToString());
	}
	else if(const FTextProperty* TextProperty = CastField<FTextProperty>(InProperty))
	{
		const FText* PropertyValue =(FText*) InPropertyData;;
		JsonWriter->WriteValue(Identifier, *PropertyValue->ToString());
	}
	else if(const FStrProperty* StrProperty = CastField<FStrProperty>(InProperty))
	{
		const FString* PropertyValue =(FString*) InPropertyData;;
		JsonWriter->WriteValue(Identifier, *PropertyValue);
	}
	else if (const FBoolProperty* BoolProperty = CastField<FBoolProperty>(InProperty))
	{
		bool Value = BoolProperty->GetPropertyValue(InPropertyData);
		JsonWriter->WriteValue(Identifier, Value);
	}
	else if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(InProperty))
	{
		FString ObjectPath;
		ObjectProperty->ExportTextItem_Direct(ObjectPath, InPropertyData, nullptr, nullptr, PPF_None, nullptr);
		JsonWriter->WriteValue(Identifier, *ObjectPath);
	}
	else
	{
		const FString* PropertyValue =(FString*) InPropertyData;;
		JsonWriter->WriteValue(Identifier, *PropertyValue);
	}
	
	return true;
}

bool FJsonConfigExporter::WriteContainerEntry(const FProperty* InProperty, const void* InPropertyData,
	const FString* InIdentifier)
{
	if (const FEnumProperty* EnumProp = CastField<const FEnumProperty>(InProperty))
	{
		const FString PropertyValue = DataTableUtils::GetPropertyValueAsStringDirect(InProperty, (uint8*)InPropertyData, EDataTableExportFlags::UseJsonObjectsForStructs);
		WriteJSONValueWithOptionalIdentifier(*JsonWriter, InIdentifier, PropertyValue);
	}
	else if (const FNumericProperty *NumProp = CastField<const FNumericProperty>(InProperty))
	{
		if (NumProp->IsEnum())
		{
			const FString PropertyValue = DataTableUtils::GetPropertyValueAsStringDirect(InProperty, (uint8*)InPropertyData, EDataTableExportFlags::UseJsonObjectsForStructs);
			WriteJSONValueWithOptionalIdentifier(*JsonWriter, InIdentifier, PropertyValue);
		}
		else if (NumProp->IsInteger())
		{
			const int64 PropertyValue = NumProp->GetSignedIntPropertyValue(InPropertyData);
			WriteJSONValueWithOptionalIdentifier(*JsonWriter, InIdentifier, PropertyValue);
		}
		else
		{
			const double PropertyValue = NumProp->GetFloatingPointPropertyValue(InPropertyData);
			WriteJSONValueWithOptionalIdentifier(*JsonWriter, InIdentifier, PropertyValue);
		}
	}
	else if (const FBoolProperty* BoolProp = CastField<const FBoolProperty>(InProperty))
	{
		const bool PropertyValue = BoolProp->GetPropertyValue(InPropertyData);
		WriteJSONValueWithOptionalIdentifier(*JsonWriter, InIdentifier, PropertyValue);
	}
	else if (const FStructProperty* StructProp = CastField<const FStructProperty>(InProperty))
	{

		WriteJSONObjectStartWithOptionalIdentifier(*JsonWriter, InIdentifier);
		WriteStruct(StructProp->Struct, InPropertyData);
		JsonWriter->WriteObjectEnd();

	}
	else if (const FArrayProperty* ArrayProp = CastField<const FArrayProperty>(InProperty))
	{
		// Cannot nest arrays
		return false;
	}
	else if (const FSetProperty* SetProp = CastField<const FSetProperty>(InProperty))
	{
		// Cannot nest sets
		return false;
	}
	else if (const FMapProperty* MapProp = CastField<const FMapProperty>(InProperty))
	{
		// Cannot nest maps
		return false;
	}
	else
	{
		const FString PropertyValue = DataTableUtils::GetPropertyValueAsStringDirect(InProperty, (uint8*)InPropertyData, EDataTableExportFlags::UseJsonObjectsForStructs);
		WriteJSONValueWithOptionalIdentifier(*JsonWriter, InIdentifier, PropertyValue);
	}

	return true;
}



void FJsonConfigExporter::WriteUObject(const UClass* Class, const UObject* Instance)
{
	bool bAnyWritten = false;
	const UObject* Defaults = Class->GetDefaultObject();
	
	for (TFieldIterator<FProperty> It(Class); It; ++It)
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
		
		const void* ValuePtr = Property->ContainerPtrToValuePtr<void>(Instance);
		const void* PropertyDefaultPtr = Property->ContainerPtrToValuePtr<void>(Defaults);
		//const void* PropertyDefaultPtr = Property->ContainerPtrToValuePtr<void>(Defaults, 0);
		
		WriteStructEntry(PropertyDefaultPtr, Property, ValuePtr);

		
	}

	if(!bAnyWritten)
	{
		UE_LOG(LogTemp,Warning,TEXT("UObject type has no properties to serialize: %s"), *Class->GetName())
	}
}


