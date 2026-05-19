// Fill out your copyright notice in the Description page of Project Settings.


#include "CBComfyUISceneCaptureComponent.h"

#include "CBComfyUI.h"
#include "CBComfyUIManager.h"
#include "ImageUtils.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Texture2D.h"
#include "RenderUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#if WITH_EDITOR
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#endif


#define LOCTEXT_NAMESPACE "ComfyTextures"

namespace
{
	bool HasReadableRuntimeMeshData(UStaticMesh* StaticMesh, FString& OutFailureReason)
	{
		if (StaticMesh == nullptr)
		{
			OutFailureReason = TEXT("Static mesh is null");
			return false;
		}

		const FStaticMeshRenderData* RenderData = StaticMesh->GetRenderData();
		if (RenderData == nullptr || RenderData->LODResources.Num() == 0)
		{
			OutFailureReason = FString::Printf(TEXT("Static mesh %s has no render LOD data"), *StaticMesh->GetName());
			return false;
		}

		const FStaticMeshLODResources& MeshLod = RenderData->LODResources[0];
		const FPositionVertexBuffer& PositionBuffer = MeshLod.VertexBuffers.PositionVertexBuffer;
		const FStaticMeshVertexBuffer& VertexBuffer = MeshLod.VertexBuffers.StaticMeshVertexBuffer;

		if (!PositionBuffer.GetAllowCPUAccess() || PositionBuffer.GetVertexData() == nullptr)
		{
			OutFailureReason = FString::Printf(TEXT("Static mesh %s has no CPU position buffer in cooked builds. Enable Allow CPU Access on the mesh asset."), *StaticMesh->GetName());
			return false;
		}

		if (!VertexBuffer.GetAllowCPUAccess() || VertexBuffer.GetTexCoordData() == nullptr)
		{
			OutFailureReason = FString::Printf(TEXT("Static mesh %s has no CPU UV buffer in cooked builds. Enable Allow CPU Access on the mesh asset."), *StaticMesh->GetName());
			return false;
		}

		if (!MeshLod.IndexBuffer.GetAllowCPUAccess())
		{
			OutFailureReason = FString::Printf(TEXT("Static mesh %s has no CPU index buffer in cooked builds. Enable Allow CPU Access on the mesh asset."), *StaticMesh->GetName());
			return false;
		}

		return true;
	}

	FVector2D GetRuntimeMeshUv(const FStaticMeshVertexBuffer& VertexBuffer, uint32 VertexIndex)
	{
		if (VertexBuffer.GetNumTexCoords() == 0 || VertexIndex >= VertexBuffer.GetNumVertices() || VertexBuffer.GetTexCoordData() == nullptr)
		{
			return FVector2D::ZeroVector;
		}

		const FVector2f UvFloat = VertexBuffer.GetUseFullPrecisionUVs()
			? VertexBuffer.GetVertexUV_Typed<EStaticMeshVertexUVType::HighPrecision>(VertexIndex, 0)
			: VertexBuffer.GetVertexUV_Typed<EStaticMeshVertexUVType::Default>(VertexIndex, 0);

		return FVector2D(UvFloat.X, UvFloat.Y);
	}
}

UCBComfyUISceneCaptureComponent::UCBComfyUISceneCaptureComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
    TexturesRenderData=MakeShared<FComfyTexturesRenderData>();

	// 初始化Runtime资源数组
	RuntimeMaterials.Reserve(8); // 预分配一些空间
	RuntimeTextures.Reserve(8);
	
	// 初始化映射表
	ActorToMaterialMap.Reserve(8);
	ActorToTextureMap.Reserve(8);
}

void UCBComfyUISceneCaptureComponent::ProcessMultipleActors(const TArray<AActor*>& Actors)
{
	if(!ComfyUIManager)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Existing ComfyUIManager is null."));
		return ;
	}

	FMinimalViewInfo ViewInfo=GetCaptureCameraViewInfo();


	TSharedPtr<FComfyTexturesCaptureOutput> CaptureResults=MakeShared<FComfyTexturesCaptureOutput>(); 

	double CaptureSceneTexturesTime = 0.0;
	{
		SCOPE_SECONDS_COUNTER(CaptureSceneTexturesTime);
		if (!CaptureSceneTextures(GetWorld(), Actors, ViewInfo,  CaptureResults))
		{
			UE_LOG(LogCBComfyUI, Error, TEXT("Failed to capture input textures"));
			return ;
		}
	}

	UE_LOG(LogCBComfyUI, Display, TEXT("Capture scene textures took %f seconds"), CaptureSceneTexturesTime);

	ActorSet = Actors;
	
	ProcessSceneTextures(CaptureResults,UploadSize,[this, CaptureResults, ViewInfo]
	{
		TArray<FComfyTexturesImageData> Images;
		TArray<FString> FileNames;

		const FComfyTexturesCaptureOutput& Output = *CaptureResults;
		
		const FComfyTexturesImageData& RawDepth = Output.RawDepth;
		
		Images.Add(Output.Depth);
		FileNames.Add("depth.png");

		Images.Add(Output.Normals);
		FileNames.Add("normals.png");

		Images.Add(Output.Color);
		FileNames.Add("color.png");

		Images.Add(Output.EdgeMask);
		FileNames.Add("edge_mask.png");

		bool bSuccess = UploadImages(Images, FileNames, [this, ViewInfo, RawDepth](const TArray<FString>& FileNames, bool bSuccess)
		{
			if (!bSuccess)
			{
				UE_LOG(LogCBComfyUI, Error, TEXT("Upload failed"));
				return;
			}

			UE_LOG(LogCBComfyUI, Verbose, TEXT("Upload complete"));

			for (const FString& FileName : FileNames)
			{
				UE_LOG(LogCBComfyUI, Verbose, TEXT("Uploaded file: %s"), *FileName);

				
				TOptional<FMatrix> CustomProjectionMatrix;
				FMatrix ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;
				UGameplayStatics::CalculateViewProjectionMatricesFromMinimalView(ViewInfo, CustomProjectionMatrix,
				  ViewMatrix, ProjectionMatrix, ViewProjectionMatrix);

				TexturesRenderData->ViewInfo = ViewInfo;
				TexturesRenderData->ViewMatrix = ViewMatrix;
				TexturesRenderData->ProjectionMatrix = ProjectionMatrix;
				TexturesRenderData->RawDepth = RawDepth;
			}
			
		});
	});

}


bool UCBComfyUISceneCaptureComponent::PrepareActors(const TArray<AActor*>& Actors, UMaterial* BaseMaterial,UTexture2D* ReferenceTexture)
{
#if WITH_EDITOR
	if (ReferenceTexture == nullptr)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Reference texture is null"));
		return false;
	}

	void* ReferenceMipData = ReferenceTexture->Source.LockMip(0);
	if (ReferenceMipData == nullptr)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Failed to lock texture mip data"));
		return false;
	}

	TArray<FColor> ReferencePixels;
	ReferencePixels.SetNumZeroed(ReferenceTexture->GetSizeX() * ReferenceTexture->GetSizeY());
	FMemory::Memcpy(ReferencePixels.GetData(), ReferenceMipData, ReferencePixels.Num() * sizeof(FColor));
	ReferenceTexture->Source.UnlockMip(0);

	FScopedTransaction Transaction(TEXT("Comfy Textures Prepare Actors"), LOCTEXT("ComfyTextures", "Prepare Actors"), nullptr);

  for (AActor* Actor : Actors)
  {
    UStaticMeshComponent* StaticMeshComponent = Actor->FindComponentByClass<UStaticMeshComponent>();

    // 若actor没有静态网格组件，则跳过
    if (StaticMeshComponent == nullptr)
    {
      continue;
    }
  	
    UMaterialInterface* Material = StaticMeshComponent->GetMaterial(0);

  	// 如果当前材质是动态材质实例，其基础材质与准备选项基础材质相同，则跳过它
    if (Material != nullptr && Material->IsA<UMaterialInstanceDynamic>())
    {
      UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(Material);
      if (MaterialInstance->Parent == BaseMaterial)
      {
        continue;
      }
    }
  	

    FBox2D ActorScreenBounds;
    if (!CalculateApproximateScreenBounds(Actor, ActorScreenBounds))
    {
      UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to calculate screen bounds for actor %s."), *Actor->GetName());
      continue;
    }

    UE_LOG(LogCBComfyUI, Verbose, TEXT("Actor %s screen bounds: %s."), *Actor->GetName(), *ActorScreenBounds.ToString());

    FVector2D SizeOnScreen = ActorScreenBounds.GetSize();
    float LargerSize = FMath::Max(SizeOnScreen.X, SizeOnScreen.Y);

   
    int TextureSize = FMath::Lerp((float)MinTextureSize, (float)MaxTextureSize, LargerSize);
    TextureSize = (float)TextureSize * TextureQualityMultiplier;

    // 获取2的幂
    TextureSize = FMath::Clamp(TextureSize, MinTextureSize, MaxTextureSize);
    TextureSize = FMath::RoundUpToPowerOfTwo(TextureSize);

    UE_LOG(LogCBComfyUI, Verbose, TEXT("Chosen texture size: %d for actor %s."), TextureSize, *Actor->GetName());

    FString TextureName = "CBTexture_" + Actor->GetActorNameOrLabel();

  	// 将参考纹理重新缩放为 TextureSize
    TArray<FColor> RescaledReferencePixels;
    RescaledReferencePixels.SetNumZeroed(TextureSize * TextureSize);

    FImageUtils::ImageResize(ReferenceTexture->GetSizeX(), ReferenceTexture->GetSizeY(), ReferencePixels, TextureSize, TextureSize, RescaledReferencePixels, false);

    UTexture2D* Texture2D = CreateTexture2D(TextureSize, TextureSize, RescaledReferencePixels);
    if (Texture2D == nullptr)
    {
      UE_LOG(LogCBComfyUI, Error, TEXT("Failed to create texture %s"), *TextureName);
      return false;
    }

    Texture2D->Rename(*TextureName);

    if (!CreateAssetPackage(Texture2D, "/Game/Textures/Generated/"))
    {
      UE_LOG(LogCBComfyUI, Error, TEXT("Failed to create asset package for texture %s"), *TextureName);
      return false;
    }

  	// 创建一个新的材质实例
    UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, nullptr);
    MaterialInstance->SetTextureParameterValue(TEXT("BaseColor"), Texture2D);

    FString MaterialName = "M_" + Actor->GetActorNameOrLabel();
    MaterialInstance->Rename(*MaterialName);

    if (!CreateAssetPackage(MaterialInstance, "/Game/Materials/Generated/"))
    {
      UE_LOG(LogCBComfyUI, Error, TEXT("Failed to create asset package for material instance %s"), *MaterialName);
      return false;
    }

    StaticMeshComponent->Modify();

  	// 将材质实例分配给静态网格组件
    StaticMeshComponent->SetMaterial(0, MaterialInstance);

    StaticMeshComponent->MarkPackageDirty();
  }

  return true;
#else
	UE_LOG(LogCBComfyUI, Log, TEXT("PrepareActors called outside editor. Falling back to PrepareActorsRuntime."));
	return PrepareActorsRuntime(Actors, BaseMaterial, ReferenceTexture);
#endif
}

bool UCBComfyUISceneCaptureComponent::PrepareActorsRuntime(const TArray<AActor*>& Actors, UMaterial* BaseMaterial, UTexture2D* ReferenceTexture)
{
	if (ReferenceTexture == nullptr)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Reference texture is null"));
		return false;
	}

	if (BaseMaterial == nullptr)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Base material is null"));
		return false;
	}

	// 清理之前的Runtime资源
	CleanupRuntimeResources();

	// 获取参考纹理的像素数据
	TArray<FColor> ReferencePixels;
	ReferencePixels.SetNumZeroed(ReferenceTexture->GetSizeX() * ReferenceTexture->GetSizeY());
	
	// Runtime模式下从UTexture2D获取纹理数据的正确方法
	FTexture2DMipMap* MipMap = nullptr;
	
	// 首先尝试从Source获取数据（如果可用）
#if WITH_EDITOR
	if (ReferenceTexture->Source.IsValid() && ReferenceTexture->Source.GetNumMips() > 0)
	{
		void* SourceMipData = ReferenceTexture->Source.LockMip(0);
		if (SourceMipData)
		{
			FMemory::Memcpy(ReferencePixels.GetData(), SourceMipData, ReferencePixels.Num() * sizeof(FColor));
			ReferenceTexture->Source.UnlockMip(0);
		}
		else
		{
			UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to get texture data from Source, using default white"));
			for (FColor& Pixel : ReferencePixels)
			{
				Pixel = FColor::White;
			}
		}
	}
	else
#endif
	{
		// Runtime模式：尝试从平台数据获取
		if (ReferenceTexture->GetPlatformData() && ReferenceTexture->GetPlatformData()->Mips.Num() > 0)
		{
			MipMap = &ReferenceTexture->GetPlatformData()->Mips[0];
			const void* MipData = MipMap->BulkData.LockReadOnly();
			if (MipData)
			{
				// 检查纹理格式并相应地读取数据
				EPixelFormat PixelFormat = ReferenceTexture->GetPixelFormat();
				if (PixelFormat == PF_B8G8R8A8 || PixelFormat == PF_R8G8B8A8)
				{
					FMemory::Memcpy(ReferencePixels.GetData(), MipData, ReferencePixels.Num() * sizeof(FColor));
				}
				else
				{
					// 如果格式不匹配，填充默认颜色
					UE_LOG(LogCBComfyUI, Warning, TEXT("Unsupported pixel format %d for reference texture, using white color"), (int32)PixelFormat);
					for (FColor& Pixel : ReferencePixels)
					{
						Pixel = FColor::White;
					}
				}
				MipMap->BulkData.Unlock();
			}
			else
			{
				UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to lock reference texture mip data, using default white"));
				for (FColor& Pixel : ReferencePixels)
				{
					Pixel = FColor::White;
				}
			}
		}
		else
		{
			UE_LOG(LogCBComfyUI, Warning, TEXT("No platform data available for reference texture, using default white"));
			for (FColor& Pixel : ReferencePixels)
			{
				Pixel = FColor::White;
			}
		}
	}

	for (AActor* Actor : Actors)
	{
		UStaticMeshComponent* StaticMeshComponent = Actor->FindComponentByClass<UStaticMeshComponent>();

		if (StaticMeshComponent == nullptr)
		{
			continue;
		}

		UMaterialInterface* Material = StaticMeshComponent->GetMaterial(0);

		// 如果当前材质已经是基于我们基础材质的动态材质实例，则跳过
		if (Material != nullptr && Material->IsA<UMaterialInstanceDynamic>())
		{
			UMaterialInstanceDynamic* MaterialInstance = Cast<UMaterialInstanceDynamic>(Material);
			if (MaterialInstance->Parent == BaseMaterial)
			{
				continue;
			}
		}

		FBox2D ActorScreenBounds;
		if (!CalculateApproximateScreenBounds(Actor, ActorScreenBounds))
		{
			UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to calculate screen bounds for actor %s."), *Actor->GetName());
			continue;
		}

		FVector2D SizeOnScreen = ActorScreenBounds.GetSize();
		float LargerSize = FMath::Max(SizeOnScreen.X, SizeOnScreen.Y);

		int TextureSize = FMath::Lerp((float)MinTextureSize, (float)MaxTextureSize, LargerSize);
		TextureSize = (float)TextureSize * TextureQualityMultiplier;
		TextureSize = FMath::Clamp(TextureSize, MinTextureSize, MaxTextureSize);
		TextureSize = FMath::RoundUpToPowerOfTwo(TextureSize);

		UE_LOG(LogCBComfyUI, Verbose, TEXT("Chosen texture size: %d for actor %s."), TextureSize, *Actor->GetName());

		// 重新缩放参考纹理
		TArray<FColor> RescaledReferencePixels;
		RescaledReferencePixels.SetNumZeroed(TextureSize * TextureSize);
		FImageUtils::ImageResize(ReferenceTexture->GetSizeX(), ReferenceTexture->GetSizeY(), ReferencePixels, 
								TextureSize, TextureSize, RescaledReferencePixels, false);

		// 创建Runtime纹理
		UTexture2D* Texture2D = CreateTexture2DRuntime(TextureSize, TextureSize, RescaledReferencePixels);
		if (Texture2D == nullptr)
		{
			UE_LOG(LogCBComfyUI, Error, TEXT("Failed to create runtime texture for actor %s"), *Actor->GetName());
			continue;
		}

		// 存储到Runtime纹理数组中以防止垃圾回收
		RuntimeTextures.Add(Texture2D);

		// 创建动态材质实例
		UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		if (!MaterialInstance)
		{
			UE_LOG(LogCBComfyUI, Error, TEXT("Failed to create dynamic material instance for actor %s"), *Actor->GetName());
			continue;
		}
		
		MaterialInstance->SetTextureParameterValue(TEXT("BaseColor"), Texture2D);
		
		// 确保材质参数更新生效
		MaterialInstance->RecacheUniformExpressions(true);

		// 存储到Runtime材质数组中
		RuntimeMaterials.Add(MaterialInstance);

		// 添加到映射表中
		ActorToTextureMap.Add(Actor, Texture2D);
		ActorToMaterialMap.Add(Actor, MaterialInstance);

		// 应用材质到组件
		StaticMeshComponent->SetMaterial(0, MaterialInstance);
		
		// 标记组件需要重新渲染
		StaticMeshComponent->MarkRenderStateDirty();

		UE_LOG(LogCBComfyUI, Log, TEXT("Successfully prepared runtime material for actor %s"), *Actor->GetName());
	}

	return true;
}

void UCBComfyUISceneCaptureComponent::ProcessRenderResults(const FString& FileName)
{
	LoadRenderResultImages(FileName,[this](bool bSuccess)
	   {
		 if (!bSuccess)
		 {
		   UE_LOG(LogCBComfyUI, Error, TEXT("Failed to load render result images"));
		   return;
		 }

		 TSharedPtr<int32> NumPending = MakeShared<int32>();

		 for (AActor* Actor : ActorSet)
		 {
		   bool bProcessSuccess = false;
		   
		   // 根据运行模式选择处理方法
		   if (IsRunningInEditor())
		   {
			   bProcessSuccess = ProcessRenderResultForActor(Actor, [&, NumPending](bool bSuccess)
				 {
				   if (!bSuccess)
				   {
					 UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to process render result for actor %s"), *Actor->GetName());
				   }
				   return true;
				 });
		   }
		   else
		   {
			   bProcessSuccess = ProcessRenderResultForActorRuntime(Actor, [&, NumPending](bool bSuccess)
				 {
				   if (!bSuccess)
				   {
					 UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to process render result for actor %s"), *Actor->GetName());
				   }
				   return true;
				 });
		   }
		   
		   if (bProcessSuccess)
		   {
			 (*NumPending)++;
		   }
		 }

		 if (*NumPending == 0)
		 {
#if WITH_EDITOR
		   if (UKismetSystemLibrary::EndTransaction() != -1)
		   {
			 UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to end transaction"));
		   }
#endif
		 }
	   });
}

bool UCBComfyUISceneCaptureComponent::CaptureSceneTextures(UWorld* World, TArray<AActor*> Actors,
                                                           const FMinimalViewInfo ViewInfo, TSharedPtr<FComfyTexturesCaptureOutput>& Output) const
{
	if (World == nullptr)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("World is null."));
		return false;
	}

	if (Actors.Num() == 0)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Actors is empty."));
		return false;
	}
	
	// create RTF RGBA8 render target
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->InitCustomFormat(CaptureSize, CaptureSize, EPixelFormat::PF_FloatRGBA, true);
	RenderTarget->UpdateResourceImmediate();

	// create scene capture component
	USceneCaptureComponent2D* SceneCapture = NewObject<USceneCaptureComponent2D>();
	SceneCapture->TextureTarget = RenderTarget;

	SceneCapture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_RenderScenePrimitives;

	SceneCapture->ShowOnlyActors = Actors;
	SceneCapture->bCaptureEveryFrame = false;
	SceneCapture->bCaptureOnMovement = false;
	SceneCapture->bAlwaysPersistRenderingState = true;
	SceneCapture->RegisterComponentWithWorld(World);
	SceneCapture->SetCameraView(ViewInfo);
	
	// capture the scene
	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_SceneColorSceneDepth;
	SceneCapture->CaptureScene();
	if (!ReadRenderTargetPixels(RenderTarget, EComfyTexturesRenderTextureMode::Depth, Output->Depth))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Failed to read render target pixels."));
		return false;
	}

	if (!ReadRenderTargetPixels(RenderTarget, EComfyTexturesRenderTextureMode::RawDepth, Output->RawDepth))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Failed to read render target pixels."));
		return false;
	}

	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_BaseColor;
	SceneCapture->CaptureScene();

	if (!ReadRenderTargetPixels(RenderTarget, EComfyTexturesRenderTextureMode::Color, Output->Color))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Failed to read render target pixels."));
		return false;
	}

	SceneCapture->CaptureSource = ESceneCaptureSource::SCS_Normal;
	SceneCapture->CaptureScene();

	if (!ReadRenderTargetPixels(RenderTarget, EComfyTexturesRenderTextureMode::Normals, Output->Normals))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Failed to read render target pixels."));
		return false;
	}
	// destroy the scene capture component
	SceneCapture->DestroyComponent();
	SceneCapture = nullptr;

	// destroy the render target
	RenderTarget->ConditionalBeginDestroy();
	RenderTarget = nullptr;

	return true;
	
}

bool UCBComfyUISceneCaptureComponent::ReadRenderTargetPixels(UTextureRenderTarget2D* InputTexture,
	EComfyTexturesRenderTextureMode Mode, FComfyTexturesImageData& OutImage) const
{
	  if (InputTexture == nullptr)
  {
    return false;
  }

  FTextureRenderTargetResource* RenderTargetResource = InputTexture->GameThread_GetRenderTargetResource();
  if (RenderTargetResource == nullptr)
  {
    return false;
  }

  static TArray<FFloat16Color> Pixels;
  Pixels.SetNumUninitialized(InputTexture->SizeX * InputTexture->SizeY);
  if (!RenderTargetResource->ReadFloat16Pixels(Pixels))
  {
    return false;
  }

  OutImage.Width = InputTexture->SizeX;
  OutImage.Height = InputTexture->SizeY;
  OutImage.Pixels.SetNum(Pixels.Num());

  if (Mode == EComfyTexturesRenderTextureMode::Depth)
  {
    float MinDepth = FLT_MAX;
    float MaxDepth = -FLT_MAX;

    for (int Index = 0; Index < Pixels.Num(); Index++)
    {
      float Depth = Pixels[Index].A;
      if (Depth >= 65504.0f)
      {
        continue;
      }

      MinDepth = FMath::Min(MinDepth, Depth);
      MaxDepth = FMath::Max(MaxDepth, Depth);
    }

    for (int Index = 0; Index < Pixels.Num(); Index++)
    {
      float Depth = Pixels[Index].A;
      Depth = FMath::Clamp(Depth, MinDepth, MaxDepth);
      Depth = (Depth - MinDepth) / (MaxDepth - MinDepth);
      Depth = FMath::Clamp(Depth, 0.0f, 1.0f);
      Depth = 1.0f - Depth;

      OutImage.Pixels[Index].R = Depth;
      OutImage.Pixels[Index].G = Depth;
      OutImage.Pixels[Index].B = Depth;
      OutImage.Pixels[Index].A = 1.0f;
    }
  }
  else if (Mode == EComfyTexturesRenderTextureMode::RawDepth)
  {
    for (int Index = 0; Index < Pixels.Num(); Index++)
    {
      float Depth = Pixels[Index].A;
      OutImage.Pixels[Index].R = Depth;
      OutImage.Pixels[Index].G = Depth;
      OutImage.Pixels[Index].B = Depth;
      OutImage.Pixels[Index].A = 1.0f;
    }
  }
  else if (Mode == EComfyTexturesRenderTextureMode::Normals)
  {
    for (int Index = 0; Index < Pixels.Num(); Index++)
    {
      FVector Normal = FVector(Pixels[Index].R, Pixels[Index].G, Pixels[Index].B);
      Normal = Normal.GetSafeNormal();
      Normal = (Normal + 1.0f) / 2.0f;
      OutImage.Pixels[Index].R = Normal.X;
      OutImage.Pixels[Index].G = Normal.Y;
      OutImage.Pixels[Index].B = Normal.Z;
      OutImage.Pixels[Index].A = 1.0f;
    }
  }
  else if (Mode == EComfyTexturesRenderTextureMode::Color)
  {
    for (int Index = 0; Index < Pixels.Num(); Index++)
    {
      OutImage.Pixels[Index].R = Pixels[Index].R;
      OutImage.Pixels[Index].G = Pixels[Index].G;
      OutImage.Pixels[Index].B = Pixels[Index].B;
      OutImage.Pixels[Index].A = 1.0f;
    }
  }
  else
  {
    UE_LOG(LogCBComfyUI, Error, TEXT("Unknown render texture mode: %d"), (int)Mode);
    return false;
  }

  return true;
}


void UCBComfyUISceneCaptureComponent::CreateEdgeMask(const FComfyTexturesImageData& Depth,
	 const FComfyTexturesImageData& Normals, FComfyTexturesImageData& OutEdgeMask) const
{
	if (Depth.Width != Normals.Width || Depth.Height != Normals.Height)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Depth and normals images have different dimensions."));
		return;
	}

	// perform edge detection on the depth and normals images
	OutEdgeMask.Width = Depth.Width;
	OutEdgeMask.Height = Depth.Height;
	OutEdgeMask.Pixels.SetNumUninitialized(Depth.Pixels.Num());

	TArray<float> DepthGrad;
	float AvgDepth = ComputeImageGradient(Depth, true, DepthGrad);

	TArray<float> NormalsGrad;
	float AvgNormals = ComputeImageGradient(Normals, false, NormalsGrad);

	const float DepthBaseThreshold = 0.01f;
	const float NormalsBaseThreshold = 0.1f;
	const float DepthScale = 8.0f;
	const float NormalsScale = 0.8f;

	float DepthThreshold = ComputeAdaptiveThreshold(DepthGrad, AvgDepth, DepthBaseThreshold);
	float NormalsThreshold = ComputeAdaptiveThreshold(NormalsGrad, AvgNormals, NormalsBaseThreshold);

	// Assuming Depth and Normals are of the same dimensions
	for (int Y = 0; Y < Depth.Height; Y++)
	{
		for (int X = 0; X < Depth.Width; X++)
		{
			// Compute gradients
			float DepthGradient = DepthGrad[Y * Depth.Width + X];
			float NormalsGradient = NormalsGrad[Y * Depth.Width + X];

			// Apply thresholds
			DepthGradient = (DepthGradient >= DepthThreshold) ? DepthGradient : 0.0f;
			NormalsGradient = (NormalsGradient >= NormalsThreshold) ? NormalsGradient : 0.0f;

			// Combine gradients for edge strength
			float EdgeStrength = FMath::Max(DepthGradient * DepthScale, NormalsGradient * NormalsScale);
			EdgeStrength = FMath::Clamp(EdgeStrength, 0.0f, 1.0f);

			// Set the pixel value in the output mask
			OutEdgeMask.Pixels[Y * Depth.Width + X] = FLinearColor(EdgeStrength, EdgeStrength, EdgeStrength, 1.0f);
		}
	}
}

float UCBComfyUISceneCaptureComponent::ComputeAdaptiveThreshold(const TArray<float>& Grad, float AverageGradient,
	float BaseThreshold, float ScaleFactor) const
{
	return BaseThreshold + ScaleFactor * AverageGradient;
}

float UCBComfyUISceneCaptureComponent::ComputeImageGradient(const FComfyTexturesImageData& Image, bool bIsDepth,
	TArray<float>& OutGrad)const
{
	OutGrad.SetNumUninitialized(Image.Pixels.Num());

	float MaxGradient = -FLT_MAX;

	for (int Y = 0; Y < Image.Height; Y++)
	{
		for (int X = 0; X < Image.Width; X++)
		{
			float Gradient = bIsDepth ? ComputeDepthGradient(Image, X, Y) : ComputeNormalsGradient(Image, X, Y);
			OutGrad[Y * Image.Width + X] = Gradient;

			MaxGradient = FMath::Max(MaxGradient, Gradient);
		}
	}

	if (FMath::IsNearlyZero(MaxGradient))
	{
		return 0.0f;
	}

	float AverageMagnitude = 0.0f;

	for (int Y = 0; Y < Image.Height; Y++)
	{
		for (int X = 0; X < Image.Width; X++)
		{
			OutGrad[Y * Image.Width + X] /= MaxGradient;
			AverageMagnitude += OutGrad[Y * Image.Width + X];
		}
	}

	AverageMagnitude /= Image.Pixels.Num();
	return AverageMagnitude;
}

int SobelX[3][3] = { {-1, 0, 1}, {-2, 0, 2}, {-1, 0, 1} };
int SobelY[3][3] = { {-1, -2, -1}, {0, 0, 0}, {1, 2, 1} };

float UCBComfyUISceneCaptureComponent::ComputeNormalsGradient(const FComfyTexturesImageData& Image, int X, int Y) const
{
	FVector GradX(0.0f, 0.0f, 0.0f);
	FVector GradY(0.0f, 0.0f, 0.0f);

	// Apply the Sobel operator
	for (int I = -1; I <= 1; I++)
	{
		for (int J = -1; J <= 1; J++)
		{
			int PixelX = FMath::Clamp(X + I, 0, Image.Width - 1);
			int PixelY = FMath::Clamp(Y + J, 0, Image.Height - 1);

			FVector Normal = FVector
			(
			  Image.Pixels[PixelY * Image.Width + PixelX].R,
			  Image.Pixels[PixelY * Image.Width + PixelX].G,
			  Image.Pixels[PixelY * Image.Width + PixelX].B
			);

			Normal -= FVector(0.5f, 0.5f, 0.5f);
			Normal *= 2.0f;
			Normal = Normal.GetSafeNormal();

			GradX += Normal * SobelX[I + 1][J + 1];
			GradY += Normal * SobelY[I + 1][J + 1];
		}
	}

	// Calculate the gradient magnitude
	FVector Gradient = GradX + GradY;
	return Gradient.Size();
}

float UCBComfyUISceneCaptureComponent::ComputeDepthGradient(const FComfyTexturesImageData& Image, int X, int Y) const
{
	float GradX = 0.0f;
	float GradY = 0.0f;

	// Apply the Sobel operator
	for (int I = -1; I <= 1; I++)
	{
		for (int J = -1; J <= 1; J++)
		{
			int PixelX = FMath::Clamp(X + I, 0, Image.Width - 1);
			int PixelY = FMath::Clamp(Y + J, 0, Image.Height - 1);

			float Value = Image.Pixels[PixelY * Image.Width + PixelX].R; // Assuming depth is stored in the red channel

			GradX += Value * SobelX[I + 1][J + 1];
			GradY += Value * SobelY[I + 1][J + 1];
		}
	}

	return FMath::Sqrt(GradX * GradX + GradY * GradY);	
}

void UCBComfyUISceneCaptureComponent::ProcessSceneTextures(const TSharedPtr<FComfyTexturesCaptureOutput>& Output, int TargetSize,
                                                           TFunction<void()> Callback) const
{
	TargetSize = FMath::RoundUpToPowerOfTwo(TargetSize);

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [this, TargetSize, Output, Callback]()
	{
		double StartTime = FPlatformTime::Seconds();

		FComfyTexturesCaptureOutput& MyImageData=*Output;
	
		  // create the edge mask
        CreateEdgeMask(MyImageData.Depth, MyImageData.Normals, MyImageData.EdgeMask);
        ResizeImage(MyImageData.EdgeMask, TargetSize, TargetSize);

        ResizeImage(MyImageData.Color, TargetSize, TargetSize);
        ResizeImage(MyImageData.Depth, TargetSize, TargetSize);
        ResizeImage(MyImageData.Normals, TargetSize, TargetSize);
		double EndTime = FPlatformTime::Seconds();
		
		UE_LOG(LogCBComfyUI, Display, TEXT("Processed  scene textures  in %f seconds"), EndTime - StartTime);

		// Ensure callback is called on game thread
		AsyncTask(ENamedThreads::GameThread, Callback);
	});
		
}

void UCBComfyUISceneCaptureComponent::ResizeImage(FComfyTexturesImageData& Image, int NewWidth, int NewHeight) const
{
	static TArray<FColor> OldPixels;
	OldPixels.SetNumUninitialized(Image.Width * Image.Height);

	for (int Y = 0; Y < Image.Height; Y++)
	{
		for (int X = 0; X < Image.Width; X++)
		{
			FLinearColor Pixel = Image.Pixels[Y * Image.Width + X];
			Pixel *= 255.0f;
			OldPixels[Y * Image.Width + X] = FColor(Pixel.R, Pixel.G, Pixel.B, Pixel.A);
		}
	}

	TArray<FColor> NewPixels;
	NewPixels.SetNumUninitialized(NewWidth * NewHeight);

	FImageUtils::ImageResize(Image.Width, Image.Height, OldPixels, NewWidth, NewHeight, NewPixels, true, false);

	Image.Width = NewWidth;
	Image.Height = NewHeight;
	Image.Pixels.SetNumUninitialized(NewWidth * NewHeight);

	for (int Y = 0; Y < Image.Height; Y++)
	{
		for (int X = 0; X < Image.Width; X++)
		{
			FColor Pixel = NewPixels[Y * Image.Width + X];
			FLinearColor NewPixel = FLinearColor(Pixel.R, Pixel.G, Pixel.B, Pixel.A);
			NewPixel /= 255.0f;
			Image.Pixels[Y * Image.Width + X] = NewPixel;
		}
	}
}

bool UCBComfyUISceneCaptureComponent::UploadImages(const TArray<FComfyTexturesImageData>& Images,
	const TArray<FString>& FileNames, TFunction<void(const TArray<FString>&, bool)> Callback)
{
	if (Images.Num() != FileNames.Num())
  {
		UE_LOG(LogCBComfyUI, Error, TEXT("Image and filename count do not match"));
		Callback(TArray<FString>(), false);
		OnComfyUISUpLoadImage.Broadcast(false);
		 return false;
  }

  // Shared state for tracking task completion and results
  struct SharedState
  {
		int32 RemainingTasks;
		TArray<FString> ResultFileNames;
		FThreadSafeBool bAllSuccessful = true;
		FCriticalSection Mutex; // Thread safety
  };
	
  TSharedPtr<SharedState> StateData = MakeShared<SharedState>();
  StateData->RemainingTasks = Images.Num();
  StateData->ResultFileNames.AddDefaulted(FileNames.Num());

  for (int32 Index = 0; Index < Images.Num(); ++Index)
  {
    Async(EAsyncExecution::ThreadPool, [this, Image = Images[Index], FileName = FileNames[Index], StateData, Index, Callback]()
      {
        TArray64<uint8> PngData;
        if (!ConvertImageToPng(Image, PngData))
        {
          FScopeLock Lock(&StateData->Mutex);
          StateData->bAllSuccessful = false;
          if (--StateData->RemainingTasks == 0)
          {
            Callback(StateData->ResultFileNames, false);
          }
          return;
        }

      ComfyUIManager->HttpClient->DoHttpFileUpload("upload/image", PngData, FileName, [StateData, Index, Callback](const TSharedPtr<FJsonObject>& Response, bool bWasSuccessful)
          {
            FScopeLock Lock(&StateData->Mutex);
            if (!bWasSuccessful)
            {
              UE_LOG(LogCBComfyUI, Error, TEXT("Failed to upload image"));
              StateData->bAllSuccessful = false;
            }
            else
            {
              FString ResultFileName;
              if (Response->TryGetStringField("name", ResultFileName))
              {
                StateData->ResultFileNames[Index] = ResultFileName;
              }
              else
              {
                UE_LOG(LogCBComfyUI, Error, TEXT("Failed to get image url"));
                StateData->bAllSuccessful = false;
              }
            }

            // Check if this is the last task
            if (--StateData->RemainingTasks == 0)
            {
              Callback(StateData->ResultFileNames, StateData->bAllSuccessful);
            }
          });
      });
  }
	OnComfyUISUpLoadImage.Broadcast(true);
	return true;
}

bool UCBComfyUISceneCaptureComponent::ConvertImageToPng(const FComfyTexturesImageData& Image,
	TArray64<uint8>& OutBytes) const
{
	UE_LOG(LogCBComfyUI, Verbose, TEXT("Converting image to PNG with Width: %d, Height: %d"), Image.Width, Image.Height);

	TArray<FColor> Image8;
	Image8.SetNum(Image.Pixels.Num());
	for (int Index = 0; Index < Image.Pixels.Num(); Index++)
	{
		FLinearColor Pixel = Image.Pixels[Index];
		// convert from linear to sRGB
		Pixel.R = FMath::Pow(Pixel.R, 1.0f / 2.2f);
		Pixel.G = FMath::Pow(Pixel.G, 1.0f / 2.2f);
		Pixel.B = FMath::Pow(Pixel.B, 1.0f / 2.2f);

		FColor OutPixel;
		OutPixel.R = FMath::Clamp(Pixel.R, 0.0f, 1.0f) * 255.0f;
		OutPixel.G = FMath::Clamp(Pixel.G, 0.0f, 1.0f) * 255.0f;
		OutPixel.B = FMath::Clamp(Pixel.B, 0.0f, 1.0f) * 255.0f;
		OutPixel.A = FMath::Clamp(Pixel.A, 0.0f, 1.0f) * 255.0f;
		Image8[Index] = OutPixel;
	}

	FImageUtils::PNGCompressImageArray(Image.Width, Image.Height, Image8, OutBytes);
	return true;
}

bool UCBComfyUISceneCaptureComponent::CalculateApproximateScreenBounds(AActor* Actor, FBox2D& OutBounds) const
{
	if (Actor == nullptr)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Actor is null."));
		return false;
	}

	if (CaptureCamera == nullptr)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("CaptureCamera is null."));
		return false;
	}
	
	FBox ActorBounds = Actor->GetComponentsBoundingBox(true);
	FVector ActorExtent = ActorBounds.GetExtent();

	FVector Corners[8];
	Corners[0] = FVector(ActorExtent.X, ActorExtent.Y, ActorExtent.Z);
	Corners[1] = FVector(ActorExtent.X, ActorExtent.Y, -ActorExtent.Z);
	Corners[2] = FVector(ActorExtent.X, -ActorExtent.Y, ActorExtent.Z);
	Corners[3] = FVector(ActorExtent.X, -ActorExtent.Y, -ActorExtent.Z);
	Corners[4] = FVector(-ActorExtent.X, ActorExtent.Y, ActorExtent.Z);
	Corners[5] = FVector(-ActorExtent.X, ActorExtent.Y, -ActorExtent.Z);
	Corners[6] = FVector(-ActorExtent.X, -ActorExtent.Y, ActorExtent.Z);
	Corners[7] = FVector(-ActorExtent.X, -ActorExtent.Y, -ActorExtent.Z);

	FVector ActorCenter = ActorBounds.GetCenter();
	for (int CornerIndex = 0; CornerIndex < 8; CornerIndex++)
	{
		Corners[CornerIndex] = ActorCenter + Corners[CornerIndex];
	}

	TOptional<FMatrix> CustomProjectionMatrix;
	FMatrix ViewMatrix;
	FMatrix ProjectionMatrix;
	FMatrix ViewProjectionMatrix;
	
	UGameplayStatics::CalculateViewProjectionMatricesFromMinimalView(GetCaptureCameraViewInfo(), CustomProjectionMatrix,
    ViewMatrix, ProjectionMatrix, ViewProjectionMatrix);

  // project the corners onto the screen
	FVector2D ScreenCorners[8];
	for (int CornerIndex = 0; CornerIndex < 8; CornerIndex++)
	{
		FVector4 HomogeneousPosition = ViewProjectionMatrix.TransformFVector4(FVector4(Corners[CornerIndex], 1.0f));
		ScreenCorners[CornerIndex] = FVector2D(HomogeneousPosition.X / HomogeneousPosition.W, HomogeneousPosition.Y / HomogeneousPosition.W);
	}

	// find the min and max screen coordinates
	float MinX = FLT_MAX;
	float MinY = FLT_MAX;
	float MaxX = -FLT_MAX;
	float MaxY = -FLT_MAX;
	
	for (int CornerIndex = 0; CornerIndex < 8; CornerIndex++)
  {
    MinX = FMath::Min(MinX, ScreenCorners[CornerIndex].X);
    MinY = FMath::Min(MinY, ScreenCorners[CornerIndex].Y);
    MaxX = FMath::Max(MaxX, ScreenCorners[CornerIndex].X);
    MaxY = FMath::Max(MaxY, ScreenCorners[CornerIndex].Y);
  }

  // clamp the screen coordinates to the screen bounds
  MinX = FMath::Clamp(MinX, -1.0f, 1.0f);
  MinY = FMath::Clamp(MinY, -1.0f, 1.0f);
  MaxX = FMath::Clamp(MaxX, -1.0f, 1.0f);
  MaxY = FMath::Clamp(MaxY, -1.0f, 1.0f);

  FVector2D Min = FVector2D(MinX, MinY);
  FVector2D Max = FVector2D(MaxX, MaxY);

  // convert from [-1, 1] to [0, 1]
  Min = Min * 0.5f + FVector2D(0.5f, 0.5f);
  Max = Max * 0.5f + FVector2D(0.5f, 0.5f);

  OutBounds = FBox2D(Min, Max);
  return true;
}

FMinimalViewInfo UCBComfyUISceneCaptureComponent::GetCaptureCameraViewInfo() const
{
	FMinimalViewInfo ViewInfo;
	
	if(!CaptureCamera)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Existing camera is null."));
		return ViewInfo;
	}
	

	UCameraComponent* CameraComponent = CaptureCamera->FindComponentByClass<UCameraComponent>();
	if (CameraComponent == nullptr)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Camera component not found."));
		return ViewInfo;
	}
	
	CameraComponent->GetCameraView(0.0f, ViewInfo);

	if (!FMath::IsNearlyEqual(ViewInfo.AspectRatio, 1.0f))
	{
		UE_LOG(LogCBComfyUI, Warning, TEXT("Camera aspect ratio is not 1.0, overriding it."));
		ViewInfo.AspectRatio = 1.0f;
	}
	
	return ViewInfo;
}

UTexture2D* UCBComfyUISceneCaptureComponent::CreateTexture2D(int Width, int Height, const TArray<FColor>& Pixels) const
{
#if WITH_EDITOR
	if (Pixels.Num() != Width * Height)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Pixels.Num() != Width * Height"));
		return nullptr;
	}

	UTexture2D* Texture2D = NewObject<UTexture2D>(UTexture2D::StaticClass());
	Texture2D->SetFlags(RF_Transactional);
	Texture2D->SRGB = true;
	Texture2D->bPreserveBorder = true;
	Texture2D->Filter = TF_Trilinear;
	Texture2D->AddToRoot();

	FTexturePlatformData* PlatformData = new FTexturePlatformData();
	PlatformData->SizeX = Width;
	PlatformData->SizeY = Height;
	PlatformData->PixelFormat = PF_B8G8R8A8;

	FTexture2DMipMap* Mip = new FTexture2DMipMap();
	Mip->SizeX = Width;
	Mip->SizeY = Height;
	Mip->BulkData.Lock(LOCK_READ_WRITE);
	int NumBytes = Width * Height * 4;
	Mip->BulkData.Realloc(NumBytes);
	Mip->BulkData.Unlock();

	void* TextureData = Mip->BulkData.Lock(LOCK_READ_WRITE);
	FMemory::Memcpy(TextureData, Pixels.GetData(), NumBytes);
	Mip->BulkData.Unlock();

	PlatformData->Mips.Add(Mip);

	Texture2D->SetPlatformData(PlatformData);
	Texture2D->UpdateResource();

	Texture2D->Source.Init(Width, Height, 1, PlatformData->Mips.Num(), ETextureSourceFormat::TSF_BGRA8, nullptr);

	// copy all the mip data

	for (int MipIndex = 0; MipIndex < PlatformData->Mips.Num(); MipIndex++)
	{
		Mip = &PlatformData->Mips[MipIndex];

		void* MipData = Texture2D->Source.LockMip(MipIndex);
		FMemory::Memcpy(MipData, Mip->BulkData.Lock(LOCK_READ_ONLY), Mip->BulkData.GetBulkDataSize());
		Mip->BulkData.Unlock();
		Texture2D->Source.UnlockMip(MipIndex);
	}

	return Texture2D;
#else
	// 在Runtime模式下使用CreateTexture2DRuntime
	return CreateTexture2DRuntime(Width, Height, Pixels);
#endif
}

UTexture2D* UCBComfyUISceneCaptureComponent::CreateTexture2DRuntime(int Width, int Height, const TArray<FColor>& Pixels) const
{
	if (Pixels.Num() != Width * Height)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Pixels.Num() != Width * Height"));
		return nullptr;
	}

	// 确保在游戏线程中创建纹理
	if (!IsInGameThread())
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("CreateTexture2DRuntime must be called from game thread"));
		return nullptr;
	}

	// UE5.4版本的动态纹理创建方法
	UTexture2D* Texture2D = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture2D)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Failed to create transient texture"));
		return nullptr;
	}

	// 设置纹理属性
	Texture2D->SRGB = true;
	Texture2D->Filter = TF_Trilinear;
	Texture2D->NeverStream = true;
	Texture2D->bNotOfflineProcessed = true;
	
	// 添加到Root以防止垃圾回收
	Texture2D->AddToRoot();

	// 等待纹理资源初始化
	if (!Texture2D->GetResource())
	{
		Texture2D->UpdateResource();
		FlushRenderingCommands(); // 等待GPU命令完成
	}

	// 将FColor数组转换为uint8数组
	TArray<uint8> ByteArray;
	ByteArray.SetNumUninitialized(Width * Height * 4);
	
	for (int32 i = 0; i < Pixels.Num(); ++i)
	{
		ByteArray[i * 4 + 0] = Pixels[i].B;
		ByteArray[i * 4 + 1] = Pixels[i].G;
		ByteArray[i * 4 + 2] = Pixels[i].R;
		ByteArray[i * 4 + 3] = Pixels[i].A;
	}

	// 使用渲染线程安全的方式更新纹理
	FUpdateTextureRegion2D* UpdateRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);
	uint8* RawData = (uint8*)FMemory::Malloc(ByteArray.Num());
	FMemory::Memcpy(RawData, ByteArray.GetData(), ByteArray.Num());

	Texture2D->UpdateTextureRegions(0, 1, UpdateRegion, Width * 4, 4, RawData,
		[](uint8* SrcData, const FUpdateTextureRegion2D* Regions)
		{
			if (SrcData)
			{
				FMemory::Free(SrcData);
			}

			delete Regions;
		});

	// 等待渲染命令完成
	FlushRenderingCommands();

	UE_LOG(LogCBComfyUI, Log, TEXT("Successfully created runtime texture %dx%d"), Width, Height);
	return Texture2D;
}


static void RasterizeTriangle(FVector2D V0, FVector2D V1, FVector2D V2, int Width, int Height, TFunction<void(int, int, const FVector&)> Callback)
{
	FVector2D Size(Width - 1, Height - 1);
	V0 *= Size;
	V1 *= Size;
	V2 *= Size;

	// Bounding box
	int MinX = FMath::FloorToInt(FMath::Max(FMath::Min3(V0.X, V1.X, V2.X), 0));
	int MinY = FMath::FloorToInt(FMath::Max(FMath::Min3(V0.Y, V1.Y, V2.Y), 0));
	int MaxX = FMath::CeilToInt(FMath::Min(FMath::Max3(V0.X, V1.X, V2.X), Width - 1));
	int MaxY = FMath::CeilToInt(FMath::Min(FMath::Max3(V0.Y, V1.Y, V2.Y), Height - 1));

	for (int Y = MinY; Y <= MaxY; Y++)
	{
		for (int X = MinX; X <= MaxX; X++)
		{
			FVector Barycentric = FMath::GetBaryCentric2D(FVector2D(X, Y), V0, V1, V2);
			if (Barycentric.X < 0 || Barycentric.Y < 0 || Barycentric.Z < 0)
			{
				continue;
			}

			Callback(X, Y, Barycentric);
		}
	}
}

static FLinearColor SampleBilinear(const TArray<FColor>& Pixels, int Width, int Height, FVector2D Uv)
{
	// clamp the UV coordinates
	Uv.X = FMath::Clamp(Uv.X, 0.0f, 1.0f);
	Uv.Y = FMath::Clamp(Uv.Y, 0.0f, 1.0f);

	// calculate the pixel coordinates
	float PixelX = Uv.X * (Width - 1);
	float PixelY = Uv.Y * (Height - 1);

	// calculate the pixel coordinates of the four surrounding pixels
	int PixelX0 = FMath::FloorToInt(PixelX);
	int PixelY0 = FMath::FloorToInt(PixelY);
	int PixelX1 = FMath::CeilToInt(PixelX);
	int PixelY1 = FMath::CeilToInt(PixelY);

	// calculate the pixel weights
	float WeightX = PixelX - PixelX0;
	float WeightY = PixelY - PixelY0;

	// sample the four surrounding pixels
	const FColor& Pixel00 = Pixels[PixelY0 * Width + PixelX0];
	const FColor& Pixel01 = Pixels[PixelY0 * Width + PixelX1];
	const FColor& Pixel10 = Pixels[PixelY1 * Width + PixelX0];
	const FColor& Pixel11 = Pixels[PixelY1 * Width + PixelX1];

	FLinearColor Pixel00L = FLinearColor(Pixel00.R, Pixel00.G, Pixel00.B, Pixel00.A) * (1.0f / 255.0f);
	FLinearColor Pixel01L = FLinearColor(Pixel01.R, Pixel01.G, Pixel01.B, Pixel01.A) * (1.0f / 255.0f);
	FLinearColor Pixel10L = FLinearColor(Pixel10.R, Pixel10.G, Pixel10.B, Pixel10.A) * (1.0f / 255.0f);
	FLinearColor Pixel11L = FLinearColor(Pixel11.R, Pixel11.G, Pixel11.B, Pixel11.A) * (1.0f / 255.0f);

	// interpolate the pixels
	FLinearColor Pixel0 = FMath::Lerp(Pixel00L, Pixel01L, WeightX);
	FLinearColor Pixel1 = FMath::Lerp(Pixel10L, Pixel11L, WeightX);
	FLinearColor Pixel = FMath::Lerp(Pixel0, Pixel1, WeightY);

	return Pixel;
}

static void ExpandTextureIslands(TArray<FColor>& Pixels, int Width, int Height, int Iterations)
{
  for (int Iteration = 0; Iteration < Iterations; Iteration++)
  {
    TArray<FColor> PixelsCopy = Pixels;

    for (int Y = 0; Y < Height; Y++)
    {
      for (int X = 0; X < Width; X++)
      {
        int Index = Y * Width + X;
        FColor Pixel = PixelsCopy[Index];
        if (Pixel.A > 0)
        {
          continue;
        }

        int NeighborCount = 0;
        FLinearColor NeighborColorSum(0.0f, 0.0f, 0.0f, 0.0f);

        // check the pixel's neighbors

        if (X > 0)
        {
          int NeighborIndex = Y * Width + (X - 1);
          FColor& NeighborPixel = PixelsCopy[NeighborIndex];
          if (NeighborPixel.A > 0)
          {
            NeighborCount++;
            NeighborColorSum += NeighborPixel.ReinterpretAsLinear();
          }
        }

        if (X < Width - 1)
        {
          int NeighborIndex = Y * Width + (X + 1);
          FColor& NeighborPixel = PixelsCopy[NeighborIndex];
          if (NeighborPixel.A > 0)
          {
            NeighborCount++;
            NeighborColorSum += NeighborPixel.ReinterpretAsLinear();
          }
        }

        if (Y > 0)
        {
          int NeighborIndex = (Y - 1) * Width + X;
          FColor& NeighborPixel = PixelsCopy[NeighborIndex];
          if (NeighborPixel.A > 0)
          {
            NeighborCount++;
            NeighborColorSum += NeighborPixel.ReinterpretAsLinear();
          }
        }

        if (Y < Height - 1)
        {
          int NeighborIndex = (Y + 1) * Width + X;
          FColor& NeighborPixel = PixelsCopy[NeighborIndex];
          if (NeighborPixel.A > 0)
          {
            NeighborCount++;
            NeighborColorSum += NeighborPixel.ReinterpretAsLinear();
          }
        }

        if (NeighborCount > 0)
        {
          Pixel.R = (NeighborColorSum.R / NeighborCount) * 255.0f;
          Pixel.G = (NeighborColorSum.G / NeighborCount) * 255.0f;
          Pixel.B = (NeighborColorSum.B / NeighborCount) * 255.0f;
          Pixel.A = 0;
          Pixels[Index] = Pixel;
        }
      }
    }
  }
}

bool UCBComfyUISceneCaptureComponent::ProcessRenderResultForActor(AActor* Actor, TFunction<void(bool)> Callback)
{
#if WITH_EDITOR
	const FTransform& ActorTransform = Actor->GetActorTransform();
	
  UStaticMeshComponent* StaticMeshComponent = Actor->FindComponentByClass<UStaticMeshComponent>();
	
  if (StaticMeshComponent == nullptr)
  {
    return false;
  }
  UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
	
  if (StaticMesh == nullptr)
  {
    UE_LOG(LogCBComfyUI, Warning, TEXT("Static mesh is null for actor %s."), *Actor->GetName());
    return false;
  }
	
  UMaterialInterface* Material = StaticMeshComponent->GetMaterial(0);
	
  if (Material == nullptr)
  {
    UE_LOG(LogCBComfyUI, Warning, TEXT("Material is null for actor %s."), *Actor->GetName());
    return false;
  }

  UTexture2D* Texture2D = nullptr;
	
  UTexture* Texture = nullptr;
  if (!Material->GetTextureParameterValue(TEXT("BaseColor"), Texture))
  {
    UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to get parameter value \"BaseColor\" for actor %s."), *Actor->GetName());
    return false;
  }
	
  if (Texture == nullptr)
  {
    UE_LOG(LogCBComfyUI, Warning, TEXT("Texture is null for actor %s."), *Actor->GetName());
    return false;
  }

  Texture2D = Cast<UTexture2D>(Texture);
  if (Texture2D == nullptr)
  {
    UE_LOG(LogCBComfyUI, Warning, TEXT("Failed to create texture for actor %s."), *Actor->GetName());
    return false;
  }

  if (Texture2D->Source.GetNumMips() <= 0)
  {
    UE_LOG(LogCBComfyUI, Warning, TEXT("No mipmaps available in texture for actor %s."), *Actor->GetName());
    return false;
  }

  int TextureWidth = Texture2D->GetSizeX();
  int TextureHeight = Texture2D->GetSizeY();

  const FStaticMeshLODResources& MeshLod = StaticMesh->GetLODForExport(0);

  struct SharedData
  {
    TArray<uint32> Indices;
    TArray<FVector> Vertices;
    TArray<FVector2D> Uvs;
    TSharedPtr<TArray<FColor>> Pixels;
    FComfyTexturesRenderDataPtr RenderData;
    FTransform ActorTransform;
    UTexture2D* Texture2D;
    AActor* Actor;
    int TextureWidth;
    int TextureHeight;
  };
	
	TSharedPtr<SharedData> StateData = MakeShared<SharedData>();
	StateData->RenderData =TexturesRenderData;
	StateData->ActorTransform = ActorTransform;
	StateData->TextureWidth = TextureWidth;
	StateData->TextureHeight = TextureHeight;
	StateData->Texture2D = Texture2D;
	StateData->Actor = Actor;
	StateData->Pixels = MakeShared<TArray<FColor>>();
	StateData->Pixels->SetNumZeroed(TextureWidth * TextureHeight);

	// 在渲染线程中安全地访问渲染资源
	ENQUEUE_RENDER_COMMAND(ReadMeshData)(
		[StateData, StaticMesh](FRHICommandListImmediate& RHICmdList)
		{
			// 获取渲染数据
			const FStaticMeshLODResources& MeshLod = StaticMesh->GetRenderData()->LODResources[0];
			
			// 复制索引数据
			MeshLod.IndexBuffer.GetCopy(StateData->Indices);

			// 获取顶点数据
			int VertexCount = MeshLod.VertexBuffers.PositionVertexBuffer.GetNumVertices();
			StateData->Vertices.SetNumUninitialized(VertexCount);
			StateData->Uvs.SetNumUninitialized(VertexCount);

			for (int32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
			{
				FVector Vertex = (FVector)MeshLod.VertexBuffers.PositionVertexBuffer.VertexPosition(VertexIndex);
				StateData->Vertices[VertexIndex] = Vertex;

				FVector2D Uv = FVector2D::ZeroVector;
				const FStaticMeshVertexBuffer& VertexBuffer = MeshLod.VertexBuffers.StaticMeshVertexBuffer;
				if (VertexBuffer.GetNumTexCoords() > 0 && (uint32)VertexIndex < VertexBuffer.GetNumVertices())
				{
					// 使用更直接的方法避免GetUseFullPrecisionUVs调用
					try
					{
						// 直接尝试获取默认精度UV
						FVector2f UvFloat = VertexBuffer.GetVertexUV_Typed<EStaticMeshVertexUVType::Default>(VertexIndex, 0);
						Uv = FVector2D(UvFloat.X, UvFloat.Y);
					}
					catch (...)
					{
						// 如果失败，使用零向量
						Uv = FVector2D::ZeroVector;
					}
				}
				StateData->Uvs[VertexIndex] = Uv;
			}
		});

	// 等待渲染命令完成
	FlushRenderingCommands();

  AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [StateData, Callback]()
    {
      const FMinimalViewInfo& ViewInfo = StateData->RenderData->ViewInfo;
      const FMatrix& ViewMatrix = StateData->RenderData->ViewMatrix;
      const FMatrix& ProjectionMatrix = StateData->RenderData->ProjectionMatrix;

      FMatrix ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;
      FIntRect ViewRect(0, 0, StateData->RenderData->OutputWidth, StateData->RenderData->OutputHeight);

      // Iterate over the faces
      for (int32 FaceIndex = 0; FaceIndex < StateData->Indices.Num(); FaceIndex += 3)
      {
      	// 每个面由 3 个索引表示
        uint32 Index0 = StateData->Indices[FaceIndex];
        uint32 Index1 = StateData->Indices[FaceIndex + 1];
        uint32 Index2 = StateData->Indices[FaceIndex + 2];

      	// 获取脸部的顶点
        const FVector& Vertex0 = StateData->Vertices[Index0];
        const FVector& Vertex1 = StateData->Vertices[Index1];
        const FVector& Vertex2 = StateData->Vertices[Index2];

        FVector FaceNormal = -FVector::CrossProduct(Vertex1 - Vertex0, Vertex2 - Vertex0).GetSafeNormal();
        FVector FaceNormalWorld = StateData->ActorTransform.TransformVector(FaceNormal);

        float FaceDot = 0.0f;
        if (ViewInfo.ProjectionMode == ECameraProjectionMode::Perspective)
        {
          FVector Vertex0World = StateData->ActorTransform.TransformPosition(Vertex0);
          FaceDot = FVector::DotProduct(FaceNormalWorld, (ViewInfo.Location - Vertex0World).GetSafeNormal());
        }
        else if (ViewInfo.ProjectionMode == ECameraProjectionMode::Orthographic)
        {
        	// 获取 viewinfo 的前向向量
          FVector Forward = ViewInfo.Rotation.Vector();
          FaceDot = FVector::DotProduct(FaceNormalWorld, -Forward);
        }

        if (FaceDot <= 0.0f)
        {
          continue;
        }

      	// 获取脸部的 UV
        const FVector2D& Uv0 = StateData->Uvs[Index0];
        const FVector2D& Uv1 = StateData->Uvs[Index1];
        const FVector2D& Uv2 = StateData->Uvs[Index2];

        RasterizeTriangle(Uv0, Uv1, Uv2, StateData->TextureWidth, StateData->TextureHeight, [&](int X, int Y, const FVector& Barycentric)
          {
            int PixelIndex = X + Y * StateData->TextureWidth;
            if (PixelIndex < 0 || PixelIndex >= StateData->Pixels->Num())
            {
              return;
            }
        	

        	// 找到像素的局部位置
            FVector LocalPosition = Barycentric.X * Vertex0 + Barycentric.Y * Vertex1 + Barycentric.Z * Vertex2;
            FVector WorldPosition = StateData->ActorTransform.TransformPosition(LocalPosition);

        	// 将世界位置投影到屏幕空间
            FPlane Result = ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));
            if (Result.W <= 0.0f)
            {
              return;
            }

        	// 其结果将是 -1..1 投影空间中的 x 和 y 坐标
            const float Rhw = 1.0f / Result.W;
            FPlane PosInScreenSpace = FPlane(Result.X * Rhw, Result.Y * Rhw, Result.Z * Rhw, Result.W);

        	// 从投影空间移动到标准化 0..1 UI 空间
            FVector2D Uv
            (
              (PosInScreenSpace.X / 2.f) + 0.5f,
              1.f - (PosInScreenSpace.Y / 2.f) - 0.5f
            );

            if (Uv.X < 0.0f || Uv.X > 1.0f || Uv.Y < 0.0f || Uv.Y > 1.0f)
            {
              return;
            }

            const FComfyTexturesImageData& RawDepth = StateData->RenderData->RawDepth;

        	// 计算像素坐标
            int PixelX = FMath::FloorToInt(Uv.X * (RawDepth.Width - 1));
            int PixelY = FMath::FloorToInt(Uv.Y * (RawDepth.Height - 1));

            float ClosestDepth = RawDepth.Pixels[PixelX + PixelY * RawDepth.Width].R;

            if (ViewInfo.ProjectionMode == ECameraProjectionMode::Perspective)
            {
              FVector ViewSpacePoint = ViewMatrix.TransformPosition(WorldPosition);

              const float Eps = 5.0f;
              if (ViewSpacePoint.Z > ClosestDepth + Eps)
              {
                return;
              }
            }
            else
            {
              FVector4 ClipSpacePoint = ProjectionMatrix.TransformFVector4(FVector4(0.0f, 0.0f, ClosestDepth, 1.0f));
              float ClipSpaceDepth = ClipSpacePoint.Z / ClipSpacePoint.W;
              const float Eps = 0.01f;
              if (PosInScreenSpace.Z < ClipSpaceDepth - Eps)
              {
                return;
              }
            }

        	// 从输入纹理获取像素颜色
            FLinearColor Pixel = SampleBilinear(StateData->RenderData->OutputPixels, StateData->RenderData->OutputWidth,
              StateData->RenderData->OutputHeight, Uv);
            Pixel.A = FMath::Clamp(FMath::Abs(FaceDot), 0.0f, 1.0f);
            Pixel *= 255.0f;
            (*StateData->Pixels)[PixelIndex] = FColor(Pixel.R, Pixel.G, Pixel.B, Pixel.A);
          });
      }

      ExpandTextureIslands(*StateData->Pixels, StateData->TextureWidth, StateData->TextureHeight, 4);

      AsyncTask(ENamedThreads::GameThread, [StateData, Callback]()
        {
          UKismetSystemLibrary::TransactObject(StateData->Texture2D);

          FColor* MipData = (FColor*)StateData->Texture2D->Source.LockMip(0);
          if (MipData == nullptr)
          {
            UE_LOG(LogCBComfyUI, Error, TEXT("Failed to lock mip 0 for texture %s."), *(StateData->Texture2D->GetName()));
            Callback(false);
            return false;
          }

          FMemory::Memcpy(MipData, StateData->Pixels->GetData(), StateData->Pixels->Num() * sizeof(FColor));

          StateData->Texture2D->Source.UnlockMip(0);
          StateData->Texture2D->MarkPackageDirty();
          StateData->Texture2D->UpdateResource();

          Callback(true);
          return true;
        });

      return true;
    });

  return true;
#else
	// Runtime模式下使用ProcessRenderResultForActorRuntime
	return ProcessRenderResultForActorRuntime(Actor, Callback);
#endif
}

bool UCBComfyUISceneCaptureComponent::ProcessRenderResultForActorRuntime(AActor* Actor, TFunction<void(bool)> Callback)
{
	const FTransform& ActorTransform = Actor->GetActorTransform();
	
	UStaticMeshComponent* StaticMeshComponent = Actor->FindComponentByClass<UStaticMeshComponent>();
	
	if (StaticMeshComponent == nullptr)
	{
		return false;
	}
	
	UStaticMesh* StaticMesh = StaticMeshComponent->GetStaticMesh();
	
	if (StaticMesh == nullptr)
	{
		UE_LOG(LogCBComfyUI, Warning, TEXT("Static mesh is null for actor %s."), *Actor->GetName());
		return false;
	}
	
	// 使用映射表查找对应的Runtime材质和纹理
	TWeakObjectPtr<UMaterialInstanceDynamic>* MaterialPtr = ActorToMaterialMap.Find(Actor);
	TWeakObjectPtr<UTexture2D>* TexturePtr = ActorToTextureMap.Find(Actor);
	
	if (!MaterialPtr || !MaterialPtr->IsValid())
	{
		UE_LOG(LogCBComfyUI, Warning, TEXT("Runtime material not found in mapping for actor %s."), *Actor->GetName());
		return false;
	}
	
	if (!TexturePtr || !TexturePtr->IsValid())
	{
		UE_LOG(LogCBComfyUI, Warning, TEXT("Runtime texture not found in mapping for actor %s."), *Actor->GetName());
		return false;
	}
	
	UMaterialInstanceDynamic* DynamicMaterial = MaterialPtr->Get();
	UTexture2D* Texture2D = TexturePtr->Get();
	
	// 验证对象有效性
	if (!DynamicMaterial || !IsValid(DynamicMaterial))
	{
		UE_LOG(LogCBComfyUI, Warning, TEXT("Dynamic material is invalid for actor %s."), *Actor->GetName());
		return false;
	}
	
	if (!Texture2D || !IsValid(Texture2D))
	{
		UE_LOG(LogCBComfyUI, Warning, TEXT("Texture2D is invalid for actor %s."), *Actor->GetName());
		return false;
	}
	
	// 额外的安全检查：验证纹理资源状态
	if (!Texture2D->GetResource() || !IsValidRef(Texture2D->GetResource()->TextureRHI))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Texture resource is invalid for actor %s."), *Actor->GetName());
		return false;
	}

	int TextureWidth = Texture2D->GetSizeX();
	int TextureHeight = Texture2D->GetSizeY();

	FString MeshDataFailureReason;
	if (!HasReadableRuntimeMeshData(StaticMesh, MeshDataFailureReason))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("%s Actor=%s"), *MeshDataFailureReason, *Actor->GetName());
		return false;
	}

	struct SharedDataRuntime
	{
		TArray<uint32> Indices;
		TArray<FVector> Vertices;
		TArray<FVector2D> Uvs;
		TSharedPtr<TArray<FColor>> Pixels;
		FComfyTexturesRenderDataPtr RenderData;
		FTransform ActorTransform;
		UTexture2D* Texture2D;
		UMaterialInstanceDynamic* DynamicMaterial;
		AActor* Actor;
		int TextureWidth;
		int TextureHeight;
	};
	
	TSharedPtr<SharedDataRuntime> StateData = MakeShared<SharedDataRuntime>();
	StateData->RenderData = TexturesRenderData;
	StateData->ActorTransform = ActorTransform;
	StateData->TextureWidth = TextureWidth;
	StateData->TextureHeight = TextureHeight;
	StateData->Texture2D = Texture2D;
	StateData->DynamicMaterial = DynamicMaterial;
	StateData->Actor = Actor;
	StateData->Pixels = MakeShared<TArray<FColor>>();
	StateData->Pixels->SetNumZeroed(TextureWidth * TextureHeight);

	// 在渲染线程中安全地访问渲染资源 - Runtime版本
	ENQUEUE_RENDER_COMMAND(ReadMeshDataRuntime)(
		[StateData, StaticMesh](FRHICommandListImmediate& RHICmdList)
		{
			// 获取渲染数据
			const FStaticMeshLODResources& MeshLod = StaticMesh->GetRenderData()->LODResources[0];
			const FPositionVertexBuffer& PositionBuffer = MeshLod.VertexBuffers.PositionVertexBuffer;
			const FStaticMeshVertexBuffer& VertexBuffer = MeshLod.VertexBuffers.StaticMeshVertexBuffer;
			
			// 复制索引数据
			MeshLod.IndexBuffer.GetCopy(StateData->Indices);

			// 获取顶点数据
			int VertexCount = PositionBuffer.GetNumVertices();
			StateData->Vertices.SetNumUninitialized(VertexCount);
			StateData->Uvs.SetNumUninitialized(VertexCount);

			for (int32 VertexIndex = 0; VertexIndex < VertexCount; VertexIndex++)
			{
				FVector Vertex = (FVector)PositionBuffer.VertexPosition(VertexIndex);
				StateData->Vertices[VertexIndex] = Vertex;

				FVector2D Uv = GetRuntimeMeshUv(VertexBuffer, static_cast<uint32>(VertexIndex));
				StateData->Uvs[VertexIndex] = Uv;
			}
		});

	// 等待渲染命令完成
	FlushRenderingCommands();

	AsyncTask(ENamedThreads::AnyBackgroundThreadNormalTask, [=, this]()
	{
		const FMinimalViewInfo& ViewInfo = StateData->RenderData->ViewInfo;
		const FMatrix& ViewMatrix = StateData->RenderData->ViewMatrix;
		const FMatrix& ProjectionMatrix = StateData->RenderData->ProjectionMatrix;

		FMatrix ViewProjectionMatrix = ViewMatrix * ProjectionMatrix;

		// 处理面数据...（与编辑器版本相同的逻辑）
		for (int32 FaceIndex = 0; FaceIndex < StateData->Indices.Num(); FaceIndex += 3)
		{
			uint32 Index0 = StateData->Indices[FaceIndex];
			uint32 Index1 = StateData->Indices[FaceIndex + 1];
			uint32 Index2 = StateData->Indices[FaceIndex + 2];

			const FVector& Vertex0 = StateData->Vertices[Index0];
			const FVector& Vertex1 = StateData->Vertices[Index1];
			const FVector& Vertex2 = StateData->Vertices[Index2];

			FVector FaceNormal = -FVector::CrossProduct(Vertex1 - Vertex0, Vertex2 - Vertex0).GetSafeNormal();
			FVector FaceNormalWorld = StateData->ActorTransform.TransformVector(FaceNormal);

			float FaceDot = 0.0f;
			if (ViewInfo.ProjectionMode == ECameraProjectionMode::Perspective)
			{
				FVector Vertex0World = StateData->ActorTransform.TransformPosition(Vertex0);
				FaceDot = FVector::DotProduct(FaceNormalWorld, (ViewInfo.Location - Vertex0World).GetSafeNormal());
			}
			else if (ViewInfo.ProjectionMode == ECameraProjectionMode::Orthographic)
			{
				FVector Forward = ViewInfo.Rotation.Vector();
				FaceDot = FVector::DotProduct(FaceNormalWorld, -Forward);
			}

			if (FaceDot <= 0.0f)
			{
				continue;
			}

			const FVector2D& Uv0 = StateData->Uvs[Index0];
			const FVector2D& Uv1 = StateData->Uvs[Index1];
			const FVector2D& Uv2 = StateData->Uvs[Index2];

			RasterizeTriangle(Uv0, Uv1, Uv2, StateData->TextureWidth, StateData->TextureHeight, [&](int X, int Y, const FVector& Barycentric)
			{
				int PixelIndex = X + Y * StateData->TextureWidth;
				if (PixelIndex < 0 || PixelIndex >= StateData->Pixels->Num())
				{
					return;
				}

				FVector LocalPosition = Barycentric.X * Vertex0 + Barycentric.Y * Vertex1 + Barycentric.Z * Vertex2;
				FVector WorldPosition = StateData->ActorTransform.TransformPosition(LocalPosition);

				FPlane Result = ViewProjectionMatrix.TransformFVector4(FVector4(WorldPosition, 1.f));
				if (Result.W <= 0.0f)
				{
					return;
				}

				const float Rhw = 1.0f / Result.W;
				FPlane PosInScreenSpace = FPlane(Result.X * Rhw, Result.Y * Rhw, Result.Z * Rhw, Result.W);

				FVector2D Uv
				(
					(PosInScreenSpace.X / 2.f) + 0.5f,
					1.f - (PosInScreenSpace.Y / 2.f) - 0.5f
				);

				if (Uv.X < 0.0f || Uv.X > 1.0f || Uv.Y < 0.0f || Uv.Y > 1.0f)
				{
					return;
				}

				const FComfyTexturesImageData& RawDepth = StateData->RenderData->RawDepth;

				int PixelX = FMath::FloorToInt(Uv.X * (RawDepth.Width - 1));
				int PixelY = FMath::FloorToInt(Uv.Y * (RawDepth.Height - 1));

				float ClosestDepth = RawDepth.Pixels[PixelX + PixelY * RawDepth.Width].R;

				if (ViewInfo.ProjectionMode == ECameraProjectionMode::Perspective)
				{
					FVector ViewSpacePoint = ViewMatrix.TransformPosition(WorldPosition);

					const float Eps = 5.0f;
					if (ViewSpacePoint.Z > ClosestDepth + Eps)
					{
						return;
					}
				}
				else
				{
					FVector4 ClipSpacePoint = ProjectionMatrix.TransformFVector4(FVector4(0.0f, 0.0f, ClosestDepth, 1.0f));
					float ClipSpaceDepth = ClipSpacePoint.Z / ClipSpacePoint.W;
					const float Eps = 0.01f;
					if (PosInScreenSpace.Z < ClipSpaceDepth - Eps)
					{
						return;
					}
				}

				FLinearColor Pixel = SampleBilinear(StateData->RenderData->OutputPixels, StateData->RenderData->OutputWidth,
					StateData->RenderData->OutputHeight, Uv);
				Pixel.A = FMath::Clamp(FMath::Abs(FaceDot), 0.0f, 1.0f);
				Pixel *= 255.0f;
				(*StateData->Pixels)[PixelIndex] = FColor(Pixel.R, Pixel.G, Pixel.B, Pixel.A);
			});
		}

		ExpandTextureIslands(*StateData->Pixels, StateData->TextureWidth, StateData->TextureHeight, 4);

		// Runtime版本：使用更安全的纹理更新方法
		AsyncTask(ENamedThreads::GameThread, [=, this]()
		{
			// 验证纹理对象的有效性
			if (!StateData->Texture2D || !IsValid(StateData->Texture2D))
			{
				UE_LOG(LogCBComfyUI, Error, TEXT("Texture2D is invalid when updating"));
				Callback(false);
				return;
			}

			// 验证纹理是否仍在RuntimeTextures数组中
			bool bTextureStillValid = false;
			for (UTexture2D* RuntimeTex : RuntimeTextures)
			{
				if (RuntimeTex == StateData->Texture2D && IsValid(RuntimeTex))
				{
					bTextureStillValid = true;
					break;
				}
			}
			
			if (!bTextureStillValid)
			{
				UE_LOG(LogCBComfyUI, Error, TEXT("Texture2D no longer in RuntimeTextures array"));
				Callback(false);
				return;
			}

			// 验证纹理资源
			if (!StateData->Texture2D->GetResource() || !IsValidRef(StateData->Texture2D->GetResource()->TextureRHI))
			{
				UE_LOG(LogCBComfyUI, Error, TEXT("Texture2D resource is null or invalid"));
				Callback(false);
				return;
			}

			// 验证动态材质是否仍在RuntimeMaterials数组中
			bool bMaterialStillValid = false;
			for (UMaterialInstanceDynamic* RuntimeMat : RuntimeMaterials)
			{
				if (RuntimeMat == StateData->DynamicMaterial && IsValid(RuntimeMat))
				{
					bMaterialStillValid = true;
					break;
				}
			}
			
			if (!bMaterialStillValid)
			{
				UE_LOG(LogCBComfyUI, Error, TEXT("Dynamic material no longer in RuntimeMaterials array"));
				Callback(false);
				return;
			}

			// 使用更安全的纹理数据创建方法
			TArray<FColor> NewPixelData = *StateData->Pixels;
			
			// 创建新的纹理来替换旧的
			UTexture2D* NewTexture = CreateTexture2DRuntime(StateData->TextureWidth, StateData->TextureHeight, NewPixelData);
			if (!NewTexture)
			{
				UE_LOG(LogCBComfyUI, Error, TEXT("Failed to create new runtime texture"));
				Callback(false);
				return;
			}
			
			// 在RuntimeTextures数组中查找并替换旧纹理
			for (int32 i = 0; i < RuntimeTextures.Num(); ++i)
			{
				if (RuntimeTextures[i] == StateData->Texture2D)
				{
					if (RuntimeTextures[i] && IsValid(RuntimeTextures[i]))
					{
						RetiredRuntimeTextures.Add(RuntimeTextures[i]);
					}

					// 替换为新纹理
					RuntimeTextures[i] = NewTexture;
					break;
				}
			}
			
			// 更新映射表
			ActorToTextureMap.Add(StateData->Actor, NewTexture);
			
			// 更新动态材质的纹理参数
			if (StateData->DynamicMaterial && IsValid(StateData->DynamicMaterial))
			{
				StateData->DynamicMaterial->SetTextureParameterValue(TEXT("BaseColor"), NewTexture);
				StateData->DynamicMaterial->RecacheUniformExpressions(true);
			}

			if (UStaticMeshComponent* StaticMeshComponent = StateData->Actor->FindComponentByClass<UStaticMeshComponent>())
			{
				StaticMeshComponent->MarkRenderStateDirty();
			}

			UE_LOG(LogCBComfyUI, Log, TEXT("Successfully replaced runtime texture for actor %s"), *(StateData->Actor->GetName()));
			Callback(true);
		});
	});

	return true;
}

void UCBComfyUISceneCaptureComponent::LoadRenderResultImages(const FString& FileName,TFunction<void(bool)> Callback)
{
	if(!ComfyUIManager)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Existing ComfyUIManager is null."));
		return ;
	}
	
	struct SharedState
	{
		FThreadSafeBool bAllSuccessful = true;
	};
	TSharedPtr<SharedState> StateData = MakeShared<SharedState>();
	Async(EAsyncExecution::ThreadPool, [this, StateData, FileName, Callback]()
		  {
			bool bSuccess =ComfyUIManager-> DownloadImage(FileName, [this, FileName, StateData, Callback](TArray<FColor> Pixels, int Width, int Height, bool bWasSuccessful)
			  {
				if (!bWasSuccessful)
				{
				  UE_LOG(LogCBComfyUI, Error, TEXT("Failed to download image %s"), *FileName);
					StateData->bAllSuccessful = false;
				}
				else
				{
				  TexturesRenderData->OutputPixels = Pixels;
				  TexturesRenderData->OutputWidth = Width;
				  TexturesRenderData->OutputHeight = Height;
				}

				Callback(StateData->bAllSuccessful);
			  });
		
		if (!bSuccess)
		{
			UE_LOG(LogCBComfyUI, Error, TEXT("Failed to download image %s"), *FileName);
			StateData->bAllSuccessful = false;
			
			Callback(StateData->bAllSuccessful);
		}
		
		  });	
	
}
#if WITH_EDITOR
bool UCBComfyUISceneCaptureComponent::CreateAssetPackage(UObject* Asset, FString PackagePath) const
{
	if (Asset == nullptr)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Asset is null."));
		return false;
	}

	if (PackagePath.Len() == 0)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Package path is empty."));
		return false;
	}

	Asset->AddToRoot();
	Asset->SetFlags(RF_Public | RF_Standalone | RF_MarkAsRootSet);
	FAssetRegistryModule::AssetCreated(Asset);

	FString PackageName = PackagePath;
	if (PackageName[PackageName.Len() - 1] != '/')
	{
		PackageName += "/";
	}

	PackageName += Asset->GetName();

	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	Asset->Rename(nullptr, Package);
	Package->MarkPackageDirty();

	FSavePackageArgs SaveArgs;
	SaveArgs.SaveFlags = SAVE_None;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;

	SaveArgs.Error = (FOutputDevice*)GLogConsole;

	FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());

	UE_LOG(LogCBComfyUI, Verbose, TEXT("Saving asset %s to %s"), *Asset->GetName(), *PackageFileName);

	if (!UPackage::SavePackage(Package, Asset, *PackageFileName, SaveArgs))
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Failed to save package to %s"), *PackageFileName);
		return false;
	}

	return true;
}
#endif

bool UCBComfyUISceneCaptureComponent::IsRunningInEditor() const
{
#if WITH_EDITOR
	return GIsEditor;
#else
	return false;
#endif
}

void UCBComfyUISceneCaptureComponent::CleanupRuntimeResources()
{
	UE_LOG(LogCBComfyUI, Log, TEXT("Cleaning up runtime resources"));

	// 清理Runtime材质
	for (UMaterialInstanceDynamic* Material : RuntimeMaterials)
	{
		if (Material && IsValid(Material))
		{
			Material->ClearParameterValues();
		}
	}
	RuntimeMaterials.Empty();

	// 清理Runtime纹理
	for (UTexture2D* Texture : RuntimeTextures)
	{
		if (Texture && IsValid(Texture))
		{
			// 从Root移除以允许垃圾回收
			if (Texture->IsRooted())
			{
				Texture->RemoveFromRoot();
			}
		}
	}
	RuntimeTextures.Empty();

	for (UTexture2D* Texture : RetiredRuntimeTextures)
	{
		if (Texture && IsValid(Texture))
		{
			if (Texture->IsRooted())
			{
				Texture->RemoveFromRoot();
			}
		}
	}
	RetiredRuntimeTextures.Empty();

	// 清理映射表
	ActorToMaterialMap.Empty();
	ActorToTextureMap.Empty();

	// 等待渲染命令完成
	FlushRenderingCommands();
}

void UCBComfyUISceneCaptureComponent::BeginDestroy()
{
	// 在组件销毁时清理Runtime资源
	CleanupRuntimeResources();
	
	Super::BeginDestroy();
}







