// Fill out your copyright notice in the Description page of Project Settings.

#include "CBComfyUIBlueprintLibrary.h"
#include "CBComfyUI.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2D.h"

bool UCBComfyUIBlueprintLibrary::IsRunningInEditor()
{
#if WITH_EDITOR
	return GIsEditor;
#else
	return false;
#endif
}

bool UCBComfyUIBlueprintLibrary::PrepareActorsAuto(UCBComfyUISceneCaptureComponent* SceneCaptureComponent, 
	const TArray<AActor*>& Actors, UMaterial* BaseMaterial, UTexture2D* ReferenceTexture)
{
	if (!SceneCaptureComponent)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("SceneCaptureComponent is null"));
		return false;
	}

	if (IsRunningInEditor())
	{
		// 编辑器模式：使用PrepareActors创建持久化资源
		return SceneCaptureComponent->PrepareActors(Actors, BaseMaterial, ReferenceTexture);
	}
	else
	{
		// Runtime模式：使用PrepareActorsRuntime创建动态资源
		return SceneCaptureComponent->PrepareActorsRuntime(Actors, BaseMaterial, ReferenceTexture);
	}
}

UTexture2D* UCBComfyUIBlueprintLibrary::CreateRuntimeTexture(int32 Width, int32 Height, FColor Color)
{
	if (Width <= 0 || Height <= 0)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Invalid texture dimensions: %dx%d"), Width, Height);
		return nullptr;
	}

	// 确保在游戏线程中创建纹理
	if (!IsInGameThread())
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("CreateRuntimeTexture must be called from game thread"));
		return nullptr;
	}

	// 创建动态纹理
	UTexture2D* Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	if (!Texture)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Failed to create transient texture"));
		return nullptr;
	}

	Texture->SRGB = true;
	Texture->Filter = TF_Trilinear;
	Texture->NeverStream = true;
	Texture->bNotOfflineProcessed = true;

	// 等待纹理资源初始化
	if (!Texture->GetResource())
	{
		Texture->UpdateResource();
		FlushRenderingCommands(); // 等待GPU命令完成
	}

	// 将FColor转换为字节数组
	TArray<uint8> ByteArray;
	ByteArray.SetNumUninitialized(Width * Height * 4);
	
	for (int32 i = 0; i < Width * Height; ++i)
	{
		ByteArray[i * 4 + 0] = Color.B;
		ByteArray[i * 4 + 1] = Color.G;
		ByteArray[i * 4 + 2] = Color.R;
		ByteArray[i * 4 + 3] = Color.A;
	}

	// 使用渲染线程安全的方式更新纹理
	FUpdateTextureRegion2D* UpdateRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);
	uint8* RawData = (uint8*)FMemory::Malloc(ByteArray.Num());
	FMemory::Memcpy(RawData, ByteArray.GetData(), ByteArray.Num());

	Texture->UpdateTextureRegions(0, 1, UpdateRegion, Width * 4, 4, RawData,
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
	return Texture;
}

UMaterialInstanceDynamic* UCBComfyUIBlueprintLibrary::CreateDynamicMaterialForActor(AActor* Actor, UMaterial* BaseMaterial, UTexture2D* Texture)
{
	if (!Actor)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Actor is null"));
		return nullptr;
	}

	if (!BaseMaterial)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("BaseMaterial is null"));
		return nullptr;
	}

	UStaticMeshComponent* MeshComponent = GetStaticMeshComponent(Actor);
	if (!MeshComponent)
	{
		UE_LOG(LogCBComfyUI, Warning, TEXT("Actor %s has no static mesh component"), *Actor->GetName());
		return nullptr;
	}

	// 创建动态材质实例
	UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, Actor);
	if (!DynamicMaterial)
	{
		UE_LOG(LogCBComfyUI, Error, TEXT("Failed to create dynamic material instance"));
		return nullptr;
	}

	// 设置纹理参数
	if (Texture)
	{
		DynamicMaterial->SetTextureParameterValue(TEXT("BaseColor"), Texture);
		
		// 确保材质参数更新
		DynamicMaterial->RecacheUniformExpressions(true);
	}

	// 应用材质到组件
	MeshComponent->SetMaterial(0, DynamicMaterial);
	
	// 标记组件需要重新渲染
	MeshComponent->MarkRenderStateDirty();

	UE_LOG(LogCBComfyUI, Log, TEXT("Successfully created dynamic material for actor %s"), *Actor->GetName());
	return DynamicMaterial;
}

UStaticMeshComponent* UCBComfyUIBlueprintLibrary::GetStaticMeshComponent(AActor* Actor)
{
	if (!Actor)
	{
		return nullptr;
	}

	return Actor->FindComponentByClass<UStaticMeshComponent>();
}

bool UCBComfyUIBlueprintLibrary::IsTransientTexture(UTexture2D* Texture)
{
	if (!Texture)
	{
		return false;
	}

	// 检查纹理是否为临时纹理（动态纹理通常具有这些标志）
	return Texture->HasAnyFlags(RF_Transient) || 
		   !Texture->GetOutermost()->HasAnyPackageFlags(PKG_PlayInEditor | PKG_ContainsMap);
}

FString UCBComfyUIBlueprintLibrary::DebugActorMaterialState(AActor* Actor)
{
	if (!Actor)
	{
		return TEXT("Actor is null");
	}

	UStaticMeshComponent* MeshComponent = GetStaticMeshComponent(Actor);
	if (!MeshComponent)
	{
		return FString::Printf(TEXT("Actor '%s' has no static mesh component"), *Actor->GetName());
	}

	UMaterialInterface* Material = MeshComponent->GetMaterial(0);
	if (!Material)
	{
		return FString::Printf(TEXT("Actor '%s' has no material"), *Actor->GetName());
	}

	FString Result = FString::Printf(TEXT("Actor: %s\n"), *Actor->GetName());

	// 检查材质类型
	if (UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Material))
	{
		Result += TEXT("Material Type: Dynamic Material Instance\n");
		
		UTexture* BaseColorTexture = nullptr;
		if (DynamicMaterial->GetTextureParameterValue(TEXT("BaseColor"), BaseColorTexture))
		{
			if (UTexture2D* Texture2D = Cast<UTexture2D>(BaseColorTexture))
			{
				Result += FString::Printf(TEXT("BaseColor Texture: %s (%dx%d)\n"), 
					*Texture2D->GetName(), Texture2D->GetSizeX(), Texture2D->GetSizeY());
				Result += FString::Printf(TEXT("Is Transient: %s\n"), 
					IsTransientTexture(Texture2D) ? TEXT("Yes") : TEXT("No"));
			}
			else
			{
				Result += TEXT("BaseColor Texture: Not a Texture2D\n");
			}
		}
		else
		{
			Result += TEXT("BaseColor Texture: Not found\n");
		}
	}
	else
	{
		Result += FString::Printf(TEXT("Material Type: %s\n"), *Material->GetClass()->GetName());
	}

	return Result;
}

bool UCBComfyUIBlueprintLibrary::ForceUpdateActorRendering(AActor* Actor)
{
	if (!Actor)
	{
		return false;
	}

	UStaticMeshComponent* MeshComponent = GetStaticMeshComponent(Actor);
	if (!MeshComponent)
	{
		return false;
	}

	// 强制更新组件的渲染状态
	MeshComponent->MarkRenderStateDirty();
	
	// 如果是动态材质，重新缓存表达式
	UMaterialInterface* Material = MeshComponent->GetMaterial(0);
	if (UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Material))
	{
		DynamicMaterial->RecacheUniformExpressions(true);
	}

	UE_LOG(LogCBComfyUI, Log, TEXT("Forced rendering update for actor %s"), *Actor->GetName());
	return true;
}
