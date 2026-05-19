<<<<<<< HEAD
# CBComfyUI

CBComfyUI 是一个 Unreal Engine 插件，用于把 ComfyUI 工作流接入 UE 场景流程。插件会从场景中生成控制图，提交 prompt JSON 到 ComfyUI，接收生成结果，再把结果图回写到目标 Actor 的材质上。

当前仓库中的实现已经覆盖：

- 编辑器模式下使用
- 打包后的 Runtime 模式使用
- ComfyUI 的 HTTP / WebSocket 通信
- 基于深度图的工作流回写
- 运行时动态纹理和动态材质实例管理

如果你直接使用本仓库自带的示例工作流，请先阅读模型下载和 ComfyUI 侧准备部分；该工作流依赖 FLUX.1-dev、Nunchaku 和 ControlNet。

## 功能一览

- 从 UE 场景生成 `depth.png`、`normals.png`、`color.png`、`edge_mask.png`
- 通过 `QueueWorkflowFromFile` / `QueueWorkflowFromString` 提交 ComfyUI prompt JSON
- 通过 `OnComfyUIOutputFile` 接收 `SaveImage` 输出文件名
- 将生成结果重新投影到 Static Mesh 材质
- 支持编辑器与打包版两套资源路径和运行时资源管理
- 支持在 UE 中动态修改 prompt、steps、guidance、ControlNet strength 等节点输入

## 重要限制

如果你要使用“控制图上传 -> ComfyUI 生成 -> 结果贴图回写到 Static Mesh”这条链路，那么所有参与回写流程的 Static Mesh 资源都必须在资源详情里勾选：

- `Allow CPU Access`

这不是可选优化，而是硬性要求。插件在回写阶段需要读取 Static Mesh 的 CPU 顶点、UV 和索引数据；如果没有开启，尤其是在打包版里，这些数据通常会被 cook 裁掉，直接导致回写失败、贴图不生效，或者表现为编辑器里能用但打包后不能用。

## 仓库中的示例资源

- 示例工作流： [Resources/工作流.json](Resources/工作流.json)
- 工作流详细说明： [Resources/工作流使用说明.md](Resources/工作流使用说明.md)
- Manager 蓝图入口： [Source/CBComfyUI/Public/CBComfyUIManager.h](Source/CBComfyUI/Public/CBComfyUIManager.h)
- Scene Capture 入口： [Source/CBComfyUI/Public/CBComfyUISceneCaptureComponent.h](Source/CBComfyUI/Public/CBComfyUISceneCaptureComponent.h)
- 蓝图工具函数： [Source/CBComfyUI/Public/CBComfyUIBlueprintLibrary.h](Source/CBComfyUI/Public/CBComfyUIBlueprintLibrary.h)

需要注意的是，仓库里的 [Resources/工作流.json](Resources/工作流.json) 是可直接提交给 ComfyUI `/prompt` 接口的 prompt JSON，不是前端画布布局 JSON。

## 环境要求

- Unreal Engine 5.5 项目环境
- Windows
- NVIDIA GPU
- 可正常运行的 ComfyUI
- 如果使用仓库内示例工作流，需要安装 ComfyUI-nunchaku 和 Nunchaku 后端

另外，目标材质需要有一个名为 `BaseColor` 的纹理参数，插件会把生成结果写到这个参数上。

## 安装插件

1. 将插件放到项目目录下的 `Plugins/CBComfyUI`。
2. 打开 UE 项目并启用插件。
3. 重新生成工程文件并编译项目。
4. 在关卡中放置 `CBComfyUIManager`。
5. 在你的逻辑 Actor 上挂载 `CBComfyUISceneCaptureComponent`，并设置：
   - `CaptureCamera`
   - `ComfyUIManager`
   - `BaseMaterial`
   - `ReferenceTexture`
6. 对所有会参与贴图回写的 Static Mesh 资源，在 Asset Details 中勾选 `Allow CPU Access`。

## ComfyUI 侧准备

如果你使用的是仓库内示例工作流，建议按下面顺序准备 ComfyUI：

1. 安装 ComfyUI
   - https://github.com/comfyanonymous/ComfyUI
2. 安装 ComfyUI-nunchaku
   - 安装文档：https://nunchaku.tech/docs/ComfyUI-nunchaku/get_started/installation.html
   - 仓库地址：https://github.com/nunchaku-tech/ComfyUI-nunchaku
3. 安装 Nunchaku 后端
   - 使用文档：https://nunchaku.tech/docs/ComfyUI-nunchaku/get_started/usage.html
4. 启动 ComfyUI，默认地址通常是 `http://127.0.0.1:8188`

Windows 用户如果想快速验证，也可以直接参考 ComfyUI-nunchaku 的预打包发行页：

- https://github.com/nunchaku-tech/ComfyUI-nunchaku/releases

## 模型权重下载地址

本插件仓库不分发任何模型权重。下面这些链接是为了跑通仓库自带的 [Resources/工作流.json](Resources/工作流.json)。

使用前请注意：

- FLUX.1-dev 及其衍生权重通常需要先在 Hugging Face 页面接受许可
- 这些模型遵循各自上游许可证，不随本仓库一起授权
- 如果你修改了工作流节点，所需模型也会随之变化

### 示例工作流所需模型

| 用途 | 工作流文件名 | ComfyUI 目录 | 下载地址 |
| --- | --- | --- | --- |
| FLUX 文本编码器 CLIP-L | `clip_l.safetensors` | `ComfyUI/models/text_encoders` | https://huggingface.co/comfyanonymous/flux_text_encoders |
| FLUX 文本编码器 T5 | `t5xxl_fp16.safetensors` | `ComfyUI/models/text_encoders` | https://huggingface.co/comfyanonymous/flux_text_encoders |
| FLUX VAE | `flux-ae.safetensors` 或 `ae.safetensors` | `ComfyUI/models/vae` | https://huggingface.co/black-forest-labs/FLUX.1-dev |
| Nunchaku 4bit FLUX 主模型 | `svdq-int4_r32-flux.1-dev.safetensors` | `ComfyUI/models/unet` | https://huggingface.co/nunchaku-ai/nunchaku-flux.1-dev |
| Shakker Union ControlNet 2.0 | `Shakker-LabsFLUX.1-dev-ControlNet-Union-Pro-2.0.safetensors` | `ComfyUI/models/controlnet` | https://huggingface.co/Shakker-Labs/FLUX.1-dev-ControlNet-Union-Pro-2.0 |

### 推荐下载命令

下面这些命令适合已经安装 Hugging Face CLI 的环境：

```bash
hf download comfyanonymous/flux_text_encoders clip_l.safetensors --local-dir ComfyUI/models/text_encoders
hf download comfyanonymous/flux_text_encoders t5xxl_fp16.safetensors --local-dir ComfyUI/models/text_encoders
hf download black-forest-labs/FLUX.1-dev ae.safetensors --local-dir ComfyUI/models/vae
hf download nunchaku-ai/nunchaku-flux.1-dev svdq-int4_r32-flux.1-dev.safetensors --local-dir ComfyUI/models/unet
```

ControlNet 推荐直接在模型页的 Files 页面下载与你工作流匹配的 `safetensors` 文件：

- https://huggingface.co/Shakker-Labs/FLUX.1-dev-ControlNet-Union-Pro-2.0/tree/main

### 模型文件名注意事项

- VAE 在上游仓库里常见文件名是 `ae.safetensors`。如果你不想改工作流，可以把它重命名为 `flux-ae.safetensors`。
- 当前工作流固定使用 `svdq-int4_r32-flux.1-dev.safetensors`。如果你使用 RTX 50 系显卡，更建议改用 FP4，并同步修改工作流节点里的模型路径。
- 旧版文档里有时会出现 `nunchaku-tech/nunchaku-flux.1-dev`，当前公开模型页是 `nunchaku-ai/nunchaku-flux.1-dev`。

## UE 侧最小接入流程

推荐按下面顺序接入：

1. 在关卡里放置 `CBComfyUIManager`
2. 设置 `ComfyURL`，默认值是 `http://127.0.0.1:8188`
3. 运行时调用 `ConnectCommfyUI`
4. 打包环境下先调用 `ConfigureForRuntime`
5. 调用 `PrepareActorsAuto` 或 `PrepareActorsRuntime`
6. 调用 `ProcessMultipleActors` 上传控制图
7. 调用 `QueueWorkflowFromFile` 或 `QueueWorkflowFromString` 提交工作流 JSON
8. 监听 `OnComfyUIOutputFile`
9. 取返回文件名数组中的图片，调用 `ProcessRenderResults`

### 蓝图里最关键的节点

- `CBComfyUIManager -> ConnectCommfyUI`
- `CBComfyUIManager -> ConfigureForRuntime`
- `CBComfyUIManager -> QueueWorkflowFromFile`
- `CBComfyUIManager -> QueueWorkflowFromString`
- `CBComfyUIManager -> OnComfyUIOutputFile`
- `CBComfyUISceneCaptureComponent -> ProcessMultipleActors`
- `CBComfyUISceneCaptureComponent -> ProcessRenderResults`
- `CBComfyUIBlueprintLibrary -> PrepareActorsAuto`

## 示例工作流说明

仓库中的 [Resources/工作流.json](插件/Resources/工作流.json) 目前对应的是一条基于深度图的 FLUX 工作流：

- UE 插件会上传 `depth.png`、`normals.png`、`color.png`、`edge_mask.png`
- 当前工作流实际上只读取 `depth.png`
- ComfyUI 使用 `FLUX.1-dev + Union ControlNet(depth) + Nunchaku 4bit DiT` 生成结果图
- `SaveImage` 节点输出文件名，插件再根据这个文件名下载图片并回写材质

如果你想动态改 prompt 或采样参数，可以使用 `SetNodeInputStringProperty`、`SetNodeInputIntProperty`、`SetNodeInputDoubleProperty` 在提交前修改节点输入。

## 打包版注意事项

### 1. Static Mesh 必须开启 Allow CPU Access

插件在回写阶段需要读取 Static Mesh 的 CPU 顶点、UV 和索引数据。对于所有参与这条流程的静态网格，请在资源设置中开启：

- `Allow CPU Access`

否则在打包版中这些数据可能被 cook 裁掉，导致回写阶段失败或直接不可用。

### 2. 工作流 JSON 是原始文件路径，不是 UAsset

`QueueWorkflowFromFile` 读取的是磁盘上的原始 JSON 文件，所以打包后不要默认认为 `Plugins/CBComfyUI/Resources/工作流.json` 一定还会按相同路径存在。

推荐做法有两个：

- 在打包设置中把工作流目录加入 `Additional Non-Asset Directories to Copy`
- 或者运行时先把 JSON 读成字符串，再调用 `QueueWorkflowFromString`

如果你主要面向打包版，第二种方式更稳。

### 3. 基础材质参数名要匹配

基础材质中需要有名为 `BaseColor` 的纹理参数，否则生成结果无法正确写回。

## 常见问题

### 打开 UE 后能连接，但没有出图

优先检查：

1. ComfyUI 是否真的加载到了工作流所需模型
2. 示例工作流里的 `LoadImage` 节点是否确实读到了 `depth.png`
3. `OnComfyUIOutputFile` 是否成功收到 `SaveImage` 返回的文件名
4. ComfyUI 地址是否与 `ComfyURL` 一致

### 编辑器里能用，打包后不能用

优先检查：

1. 是否调用了 `ConfigureForRuntime`
2. 是否使用了 `PrepareActorsAuto` 或 `PrepareActorsRuntime`
3. 目标 Static Mesh 是否开启了 `Allow CPU Access`
4. 工作流 JSON 是否被一并拷贝进打包目录，或者是否已经改成 `QueueWorkflowFromString`

### 模型名不匹配

如果你下载到的模型文件名和工作流中的名字不一致，有两种处理方式：

- 直接重命名模型文件
- 修改工作流对应节点里的文件名字段

## 代码入口

如果你要继续扩展插件，优先看这几个文件：

- [Source/CBComfyUI/Public/CBComfyUIManager.h](Source/CBComfyUI/Public/CBComfyUIManager.h)
- [Source/CBComfyUI/Private/CBComfyUIManager.cpp](Source/CBComfyUI/Private/CBComfyUIManager.cpp)
- [Source/CBComfyUI/Public/CBComfyUISceneCaptureComponent.h](Source/CBComfyUI/Public/CBComfyUISceneCaptureComponent.h)
- [Source/CBComfyUI/Private/CBComfyUISceneCaptureComponent.cpp](Source/CBComfyUI/Private/CBComfyUISceneCaptureComponent.cpp)
- [Source/CBComfyUI/Public/CBComfyUIBlueprintLibrary.h](Source/CBComfyUI/Public/CBComfyUIBlueprintLibrary.h)

如果你只想直接跑通仓库里的示例流程，建议先看 [Resources/工作流使用说明.md](Resources/工作流使用说明.md)。
=======
# CBComfyUI

CBComfyUI 是一个 Unreal Engine 插件，用于把 ComfyUI 工作流接入 UE 场景流程。插件会从场景中生成控制图，提交 prompt JSON 到 ComfyUI，接收生成结果，再把结果图回写到目标 Actor 的材质上。

当前仓库中的实现已经覆盖：

- 编辑器模式下使用
- 打包后的 Runtime 模式使用
- ComfyUI 的 HTTP / WebSocket 通信
- 基于深度图的工作流回写
- 运行时动态纹理和动态材质实例管理

如果你直接使用本仓库自带的示例工作流，请先阅读模型下载和 ComfyUI 侧准备部分；该工作流依赖 FLUX.1-dev、Nunchaku 和 ControlNet。

## 功能一览

- 从 UE 场景生成 `depth.png`、`normals.png`、`color.png`、`edge_mask.png`
- 通过 `QueueWorkflowFromFile` / `QueueWorkflowFromString` 提交 ComfyUI prompt JSON
- 通过 `OnComfyUIOutputFile` 接收 `SaveImage` 输出文件名
- 将生成结果重新投影到 Static Mesh 材质
- 支持编辑器与打包版两套资源路径和运行时资源管理
- 支持在 UE 中动态修改 prompt、steps、guidance、ControlNet strength 等节点输入

## 重要限制

如果你要使用“控制图上传 -> ComfyUI 生成 -> 结果贴图回写到 Static Mesh”这条链路，那么所有参与回写流程的 Static Mesh 资源都必须在资源详情里勾选：

- `Allow CPU Access`

这不是可选优化，而是硬性要求。插件在回写阶段需要读取 Static Mesh 的 CPU 顶点、UV 和索引数据；如果没有开启，尤其是在打包版里，这些数据通常会被 cook 裁掉，直接导致回写失败、贴图不生效，或者表现为编辑器里能用但打包后不能用。


## 环境要求

- Unreal Engine 5.5 项目环境
- Windows
- NVIDIA GPU
- 可正常运行的 ComfyUI
- 如果使用仓库内示例工作流，需要安装 ComfyUI-nunchaku 和 Nunchaku 后端

另外，目标材质需要有一个名为 `BaseColor` 的纹理参数，插件会把生成结果写到这个参数上。

## 安装插件

1. 将插件放到项目目录下的 `Plugins/CBComfyUI`。
2. 打开 UE 项目并启用插件。
3. 重新生成工程文件并编译项目。
4. 在关卡中放置 `CBComfyUIManager`。
5. 在你的逻辑 Actor 上挂载 `CBComfyUISceneCaptureComponent`，并设置：
   - `CaptureCamera`
   - `ComfyUIManager`
   - `BaseMaterial`
   - `ReferenceTexture`
6. 对所有会参与贴图回写的 Static Mesh 资源，在 Asset Details 中勾选 `Allow CPU Access`。

## ComfyUI 侧准备

如果你使用的是仓库内示例工作流，建议按下面顺序准备 ComfyUI：

1. 安装 ComfyUI
   - https://github.com/comfyanonymous/ComfyUI
2. 安装 ComfyUI-nunchaku
   - 安装文档：https://nunchaku.tech/docs/ComfyUI-nunchaku/get_started/installation.html
   - 仓库地址：https://github.com/nunchaku-tech/ComfyUI-nunchaku
3. 安装 Nunchaku 后端
   - 使用文档：https://nunchaku.tech/docs/ComfyUI-nunchaku/get_started/usage.html
4. 启动 ComfyUI，默认地址通常是 `http://127.0.0.1:8188`

Windows 用户如果想快速验证，也可以直接参考 ComfyUI-nunchaku 的预打包发行页：

- https://github.com/nunchaku-tech/ComfyUI-nunchaku/releases

## 模型权重下载地址

本插件仓库不分发任何模型权重。下面这些链接是为了跑通仓库自带的 [Resources/工作流.json](Resources/工作流.json)。

使用前请注意：

- FLUX.1-dev 及其衍生权重通常需要先在 Hugging Face 页面接受许可
- 这些模型遵循各自上游许可证，不随本仓库一起授权
- 如果你修改了工作流节点，所需模型也会随之变化

### 示例工作流所需模型

| 用途 | 工作流文件名 | ComfyUI 目录 | 下载地址 |
| --- | --- | --- | --- |
| FLUX 文本编码器 CLIP-L | `clip_l.safetensors` | `ComfyUI/models/text_encoders` | https://huggingface.co/comfyanonymous/flux_text_encoders |
| FLUX 文本编码器 T5 | `t5xxl_fp16.safetensors` | `ComfyUI/models/text_encoders` | https://huggingface.co/comfyanonymous/flux_text_encoders |
| FLUX VAE | `flux-ae.safetensors` 或 `ae.safetensors` | `ComfyUI/models/vae` | https://huggingface.co/black-forest-labs/FLUX.1-dev |
| Nunchaku 4bit FLUX 主模型 | `svdq-int4_r32-flux.1-dev.safetensors` | `ComfyUI/models/unet` | https://huggingface.co/nunchaku-ai/nunchaku-flux.1-dev |
| Shakker Union ControlNet 2.0 | `Shakker-LabsFLUX.1-dev-ControlNet-Union-Pro-2.0.safetensors` | `ComfyUI/models/controlnet` | https://huggingface.co/Shakker-Labs/FLUX.1-dev-ControlNet-Union-Pro-2.0 |

### 推荐下载命令

下面这些命令适合已经安装 Hugging Face CLI 的环境：

```bash
hf download comfyanonymous/flux_text_encoders clip_l.safetensors --local-dir ComfyUI/models/text_encoders
hf download comfyanonymous/flux_text_encoders t5xxl_fp16.safetensors --local-dir ComfyUI/models/text_encoders
hf download black-forest-labs/FLUX.1-dev ae.safetensors --local-dir ComfyUI/models/vae
hf download nunchaku-ai/nunchaku-flux.1-dev svdq-int4_r32-flux.1-dev.safetensors --local-dir ComfyUI/models/unet
```

ControlNet 推荐直接在模型页的 Files 页面下载与你工作流匹配的 `safetensors` 文件：

- https://huggingface.co/Shakker-Labs/FLUX.1-dev-ControlNet-Union-Pro-2.0/tree/main

### 模型文件名注意事项

- VAE 在上游仓库里常见文件名是 `ae.safetensors`。如果你不想改工作流，可以把它重命名为 `flux-ae.safetensors`。
- 当前工作流固定使用 `svdq-int4_r32-flux.1-dev.safetensors`。如果你使用 RTX 50 系显卡，更建议改用 FP4，并同步修改工作流节点里的模型路径。
- 旧版文档里有时会出现 `nunchaku-tech/nunchaku-flux.1-dev`，当前公开模型页是 `nunchaku-ai/nunchaku-flux.1-dev`。

## UE 侧最小接入流程

推荐按下面顺序接入：

1. 在关卡里放置 `CBComfyUIManager`
2. 设置 `ComfyURL`，默认值是 `http://127.0.0.1:8188`
3. 运行时调用 `ConnectCommfyUI`
4. 打包环境下先调用 `ConfigureForRuntime`
5. 调用 `PrepareActorsAuto` 或 `PrepareActorsRuntime`
6. 调用 `ProcessMultipleActors` 上传控制图
7. 调用 `QueueWorkflowFromFile` 或 `QueueWorkflowFromString` 提交工作流 JSON
8. 监听 `OnComfyUIOutputFile`
9. 取返回文件名数组中的图片，调用 `ProcessRenderResults`

### 蓝图里最关键的节点

- `CBComfyUIManager -> ConnectCommfyUI`
- `CBComfyUIManager -> ConfigureForRuntime`
- `CBComfyUIManager -> QueueWorkflowFromFile`
- `CBComfyUIManager -> QueueWorkflowFromString`
- `CBComfyUIManager -> OnComfyUIOutputFile`
- `CBComfyUISceneCaptureComponent -> ProcessMultipleActors`
- `CBComfyUISceneCaptureComponent -> ProcessRenderResults`
- `CBComfyUIBlueprintLibrary -> PrepareActorsAuto`

## 示例工作流说明


- UE 插件会上传 `depth.png`、`normals.png`、`color.png`、`edge_mask.png`
- 当前工作流实际上只读取 `depth.png`
- ComfyUI 使用 `FLUX.1-dev + Union ControlNet(depth) + Nunchaku 4bit DiT` 生成结果图
- `SaveImage` 节点输出文件名，插件再根据这个文件名下载图片并回写材质

如果你想动态改 prompt 或采样参数，可以使用 `SetNodeInputStringProperty`、`SetNodeInputIntProperty`、`SetNodeInputDoubleProperty` 在提交前修改节点输入。

## 打包版注意事项

### 1. Static Mesh 必须开启 Allow CPU Access

插件在回写阶段需要读取 Static Mesh 的 CPU 顶点、UV 和索引数据。对于所有参与这条流程的静态网格，请在资源设置中开启：

- `Allow CPU Access`

否则在打包版中这些数据可能被 cook 裁掉，导致回写阶段失败或直接不可用。

### 2. 工作流 JSON 是原始文件路径，不是 UAsset

`QueueWorkflowFromFile` 读取的是磁盘上的原始 JSON 文件，所以打包后不要默认认为 `Plugins/CBComfyUI/Resources/工作流.json` 一定还会按相同路径存在。

推荐做法有两个：

- 在打包设置中把工作流目录加入 `Additional Non-Asset Directories to Copy`
- 或者运行时先把 JSON 读成字符串，再调用 `QueueWorkflowFromString`

如果你主要面向打包版，第二种方式更稳。

### 3. 基础材质参数名要匹配

基础材质中需要有名为 `BaseColor` 的纹理参数，否则生成结果无法正确写回。

## 常见问题

### 打开 UE 后能连接，但没有出图

优先检查：

1. ComfyUI 是否真的加载到了工作流所需模型
2. 示例工作流里的 `LoadImage` 节点是否确实读到了 `depth.png`
3. `OnComfyUIOutputFile` 是否成功收到 `SaveImage` 返回的文件名
4. ComfyUI 地址是否与 `ComfyURL` 一致

### 编辑器里能用，打包后不能用

优先检查：

1. 是否调用了 `ConfigureForRuntime`
2. 是否使用了 `PrepareActorsAuto` 或 `PrepareActorsRuntime`
3. 目标 Static Mesh 是否开启了 `Allow CPU Access`
4. 工作流 JSON 是否被一并拷贝进打包目录，或者是否已经改成 `QueueWorkflowFromString`

### 模型名不匹配

如果你下载到的模型文件名和工作流中的名字不一致，有两种处理方式：

- 直接重命名模型文件
- 修改工作流对应节点里的文件名字段
