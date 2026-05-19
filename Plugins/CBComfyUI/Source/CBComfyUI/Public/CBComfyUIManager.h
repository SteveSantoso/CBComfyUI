// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CBComfyUIClient.h"
#include "Json/CBJsonObject.h"
#include "Subsystems/EngineSubsystem.h"
#include "CBComfyUIManager.generated.h"



UENUM(BlueprintType)
enum class EComfyUIState : uint8
{
	Idle,//连接并处于闲置状态
	Start,//队列开始
	Processing,//队列中
	Successed,//队列执行完成
	Error,//队列执行错误
	Disconnected//未连接
  };


USTRUCT(BlueprintType)
struct FComfyTexturesImageData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FLinearColor> Pixels;

	UPROPERTY(BlueprintReadOnly)
	int Width = 0;

	UPROPERTY(BlueprintReadOnly)
	int Height = 0;
};



struct FComfyTexturesRenderData
{
	FMinimalViewInfo ViewInfo;

	FMatrix ViewMatrix;

	FMatrix ProjectionMatrix;

	TArray<FColor> OutputPixels;

	FComfyTexturesImageData RawDepth;

	int OutputWidth = 0;

	int OutputHeight = 0;
 
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComfyUIStateChanged, EComfyUIState, State);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComfyUIMessage, const UCBJsonObject*, Json);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComfyUIProgress, int32, value,int32,MaxValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComfyUIOutputFile, int32, Node,const TArray<FString>&,OutputFileNames);
/**
 * 
 */
UCLASS(hidecategories = (Movement, Collision, Rendering, HLOD, WorldPartition, DataLayers, Transformation,Transform, Replication, Actor, Cooking,Physics),Blueprintable)
class CBCOMFYUI_API ACBComfyUIManager  : public AActor
{
	GENERATED_BODY()

public:

	/**是否远程接入ComfyUI*/
	UPROPERTY(EditAnywhere,BlueprintReadWrite, config, Category = "ComfyUI || Settings")
	bool bRemote=false;
	
	/**是否激活ComfyUI*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite,config, Category = "ComfyUI || Settings",meta = (EditConditionHides, EditCondition = "bRemote==false"))
	bool bActive=false;

	/**服务器连接地址*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite,config, Category = "ComfyUI || Settings", meta = (DisplayName = "ComfyUI URL"))
	FString ComfyURL = "http://127.0.0.1:8188";
	
	
	/**Comfyui状态*/
	UPROPERTY(BlueprintReadOnly,EditAnywhere, Category = "CBComfyUI")
	EComfyUIState Status ;

	/**执行当前队列的PromptID*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere,Category = "CBComfyUI",DisplayName="Prompt ID")
	FString PromptID ;

	/**执行当前队列的节点*/
	UPROPERTY(BlueprintReadOnly, EditAnywhere,Category = "CBComfyUI")
	int32 CurrentNode ;
	
public:
	/**状态变化*/
	UPROPERTY(BlueprintAssignable, Category = "CBComfyUI|Event")
	FOnComfyUIStateChanged OnComfyUIStateChanged;

	/**Comfyui的WebSocket消息*/
	UPROPERTY(BlueprintAssignable, Category = "CBComfyUI|Event")
	FOnComfyUIMessage OnComfyUIMessage;

	/**执行采样进度*/
	UPROPERTY(BlueprintAssignable, Category = "CBComfyUI|Event")
	FOnComfyUIProgress OnComfyUIProgress;

	/**导出保存的文件名字*/
	UPROPERTY(BlueprintAssignable, Category = "CBComfyUI|Event")
	FOnComfyUIOutputFile OnComfyUIOutputFile;
	
public:
	ACBComfyUIManager(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	virtual void PostLoad() override;
	virtual void PostActorCreated() override;
	virtual void BeginDestroy() override;

	UFUNCTION(BlueprintPure, Category = "CBComfyUI", meta = (WorldContext = "WorldContextObject"),DisplayName="Get CB ComfyUI Manager")
	static ACBComfyUIManager* GetCBComfyUIManager(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	void CloseWebSocket();
	
#if WITH_EDITORONLY_DATA
private:
	/** Billboard Component displayed in editor */
	UPROPERTY()
	TObjectPtr<class UBillboardComponent> SpriteComponent;
public:
#endif
	
public:
	/**连接Comfyui的WebSocket服务*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	void ConnectCommfyUI();

	/**判断是否连接上comfyui*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	bool IsConnected() const;

	/**通过字符串来执行comfyui队列*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	bool QueueWorkflowFromString(const FString Workflow);
	
	/**通过Json文本来执行comfyui队列*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	bool QueueWorkflowFromFile(const FString FilePath);
	
	/**通过JsonObject来执行comfyui队列*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	bool QueueWorkflowFromJsonObject(UCBJsonObject* JsonObject);

	/**终止队列*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	void Interrupt() ;

	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	void ClearQueue() ;

	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	void CancelJob();

	/**
	 * 清空缓存
	 * @param bUnloadModels 是否清空模型加载缓存，ture为清空
	 */
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	void FreeComfyMemory(bool bUnloadModels);
	
	/**通过Object节点设置参数值（double）*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	UCBJsonObject* SetNodeInputDoubleProperty(UCBJsonObject* JsonWrkflow,const FString& NodeName, const FString& PropertyName, double Value);

	/**通过Object节点设置参数值（int）*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	UCBJsonObject* SetNodeInputIntProperty(UCBJsonObject* JsonWrkflow,const FString& NodeName, const FString& PropertyName, int Value);

	/**通过Object节点设置参数值（string）*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	UCBJsonObject* SetNodeInputStringProperty(UCBJsonObject* JsonWrkflow,const FString& NodeName, const FString& PropertyName, const FString& Value);

	/**通过Object节点获取参数值（double）*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	bool GetNodeInputDoubleProperty(UCBJsonObject* JsonWrkflow, const FString& NodeName, const FString& PropertyName, double& OutValue);

	/**通过Object节点获取参数值（int）*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	bool GetNodeInputIntProperty(UCBJsonObject* JsonWrkflow, const FString& NodeName, const FString& PropertyName, int& OutValue);

	/**通过Object节点获取参数值（String）*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	bool GetNodeInputStringProperty(UCBJsonObject* JsonWrkflow, const FString& NodeName, const FString& PropertyName, FString& OutValue);

	/**设置连接超时时间（秒）*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	void SetConnectionTimeout(float TimeoutSeconds);

	/**设置HTTP请求超时时间（秒）*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	void SetHttpTimeout(float TimeoutSeconds);

	/**启用/禁用详细日志*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	void SetVerboseLogging(bool bEnable);

	/**检查是否在编辑器中运行*/
	UFUNCTION(BlueprintPure, Category = "CBComfyUI")
	bool IsRunningInEditor() const;

	/**配置Runtime模式优化*/
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI")
	void ConfigureForRuntime(bool bEnableAutoReconnect = true, float ReconnectInterval = 5.0f);

public:
	bool QueueWorkflowFromJsonObject(TSharedPtr<FJsonObject> JsonObject);
	
	void SetNodeInputProperty(FJsonObject& Workflow, const FString& NodeName, const FString& PropertyName, double Value);
	void SetNodeInputProperty(FJsonObject& Workflow, const FString& NodeName, const FString& PropertyName, int Value);
	void SetNodeInputProperty(FJsonObject& Workflow, const FString& NodeName, const FString& PropertyName, const FString& Value);

	bool GetNodeInputProperty(FJsonObject& Workflow, const FString& NodeName, const FString& PropertyName, int& OutValue);
	bool GetNodeInputProperty(FJsonObject& Workflow, const FString& NodeName, const FString& PropertyName, FString& OutValue);
	bool GetNodeInputProperty(FJsonObject& Workflow, const FString& NodeName, const FString& PropertyName, double& OutValue);

	bool DownloadImage(const FString& FileName, TFunction<void(TArray<FColor>, int, int, bool)> Callback);
	
public:

	TUniquePtr<FCBComfyUIClient> HttpClient;
private:
	FString GetBaseUrl() const;

	void HandleWebSocketMessage(const TSharedPtr<FJsonObject>& Message);

public:
	int32 QueueProgressValue;
	int32 QueueProgressMax;

	// Runtime optimization settings
	float ConnectionTimeout;
	float HttpTimeout;
	bool bVerboseLogging;

	FTimerHandle ReconnectionTimerHandle;

protected:
	
private:
	void AttemptReconnection();
	
};
