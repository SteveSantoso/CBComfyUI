// Fill out your copyright notice in the Description page of Project Settings.


#include "CBComfyUIClient.h"

#include "CBComfyUI.h"
#include "HttpModule.h"
#include "WebSocketsModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"


FCBComfyUIClient::FCBComfyUIClient(const FString& URL)
	:ClientId(FGuid::NewGuid().ToString())
	,BaseUrl(URL)
{
	if (!FModuleManager::Get().IsModuleLoaded("WebSockets"))
	{
		FModuleManager::Get().LoadModule("WebSockets");
		UE_LOG(LogCBComfyUI, Log, TEXT("Loaded WebSockets module"));
	}
}

void FCBComfyUIClient::ConnectWebSocket()
{
	if (WebSocket.IsValid())
	{
		if (WebSocket->IsConnected())
		{
			UE_LOG(LogCBComfyUI, Log, TEXT("Closing existing connection"));
			WebSocket->Close();
		}
	}

	FString WsUrl = BaseUrl;
	FString Proto = "ws://";

	if (WsUrl.StartsWith("http://"))
	{
		WsUrl = WsUrl.RightChop(7);
	}
	else if (WsUrl.StartsWith("https://"))
	{
		WsUrl = WsUrl.RightChop(8);
		Proto = "wss://";
	}

	WsUrl = Proto + WsUrl + "/ws?clientId=" + ClientId;
  
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(WsUrl, TEXT("ws"));

	auto OnStateChanged = OnWebSocketStateChanged;

	WebSocket->OnConnected().AddLambda([OnStateChanged]()
	  {
		UE_LOG(LogCBComfyUI, Log, TEXT("Connected to ComfyUI"));

		if (OnStateChanged)
		{
		  OnStateChanged(true);
		}
	  });

	auto OnMessage = OnWebSocketMessage;

	WebSocket->OnMessage().AddLambda([OnMessage](const FString& Message)
	  {
		TSharedPtr<FJsonObject> JsonObj;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Message);

		if (FJsonSerializer::Deserialize(Reader, JsonObj))
		{
		  if (OnMessage)
		  {
			OnMessage(JsonObj);
		  }
		}
		else
		{
		  UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to deserialize JSON message from ComfyUI: %s"), *Message);
		}
	  });

	WebSocket->OnConnectionError().AddLambda([OnStateChanged](const FString& Error)
	  {
		UE_LOG(LogCBComfyUI, Warning, TEXT("Error connecting to ComfyUI: %s"), *Error);

		if (OnStateChanged)
		{
		  OnStateChanged(false);
		}
	  });

	WebSocket->OnClosed().AddLambda([OnStateChanged](int32 StatusCode, const FString& Reason, bool bWasClean)
	  {
		UE_LOG(LogCBComfyUI, Log, TEXT("Connection to ComfyUI closed: %s"), *Reason);

		if (OnStateChanged)
		{
			OnStateChanged(false);
		}
	})

	;

	UE_LOG(LogCBComfyUI, Log, TEXT("Connecting to ComfyUI at %s"), *WsUrl);
	WebSocket->Connect();
}

bool FCBComfyUIClient::IsConnected() const
{
	return WebSocket.IsValid() && WebSocket->IsConnected();
}

void FCBComfyUIClient::CloseWebSocket()
{
	if(IsConnected())
	{
		WebSocket->Close();
	}
}
void FCBComfyUIClient::SetWebSocketStateChangedCallback(TFunction<void(bool)> Callback)
{
	OnWebSocketStateChanged = Callback;
}

void FCBComfyUIClient::SetWebSocketMessageCallback(TFunction<void(const TSharedPtr<FJsonObject>&)> Callback)
{
	OnWebSocketMessage = Callback;
}

bool FCBComfyUIClient::DoHttpGetRequest(const FString& Url,
	TFunction<void(const TSharedPtr<FJsonObject>&, bool)> Callback) const
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("GET");
	HttpRequest->SetURL(BaseUrl + "/" + Url);
	HttpRequest->SetTimeout(30.0f); // Default timeout, can be made configurable
	HttpRequest->OnProcessRequestComplete()
	  .BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
		  if (!bWasSuccessful || !Response.IsValid())
		  {
			UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to receive valid response"));
			Callback(nullptr, false);
			return;
		  }

		  FString ResponseString = Response->GetContentAsString();
		  TSharedPtr<FJsonObject> ResponseJson;

		  if (Response->GetContentType().StartsWith("application/json"))
		  {
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

			if (!FJsonSerializer::Deserialize(Reader, ResponseJson))
			{
			  UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to deserialize response JSON: %s"), *ResponseString);
			  Callback(nullptr, false);
			  return;
			}
		  }
		  else
		  {
			ResponseJson = MakeShareable(new FJsonObject());
			ResponseJson->SetStringField("response", ResponseString);
		  }

		  Callback(MoveTemp(ResponseJson), true);
		});

	return HttpRequest->ProcessRequest();	
}bool FCBComfyUIClient::DoHttpGetRequestRaw(const FString& Url,
	TFunction<void(const TArray<uint8>&, bool)> Callback) const
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("GET");
	HttpRequest->SetURL(BaseUrl + "/" + Url);
	HttpRequest->SetTimeout(30.0f); // Default timeout
	HttpRequest->OnProcessRequestComplete()
	  .BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
		  if (!bWasSuccessful || !Response.IsValid())
		  {
			UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to receive valid response"));
			Callback(TArray<uint8>(), false);
			return;
		  }

		  Callback(Response->GetContent(), true);
		});

	return HttpRequest->ProcessRequest();
}

bool FCBComfyUIClient::DoHttpPostRequest(const FString& Url, const TSharedPtr<FJsonObject>& Payload,
	TFunction<void(const TSharedPtr<FJsonObject>&, bool)> Callback) const
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetVerb("POST");
	HttpRequest->SetURL(BaseUrl + "/" + Url);
	HttpRequest->SetHeader("Content-Type", "application/json");
	HttpRequest->SetTimeout(60.0f); // Default timeout for POST requests

	if (Payload.IsValid())
	{
		FString RequestBody;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
		FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);
		HttpRequest->SetContentAsString(RequestBody);
	}

	HttpRequest->OnProcessRequestComplete()
	  .BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
		{
		  if (!bWasSuccessful || !Response.IsValid())
		  {
			UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to receive valid response"));
			Callback(nullptr, false);
			return;
		  }

		  FString ResponseString = Response->GetContentAsString();
		  TSharedPtr<FJsonObject> ResponseJson;

		  if (Response->GetContentType().StartsWith("application/json"))
		  {
			TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

			if (!FJsonSerializer::Deserialize(Reader, ResponseJson))
			{
			  UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to deserialize response JSON：%s"), *ResponseString);
			  Callback(nullptr, false);
			  return;
			}
		  }
		  else
		  {
			ResponseJson = MakeShareable(new FJsonObject());
			ResponseJson->SetStringField("response", ResponseString);
		  }

		  Callback(MoveTemp(ResponseJson), true);
		});

	return HttpRequest->ProcessRequest();
}

bool FCBComfyUIClient::DoHttpFileUpload(const FString& Url, const TArray64<uint8>& FileData, const FString& FileName,
	TFunction<void(const TSharedPtr<FJsonObject>&, bool)> Callback) const
{
	TSharedRef<IHttpRequest> HttpRequest = FHttpModule::Get().CreateRequest();
  HttpRequest->SetVerb("POST");
  HttpRequest->SetURL(BaseUrl + "/" + Url);

  // do an http post with the file in the image field

  HttpRequest->SetHeader("Content-Type", "multipart/form-data; boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW");

  TArray<uint8> Content;

  // http form data wtf

  FString Boundary = "------WebKitFormBoundary7MA4YWxkTrZu0gW";
  Content.Append((uint8*)TCHAR_TO_ANSI(*Boundary), Boundary.Len());
  Content.Append((uint8*)"\r\n", 2);

  // add overwite=1 to the form data

  FString Overwrite = "Content-Disposition: form-data; name=\"overwrite\"\r\n\r\n1\r\n";
  Content.Append((uint8*)TCHAR_TO_ANSI(*Overwrite), Overwrite.Len());
  Content.Append((uint8*)TCHAR_TO_ANSI(*Boundary), Boundary.Len());
  Content.Append((uint8*)"\r\n", 2);

  // add the file to the form data

  FString ContentDisposition = "Content-Disposition: form-data; name=\"image\"; filename=\"" + FileName + "\"";
  Content.Append((uint8*)TCHAR_TO_ANSI(*ContentDisposition), ContentDisposition.Len());
  Content.Append((uint8*)"\r\n", 2);
  FString ContentType = "Content-Type: image/png";
  Content.Append((uint8*)TCHAR_TO_ANSI(*ContentType), ContentType.Len());
  Content.Append((uint8*)"\r\n", 2);
  Content.Append((uint8*)"\r\n", 2);
  Content.Append(FileData.GetData(), FileData.Num());
  Content.Append((uint8*)"\r\n", 2);
  Content.Append((uint8*)TCHAR_TO_ANSI(*Boundary), Boundary.Len());
  Content.Append((uint8*)"--", 2);

  HttpRequest->SetContent(Content);

  HttpRequest->OnProcessRequestComplete()
    .BindLambda([Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
      {
        if (!bWasSuccessful || !Response.IsValid())
        {
          UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to receive valid response"));
          Callback(nullptr, false);
          return;
        }

        FString ResponseString = Response->GetContentAsString();
        TSharedPtr<FJsonObject> ResponseJson;

        if (Response->GetContentType().StartsWith("application/json"))
        {
          TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseString);

          if (!FJsonSerializer::Deserialize(Reader, ResponseJson))
          {
            UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to deserialize response JSON:%s"), *ResponseString);
            Callback(nullptr, false);
            return;
          }
        }
        else
        {
          ResponseJson = MakeShareable(new FJsonObject());
          ResponseJson->SetStringField("response", ResponseString);
        }

        Callback(MoveTemp(ResponseJson), true);
      });

  return HttpRequest->ProcessRequest();
}
