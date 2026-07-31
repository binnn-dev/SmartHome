#include "stm32f10x.h"
#include "OLED_Font.h"

#define OLED_W_SCL(x)   GPIO_WriteBit(GPIOB, GPIO_Pin_10, (BitAction)(x))
#define OLED_W_SDA(x)   GPIO_WriteBit(GPIOB, GPIO_Pin_11, (BitAction)(x))

static void OLED_I2C_Init(void) {
    GPIO_InitTypeDef gpio_init;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    gpio_init.GPIO_Mode = GPIO_Mode_Out_OD;
    gpio_init.GPIO_Speed = GPIO_Speed_50MHz;
    gpio_init.GPIO_Pin = GPIO_Pin_10;
    GPIO_Init(GPIOB, &gpio_init);
    gpio_init.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOB, &gpio_init);

    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

static void OLED_I2C_Start(void) {
    OLED_W_SDA(1);
    OLED_W_SCL(1);
    OLED_W_SDA(0);
    OLED_W_SCL(0);
}

static void OLED_I2C_Stop(void) {
    OLED_W_SDA(0);
    OLED_W_SCL(1);
    OLED_W_SDA(1);
}

static void OLED_I2C_SendByte(uint8_t byte) {
    uint8_t i;
    for (i = 0; i < 8; i++) {
        OLED_W_SDA(!!(byte & (0x80 >> i)));
        OLED_W_SCL(1);
        OLED_W_SCL(0);
    }
    OLED_W_SCL(1);
    OLED_W_SCL(0);
}

static void OLED_WriteCommand(uint8_t command) {
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);
    OLED_I2C_SendByte(0x00);
    OLED_I2C_SendByte(command);
    OLED_I2C_Stop();
}

static void OLED_WriteData(uint8_t data) {
    OLED_I2C_Start();
    OLED_I2C_SendByte(0x78);
    OLED_I2C_SendByte(0x40);
    OLED_I2C_SendByte(data);
    OLED_I2C_Stop();
}

static void OLED_SetCursor(uint8_t y, uint8_t x) {
    OLED_WriteCommand(0xB0 | y);
    OLED_WriteCommand(0x10 | ((x & 0xF0) >> 4));
    OLED_WriteCommand(0x00 | (x & 0x0F));
}

uint32_t OLED_Pow(uint32_t x, uint32_t y) {
    uint32_t result = 1;
    while (y--) {
        result *= x;
    }
    return result;
}

void OLED_Clear(void) {
    uint8_t i, j;
    for (j = 0; j < 8; j++) {
        OLED_SetCursor(j, 0);
        for (i = 0; i < 128; i++) {
            OLED_WriteData(0x00);
        }
    }
}

void OLED_ShowChar(uint8_t line, uint8_t column, char ch) {
    uint8_t i;
    OLED_SetCursor((line - 1) * 2, (column - 1) * 8);
    for (i = 0; i < 8; i++) {
        OLED_WriteData(OLED_F8x16[ch - ' '][i]);
    }
    OLED_SetCursor((line - 1) * 2 + 1, (column - 1) * 8);
    for (i = 0; i < 8; i++) {
        OLED_WriteData(OLED_F8x16[ch - ' '][i + 8]);
    }
}

void OLED_ShowString(uint8_t line, uint8_t column, char *string) {
    uint8_t i;
    for (i = 0; string[i] != '\0'; i++) {
        OLED_ShowChar(line, column + i, string[i]);
    }
}

void OLED_ShowNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length) {
    uint8_t i;
    for (i = 0; i < length; i++) {
        OLED_ShowChar(line, column + i,
                      number / OLED_Pow(10, length - i - 1) % 10 + '0');
    }
}

void OLED_ShowSignedNum(uint8_t line, uint8_t column, int32_t number, uint8_t length) {
    uint8_t i;
    uint32_t number1;
    if (number >= 0) {
        OLED_ShowChar(line, column, '+');
        number1 = number;
    } else {
        OLED_ShowChar(line, column, '-');
        number1 = -number;
    }
    for (i = 0; i < length; i++) {
        OLED_ShowChar(line, column + i + 1,
                      number1 / OLED_Pow(10, length - i - 1) % 10 + '0');
    }
}

void OLED_ShowHexNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length) {
    uint8_t i, single_number;
    for (i = 0; i < length; i++) {
        single_number = number / OLED_Pow(16, length - i - 1) % 16;
        if (single_number < 10) {
            OLED_ShowChar(line, column + i, single_number + '0');
        } else {
            OLED_ShowChar(line, column + i, single_number - 10 + 'A');
        }
    }
}

void OLED_ShowBinNum(uint8_t line, uint8_t column, uint32_t number, uint8_t length) {
    uint8_t i;
    for (i = 0; i < length; i++) {
        OLED_ShowChar(line, column + i,
                      number / OLED_Pow(2, length - i - 1) % 2 + '0');
    }
}

void OLED_Init(void) {
    uint32_t i, j;

    for (i = 0; i < 1000; i++) {
        for (j = 0; j < 1000; j++) {
        }
    }

    OLED_I2C_Init();

    OLED_WriteCommand(0xAE);
    OLED_WriteCommand(0xD5);
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xA8);
    OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xD3);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x40);
    OLED_WriteCommand(0xA1);
    OLED_WriteCommand(0xC8);
    OLED_WriteCommand(0xDA);
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0x81);
    OLED_WriteCommand(0xCF);
    OLED_WriteCommand(0xD9);
    OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDB);
    OLED_WriteCommand(0x30);
    OLED_WriteCommand(0xA4);
    OLED_WriteCommand(0xA6);
    OLED_WriteCommand(0x8D);
    OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF);
    OLED_Clear();
}
