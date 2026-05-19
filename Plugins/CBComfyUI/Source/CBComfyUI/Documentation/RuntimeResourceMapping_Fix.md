# Runtime纹理资源管理修复

## 问题描述
尽管之前修复了基本的D3D12RHI崩溃问题，Runtime模式下仍然出现访问违例错误：
```
UnrealEditor_D3D12RHI!FD3D12ResourceBinder::SetTexture()
```

根本原因是`ProcessRenderResultForActorRuntime`函数没有正确使用`RuntimeMaterials`和`RuntimeTextures`数组来管理资源生命周期。

## 核心问题

### 1. 资源追踪不准确
- 直接从材质获取纹理引用，而不是从受管理的Runtime数组获取
- 没有验证纹理是否仍在有效的管理范围内
- 缺乏Actor与其对应资源的精确映射关系

### 2. 生命周期管理缺陷
- 纹理可能在GPU仍在使用时被垃圾回收
- 动态材质实例的引用可能变成悬空指针
- 缺乏对资源有效性的实时验证

## 修复方案

### 1. 资源映射表系统
**新增组件**:
```cpp
// Actor到材质和纹理的映射表，用于Runtime模式下的精确管理
UPROPERTY(Transient)
TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<UMaterialInstanceDynamic>> ActorToMaterialMap;

UPROPERTY(Transient)
TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<UTexture2D>> ActorToTextureMap;
```

**优势**:
- 直接通过Actor查找对应的Runtime资源
- 使用`TWeakObjectPtr`避免循环引用
- 精确控制每个Actor的资源状态

### 2. 安全的资源查找机制
**修复前**:
```cpp
// 危险：直接从材质获取纹理，可能已失效
UMaterialInterface* Material = StaticMeshComponent->GetMaterial(0);
UMaterialInstanceDynamic* DynamicMaterial = Cast<UMaterialInstanceDynamic>(Material);
UTexture* Texture = nullptr;
DynamicMaterial->GetTextureParameterValue(TEXT("BaseColor"), Texture);
```

**修复后**:
```cpp
// 安全：从映射表查找受管理的资源
TWeakObjectPtr<UMaterialInstanceDynamic>* MaterialPtr = ActorToMaterialMap.Find(Actor);
TWeakObjectPtr<UTexture2D>* TexturePtr = ActorToTextureMap.Find(Actor);

if (!MaterialPtr || !MaterialPtr->IsValid()) {
    return false; // 资源已失效
}

UMaterialInstanceDynamic* DynamicMaterial = MaterialPtr->Get();
UTexture2D* Texture2D = TexturePtr->Get();
```

### 3. 纹理替换策略优化
**核心改进**:
- 创建新纹理替代原地更新
- 确保旧纹理正确释放
- 更新所有相关的引用和映射

**实现代码**:
```cpp
// 创建新的纹理来替换旧的
UTexture2D* NewTexture = CreateTexture2DRuntime(Width, Height, NewPixelData);

// 在RuntimeTextures数组中替换
for (int32 i = 0; i < RuntimeTextures.Num(); ++i) {
    if (RuntimeTextures[i] == StateData->Texture2D) {
        // 清理旧纹理
        if (RuntimeTextures[i]->IsRooted()) {
            RuntimeTextures[i]->RemoveFromRoot();
        }
        // 替换为新纹理
        RuntimeTextures[i] = NewTexture;
        break;
    }
}

// 更新映射表
ActorToTextureMap.Add(StateData->Actor, NewTexture);
```

### 4. 强化的资源验证
**多层验证机制**:
```cpp
// 1. 映射表存在性验证
if (!MaterialPtr || !MaterialPtr->IsValid()) {
    return false;
}

// 2. 对象有效性验证
if (!DynamicMaterial || !IsValid(DynamicMaterial)) {
    return false;
}

// 3. GPU资源状态验证
if (!Texture2D->GetResource() || !IsValidRef(Texture2D->GetResource()->TextureRHI)) {
    return false;
}

// 4. Runtime数组一致性验证
bool bTextureStillValid = false;
for (UTexture2D* RuntimeTex : RuntimeTextures) {
    if (RuntimeTex == StateData->Texture2D && IsValid(RuntimeTex)) {
        bTextureStillValid = true;
        break;
    }
}
```

## 技术实现细节

### 1. 初始化阶段
**在PrepareActorsRuntime中**:
```cpp
// 清理之前的资源
RuntimeMaterials.Empty();
RuntimeTextures.Empty();
ActorToMaterialMap.Empty();
ActorToTextureMap.Empty();

// 为每个Actor创建资源后添加到映射表
ActorToTextureMap.Add(Actor, Texture2D);
ActorToMaterialMap.Add(Actor, MaterialInstance);
```

### 2. 更新阶段
**在ProcessRenderResultForActorRuntime中**:
```cpp
// 使用映射表安全查找资源
TWeakObjectPtr<UTexture2D>* TexturePtr = ActorToTextureMap.Find(Actor);
if (!TexturePtr || !TexturePtr->IsValid()) {
    return false;
}

// 创建新纹理并更新所有引用
UTexture2D* NewTexture = CreateTexture2DRuntime(Width, Height, PixelData);
ActorToTextureMap.Add(StateData->Actor, NewTexture);
```

### 3. 清理阶段
**在CleanupRuntimeResources中**:
```cpp
// 清理映射表
ActorToMaterialMap.Empty();
ActorToTextureMap.Empty();

// 等待渲染命令完成
FlushRenderingCommands();
```

## 性能和内存优化

### 1. 预分配策略
```cpp
// 在构造函数中预分配空间
RuntimeMaterials.Reserve(8);
RuntimeTextures.Reserve(8);
ActorToMaterialMap.Reserve(8);
ActorToTextureMap.Reserve(8);
```

### 2. 智能指针使用
- `TWeakObjectPtr`：避免循环引用，自动处理对象销毁
- `TSharedPtr`：用于数据传递，确保数据生命周期
- Root引用：防止纹理过早被垃圾回收

### 3. 批量操作优化
- 一次性清理所有资源
- 批量更新映射表
- 减少渲染线程同步次数

## 调试和监控

### 1. 详细日志
```cpp
UE_LOG(LogCBComfyUI, Warning, TEXT("Runtime material not found in mapping for actor %s"), *Actor->GetName());
UE_LOG(LogCBComfyUI, Error, TEXT("Texture resource is invalid for actor %s"), *Actor->GetName());
```

### 2. 资源状态检查
```cpp
// 检查映射表一致性
bool bMappingConsistent = (ActorToMaterialMap.Num() == ActorToTextureMap.Num());

// 检查数组和映射表数量匹配
bool bArraysMatch = (RuntimeTextures.Num() == ActorToTextureMap.Num());
```

### 3. 内存泄漏检测
- 监控Root引用的纹理数量
- 验证映射表在清理后为空
- 检查WeakPtr的有效性

## 兼容性说明

### UE5.4特性
- 完全兼容新的渲染管线
- 正确处理D3D12RHI资源绑定
- 支持异步渲染命令队列

### 向后兼容性
- 编辑器模式功能保持不变
- API接口没有破坏性变更
- 可以平滑升级现有项目

## 测试验证

### 1. 稳定性测试
```cpp
// 快速创建和销毁多个Actor
for (int i = 0; i < 20; ++i) {
    PrepareActorsRuntime(Actors, BaseMaterial, RefTexture);
    ProcessRenderResults("test_image.png");
    CleanupRuntimeResources();
}
```

### 2. 内存监控
- 使用UE的内存分析器监控纹理使用
- 检查是否有内存泄漏
- 验证垃圾回收工作正常

### 3. GPU资源验证
- 使用RenderDoc检查GPU资源状态
- 验证纹理绑定的正确性
- 监控D3D12命令队列

## 故障排除指南

### 如果仍然崩溃
1. **检查基础材质**：确保包含正确的BaseColor参数
2. **验证Actor有效性**：确保Actor在处理过程中没有被销毁
3. **监控资源生命周期**：使用断点检查资源创建和销毁时机
4. **检查线程安全**：确保所有纹理操作在正确的线程中执行

### 常见问题
1. **映射表不一致**：检查是否正确清理和重建映射关系
2. **WeakPtr失效**：验证对象没有被意外销毁
3. **纹理Root状态**：确保纹理正确添加和移除Root引用

## 更新记录
- 2024.09.02: 实现基于映射表的Runtime资源管理
- 解决了资源追踪不准确的问题
- 添加了多层资源验证机制
- 实现了安全的纹理替换策略
