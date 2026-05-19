// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CBComfyUISceneCaptureComponent.h"
#include "CBComfyUIBlueprintLibrary.generated.h"

/**
 * 蓝图函数库，提供CBComfyUI插件的便捷函数
 */
UCLASS()
class CBCOMFYUI_API UCBComfyUIBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	/**
	 * 检查是否在编辑器中运行
	 * @return 如果在编辑器中则返回true，否则返回false
	 */
	UFUNCTION(BlueprintPure, Category = "CBComfyUI|Utilities", meta = (DisplayName = "Is Running In Editor"))
	static bool IsRunningInEditor();

	/**
	 * 为多个Actor准备材质（自动选择编辑器或Runtime版本）
	 * @param SceneCaptureComponent - Scene Capture组件
	 * @param Actors - 要处理的Actor数组
	 * @param BaseMaterial - 基础材质
	 * @param ReferenceTexture - 参考纹理
	 * @return 是否成功
	 */
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI|Utilities", meta = (DisplayName = "Prepare Actors Auto"))
	static bool PrepareActorsAuto(UCBComfyUISceneCaptureComponent* SceneCaptureComponent, 
		const TArray<AActor*>& Actors, UMaterial* BaseMaterial, UTexture2D* ReferenceTexture);

	/**
	 * 创建动态纹理（Runtime版本）
	 * @param Width - 纹理宽度
	 * @param Height - 纹理高度
	 * @param Color - 填充颜色
	 * @return 创建的纹理
	 */
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI|Utilities", meta = (DisplayName = "Create Runtime Texture"))
	static UTexture2D* CreateRuntimeTexture(int32 Width, int32 Height, FColor Color = FColor::White);

	/**
	 * 为Actor创建动态材质实例
	 * @param Actor - 目标Actor
	 * @param BaseMaterial - 基础材质
	 * @param Texture - 要应用的纹理
	 * @return 创建的动态材质实例
	 */
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI|Utilities", meta = (DisplayName = "Create Dynamic Material for Actor"))
	static UMaterialInstanceDynamic* CreateDynamicMaterialForActor(AActor* Actor, UMaterial* BaseMaterial, UTexture2D* Texture);

	/**
	 * 获取Actor的静态网格组件
	 * @param Actor - 目标Actor
	 * @return 静态网格组件（如果存在）
	 */
	UFUNCTION(BlueprintPure, Category = "CBComfyUI|Utilities", meta = (DisplayName = "Get Static Mesh Component"))
	static UStaticMeshComponent* GetStaticMeshComponent(AActor* Actor);

	/**
	 * 检查纹理是否为动态纹理
	 * @param Texture - 要检查的纹理
	 * @return 是否为动态纹理
	 */
	UFUNCTION(BlueprintPure, Category = "CBComfyUI|Utilities", meta = (DisplayName = "Is Transient Texture"))
	static bool IsTransientTexture(UTexture2D* Texture);

	/**
	 * 调试：验证Actor的材质和纹理状态
	 * @param Actor - 要检查的Actor
	 * @return 状态描述字符串
	 */
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI|Debug", meta = (DisplayName = "Debug Actor Material State"))
	static FString DebugActorMaterialState(AActor* Actor);

	/**
	 * 调试：强制更新Actor的材质渲染
	 * @param Actor - 要更新的Actor
	 * @return 是否成功
	 */
	UFUNCTION(BlueprintCallable, Category = "CBComfyUI|Debug", meta = (DisplayName = "Force Update Actor Rendering"))
	static bool ForceUpdateActorRendering(AActor* Actor);
};
