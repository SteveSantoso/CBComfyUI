// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CBComfyUIManager.h"
#include "Components/ActorComponent.h"
#include "CBComfyUISceneCaptureComponent.generated.h"


USTRUCT(BlueprintType)
struct FComfyTexturesCaptureOutput
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FComfyTexturesImageData RawDepth;

	UPROPERTY(BlueprintReadOnly)
	FComfyTexturesImageData Depth;

	UPROPERTY(BlueprintReadOnly)
	FComfyTexturesImageData Normals;

	UPROPERTY(BlueprintReadOnly)
	FComfyTexturesImageData Color;

	UPROPERTY(BlueprintReadOnly)
	FComfyTexturesImageData EdgeMask;
};

UENUM(BlueprintType)
enum class EComfyTexturesRenderTextureMode : uint8
{
	Depth,
	RawDepth,
	Normals,
	Color
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnComfyUISUpLoadImage,bool,bSucceed );
UCLASS( ClassGroup=(CBComfyUI), meta=(BlueprintSpawnableComponent) )
class CBCOMFYUI_API UCBComfyUISceneCaptureComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	using FComfyTexturesRenderDataPtr = TSharedPtr<FComfyTexturesRenderData>;
	
	UCBComfyUISceneCaptureComponent();
	
public:
	/**生成纹理的最小纹理大小应为 2 的幂*/
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "ComfyUI | Settings", meta = (DisplayName = "Min. Texture Size"))
	int MinTextureSize = 64;

	/**生成纹理的最大纹理大小应为 2 的幂*/
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "ComfyUI | Settings", meta = (DisplayName = "Max. Texture Size"))
	int MaxTextureSize = 4096;

	/**纹理质量的乘数，越高越好*/
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "ComfyUI | Settings", meta = (DisplayName = "Texture Quality Multiplier"))
	float TextureQualityMultiplier = 0.5f;

	/**场景捕捉纹理的大小，应为 2 的幂*/
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "ComfyUI | Settings", meta = (DisplayName = "Capture Size"))
	int CaptureSize = 2048;

	/**上传的图片*/
	UPROPERTY(BlueprintReadWrite,EditAnywhere, Category = "ComfyUI | Settings", meta = (DisplayName = "Upload Size"))
	int UploadSize = 1024;

	/**捕获的Actor*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="ComfyUI | Settings")
	ACameraActor* CaptureCamera=nullptr;

	/**ComfyUIManager*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite,Category="ComfyUI | Settings")
	ACBComfyUIManager* ComfyUIManager=nullptr;

public:
	UPROPERTY(BlueprintAssignable, Category = "CBComfyUI | Event")
	FOnComfyUISUpLoadImage OnComfyUISUpLoadImage;
	
public:
	/**基于Actors上传为多个图片*/
	UFUNCTION(BlueprintCallable, Category = "ComfyUISceneCapture")
	void ProcessMultipleActors(const TArray<AActor*>& Actors);

	/** 编辑器下生成多个选中的Actor；非编辑器环境自动回退到 Runtime 实现 */
	UFUNCTION(BlueprintCallable, Category = "ComfyUISceneCapture")
	bool PrepareActors(const TArray<AActor*>& Actor,UMaterial* BaseMaterial,UTexture2D* ReferenceTexture);

	/** Runtime下设置Actor材质 - 使用动态材质实例*/
	UFUNCTION(BlueprintCallable, Category = "ComfyUISceneCapture")
	bool PrepareActorsRuntime(const TArray<AActor*>& Actors, UMaterial* BaseMaterial, UTexture2D* ReferenceTexture);

	UFUNCTION(BlueprintCallable, Category = "ComfyUISceneCapture")
	void ProcessRenderResults(const FString& FileName);
	
private:
	bool CaptureSceneTextures(UWorld* World, TArray<AActor*> Actors,const FMinimalViewInfo ViewInfo, TSharedPtr<FComfyTexturesCaptureOutput>& Output) const;

	bool ReadRenderTargetPixels(UTextureRenderTarget2D* InputTexture, EComfyTexturesRenderTextureMode Mode, FComfyTexturesImageData& OutImage) const;

	void CreateEdgeMask(const FComfyTexturesImageData& Depth,  const FComfyTexturesImageData& Normals, FComfyTexturesImageData& OutEdgeMask) const;
	float ComputeAdaptiveThreshold(const TArray<float>& Grad, float AverageGradient, float BaseThreshold, float ScaleFactor = 1.0f) const;
	float ComputeImageGradient(const FComfyTexturesImageData& Image, bool bIsDepth, TArray<float>& OutGrad) const;
	float ComputeNormalsGradient(const FComfyTexturesImageData& Image, int X, int Y) const;
	float ComputeDepthGradient(const FComfyTexturesImageData& Image, int X, int Y) const;

	void ProcessSceneTextures( const TSharedPtr<FComfyTexturesCaptureOutput>& Output, int TargetSize, TFunction<void()> Callback) const  ;
	void ResizeImage( FComfyTexturesImageData& Image, int NewWidth, int NewHeight) const;

	bool UploadImages(const TArray<FComfyTexturesImageData>& Images, const TArray<FString>& FileNames, TFunction<void(const TArray<FString>&, bool)> Callback);
	bool ConvertImageToPng(const FComfyTexturesImageData& Image, TArray64<uint8>& OutBytes) const;

	bool CalculateApproximateScreenBounds(AActor* Actor, FBox2D& OutBounds) const;

	FMinimalViewInfo GetCaptureCameraViewInfo() const;

	UTexture2D* CreateTexture2D(int Width, int Height, const TArray<FColor>& Pixels) const;

	/** Runtime创建动态纹理 */
	UTexture2D* CreateTexture2DRuntime(int Width, int Height, const TArray<FColor>& Pixels) const;

	bool ProcessRenderResultForActor(AActor* Actor, TFunction<void(bool)> Callback);
	
	/** Runtime版本的结果处理 */
	bool ProcessRenderResultForActorRuntime(AActor* Actor, TFunction<void(bool)> Callback);
	
	void LoadRenderResultImages(const FString& FileName,TFunction<void(bool)> Callback);

	
#if WITH_EDITOR
	bool CreateAssetPackage(UObject* Asset, FString PackagePath) const;
#endif

	/** 检查是否在编辑器模式下运行 */
	bool IsRunningInEditor() const;

public:
	/** 清理Runtime资源 */
	UFUNCTION(BlueprintCallable, Category = "ComfyUISceneCapture")
	void CleanupRuntimeResources();

protected:
	/** 在组件销毁时清理资源 */
	virtual void BeginDestroy() override;

private:
	TSharedPtr<FComfyTexturesRenderData>  TexturesRenderData;
	
	TArray<AActor*> ActorSet;

	/** Runtime模式下存储动态创建的材质实例 */
	UPROPERTY(Transient)
	TArray<UMaterialInstanceDynamic*> RuntimeMaterials;

	/** Runtime模式下存储动态创建的纹理 */
	UPROPERTY()
	TArray<UTexture2D*> RuntimeTextures;

	/** 等待在安全时机释放的旧Runtime纹理 */
	UPROPERTY()
	TArray<UTexture2D*> RetiredRuntimeTextures;

	/** Actor到材质和纹理的映射表，用于Runtime模式下的精确管理 */
	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<UMaterialInstanceDynamic>> ActorToMaterialMap;
	
	UPROPERTY()
	TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<UTexture2D>> ActorToTextureMap;
};


