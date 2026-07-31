# STM32 智能家居传感器项目接口文档

## 项目概述

本项目基于 STM32F103 微控制器，集成了多种传感器模块，通过 ESP8266 WiFi 模块实现与安卓端的数据通信。系统支持门磁检测、人体红外感应、温湿度监测、燃气检测和烟雾检测功能。

---

## 硬件接口定义

### 1. GPIO 引脚分配表

| 功能模块 | 信号名称 | 引脚 | 方向 | 说明 |
|---------|---------|------|------|------|
| **LED 指示** | LED_RED | PB13 | 输出 | 红色LED，报警时亮 |
| | LED_GREEN | PB12 | 输出 | 绿色LED，正常时亮 |
| **蜂鸣器** | BEEP | PA12 | 输出 | 报警蜂鸣器 |
| **ESP8266 控制** | ESP_EN | PB4 | 输出 | ESP8266 使能 |
| | ESP_RST | PB5 | 输出 | ESP8266 复位 |
| | ESP_IO0 | PB6 | 输出 | ESP8266 GPIO0 |
| | ESP_IO2 | PB7 | 输出 | ESP8266 GPIO2 |
| **数字输入** | DOOR (门磁) | PA0 | 输入 | 门磁传感器 |
| | PIR (红外) | PA5 | 输入 | 人体红外传感器 |
| | SMOKE_DO | PA6 | 输入 | 烟雾传感器数字输出 |
| | GAS_DO | PA10 | 输入 | 燃气传感器数字输出 |
| **ADC 输入** | SMOKE_ADC | PA7 | 模拟 | 烟雾传感器模拟输出 (ADC_Channel_7) |
| | GAS_ADC | PB1 | 模拟 | 燃气传感器模拟输出 (ADC_Channel_9) |
| **DHT11** | DHT11_DATA | PA4 | 双向 | 温湿度传感器数据 |
| **OLED** | OLED_SCL | PB10 | 输出 | OLED I2C 时钟 |
| | OLED_SDA | PB11 | 输出 | OLED I2C 数据 |
| **串口通信** | USART2_TX | PA2 | 输出 | ESP8266 通信发送 |
| | USART2_RX | PA3 | 输入 | ESP8266 通信接收 |

---

## 软件模块接口

### 1. Bsp_Config 模块 (Bsp_Config.h)

硬件抽象层，负责 GPIO 和 ADC 初始化。

#### 宏定义

| 宏名 | 参数 | 功能 |
|------|------|------|
| `LED_RED(x)` | x: 0/1 | 控制红色LED，1=亮，0=灭 |
| `LED_GREEN(x)` | x: 0/1 | 控制绿色LED，1=亮，0=灭 |
| `BEEP(x)` | x: 0/1 | 控制蜂鸣器，1=响，0=停 |
| `ESP_EN(x)` | x: 0/1 | ESP8266 使能控制 |
| `ESP_RST(x)` | x: 0/1 | ESP8266 复位控制 |
| `ESP_IO0(x)` | x: 0/1 | ESP8266 GPIO0 控制 |
| `ESP_IO2(x)` | x: 0/1 | ESP8266 GPIO2 控制 |
| `GET_DOOR` | - | 读取门磁状态 (非0=触发) |
| `GET_PIR` | - | 读取红外状态 (非0=触发) |
| `GET_SMOKE_DO` | - | 读取烟雾数字输出 |
| `GET_GAS_DO` | - | 读取燃气数字输出 |

#### 函数接口

```c
void Bsp_Init(void);
```
- **功能**: 初始化所有 GPIO 和 ADC
- **调用时机**: 系统启动时

```c
uint16_t Get_ADC_Value(uint8_t adc_channel);
```
- **功能**: 读取指定 ADC 通道的采样值
- **参数**: `adc_channel` - ADC 通道号 (ADC_Channel_7 或 ADC_Channel_9)
- **返回**: 12位 ADC 转换结果 (0-4095)

---

### 2. DHT11 模块 (DHT11.h)

温湿度传感器驱动。

#### 函数接口

```c
void DHT11_Init(void);
```
- **功能**: 初始化 DHT11 传感器
- **调用时机**: 系统启动时

```c
uint8_t DHT11_Read(uint8_t *temp, uint8_t *humi);
```
- **功能**: 读取温湿度数据
- **参数**:
  - `temp`: 输出参数，温度值 (整数，单位℃)
  - `humi`: 输出参数，湿度值 (整数，单位%)
- **返回**: 1=成功，0=失败

---

### 3. ESP8266 模块 (ESP8266.h)

WiFi 通信模块驱动。

#### 函数接口

```c
void ESP8266_Init(void);
```
- **功能**: 初始化 ESP8266，配置为 AP 模式并启动 TCP Server
- **配置参数**:
  - WiFi SSID: `SmartHome_WiFi`
  - WiFi 密码: `12345678`
  - 信道: 1
  - 加密方式: WPA2_PSK
  - 服务器端口: 8080
  - IP地址: 192.168.4.1
- **调用时机**: 系统启动时

```c
void ESP8266_SendString(const char *str);
```
- **功能**: 通过 ESP8266 发送字符串到连接的客户端
- **参数**: `str` - 要发送的字符串
- **说明**: 自动添加 AT+CIPSEND 指令前缀

---

### 4. OLED 模块 (OLED.h)

OLED 显示屏驱动 (I2C 接口，128x64 分辨率)。

#### 函数接口

```c
void OLED_Init(void);
```
- **功能**: 初始化 OLED 显示屏
- **调用时机**: 系统启动时

```c
void OLED_Clear(void);
```
- **功能**: 清屏

```c
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);
```
- **功能**: 显示单个字符
- **参数**:
  - `Line`: 行号 (1-4)
  - `Column`: 列号 (1-16)
  - `Char`: 要显示的字符

```c
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);
```
- **功能**: 显示字符串
- **参数**:
  - `Line`: 行号 (1-4)
  - `Column`: 列号 (1-16)
  - `String`: 要显示的字符串

```c
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
```
- **功能**: 显示无符号十进制数字
- **参数**:
  - `Line`: 行号 (1-4)
  - `Column`: 列号 (1-16)
  - `Number`: 要显示的数字
  - `Length`: 显示位数

```c
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);
```
- **功能**: 显示带符号十进制数字

```c
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
```
- **功能**: 显示十六进制数字

```c
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);
```
- **功能**: 显示二进制数字

---

### 5. Serial 模块 (Serial.h)

串口通信驱动 (USART2)。

#### 函数接口

```c
void Serial_Init(uint32_t baud_rate);
```
- **功能**: 初始化串口
- **参数**: `baud_rate` - 波特率 (如 115200)

```c
void Serial_SendString(const char *string);
```
- **功能**: 发送字符串
- **参数**: `string` - 要发送的字符串

```c
uint8_t Serial_ReadByte(uint8_t *byte, uint32_t timeout_ms);
```
- **功能**: 读取单个字节
- **参数**:
  - `byte`: 输出参数，读取到的字节
  - `timeout_ms`: 超时时间 (毫秒)
- **返回**: 1=成功，0=超时

```c
uint16_t Serial_ReadResponse(char *buffer, uint16_t buffer_size, uint32_t timeout_ms);
```
- **功能**: 读取响应数据
- **参数**:
  - `buffer`: 数据缓冲区
  - `buffer_size`: 缓冲区大小
  - `timeout_ms`: 超时时间 (毫秒)
- **返回**: 实际读取的字节数

---

### 6. Delay 模块 (Delay.h)

延时函数。

#### 函数接口

```c
void Delay_us(uint32_t us);
```
- **功能**: 微秒级延时

```c
void Delay_ms(uint32_t ms);
```
- **功能**: 毫秒级延时

```c
void Delay_s(uint32_t s);
```
- **功能**: 秒级延时

---

## 通信协议

### WiFi 数据格式

ESP8266 作为 TCP Server，监听端口 8080。数据发送格式为 CSV 格式：

```
门磁,红外,温度,湿度,烟雾,燃气\n
```

#### 字段说明

| 字段 | 类型 | 说明 |
|------|------|------|
| 门磁 | 0/1 | 0=正常，1=报警(门打开) |
| 红外 | 0/1 | 0=正常，1=报警(有人) |
| 温度 | 整数 | 摄氏度 (℃) |
| 湿度 | 整数 | 百分比 (%) |
| 烟雾 | 0/1 | 0=正常，1=报警(超过阈值) |
| 燃气 | 0/1 | 0=正常，1=报警(超过阈值) |

#### 示例数据

```
0,0,25,60,0,0
```
表示：门磁正常、无人、温度25℃、湿度60%、烟雾正常、燃气正常

```
1,1,38,85,1,0
```
表示：门打开、有人、温度38℃、湿度85%、烟雾报警、燃气正常

---

## 报警阈值配置

在 `main.c` 中定义：

| 参数 | 默认值 | 说明 |
|------|--------|------|
| `LIMIT_TEMP` | 38 | 温度报警阈值 (℃) |
| `LIMIT_HUMI` | 80 | 湿度报警阈值 (%) |
| `LIMIT_GAS` | 3000 | 燃气 ADC 报警阈值 |
| `LIMIT_SMOKE` | 3500 | 烟雾 ADC 报警阈值 |

---

## 系统启动流程

1. **Bsp_Init()** - 初始化 GPIO 和 ADC
2. **OLED_Init()** - 初始化显示屏
3. **DHT11_Init()** - 初始化温湿度传感器
4. **ESP8266_Init()** - 初始化 WiFi 模块

---

## 主循环逻辑

```
while(1) {
    1. 读取门磁和红外状态
    2. 根据状态切换 LED (红/绿)
    3. 读取 DHT11 温湿度
    4. 读取烟雾和燃气 ADC 值
    5. 判断是否超过阈值，触发蜂鸣器
    6. 更新 OLED 显示
    7. 发送数据到 WiFi
    8. 延时 1 秒
}
```

---

## OLED 显示布局

| 行号 | 显示内容 | 示例 |
|------|---------|------|
| 第1行 | 温湿度状态 | `T:25C H:60% OK` 或 `T:--C H:--%% ER` |
| 第2行 | 燃气 ADC 值 | `GAS :1234` |
| 第3行 | 烟雾 ADC 值 | `SMOK:2345` |
| 第4行 | 报警状态位 | `D0 P0 S0 G0 A0` |

**第4行状态位说明**:
- D: 门磁 (Door)
- P: 红外 (PIR)
- S: 烟雾 (Smoke)
- G: 燃气 (Gas)
- A: 总报警 (Alarm)

---

## 文件结构

```
门磁红外温湿度燃气烟雾esp8266/
├── Hardware/           # 硬件驱动层
│   ├── Bsp_Config.c/h  # GPIO/ADC 配置
│   ├── DHT11.c/h       # 温湿度传感器
│   ├── ESP8266.c/h     # WiFi 模块
│   ├── OLED.c/h        # OLED 显示屏
│   ├── OLED_Font.h     # 字库
│   └── Serial.c/h      # 串口通信
├── System/             # 系统层
│   ├── Delay.c/h       # 延时函数
├── User/               # 应用层
│   ├── main.c          # 主程序
│   ├── stm32f10x_conf.h
│   ├── stm32f10x_it.c/h
├── Library/            # STM32 标准库
├── Start/              # 启动文件
└── Project.uvprojx     # Keil 工程文件
```

---

## 版本信息

- **MCU**: STM32F103C8T6
- **开发环境**: Keil MDK-ARM
- **标准库**: STM32F10x_StdPeriph_Lib
- **最后更新**: 2024

---

## 注意事项

1. ESP8266 使用硬件 USART2 (PA2/PA3) 通信，波特率 115200
2. DHT11 数据引脚为 PA4
3. OLED 使用软件 I2C (PB10/PB11)，地址 0x78
4. 主循环周期为 1 秒，避免数据发送过于频繁
5. ADC 采样使用单次模式，每次读取前切换通道
