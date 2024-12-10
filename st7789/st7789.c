#include <string.h>
#include <inttypes.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <driver/spi_master.h>
#include <driver/gpio.h>
#include "esp_log.h"

#include "st7789.h"
#include "soc/soc.h"
#include "soc/spi_reg.h"

#define TAG "ST7789"
#define _DEBUG_ 0

#define SPI_HOST SPI2_HOST  // FSPI
#define SPI_PORT SPI2_HOST+1

#define TFT_SPI_MODE 3

#define _spi_cmd       (volatile uint32_t*)(SPI_CMD_REG(SPI_PORT))
#define _spi_user      (volatile uint32_t*)(SPI_USER_REG(SPI_PORT))
#define _spi_mosi_dlen (volatile uint32_t*)(SPI_MS_DLEN_REG(SPI_PORT))
#define _spi_w         (volatile uint32_t*)(SPI_W0_REG(SPI_PORT))

#define SPI_BUSY_CHECK while (*_spi_cmd&SPI_USR)

#if (TFT_SPI_MODE == SPI_MODE1) || (TFT_SPI_MODE == SPI_MODE2)
    #define SET_BUS_WRITE_MODE *_spi_user = SPI_USR_MOSI | SPI_CK_OUT_EDGE
    #define SET_BUS_READ_MODE  *_spi_user = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN | SPI_CK_OUT_EDGE
#else
    #define SET_BUS_WRITE_MODE *_spi_user = SPI_USR_MOSI
    #define SET_BUS_READ_MODE  *_spi_user = SPI_USR_MOSI | SPI_USR_MISO | SPI_DOUTDIN
#endif

#define SPI_DEFAULT_FREQUENCY SPI_MASTER_FREQ_80M;

static const int SPI_Command_Mode = 0;
static const int SPI_Data_Mode = 1;
// static const int SPI_Frequency = SPI_MASTER_FREQ_20M;
// static const int SPI_Frequency = SPI_MASTER_FREQ_26M;
// static const int SPI_Frequency = SPI_MASTER_FREQ_40M;
// static const int SPI_Frequency = 60000000;
// static const int SPI_Frequency = SPI_MASTER_FREQ_80M;

#define TFT_CMD_MADCTL 0x36
#define TFT_CMD_CASET 0x2A // Column Address Set
#define TFT_CMD_RASET 0x2B // Row Address Set
#define TFT_CMD_RAMWR 0x2C // Memory Write
// Flags for TFT_MADCTL
#define TFT_MAD_MY 0x80
#define TFT_MAD_MX 0x40
#define TFT_MAD_MV 0x20
#define TFT_MAD_ML 0x10
#define TFT_MAD_RGB 0x00
#define TFT_MAD_BGR 0x08
#define TFT_MAD_MH 0x04
#define TFT_MAD_SS 0x02
#define TFT_MAD_GS 0x01

int clock_speed_hz = SPI_DEFAULT_FREQUENCY;
#define TRANSFER_BATCH_SIZE 4092 // 4096 bytes when enable dma

void spi_clock_speed(int speed)
{
    ESP_LOGI(TAG, "SPI clock speed=%d MHz", speed / 1000000);
    clock_speed_hz = speed;
}

void spi_master_init(TFT_t *dev, int16_t GPIO_MOSI, int16_t GPIO_SCLK, int16_t GPIO_CS, int16_t GPIO_DC, int16_t GPIO_RESET, int16_t GPIO_BL)
{
    esp_err_t ret;
    ESP_LOGI(TAG, "GPIO_CS=%d", GPIO_CS);
    if (GPIO_CS >= 0)
    {
        // gpio_pad_select_gpio( GPIO_CS );
        gpio_reset_pin(GPIO_CS);
        gpio_set_direction(GPIO_CS, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_CS, 0);
    }

    ESP_LOGI(TAG, "GPIO_DC=%d", GPIO_DC);
    gpio_reset_pin(GPIO_DC);
    gpio_set_direction(GPIO_DC, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_DC, 0);

    ESP_LOGI(TAG, "GPIO_RESET=%d", GPIO_RESET);
    if (GPIO_RESET >= 0)
    {
        // gpio_pad_select_gpio( GPIO_RESET );
        gpio_reset_pin(GPIO_RESET);
        gpio_set_direction(GPIO_RESET, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_RESET, 1);
        delayMS(50);
        gpio_set_level(GPIO_RESET, 0);
        delayMS(50);
        gpio_set_level(GPIO_RESET, 1);
        delayMS(50);
    }

    ESP_LOGI(TAG, "GPIO_BL=%d", GPIO_BL);
    if (GPIO_BL >= 0)
    {
        // gpio_pad_select_gpio(GPIO_BL);
        gpio_reset_pin(GPIO_BL);
        gpio_set_direction(GPIO_BL, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_BL, 1);
    }

    ESP_LOGI(TAG, "GPIO_MOSI=%d", GPIO_MOSI);
    ESP_LOGI(TAG, "GPIO_SCLK=%d", GPIO_SCLK);
    spi_bus_config_t buscfg = {
        .mosi_io_num = GPIO_MOSI,
        .miso_io_num = -1,
        .sclk_io_num = GPIO_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0, //default to 4092
        .flags = 0};

    ret = spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO); // enable DMA
    ESP_LOGD(TAG, "spi_bus_initialize=%d", ret);
    assert(ret == ESP_OK);

    spi_device_interface_config_t devcfg;
    memset(&devcfg, 0, sizeof(devcfg));
    // devcfg.clock_speed_hz = SPI_Frequency;
    devcfg.clock_speed_hz = clock_speed_hz;
    devcfg.queue_size = 7;
    // devcfg.mode = 2;
    devcfg.mode = 3;
    devcfg.flags = SPI_DEVICE_NO_DUMMY;

    if (GPIO_CS >= 0)
    {
        devcfg.spics_io_num = GPIO_CS;
    }
    else
    {
        devcfg.spics_io_num = -1;
    }

    spi_device_handle_t handle;
    ret = spi_bus_add_device(SPI_HOST, &devcfg, &handle);
    ESP_LOGD(TAG, "spi_bus_add_device=%d", ret);
    assert(ret == ESP_OK);
    dev->_dc = GPIO_DC;
    dev->_bl = GPIO_BL;
    dev->_SPIHandle = handle;
}

bool spi_master_write_byte(spi_device_handle_t SPIHandle, const uint8_t *Data, size_t DataLength)
{
    spi_transaction_t SPITransaction;
    esp_err_t ret;

    if (DataLength > 0)
    {
        memset(&SPITransaction, 0, sizeof(spi_transaction_t));
        SPITransaction.length = DataLength * 8;
        SPITransaction.tx_buffer = Data;
#if 1
        ret = spi_device_transmit(SPIHandle, &SPITransaction);
#else
        ret = spi_device_polling_transmit(SPIHandle, &SPITransaction);
#endif
        assert(ret == ESP_OK);
    }

    return true;
}

bool spi_master_write_command(TFT_t *dev, uint8_t cmd)
{
    static uint8_t Byte = 0;
    Byte = cmd;
    gpio_set_level(dev->_dc, SPI_Command_Mode);
    return spi_master_write_byte(dev->_SPIHandle, &Byte, 1);
}

bool spi_master_write_data_byte(TFT_t *dev, uint8_t data)
{
    static uint8_t Byte = 0;
    Byte = data;
    gpio_set_level(dev->_dc, SPI_Data_Mode);
    return spi_master_write_byte(dev->_SPIHandle, &Byte, 1);
}

bool spi_master_write_data_word(TFT_t *dev, uint16_t data)
{
    static uint8_t Byte[2];
    Byte[0] = (data >> 8) & 0xFF;
    Byte[1] = data & 0xFF;
    gpio_set_level(dev->_dc, SPI_Data_Mode);
    return spi_master_write_byte(dev->_SPIHandle, Byte, 2);
}

bool spi_master_write_addr(TFT_t *dev, uint16_t addr1, uint16_t addr2)
{
    static uint8_t Byte[4];
    Byte[0] = (addr1 >> 8) & 0xFF;
    Byte[1] = addr1 & 0xFF;
    Byte[2] = (addr2 >> 8) & 0xFF;
    Byte[3] = addr2 & 0xFF;
    gpio_set_level(dev->_dc, SPI_Data_Mode);
    return spi_master_write_byte(dev->_SPIHandle, Byte, 4);
}

bool spi_master_write_color(TFT_t *dev, uint16_t color, uint16_t size)
{
    static uint8_t Byte[1024];
    int index = 0;
    for (int i = 0; i < size; i++)
    {
        Byte[index++] = (color >> 8) & 0xFF;
        Byte[index++] = color & 0xFF;
    }
    gpio_set_level(dev->_dc, SPI_Data_Mode);
    return spi_master_write_byte(dev->_SPIHandle, Byte, size * 2);
}

bool spi_master_write_colors(TFT_t *dev, uint16_t *colors, uint16_t size)
{
    static uint8_t Byte[TRANSFER_BATCH_SIZE];
    int index = 0;
    for (int i = 0; i < size; i++)
    {
        Byte[index++] = (colors[i] >> 8) & 0xFF;
        Byte[index++] = colors[i] & 0xFF;
    }
    gpio_set_level(dev->_dc, SPI_Data_Mode);
    return spi_master_write_byte(dev->_SPIHandle, Byte, size * 2);
}

void delayMS(int ms)
{
    int _ms = ms + (portTICK_PERIOD_MS - 1);
    TickType_t xTicksToDelay = _ms / portTICK_PERIOD_MS;
    ESP_LOGD(TAG, "ms=%d _ms=%d portTICK_PERIOD_MS=%" PRIu32 " xTicksToDelay=%" PRIu32, ms, _ms, portTICK_PERIOD_MS, xTicksToDelay);
    vTaskDelay(xTicksToDelay);
}

void lcdInit(TFT_t *dev, int width, int height, int offsetx, int offsety)
{
    dev->_width = width;
    dev->_height = height;
    dev->_offsetx = offsetx;
    dev->_offsety = offsety;
    dev->_font_direction = DIRECTION0;
    dev->_font_fill = false;
    dev->_font_underline = false;

    spi_master_write_command(dev, 0x01); // Software Reset
    delayMS(150);

    spi_master_write_command(dev, 0x11); // Sleep Out
    delayMS(255);

    spi_master_write_command(dev, 0x3A); // Interface Pixel Format
    spi_master_write_data_byte(dev, 0x55);
    delayMS(10);

    spi_master_write_command(dev, 0x36); // Memory Data Access Control
    spi_master_write_data_byte(dev, 0x00);

    spi_master_write_command(dev, TFT_CMD_CASET); // Column Address Set
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0xF0);

    spi_master_write_command(dev, TFT_CMD_RASET); // Row Address Set
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0x00);
    spi_master_write_data_byte(dev, 0xF0);

    spi_master_write_command(dev, 0x21); // Display Inversion On
    delayMS(10);

    spi_master_write_command(dev, 0x13); // Normal Display Mode On
    delayMS(10);

    spi_master_write_command(dev, 0x29); // Display ON
    delayMS(255);

    if (dev->_bl >= 0)
    {
        gpio_set_level(dev->_bl, 1);
    }
}

void lcdRotate(TFT_t *dev, DIRECTION dir)
{
    dir = dir % 4;
    uint8_t val = 0x0;
    spi_master_write_command(dev, TFT_CMD_MADCTL);
    if (dir == 1)
    {
        val = TFT_MAD_MX | TFT_MAD_MV | TFT_MAD_RGB; // 顺时针旋转 90°
    }
    else if (dir == 2)
    {
        val = TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_RGB; // 顺时针旋转 180
    }
    else if (dir == 3)
    {
        val = TFT_MAD_MY | TFT_MAD_MV | TFT_MAD_RGB; // 逆时针旋转 90°
    }
    spi_master_write_data_byte(dev, val);
}

// Draw pixel
// x:X coordinate
// y:Y coordinate
// color:color
void lcdDrawPixel(TFT_t *dev, uint16_t x, uint16_t y, uint16_t color)
{
    if (x >= dev->_width)
        return;
    if (y >= dev->_height)
        return;

    uint16_t _x = x + dev->_offsetx;
    uint16_t _y = y + dev->_offsety;

    spi_master_write_command(dev, TFT_CMD_CASET); // set column(x) address
    spi_master_write_addr(dev, _x, _x);
    spi_master_write_command(dev, TFT_CMD_RASET); // set Page(y) address
    spi_master_write_addr(dev, _y, _y);
    spi_master_write_command(dev, TFT_CMD_RAMWR); // Memory Write
    // spi_master_write_data_word(dev, color);
    spi_master_write_colors(dev, &color, 1);
}

// Draw multi pixel
// x:X coordinate
// y:Y coordinate
// size:Number of colors
// colors:colors
void lcdDrawMultiPixels(TFT_t *dev, uint16_t x, uint16_t y, uint16_t size, uint16_t *colors)
{
    if (x + size > dev->_width)
        return;
    if (y >= dev->_height)
        return;
    uint16_t _x1 = x + dev->_offsetx;
    uint16_t _x2 = _x1 + (size - 1);
    uint16_t _y1 = y + dev->_offsety;
    uint16_t _y2 = _y1;

    spi_master_write_command(dev, TFT_CMD_CASET); // set column(x) address
    spi_master_write_addr(dev, _x1, _x2);
    spi_master_write_command(dev, TFT_CMD_RASET); // set Page(y) address
    spi_master_write_addr(dev, _y1, _y2);
    spi_master_write_command(dev, TFT_CMD_RAMWR); // Memory Write
    spi_master_write_colors(dev, colors, size);
}


// Display OFF
void lcdDisplayOff(TFT_t *dev)
{
    spi_master_write_command(dev, 0x28); // Display off
}

// Display ON
void lcdDisplayOn(TFT_t *dev)
{
    spi_master_write_command(dev, 0x29); // Display on
}


// Backlight OFF
void lcdBacklightOff(TFT_t *dev)
{
    if (dev->_bl >= 0)
    {
        gpio_set_level(dev->_bl, 0);
    }
}

// Backlight ON
void lcdBacklightOn(TFT_t *dev)
{
    if (dev->_bl >= 0)
    {
        gpio_set_level(dev->_bl, 1);
    }
}

// Display Inversion Off
void lcdInversionOff(TFT_t *dev)
{
    spi_master_write_command(dev, 0x20); // Display Inversion Off
}

// Display Inversion On
void lcdInversionOn(TFT_t *dev)
{
    spi_master_write_command(dev, 0x21); // Display Inversion On
}

void lcdPushPixels(TFT_t * dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t * colors)
{    
    int width = x2 - x1 + 1;
    int height = y2 - y1 + 1;
    int batchCount = TRANSFER_BATCH_SIZE / sizeof(uint16_t);
    int batchHeight = batchCount / width;
    batchCount = batchHeight * width;
    int y = y1;
    int remain = width * height;
    uint32_t total = remain;
    uint16_t* addr = colors;
    for (int i=0; i<total;) {
        int writeCount = remain > batchCount ? batchCount : remain;
        spi_master_write_command(dev, TFT_CMD_CASET); // set column(x) address
        spi_master_write_addr(dev, x1, x2);
        spi_master_write_command(dev, TFT_CMD_RASET); // set Page(y) address
        spi_master_write_addr(dev, y, y + batchHeight);
        spi_master_write_command(dev, TFT_CMD_RAMWR); // Memory Write
        spi_master_write_colors(dev, addr, writeCount);
        addr += writeCount;
        y += batchHeight;
        i += batchCount;
        remain -= batchCount;
    }
}

// void lcdFastPushPixels(TFT_t * dev, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint16_t * colors) {
//     spi_master_write_command(dev, TFT_CMD_CASET); // set column(x) address
//     spi_master_write_addr(dev, x1, x2);
//     spi_master_write_command(dev, TFT_CMD_RASET); // set Page(y) address
//     spi_master_write_addr(dev, y1, y2);
//     spi_master_write_command(dev, TFT_CMD_RAMWR); // Memory Write
//     gpio_set_level(dev->_dc, SPI_Data_Mode);
//     SET_BUS_WRITE_MODE;
//     uint32_t *data = (uint32_t*)colors;
//     int width = x2 - x1 + 1;
//     int height = y2 - y1 + 1;
//     uint32_t len = width * height;
//     if (len > 31)
//     {
//         WRITE_PERI_REG(SPI_MS_DLEN_REG(SPI_PORT), 511);
//         while(len>31)
//         {
//             while (READ_PERI_REG(SPI_CMD_REG(SPI_PORT))&SPI_USR);
//             WRITE_PERI_REG(SPI_W0_REG(SPI_PORT),  *data++);
//             WRITE_PERI_REG(SPI_W1_REG(SPI_PORT),  *data++);
//             WRITE_PERI_REG(SPI_W2_REG(SPI_PORT),  *data++);
//             WRITE_PERI_REG(SPI_W3_REG(SPI_PORT),  *data++);
//             WRITE_PERI_REG(SPI_W4_REG(SPI_PORT),  *data++);
//             WRITE_PERI_REG(SPI_W5_REG(SPI_PORT),  *data++);
//             WRITE_PERI_REG(SPI_W6_REG(SPI_PORT),  *data++);
//             WRITE_PERI_REG(SPI_W7_REG(SPI_PORT),  *data++);
//             WRITE_PERI_REG(SPI_W8_REG(SPI_PORT),  *data++);
//             WRITE_PERI_REG(SPI_W9_REG(SPI_PORT),  *data++);
//             WRITE_PERI_REG(SPI_W10_REG(SPI_PORT), *data++);
//             WRITE_PERI_REG(SPI_W11_REG(SPI_PORT), *data++);
//             WRITE_PERI_REG(SPI_W12_REG(SPI_PORT), *data++);
//             WRITE_PERI_REG(SPI_W13_REG(SPI_PORT), *data++);
//             WRITE_PERI_REG(SPI_W14_REG(SPI_PORT), *data++);
//             WRITE_PERI_REG(SPI_W15_REG(SPI_PORT), *data++);
//         #if CONFIG_IDF_TARGET_ESP32S3
//             SET_PERI_REG_MASK(SPI_CMD_REG(SPI_PORT), SPI_UPDATE);
//             while (READ_PERI_REG(SPI_CMD_REG(SPI_PORT))&SPI_UPDATE);
//         #endif
//             SET_PERI_REG_MASK(SPI_CMD_REG(SPI_PORT), SPI_USR);
//             len -= 32;
//         }
//     }
//     if (len)
//     {
//         while (READ_PERI_REG(SPI_CMD_REG(SPI_PORT))&SPI_USR);
//         WRITE_PERI_REG(SPI_MS_DLEN_REG(SPI_PORT), (len << 4) - 1);
//         for (uint32_t i=0; i <= (len<<1); i+=4) WRITE_PERI_REG((SPI_W0_REG(SPI_PORT) + i), *data++);
//     #if CONFIG_IDF_TARGET_ESP32S3
//         SET_PERI_REG_MASK(SPI_CMD_REG(SPI_PORT), SPI_UPDATE);
//         while (READ_PERI_REG(SPI_CMD_REG(SPI_PORT))&SPI_UPDATE);
//     #endif
//         SET_PERI_REG_MASK(SPI_CMD_REG(SPI_PORT), SPI_USR);
//     }
//     while (READ_PERI_REG(SPI_CMD_REG(SPI_PORT))&SPI_USR);
//     SPI_BUSY_CHECK;
//     SET_BUS_READ_MODE;
// }