# 智能家居多传感器监控系统 (Smart Home Sensor System)

本项目是一套基于 STM32F103 的智能家居环境监测全栈解决方案。通过多种传感器实时采集室内数据，利用 ESP8266 构建 TCP 局域网服务器，并在 Android 端实现实时数据监控与阈值报警。

---

## 🌿 仓库分支说明 (Branch Directory)

为了保证各端代码及硬件设计的独立与干净，本项目采用 **多分支管理架构**。请根据需求切换或拉取对应分支：

| 分支名称         | 包含内容                              | 适用 IDE / 工具               |
| :--------------- | :------------------------------------ | :---------------------------- |
| 🚩 **`main`**     | 项目总体文档、通信协议、演示效果      | Markdown 阅读                 |
| 🔌 **`firmware`** | STM32F103C8T6 下位机 C 源码及工程文件 | Keil MDK v5                   |
| 📱 **`app`**      | Android 客户端源码                    | Android Studio                |
| 📐 **`hardware`** | 硬件原理图、PCB 图纸及 BOM 清单       | Altium Designer / KiCad / PDF |

---

## 🚀 快速拉取指定模块代码

如果你只想获取项目的某一部分，可以使用以下命令：

### 1. 只获取下位机固件代码
```bash
git clone -b firmware [https://github.com/kuailedetaiyang/SmartHome.git](https://github.com/kuailedetaiyang/SmartHome.git)
```

### 2. 只获取 App 源码

```bash
git clone -b app [https://github.com/kuailedetaiyang/SmartHome.git](https://github.com/kuailedetaiyang/SmartHome.git)
```

### 3. 获取硬件原理图资料

```bash
git clone -b hardware [https://github.com/kuailedetaiyang/SmartHome.git](https://github.com/kuailedetaiyang/SmartHome.git)
```

## 📡 核心通信协议简述

- **传输层**: ESP8266 AP 模式 (TCP Server: `192.168.4.1:8080`)

- **数据格式**: CSV 格式，1 秒周期推送

  ```
  门磁,红外,温度,湿度,烟雾,燃气\n
  示例: 0,0,25,60,0,0
  ```

> 详细的寄存器定义与硬件接线表，请在切换至 `hardware` 或 `firmware` 分支后查看其独立文档。
