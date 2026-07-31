#include "ESP8266.h"
#include "Bsp_Config.h"
#include "OLED.h"
#include "Serial.h"
#include "Delay.h"
#include <stdio.h>
#include <string.h>

static void ESP8266_SendRawString(const char *str);
static void ESP8266_SendCommand(const char *cmd, uint32_t delay_ms);
static void ESP8266_ShowStatus(const char *line1, const char *line2);

static void ESP8266_SendRawString(const char *str) {
    Serial_SendString(str);
}

/* OLED 每行固定 16 个字符，这里统一补空格，避免旧内容残留。 */
static void ESP8266_ShowStatus(const char *line1, const char *line2) {
    char buf1[17];
    char buf2[17];
    uint8_t i;

    for (i = 0; i < 16; i++) {
        buf1[i] = ' ';
        buf2[i] = ' ';
    }
    buf1[16] = '\0';
    buf2[16] = '\0';

    for (i = 0; i < 16 && line1[i] != '\0'; i++) {
        buf1[i] = line1[i];
    }
    for (i = 0; i < 16 && line2[i] != '\0'; i++) {
        buf2[i] = line2[i];
    }

    OLED_ShowString(1, 1, buf1);
    OLED_ShowString(2, 1, buf2);
}

void ESP8266_Init(void) {
    /* 对应当前硬件连接，B4 拉高等价于把 EN 拉到 3V3。 */
    ESP_EN(1);
    ESP_RST(1);
    ESP_IO0(1);
    ESP_IO2(1);

    /* 改用硬件 USART2(PA2/PA3) 和 ESP8266 通信，稳定性高于原软件串口。 */
    ESP8266_ShowStatus("WiFi Booting...", "UART2 @115200");
    Serial_Init(115200);
    Delay_ms(1500);

    /* 使用主工程直接完成 ESP8266 配置。
       这里保留必要等待，同时在 OLED 上提示当前阶段。 */
    ESP8266_ShowStatus("WiFi Config...", "AT");
    ESP8266_SendCommand("AT\r\n", 800);

    ESP8266_ShowStatus("WiFi Config...", "AP Mode");
    ESP8266_SendCommand("AT+CWMODE=2\r\n", 800);

    ESP8266_ShowStatus("WiFi Config...", "Set AP Info");
    ESP8266_SendCommand("AT+CWSAP=\"SmartHome_WiFi\",\"12345678\",1,3\r\n", 1800);

    ESP8266_ShowStatus("WiFi Config...", "Enable MUX");
    ESP8266_SendCommand("AT+CIPMUX=1\r\n", 800);

    ESP8266_ShowStatus("WiFi Config...", "Reset Server");
    ESP8266_SendCommand("AT+CIPSERVER=0\r\n", 500);

    ESP8266_ShowStatus("WiFi Config...", "Open 8080");
    ESP8266_SendCommand("AT+CIPSERVER=1,8080\r\n", 1000);

    ESP8266_ShowStatus("WiFi Ready", "192.168.4.1");
    Delay_ms(600);
}

static void ESP8266_SendCommand(const char *cmd, uint32_t delay_ms) {
    ESP8266_SendRawString(cmd);
    Delay_ms(delay_ms);
}

void ESP8266_SendString(const char *str) {
    char cmd[32];
    uint16_t len = (uint16_t)strlen(str);

    /* 手机连上热点并建立 socket 后，默认向 0 号客户端转发 CSV 数据。 */
    sprintf(cmd, "AT+CIPSEND=0,%u\r\n", len);
    ESP8266_SendRawString(cmd);
    Delay_ms(120);
    ESP8266_SendRawString(str);
    Delay_ms(150);
}
