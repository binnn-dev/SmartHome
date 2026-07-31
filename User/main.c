#include "stm32f10x.h"
#include "Delay.h"
#include "OLED.h"
#include "Bsp_Config.h"
#include "DHT11.h"
#include "ESP8266.h"
#include <stdio.h>

#define LIMIT_TEMP  38
#define LIMIT_HUMI  80
#define LIMIT_GAS   3000
#define LIMIT_SMOKE 3500

/* 4 行 OLED 每行最多显示 16 个字符。
   这里统一补空格，避免数值位数变化时旧字符残留在屏幕上。 */
static void OLED_ShowLine(uint8_t line, char *text) {
    char buf[17];
    uint8_t i;

    for (i = 0; i < 16; i++) {
        buf[i] = ' ';
    }
    buf[16] = '\0';

    for (i = 0; i < 16 && text[i] != '\0'; i++) {
        buf[i] = text[i];
    }

    OLED_ShowString(line, 1, buf);
}

int main(void) {
    /* DHT11 返回整数温湿度，当前和安卓端联调时按整数发送。 */
    uint8_t dht_temp = 0;
    uint8_t dht_humi = 0;
    uint8_t dht_ok;
    /* 门磁 / 红外报警位，约定 0=正常，1=报警。 */
    uint8_t door_alarm;
    uint8_t pir_alarm;
    /* 烟雾 / 燃气先采 ADC，再根据阈值换算成 0/1 报警位。 */
    uint8_t smoke_alarm;
    uint8_t gas_alarm;
    /* 总报警位用于蜂鸣器联动和 OLED 状态显示。 */
    uint8_t alarm_active;
    /* 保存烟雾 / 燃气 ADC 原始值，OLED 显示原始值，WiFi 只发报警位。 */
    uint16_t gas;
    uint16_t smoke;
    char oled_line[32];
    /* 安卓端协议固定为：门磁,红外,温度,湿度,烟雾,燃气\n */
    char wifi_data[64];

    /* 启动顺序优先点亮 OLED，避免 ESP 初始化等待阶段看起来像“黑屏死机”。 */
    Bsp_Init();
    OLED_Init();
    OLED_ShowLine(1, "System Booting");
    OLED_ShowLine(2, "Init Sensors...");
    DHT11_Init();
    OLED_ShowLine(2, "Init WiFi...");
    ESP8266_Init();
    OLED_ShowLine(1, "System Online");
    OLED_ShowLine(2, "WiFi Ready");
    OLED_ShowLine(3, "IP 192.168.4.1");
    OLED_ShowLine(4, "Port 8080");
    Delay_ms(800);
    OLED_Clear();

    while (1) {
        /* 直接读取门磁和红外输入引脚。
           如果以后更换模块，只需要在这里统一调整触发极性。 */
        door_alarm = (GET_DOOR != 0);
        pir_alarm = (GET_PIR != 0);

        /* 门磁或红外任一触发时亮红灯，否则亮绿灯。 */
        if (door_alarm || pir_alarm) {
            LED_RED(0);
            LED_GREEN(1);
        } else {
            LED_RED(1);
            LED_GREEN(0);
        }

        /* DHT11 读取失败时清零，避免把上一次的旧值继续发送给安卓端。 */
        dht_ok = DHT11_Read(&dht_temp, &dht_humi);
        if (!dht_ok) {
            dht_temp = 0;
            dht_humi = 0;
        }

        /* 本地保留烟雾 / 燃气 ADC 原始值，便于在 OLED 上观察传感器变化趋势。 */
        smoke = Get_ADC_Value(ADC_Channel_7);
        gas = Get_ADC_Value(ADC_Channel_9);

        /* 安卓端要求烟雾 / 燃气字段发 0/1，所以这里用阈值把 ADC 值转换成报警位。 */
        smoke_alarm = (smoke > LIMIT_SMOKE) ? 1 : 0;
        gas_alarm = (gas > LIMIT_GAS) ? 1 : 0;

        /* 任一超限就触发总报警，驱动蜂鸣器。 */
        alarm_active = (dht_temp > LIMIT_TEMP || dht_humi > LIMIT_HUMI ||
                        gas_alarm || smoke_alarm);

        if (alarm_active) {
            BEEP(1);
        } else {
            BEEP(0);
        }

        /* 第 1 行显示温湿度和 DHT 状态，DHT 失败时显示 ER。 */
        if (dht_ok) {
            sprintf(oled_line, "T:%2uC H:%2u%% OK", dht_temp, dht_humi);
        } else {
            sprintf(oled_line, "T:--C H:--%% ER");
        }
        OLED_ShowLine(1, oled_line);

        sprintf(oled_line, "GAS :%4u", gas);
        OLED_ShowLine(2, oled_line);

        sprintf(oled_line, "SMOK:%4u", smoke);
        OLED_ShowLine(3, oled_line);

        /* 第 4 行压缩显示各个报警位：Door / PIR / Smoke / Gas / Alarm。 */
        sprintf(oled_line, "D%u P%u S%u G%u A%u",
                door_alarm ? 1 : 0,
                pir_alarm ? 1 : 0,
                smoke_alarm,
                gas_alarm,
                alarm_active ? 1 : 0);
        OLED_ShowLine(4, oled_line);

        /* 严格匹配安卓端 Socket 协议：
           门磁,红外,温度,湿度,烟雾,燃气\n
           其中门磁/红外/烟雾/燃气都是 0/1，温湿度当前先发整数。 */
        sprintf(wifi_data,
                "%d,%d,%d,%d,%d,%d\n",
                door_alarm ? 1 : 0,
                pir_alarm ? 1 : 0,
                dht_temp,
                dht_humi,
                smoke_alarm,
                gas_alarm);	

        /* 发送给 ESP8266，再由 ESP8266 TCP Server 转发给安卓客户端。 */
        ESP8266_SendString(wifi_data);

        /* 1 秒刷新一次，避免 OLED 刷新过快，也给 WiFi 发送留出缓冲时间。 */
        Delay_ms(1000);
    }
}
