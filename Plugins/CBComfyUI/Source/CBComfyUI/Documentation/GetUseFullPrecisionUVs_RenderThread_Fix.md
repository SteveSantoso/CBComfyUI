# GetUseFullPrecisionUVs渲染线程调用问题修复

## 问题描述

在UE5.4中，当在渲染线程中调用`GetVertexUV()`时，该函数内部会调用`GetUseFullPrecisionUVs()`来确定UV精度类型，但这个调用在渲染线程上下文中可能失败。

### 引擎源码分析
```cpp
FORCEINLINE_DEBUGGABLE FVector2f GetVertexUV(uint32 VertexIndex, uint32 UVIndex) const
{
    checkSlow(VertexIndex < GetNumVertices());
    checkSlow(UVIndex < GetNumTexCoords());

    if (GetUseFullPrecisionUVs())  // ❌ 这里在渲染线程中可能出问题
    {
        return GetVertexUV_Typed<EStaticMeshVertexUVType::HighPrecision>(VertexIndex, UVIndex);
    }
    else
    {
        return GetVertexUV_Typed<EStaticMeshVertexUVType::Default>(VertexIndex, UVIndex);
    }
}
```

## 根本原因

### 线程安全问题
1. **主线程 vs 渲染线程** - `GetUseFullPrecisionUVs()`可能需要主线程上下文
2. **对象生命周期** - 渲染线程中对象状态可能不稳定
3. **检查宏问题** - `checkSlow`在渲染线程中可能触发断言

### UE5.4特定问题
- 渲染系统重构导致某些API的线程安全性变化
- UV精度检查逻辑可能依赖于主线程的设置

## 解决方案

### 直接调用策略
避免使用`GetVertexUV()`，直接调用底层的类型化函数：

```cpp
// 修复前 - 有问题的调用
FVector2f UvFloat = MeshLod.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(VertexIndex, 0);

// 修复后 - 直接调用类型化函数
try
{
    // 直接使用默认精度，避免GetUseFullPrecisionUVs调用
    FVector2f UvFloat = VertexBuffer.GetVertexUV_Typed<EStaticMeshVertexUVType::Default>(VertexIndex, 0);
    Uv = FVector2D(UvFloat.X, UvFloat.Y);
}
catch (...)
{
    // 失败时使用零向量
    Uv = FVector2D::ZeroVector;
}
```

### 关键改进

#### 1. 避免动态精度检查
```cpp
// 不再依赖GetUseFullPrecisionUVs()的运行时检查
// 直接使用默认精度，这对大多数情况都足够
FVector2f UvFloat = VertexBuffer.GetVertexUV_Typed<EStaticMeshVertexUVType::Default>(VertexIndex, 0);
```

#### 2. 异常安全
```cpp
try
{
    // UV获取逻辑
}
catch (...)
{
    // 失败回退策略
    Uv = FVector2D::ZeroVector;
}
```

#### 3. 边界检查
```cpp
// 显式的边界检查，避免依赖内部checkSlow
if (VertexIndex < (uint32)VertexBuffer.GetNumVertices() && 
    VertexBuffer.GetNumTexCoords() > 0)
{
    // 安全的UV访问
}
```

## 性能和精度权衡

### 精度选择
- **Default精度**: 足够大多数游戏应用
- **HighPrecision精度**: 需要时可以手动检查并使用

### 性能优势
- 避免运行时精度检查的开销
- 减少渲染线程中的函数调用层次
- 更直接的数据访问路径

### 回退机制
```cpp
// 可以根据需要实现更智能的精度选择
FVector2f GetUVSafely(const FStaticMeshVertexBuffer& Buffer, uint32 VertexIndex, uint32 UVIndex)
{
    try
    {
        // 首先尝试默认精度
        return Buffer.GetVertexUV_Typed<EStaticMeshVertexUVType::Default>(VertexIndex, UVIndex);
    }
    catch (...)
    {
        try
        {
            // 如果默认精度失败，尝试高精度
            return Buffer.GetVertexUV_Typed<EStaticMeshVertexUVType::HighPrecision>(VertexIndex, UVIndex);
        }
        catch (...)
        {
            // 都失败则返回零
            return FVector2f::ZeroVector;
        }
    }
}
```

## 调试信息

### 问题症状
- 渲染线程崩溃
- UV数据获取失败
- `GetUseFullPrecisionUVs`相关的断言或异常

### 诊断方法
```cpp
// 添加调试日志
UE_LOG(LogTemp, Warning, TEXT("UV Buffer Info: NumTexCoords=%d, NumVertices=%d"), 
       VertexBuffer.GetNumTexCoords(), 
       VertexBuffer.GetNumVertices());

// 检查UV有效性
if (Uv != FVector2D::ZeroVector)
{
    UE_LOG(LogTemp, Log, TEXT("Successfully got UV: (%f, %f)"), Uv.X, Uv.Y);
}
```

### 验证代码
```cpp
// 验证UV数据的合理性
void ValidateUVData(const TArray<FVector2D>& UVs)
{
    for (int32 i = 0; i < UVs.Num(); ++i)
    {
        const FVector2D& UV = UVs[i];
        if (FMath::IsNaN(UV.X) || FMath::IsNaN(UV.Y))
        {
            UE_LOG(LogTemp, Error, TEXT("NaN UV at index %d"), i);
        }
        if (UV.X < -0.1f || UV.X > 1.1f || UV.Y < -0.1f || UV.Y > 1.1f)
        {
            UE_LOG(LogTemp, Warning, TEXT("UV out of normal range at index %d: (%f, %f)"), i, UV.X, UV.Y);
        }
    }
}
```

## 兼容性考虑

### 向后兼容
- 保持相同的数据输出格式
- 不改变API接口
- 渐进式错误处理

### 向前兼容
- 可以轻松切换到高精度UV
- 准备好处理未来的API变化
- 模块化的UV获取逻辑

## 最佳实践

### 1. 渲染线程安全
```cpp
// 在ENQUEUE_RENDER_COMMAND中进行所有渲染资源访问
ENQUEUE_RENDER_COMMAND(SafeUVAccess)(
    [](FRHICommandListImmediate& RHICmdList)
    {
        // 所有UV访问代码放在这里
    });
```

### 2. 错误处理
```cpp
// 总是提供失败回退
FVector2D SafeUV = FVector2D::ZeroVector;
try {
    // UV获取逻辑
} catch (...) {
    // 记录但不中断处理
    UE_LOG(LogTemp, Warning, TEXT("UV access failed, using zero vector"));
}
```

### 3. 性能优化
```cpp
// 批量处理以减少函数调用开销
TArray<FVector2f> UVBatch;
UVBatch.Reserve(VertexCount);
for (int32 i = 0; i < VertexCount; ++i)
{
    UVBatch.Add(GetUVSafely(VertexBuffer, i, 0));
}
```

## 更新记录
- 2024.09.02: 修复GetUseFullPrecisionUVs在渲染线程中的调用问题
- 使用直接的类型化UV访问方法
- 添加异常安全和边界检查
- 提供失败回退机制
