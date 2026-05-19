# D3D12RHI纹理访问违例崩溃修复

## 问题描述
在Runtime模式下，插件在处理纹理更新时发生D3D12RHI相关的EXCEPTION_ACCESS_VIOLATION崩溃：

```
UnrealEditor_D3D12RHI!FD3D12ResourceBinder::SetTexture()
UnrealEditor_D3D12RHI!FD3D12CommandContext::CommitGraphicsResourceTables()
```

## 崩溃原因分析

### 主要原因
1. **纹理资源生命周期管理不当** - 纹理在GPU仍在使用时被释放
2. **渲染线程同步问题** - 游戏线程和渲染线程之间的数据竞争
3. **纹理状态无效** - 纹理对象在无效状态下被渲染系统引用
4. **内存指针失效** - 传递给UpdateTextureRegions的数据指针在异步操作中失效

### 技术细节
- D3D12RHI在提交Graphics Resource Tables时尝试访问已释放的纹理资源
- 动态纹理在CreateTransient后立即更新可能导致资源状态不同步
- 跨线程的内存访问没有适当的保护机制

## 修复方案

### 1. 纹理创建安全化
**修复位置**: `CreateTexture2DRuntime`函数

**关键改进**:
```cpp
// 确保在游戏线程中创建
if (!IsInGameThread()) {
    return nullptr;
}

// 等待纹理资源初始化
if (!Texture2D->GetResource()) {
    Texture2D->UpdateResource();
    FlushRenderingCommands(); // 等待GPU命令完成
}

// 使用渲染线程安全的更新方式
ENQUEUE_RENDER_COMMAND(UpdateTexture2DData)(
    [Texture2D, UpdateRegion, ByteArrayPtr, Width](FRHICommandListImmediate& RHICmdList) {
        if (Texture2D && Texture2D->GetResource() && 
            IsValidRef(Texture2D->GetResource()->TextureRHI)) {
            Texture2D->UpdateTextureRegions(0, 1, &UpdateRegion, 
                Width * 4, 4, ByteArrayPtr->GetData());
        }
        delete ByteArrayPtr; // 安全清理
    });
```

### 2. 纹理更新线程安全
**修复位置**: `ProcessRenderResultForActorRuntime`函数

**关键改进**:
```cpp
// 验证纹理对象有效性
if (!StateData->Texture2D || !IsValid(StateData->Texture2D)) {
    Callback(false);
    return;
}

// 验证纹理资源
if (!StateData->Texture2D->GetResource()) {
    Callback(false);
    return;
}

// 内存拷贝避免指针失效
TArray<uint8>* ByteArrayPtr = new TArray<uint8>(ByteArray);

// 使用ENQUEUE_RENDER_COMMAND确保线程安全
ENQUEUE_RENDER_COMMAND(UpdateRuntimeTextureData)(
    [TexturePtr, UpdateRegion, ByteArrayPtr, StateData](FRHICommandListImmediate& RHICmdList) {
        if (TexturePtr && IsValid(TexturePtr) && TexturePtr->GetResource() && 
            IsValidRef(TexturePtr->GetResource()->TextureRHI)) {
            TexturePtr->UpdateTextureRegions(0, 1, &UpdateRegion, 
                StateData->TextureWidth * 4, 4, ByteArrayPtr->GetData());
        }
        delete ByteArrayPtr; // 清理内存
    });

FlushRenderingCommands(); // 等待完成
```

### 3. 资源生命周期管理
**新增功能**: 资源清理机制

**实现**:
```cpp
// 在组件中添加资源数组
UPROPERTY(Transient)
TArray<UMaterialInstanceDynamic*> RuntimeMaterials;

UPROPERTY(Transient)
TArray<UTexture2D*> RuntimeTextures;

// 清理函数
void CleanupRuntimeResources() {
    for (UTexture2D* Texture : RuntimeTextures) {
        if (Texture && IsValid(Texture) && Texture->IsRooted()) {
            Texture->RemoveFromRoot();
        }
    }
    RuntimeTextures.Empty();
    FlushRenderingCommands();
}

// 在BeginDestroy中自动清理
virtual void BeginDestroy() override {
    CleanupRuntimeResources();
    Super::BeginDestroy();
}
```

### 4. 渲染状态同步
**改进内容**:
- 在纹理更新前后使用`FlushRenderingCommands()`
- 验证纹理RHI资源的有效性
- 使用`IsValidRef`检查RHI对象状态
- 确保材质参数重新缓存

## 技术要点

### UE5.4 D3D12RHI兼容性
- 使用`ENQUEUE_RENDER_COMMAND`替代直接调用
- 验证`TextureRHI`的有效性
- 正确处理异步渲染命令
- 使用安全的内存管理模式

### 线程安全模式
- 游戏线程：纹理创建、验证、回调
- 渲染线程：实际的GPU资源更新
- 后台线程：像素数据处理
- 同步点：`FlushRenderingCommands()`

### 内存管理策略
- 使用`new/delete`管理传递给渲染线程的数据
- 避免在渲染命令中引用栈上的变量
- 正确处理纹理的Root状态
- 使用Transient属性标记临时对象

## 测试验证

### 1. 崩溃复现测试
```cpp
// 在Runtime模式下快速创建和更新多个纹理
for (int i = 0; i < 10; ++i) {
    UTexture2D* Texture = CreateRuntimeTexture(512, 512, FColor::Red);
    // 立即更新纹理数据
    // 检查是否还会崩溃
}
```

### 2. 内存泄漏检测
```cpp
// 使用UE的内存分析工具
// 检查Runtime纹理是否正确释放
// 验证没有悬空指针
```

### 3. 多线程压力测试
```cpp
// 同时在多个线程中操作纹理
// 验证线程安全性
// 检查渲染状态一致性
```

## 性能影响

### 优化措施
- `FlushRenderingCommands()`会影响性能，只在必要时使用
- 批量处理纹理更新以减少同步点
- 使用纹理池重用资源
- 避免频繁的纹理创建和销毁

### 监控指标
- 渲染线程同步时间
- 纹理创建频率
- 内存使用量
- GPU命令队列深度

## 故障排除

### 如果仍然崩溃
1. **检查基础材质** - 确保包含正确的纹理参数
2. **验证纹理尺寸** - 必须是有效的2的幂次
3. **检查像素格式** - 确保PF_B8G8R8A8兼容性
4. **调试渲染状态** - 使用RenderDoc或类似工具

### 调试技巧
```cpp
// 在崩溃点添加验证
check(Texture && IsValid(Texture));
check(Texture->GetResource());
check(IsValidRef(Texture->GetResource()->TextureRHI));

// 使用详细日志
UE_LOG(LogCBComfyUI, VeryVerbose, TEXT("Texture state: %s"), 
    Texture ? TEXT("Valid") : TEXT("Null"));
```

### 常见问题
1. **纹理过早释放** - 检查GC设置和Root状态
2. **渲染命令排序** - 确保依赖关系正确
3. **资源状态不一致** - 验证创建和更新的时机
4. **平台差异** - 测试不同的GPU驱动程序

## 兼容性说明

### 支持的版本
- UE5.4: 完全支持，使用新的渲染命令模式
- UE5.3: 基本支持，可能需要调整部分API
- D3D12: 主要目标平台
- Vulkan: 理论支持，需要测试验证

### 平台注意事项
- Windows: 主要开发和测试平台
- Console: 需要适配特定的渲染管线
- Mobile: 可能需要调整纹理格式

## 更新记录
- 2024.09.02: 修复D3D12RHI纹理访问违例崩溃
- 添加了完整的资源生命周期管理
- 实现了线程安全的纹理更新机制
- 增强了错误处理和调试支持
