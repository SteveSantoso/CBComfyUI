// Fill out your copyright notice in the Description page of Project Settings.


#include "CBComfyUIManager.h"

#include "CBComfyUI.h"
#include "CBComfyUIClient.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Components/BillboardComponent.h"
#include "Kismet/GameplayStatics.h"

ACBComfyUIManager::ACBComfyUIManager(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{

	Status = EComfyUIState::Disconnected;
	
#if WITH_EDITORONLY_DATA

	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	RootComponent = SpriteComponent;
	if (!IsRunningCommandlet())
	{
		// Structure to hold one-time initialization
		struct FConstructorStatics
		{
			ConstructorHelpers::FObjectFinderOptional<UTexture2D> CBComfyUITextureObject;
			FName ID_CBComfyUI;
			FText NAME_CBComfyUI;
			FConstructorStatics()
				: CBComfyUITextureObject(TEXT("/CBComfyUI/UI/Icons/S_CBComfyUI"))
				, ID_CBComfyUI(TEXT("CBComfyUI"))
				, NAME_CBComfyUI(NSLOCTEXT("SpriteCategory", "CBComfyUI", "CBComfyUI"))
			{
			}
		};
		static FConstructorStatics ConstructorStatics;

		if (SpriteComponent)
		{
			SpriteComponent->Sprite = ConstructorStatics.CBComfyUITextureObject.Get();
			SpriteComponent->SpriteInfo.Category = ConstructorStatics.ID_CBComfyUI;
			SpriteComponent->SpriteInfo.DisplayName = ConstructorStatics.NAME_CBComfyUI;
		}
	}
#endif // WITH_EDITORONLY_DATA
	
	bAllowTickBeforeBeginPlay = true;
	bReplicates = false;
	SetReplicatingMovement(false);
	SetCanBeDamaged(false);
	bEnableAutoLODGeneration = false;

	// Initialize runtime optimization settings
	ConnectionTimeout = 30.0f;
	HttpTimeout = 60.0f;
	bVerboseLogging = false;
}

void ACBComfyUIManager::PostLoad()
{
	Super::PostLoad();
}

void ACBComfyUIManager::PostActorCreated()
{
	Super::PostActorCreated();
}

void ACBComfyUIManager::BeginDestroy()
{
	Super::BeginDestroy();

	// Clean up resources
	if (HttpClient.IsValid())
	{
		HttpClient->CloseWebSocket();
		HttpClient.Reset();
	}

	// Clear any pending callbacks
	OnComfyUIStateChanged.Clear();
	OnComfyUIMessage.Clear();
	OnComfyUIProgress.Clear();
	OnComfyUIOutputFile.Clear();
}

ACBComfyUIManager* ACBComfyUIManager::GetCBComfyUIManager(UObject* WorldContextObject)
{
	ACBComfyUIManager* Actor = nullptr;

	if (UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(World, ACBComfyUIManager::StaticClass(), Actors);
		int NbActors = Actors.Num();
		if (NbActors == 0)
		{
			UE_LOG(LogCBComfyUI, Error, TEXT("CBComfyUIManager actor not found. Please add one to your world to configure your geo referencing system."));
		}
		else if (NbActors > 1)
		{
			UE_LOG(LogCBComfyUI, Error, TEXT("Multiple CBComfyUIManager actors found. Only one actor should be used to configure your geo referencing system"));
		}
		else
		{
			Actor = Cast<ACBComfyUIManager>(Actors[0]);
		}
	}

	return Actor;
}

void ACBComfyUIManager::CloseWebSocket()
{
 if(HttpClient &&  HttpClient->IsConnected())
 {
 	HttpClient->CloseWebSocket();
 }
}


void ACBComfyUIManager::ConnectCommfyUI()
{
	if (!HttpClient.IsValid())
	{
		HttpClient = MakeUnique<FCBComfyUIClient>(GetBaseUrl());
	}
	
	// Set up reconnection timer
	if (!ReconnectionTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(ReconnectionTimerHandle, this, &ACBComfyUIManager::AttemptReconnection, 5.0f, true);
	}

	TWeakObjectPtr<ACBComfyUIManager> WeakThis(this);

	HttpClient->SetWebSocketStateChangedCallback([WeakThis](bool bConnected)
	  {
		if (!WeakThis.IsValid())
		{
		  return;
		}

		ACBComfyUIManager* This = WeakThis.Get();

		if (bConnected)
		{
			This->Status = EComfyUIState::Idle;
			This->OnComfyUIStateChanged.Broadcast(This->Status);
			// Clear reconnection timer on successful connection
			if (This->ReconnectionTimerHandle.IsValid())
			{
				This->GetWorld()->GetTimerManager().ClearTimer(This->ReconnectionTimerHandle);
			}
		}
		else
		{
		  This->Status = EComfyUIState::Disconnected;
		  This->OnComfyUIStateChanged.Broadcast(This->Status);
		}
	  });

	HttpClient->SetWebSocketMessageCallback([WeakThis](const TSharedPtr<FJsonObject>& Message)
	  {
		if (!WeakThis.IsValid())
		{
		  return;
		}

		ACBComfyUIManager* This = WeakThis.Get();

		This->HandleWebSocketMessage(Message);
	  });

	HttpClient->ConnectWebSocket();
}

bool ACBComfyUIManager::IsConnected() const
{
	if (!HttpClient.IsValid())
	{
		return false;
	}

	return HttpClient->IsConnected();
}

bool ACBComfyUIManager::QueueWorkflowFromString(const FString InWorkflow)
{
	if (!IsConnected())
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Not connected to ComfyUI"));
		return false;
	}

	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InWorkflow);
	TSharedPtr<FJsonObject> Workflow;

	if (!FJsonSerializer::Deserialize(Reader, Workflow))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Failed to deserialize workflow JSON"));
		return false;
	}

	return QueueWorkflowFromJsonObject(Workflow);

}

bool ACBComfyUIManager::QueueWorkflowFromFile(const FString FilePath)
{
	if (!FPaths::FileExists(FilePath))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Workflow JSON file does not exist: %s"), *FilePath);
		return false;
	}

	FString JsonString = "";
	if (!FFileHelper::LoadFileToString(JsonString, *FilePath))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Failed to load workflow JSON file: %s"), *FilePath);
		return false;
	}

	return QueueWorkflowFromString(JsonString);
}

bool ACBComfyUIManager::QueueWorkflowFromJsonObject(UCBJsonObject* JsonObject)
{
	return QueueWorkflowFromJsonObject(JsonObject->GetRootObject());
}

void ACBComfyUIManager::Interrupt() 
{
	if (!IsConnected())
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Not connected to ComfyUI"));
		return;
	}

	HttpClient->DoHttpPostRequest("interrupt", nullptr, [this](const TSharedPtr<FJsonObject>& Response, bool bWasSuccessful)
	  {
		if (!bWasSuccessful)
		{
		  UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to send interrupt request"));
		  return;
		}

		UE_LOG(LogCBComfyUI, Verbose, TEXT("Interrupt request successful"));
		
	  });

	Status = EComfyUIState::Idle;
	OnComfyUIStateChanged.Broadcast(Status);
}

void ACBComfyUIManager::ClearQueue() 
{
	if (!IsConnected())
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Not connected to ComfyUI"));
		return;
	}

	TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetBoolField("clear", true);

	HttpClient->DoHttpPostRequest("queue", Payload, [this](const TSharedPtr<FJsonObject>& Response, bool bWasSuccessful)
	  {
		if (!bWasSuccessful)
		{
		  UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to send clear request"));
		  return;
		}

		UE_LOG(LogCBComfyUI, Verbose, TEXT("Clear request successful"));
	  });
}

void ACBComfyUIManager::CancelJob()
{
	Interrupt();
	ClearQueue();
}

void ACBComfyUIManager::FreeComfyMemory(bool bUnloadModels)
{
	if (!IsConnected())
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Not connected to ComfyUI"));
		return;
	}

	TSharedPtr<FJsonObject> Payload = nullptr;

	if (bUnloadModels)
	{
		Payload = MakeShared<FJsonObject>();
		Payload->SetBoolField("free_memory", true);
		Payload->SetBoolField("unload_models", true);

		HttpClient->DoHttpPostRequest("free", Payload, [this](const TSharedPtr<FJsonObject>& Response, bool bWasSuccessful)
		  {
			if (!bWasSuccessful)
			{
			  UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to send cleanup request"));
			  return;
			}

			UE_LOG(LogCBComfyUI, Verbose, TEXT("Cleanup request successful"));
		  });
	}

	Payload = MakeShared<FJsonObject>();
	Payload->SetBoolField("clear", true);

	HttpClient->DoHttpPostRequest("history", Payload, [this](const TSharedPtr<FJsonObject>& Response, bool bWasSuccessful)
	  {
		if (!bWasSuccessful)
		{
		  UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to send history clear request"));
		  return;
		}

		UE_LOG(LogCBComfyUI, Verbose, TEXT("History clear request successful"));
	  });
}

UCBJsonObject* ACBComfyUIManager::SetNodeInputDoubleProperty(UCBJsonObject* JsonWrkflow, const FString& NodeName,
                                                             const FString& PropertyName, double Value)
{
	UCBJsonObject* OutJson=NewObject<UCBJsonObject>();
	TSharedPtr<FJsonObject> RootJson=JsonWrkflow->GetRootObject();
	SetNodeInputProperty(*RootJson,NodeName,PropertyName,Value);
	OutJson->SetRootObject(RootJson);
	return OutJson;
}

UCBJsonObject* ACBComfyUIManager::SetNodeInputIntProperty(UCBJsonObject* JsonWrkflow, const FString& NodeName,
	const FString& PropertyName, int Value)
{
	UCBJsonObject* OutJson=NewObject<UCBJsonObject>();
	TSharedPtr<FJsonObject> RootJson=JsonWrkflow->GetRootObject();
	SetNodeInputProperty(*RootJson,NodeName,PropertyName,Value);
	OutJson->SetRootObject(RootJson);
	return OutJson;
}

UCBJsonObject* ACBComfyUIManager::SetNodeInputStringProperty(UCBJsonObject* JsonWrkflow, const FString& NodeName,
	const FString& PropertyName, const FString& Value)
{
	UCBJsonObject* OutJson=NewObject<UCBJsonObject>();
	TSharedPtr<FJsonObject> RootJson=JsonWrkflow->GetRootObject();
	SetNodeInputProperty(*RootJson,NodeName,PropertyName,Value);
	OutJson->SetRootObject(RootJson);
	return OutJson;
}

bool ACBComfyUIManager::GetNodeInputDoubleProperty(UCBJsonObject* JsonWrkflow, const FString& NodeName,
	const FString& PropertyName, double& OutValue)
{
	TSharedPtr<FJsonObject> Payload =JsonWrkflow->GetRootObject();
	return GetNodeInputProperty(*Payload,NodeName,PropertyName,OutValue);
}

bool ACBComfyUIManager::GetNodeInputIntProperty(UCBJsonObject* JsonWrkflow, const FString& NodeName,
	const FString& PropertyName, int& OutValue)
{
	TSharedPtr<FJsonObject> Payload =JsonWrkflow->GetRootObject();
	return GetNodeInputProperty(*Payload,NodeName,PropertyName,OutValue);
}

bool ACBComfyUIManager::GetNodeInputStringProperty(UCBJsonObject* JsonWrkflow, const FString& NodeName,
	const FString& PropertyName, FString& OutValue)
{
	TSharedPtr<FJsonObject> Payload =JsonWrkflow->GetRootObject();
	return GetNodeInputProperty(*Payload,NodeName,PropertyName,OutValue);
}

bool ACBComfyUIManager::QueueWorkflowFromJsonObject(TSharedPtr<FJsonObject> JsonObject)
{
	TSharedPtr<FJsonObject> Payload = MakeShared<FJsonObject>();
	Payload->SetStringField("client_id", HttpClient->ClientId);
	Payload->SetObjectField("prompt", JsonObject);
	
	TWeakObjectPtr<ACBComfyUIManager> WeakThis(this);

	return HttpClient->DoHttpPostRequest("prompt", Payload, [WeakThis](const TSharedPtr<FJsonObject>& Response, bool bWasSuccessful)
	{
		if (!WeakThis.IsValid())
		{
			//todo
		  return;
		}
		
	});
}

void ACBComfyUIManager::SetNodeInputProperty(FJsonObject& Workflow, const FString& NodeName,
                                             const FString& PropertyName, double Value)
{
	const TSharedPtr<FJsonObject>* Node;
	if (!(&Workflow)->TryGetObjectField(NodeName, Node) || !Node->IsValid())
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("JsonObject not found:%s"),*NodeName);
		return;
	}
	
	const TSharedPtr<FJsonObject>* Inputs;
	if (!(*Node)->TryGetObjectField("inputs", Inputs) || !Inputs->IsValid())
	{
		return;
	}

	if ((*Inputs)->HasField(PropertyName))
	{
		(*Inputs)->SetNumberField(PropertyName, Value);
	}
}

void ACBComfyUIManager::SetNodeInputProperty(FJsonObject& Workflow, const FString& NodeName,
	const FString& PropertyName, int Value)
{
	const TSharedPtr<FJsonObject>* Node;
	if (!(&Workflow)->TryGetObjectField(NodeName, Node) || !Node->IsValid())
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("JsonObject not found:%s"),*NodeName);
		return;
	}
	
	const TSharedPtr<FJsonObject>* Inputs;
	if (!(*Node)->TryGetObjectField("inputs", Inputs) || !Inputs->IsValid())
	{
		return;
	}

	if ((*Inputs)->HasField(PropertyName))
	{
		(*Inputs)->SetNumberField(PropertyName, Value);
	}
}

void ACBComfyUIManager::SetNodeInputProperty(FJsonObject& Workflow, const FString& NodeName,
	const FString& PropertyName, const FString& Value)
{
	const TSharedPtr<FJsonObject>* Node;
	if (!(&Workflow)->TryGetObjectField(NodeName, Node) || !Node->IsValid())
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("JsonObject not found:%s"),*NodeName);
		return;
	}
	
	const TSharedPtr<FJsonObject>* Inputs;
	if (!(*Node)->TryGetObjectField("inputs", Inputs) || !Node->IsValid())
	{
		return;
	}

	if ((*Inputs)->HasField(PropertyName))
	{
		(*Inputs)->SetStringField(PropertyName, Value);
	}
}

bool ACBComfyUIManager::GetNodeInputProperty(FJsonObject& Workflow, const FString& NodeName,
	const FString& PropertyName, int& OutValue)
{
	const TSharedPtr<FJsonObject>* Node;
	if (!(&Workflow)->TryGetObjectField(NodeName, Node) || !Node->IsValid())
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("JsonObject not found:%s"),*NodeName);
		return false;
	}
	
	const TSharedPtr<FJsonObject>* Inputs;
	if (!(*Node)->TryGetObjectField("inputs", Inputs) || !Inputs->IsValid())
	{
		return false;
	}

	if (!(*Inputs)->HasField(PropertyName))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Property Name not found:%s"),*PropertyName);
		return false;

	}

	(*Inputs)->TryGetNumberField(PropertyName, OutValue);

	return true;
}

bool ACBComfyUIManager::GetNodeInputProperty(FJsonObject& Workflow, const FString& NodeName,
	const FString& PropertyName, FString& OutValue)
{
	const TSharedPtr<FJsonObject>* Node;
	if (!(&Workflow)->TryGetObjectField(NodeName, Node) || !Node->IsValid())
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("JsonObject not found:%s"),*NodeName);
		return false;
	}
	
	const TSharedPtr<FJsonObject>* Inputs;
	if (!(*Node)->TryGetObjectField("inputs", Inputs) || !Inputs->IsValid())
	{
		return false;
	}

	if (!(*Inputs)->HasField(PropertyName))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Property Name not found:%s"),*PropertyName);
		return false;

	}

	(*Inputs)->TryGetStringField(PropertyName, OutValue);

	return true;
}

bool ACBComfyUIManager::GetNodeInputProperty(FJsonObject& Workflow, const FString& NodeName,
	const FString& PropertyName, double& OutValue)
{
	const TSharedPtr<FJsonObject>* Node;
	if (!(&Workflow)->TryGetObjectField(NodeName, Node) || !Node->IsValid())
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("JsonObject not found:%s"),*NodeName);
		return false;
	}
	
	const TSharedPtr<FJsonObject>* Inputs;
	if (!(*Node)->TryGetObjectField("inputs", Inputs) || !Inputs->IsValid())
	{
		return false;
	}

	if (!(*Inputs)->HasField(PropertyName))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Property Name not found:%s"),*PropertyName);
		return false;

	}

	(*Inputs)->TryGetNumberField(PropertyName, OutValue);

	return true;
}


FString ACBComfyUIManager::GetBaseUrl() const
{
	
	FString BaseUrl = ComfyURL;

	if (!BaseUrl.StartsWith("http://") && !BaseUrl.StartsWith("https://"))
	{
		BaseUrl = "http://" + BaseUrl;
	}

	// strip trailing slash

	if (BaseUrl.EndsWith("/"))
	{
		BaseUrl = BaseUrl.LeftChop(1);
	}

	return BaseUrl;
}

void ACBComfyUIManager::HandleWebSocketMessage(const TSharedPtr<FJsonObject>& Message)
{
	
	FString MessageType;
	if (!Message->TryGetStringField("type", MessageType))
	{
		UE_LOG(LogCBComfyUI, Warning, TEXT("Websocket message missing type field"));
		return;
	}

	const TSharedPtr<FJsonObject>* MessageData;
	if (!Message->TryGetObjectField("data", MessageData))
	{
		UE_LOG(LogCBComfyUI, Warning, TEXT("Websocket message missing data field"));
		return;
	}
	
	if(MessageType == "crystools.monitor")
	{
		//检查硬件的使用率，后续调用
		return;
	}

	if (bVerboseLogging)
	{
		UCBJsonObject* Json=NewObject<UCBJsonObject>();
		Json->SetRootObject(Message);
		OnComfyUIMessage.Broadcast(Json);
	}
	else
	{
		// In non-verbose mode, only broadcast important messages
		if (MessageType == "execution_start" || MessageType == "execution_success" || MessageType == "execution_error")
		{
			UCBJsonObject* Json=NewObject<UCBJsonObject>();
			Json->SetRootObject(Message);
			OnComfyUIMessage.Broadcast(Json);
		}
	}
	
	if (MessageType == "execution_start")
	{
		Status = EComfyUIState::Start;
		OnComfyUIStateChanged.Broadcast(Status);

		if (!(*MessageData)->TryGetStringField("prompt_id", PromptID))
		{
			UE_LOG(LogCBComfyUI, Warning, TEXT("Websocket message missing prompt_id field"));
			Status = EComfyUIState::Error;
			return;
		}
		
	}
	else if (MessageType == "execution_cached")
	{
		FString CurrentPromptID;
		if (!(*MessageData)->TryGetStringField("prompt_id", CurrentPromptID))
		{
			UE_LOG(LogCBComfyUI, Warning, TEXT("Websocket message missing prompt_id field"));
			Status = EComfyUIState::Error;
			return;
		}

		if(!CurrentPromptID.Equals(PromptID))
		{
			UE_LOG(LogCBComfyUI, Warning, TEXT("current PromptID is not match！"));
			Status = EComfyUIState::Error;
			return;
		}
		
		Status = EComfyUIState::Processing;
		OnComfyUIStateChanged.Broadcast(Status);
	}
	else if (MessageType == "executing")
	{
		
		if (!(*MessageData)->TryGetNumberField("node", CurrentNode))
		{
			if (bVerboseLogging)
			{
				UE_LOG(LogCBComfyUI, Verbose, TEXT("Websocket message missing node field"));
			}
			return;
		}
		
	}
	else if(MessageType == "progress")
	{
		
		if (!(*MessageData)->TryGetNumberField("value", QueueProgressValue))
		{
			UE_LOG(LogCBComfyUI, Warning, TEXT("Websocket message missing value field"));
			return;
		}
		
		if (!(*MessageData)->TryGetNumberField("max", QueueProgressMax))
		{
			UE_LOG(LogCBComfyUI, Warning, TEXT("Websocket message missing max field"));
			return;
		}

		OnComfyUIProgress.Broadcast(QueueProgressValue,QueueProgressMax);
	}
	else if(MessageType == "executed")
	{
		const TSharedPtr<FJsonObject>* OutputData;
		if (!(*MessageData)->TryGetObjectField("output", OutputData))
		{
			UE_LOG(LogCBComfyUI, Warning, TEXT("Websocket message missing output field"));
			return;
		}

		// "output": {"images": [{"filename": "ComfyUI_00062_.png", "subfolder": "", "type": "output"}]}, "prompt_id": "30840158-3b72-4c3e-9112-7efea0a3a2a8"}
		const TArray<TSharedPtr<FJsonValue>>* Images;
		TArray<FString> FileNames;
		if (!(*OutputData)->TryGetArrayField("images", Images))
		{
			UE_LOG(LogCBComfyUI, Warning, TEXT("Websocket message missing images field"));
			return;
		}

		for (const TSharedPtr<FJsonValue>& Image : *Images)
		{
			const TSharedPtr<FJsonObject>* ImageObject;
			if (!Image->TryGetObject(ImageObject))
			{
				UE_LOG(LogCBComfyUI, Warning, TEXT("Websocket message missing image object"));
				return;
			}

			FString Filename;
			if (!(*ImageObject)->TryGetStringField("filename", Filename))
			{
				UE_LOG(LogCBComfyUI, Warning, TEXT("Websocket message missing filename field"));
				return;
			}

			FString Subfolder;
			if (!(*ImageObject)->TryGetStringField("subfolder", Subfolder))
			{
				UE_LOG(LogCBComfyUI, Warning, TEXT("Websocket message missing subfolder field"));
				return;
			}

			FString Type;
			if (!(*ImageObject)->TryGetStringField("type", Type))
			{
				UE_LOG(LogCBComfyUI, Warning, TEXT("Websocket message missing type field"));
				return;
			}

			if (Type != "output")
			{
				continue;
			}

			FileNames.Add(Filename);
		}

		OnComfyUIOutputFile.Broadcast(CurrentNode,FileNames);
	}
	else if(MessageType == "execution_success")
	{
		Status = EComfyUIState::Successed;
		OnComfyUIStateChanged.Broadcast(Status);

		Status = EComfyUIState::Idle;
		OnComfyUIStateChanged.Broadcast(Status);
	}
	else if(MessageType == "execution_error")
	{
		Status = EComfyUIState::Error;
		OnComfyUIStateChanged.Broadcast(Status);

		Status = EComfyUIState::Idle;
		OnComfyUIStateChanged.Broadcast(Status);
	}
}

bool ACBComfyUIManager::DownloadImage(const FString& FileName,
	TFunction<void(TArray<FColor>, int, int, bool)> Callback) 
{
	FString Url = "view?filename=" + FileName;

	return HttpClient->DoHttpGetRequestRaw(Url, [Callback](const TArray<uint8>& PngData, bool bWasSuccessful)
   {
	 TArray<FColor> Pixels;

	 if (!bWasSuccessful)
	 {
	   UE_LOG(LogCBComfyUI, Error, TEXT("Failed to download image"));
	   Callback(Pixels, 0, 0, false);
	   return;
	 }

	 // Create an image wrapper using the PNG format
	 IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));
	 TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::PNG);

	 // Set the compressed data for the image wrapper
	 if (!ImageWrapper.IsValid() || !ImageWrapper->SetCompressed(PngData.GetData(), PngData.Num()))
	 {
	   UE_LOG(LogCBComfyUI, Error, TEXT("Failed to decompress image"));
	   Callback(Pixels, 0, 0, false);
	   return;
	 }

	 // Decompress the image data
	 TArray<uint8> RawData;
	 if (!ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, RawData))
	 {
	   UE_LOG(LogCBComfyUI, Error, TEXT("Failed to decompress image"));
	   Callback(Pixels, 0, 0, false);
	   return;
	 }

	 int Width = ImageWrapper->GetWidth();
	 int Height = ImageWrapper->GetHeight();

	 Pixels.SetNumUninitialized(Width * Height);

	 // Copy the decompressed pixel data

	 const uint8* PixelData = RawData.GetData();

	 for (int32 PixelIndex = 0; PixelIndex < Pixels.Num(); ++PixelIndex)
	 {
	   int32 Index = PixelIndex * 4; // 4 bytes per pixel (BGRA)
	   FColor& Pixel = Pixels[PixelIndex];
	   Pixel.B = PixelData[Index];
	   Pixel.G = PixelData[Index + 1];
	   Pixel.R = PixelData[Index + 2];
	   Pixel.A = PixelData[Index + 3];
	 }

		Callback(Pixels, Width, Height, true);
   });
}

void ACBComfyUIManager::SetConnectionTimeout(float TimeoutSeconds)
{
	ConnectionTimeout = FMath::Max(TimeoutSeconds, 1.0f);
	if (bVerboseLogging)
	{
		UE_LOG(LogCBComfyUI, Log, TEXT("Connection timeout set to %f seconds"), ConnectionTimeout);
	}
}

void ACBComfyUIManager::SetHttpTimeout(float TimeoutSeconds)
{
	HttpTimeout = FMath::Max(TimeoutSeconds, 1.0f);
	if (bVerboseLogging)
	{
		UE_LOG(LogCBComfyUI, Log, TEXT("HTTP timeout set to %f seconds"), HttpTimeout);
	}
}

void ACBComfyUIManager::SetVerboseLogging(bool bEnable)
{
	bVerboseLogging = bEnable;
	if (bVerboseLogging)
	{
		UE_LOG(LogCBComfyUI, Log, TEXT("Verbose logging enabled"));
	}
	else
	{
		UE_LOG(LogCBComfyUI, Log, TEXT("Verbose logging disabled"));
	}
}

void ACBComfyUIManager::AttemptReconnection()
{
	if (IsConnected())
	{
		// Already connected, clear timer
		if (ReconnectionTimerHandle.IsValid())
		{
			GetWorld()->GetTimerManager().ClearTimer(ReconnectionTimerHandle);
		}
		return;
	}

	if (bVerboseLogging)
	{
		UE_LOG(LogCBComfyUI, Log, TEXT("Attempting to reconnect to ComfyUI..."));
	}

	// Attempt reconnection
	if (HttpClient.IsValid())
	{
		HttpClient->ConnectWebSocket();
	}
	else
	{
		ConnectCommfyUI();
	}
}

bool ACBComfyUIManager::IsRunningInEditor() const
{
#if WITH_EDITOR
	return GIsEditor;
#else
	return false;
#endif
}

void ACBComfyUIManager::ConfigureForRuntime(bool bEnableAutoReconnect, float ReconnectInterval)
{
	if (!IsRunningInEditor())
	{
		// Runtime模式下的优化配置
		SetVerboseLogging(false); // 减少日志输出
		SetConnectionTimeout(15.0f); // 较短的连接超时
		SetHttpTimeout(30.0f); // 较短的HTTP超时
		
		if (bEnableAutoReconnect)
		{
			// 设置自动重连
			if (ReconnectionTimerHandle.IsValid())
			{
				GetWorld()->GetTimerManager().ClearTimer(ReconnectionTimerHandle);
			}
			
			GetWorld()->GetTimerManager().SetTimer(ReconnectionTimerHandle, 
				this, &ACBComfyUIManager::AttemptReconnection, 
				FMath::Max(ReconnectInterval, 1.0f), true);
		}
		
		UE_LOG(LogCBComfyUI, Log, TEXT("CBComfyUIManager configured for runtime mode"));
	}
	else
	{
		UE_LOG(LogCBComfyUI, Log, TEXT("CBComfyUIManager is running in editor mode"));
	}
}