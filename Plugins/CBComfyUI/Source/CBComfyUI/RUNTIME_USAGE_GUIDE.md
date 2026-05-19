# CBComfyUI Plugin - Runtime and Editor Usage Guide

## 概述

这个插件现在支持在**编辑器模式**和**Runtime模式**下运行，解决了之前`PrepareActors`等函数无法在Runtime运行的问题。

## 主要改进

### 1. 双模式支持
- **编辑器模式**：创建持久化资源（纹理和材质资产）
- **Runtime模式**：创建动态资源（临时纹理和动态材质实例）

### 2. 新增函数

#### UCBComfyUISceneCaptureComponent
- `PrepareActorsRuntime()` - Runtime版本的Actor准备函数
- `CreateTexture2DRuntime()` - Runtime版本的纹理创建
- `ProcessRenderResultForActorRuntime()` - Runtime版本的结果处理
- `IsRunningInEditor()` - 检查运行模式

#### ACBComfyUIManager
- `IsRunningInEditor()` - 检查运行模式
- `ConfigureForRuntime()` - Runtime优化配置

#### UCBComfyUIBlueprintLibrary（新增）
- `PrepareActorsAuto()` - 自动选择编辑器或Runtime版本
- `CreateRuntimeTexture()` - 创建动态纹理
- `CreateDynamicMaterialForActor()` - 为Actor创建动态材质
- `IsTransientTexture()` - 检查是否为动态纹理

## 使用方法

### 编辑器模式下的使用

```cpp
// C++代码示例
UCBComfyUISceneCaptureComponent* CaptureComponent = GetCaptureComponent();
TArray<AActor*> Actors = GetSelectedActors();
UMaterial* BaseMaterial = LoadBaseMaterial();
UTexture2D* ReferenceTexture = LoadReferenceTexture();

// 编辑器模式：创建持久化资源
bool bSuccess = CaptureComponent->PrepareActors(Actors, BaseMaterial, ReferenceTexture);
```

### Runtime模式下的使用

```cpp
// C++代码示例
UCBComfyUISceneCaptureComponent* CaptureComponent = GetCaptureComponent();
TArray<AActor*> Actors = GetTargetActors();
UMaterial* BaseMaterial = LoadBaseMaterial();
UTexture2D* ReferenceTexture = LoadReferenceTexture();

// Runtime模式：创建动态资源
bool bSuccess = CaptureComponent->PrepareActorsRuntime(Actors, BaseMaterial, ReferenceTexture);
```

### 自动模式（推荐）

```cpp
// 使用蓝图库函数自动选择合适的模式
bool bSuccess = UCBComfyUIBlueprintLibrary::PrepareActorsAuto(
    CaptureComponent, Actors, BaseMaterial, ReferenceTexture);
```

### 蓝图使用示例

1. **检查运行模式**
   ```
   IsRunningInEditor -> Branch
   ```

2. **自动准备Actor**
   ```
   PrepareActorsAuto -> Print String (Success/Failure)
   ```

3. **Runtime优化配置**
   ```
   BeginPlay -> ConfigureForRuntime(EnableAutoReconnect=true, ReconnectInterval=5.0)
   ```

## Runtime模式的关键差异

### 资源管理
- **编辑器模式**：资源保存到磁盘，可持久化
- **Runtime模式**：资源存储在内存中，应用关闭后丢失

### 性能优化
- Runtime模式自动禁用详细日志
- 较短的连接和HTTP超时时间
- 自动重连机制

### 内存管理
- Runtime资源存储在组件的`RuntimeMaterials`和`RuntimeTextures`数组中
- 防止垃圾回收机制
- 组件销毁时自动清理

## 配置建议

### 编辑器开发环境
```cpp
// 编辑器模式配置
Manager->SetVerboseLogging(true);
Manager->SetConnectionTimeout(30.0f);
Manager->SetHttpTimeout(60.0f);
```

### 生产环境（Runtime）
```cpp
// Runtime模式配置
Manager->ConfigureForRuntime(true, 5.0f); // 启用自动重连，5秒间隔
```

## 故障排除

### 常见问题

1. **编辑器模式下资源创建失败**
   - 检查是否有足够的磁盘空间
   - 确保项目有写入权限
   - 验证资源路径是否正确

2. **Runtime模式下纹理未更新**
   - 确保使用了`PrepareActorsRuntime()`而不是`PrepareActors()`
   - 检查动态材质实例是否正确创建
   - 验证纹理数据是否正确传递

3. **连接问题**
   - 使用`ConfigureForRuntime()`进行Runtime优化
   - 检查ComfyUI服务器是否运行
   - 验证网络连接和防火墙设置

### 调试技巧

1. 使用`IsRunningInEditor()`检查运行模式
2. 启用详细日志（编辑器模式）
3. 检查`RuntimeMaterials`和`RuntimeTextures`数组的内容
4. 使用`IsTransientTexture()`验证纹理类型

## 性能注意事项

### Runtime模式
- 动态纹理创建比持久化纹理快
- 内存使用较高（纹理保存在RAM中）
- 适合实时生成和临时使用

### 编辑器模式
- 资源创建较慢（需要写入磁盘）
- 内存使用较低（资源可从磁盘加载）
- 适合开发和资源制作

## 最佳实践

1. **开发阶段**：使用编辑器模式创建和测试资源
2. **生产环境**：使用Runtime模式进行实时生成
3. **混合使用**：使用`PrepareActorsAuto()`让系统自动选择
4. **性能监控**：在Runtime模式下监控内存使用
5. **错误处理**：始终检查函数返回值并处理失败情况

## 示例蓝图设置

```
Event BeginPlay
    -> IsRunningInEditor
        -> True: Print "Running in Editor Mode"
        -> False: ConfigureForRuntime (AutoReconnect=true, Interval=5.0)
                  -> Print "Configured for Runtime"

Event ProcessTextures
    -> PrepareActorsAuto (Actors, BaseMaterial, ReferenceTexture)
        -> Success: ProcessMultipleActors
        -> Failure: Print "Failed to prepare actors"
```

这个优化后的插件现在可以在任何环境下稳定运行，无论是编辑器开发还是最终的游戏Runtime环境。
