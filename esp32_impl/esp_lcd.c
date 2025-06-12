#include <stdio.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_commands.h"
#include "../devices/lcd_interface.h"

#define SPI_HOST  SPI2_HOST
#define TRANSFER_BATCH_SIZE 4095 // 4096 bytes when enable dma

esp_lcd_panel_io_handle_t io_handle = NULL;
esp_lcd_panel_handle_t panel_handle = NULL;

int initSPIBus(int sclk, int mosi, int dc, int cs, int freq) {
    spi_bus_config_t buscfg = {
        .sclk_io_num = sclk,
        .mosi_io_num = mosi,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 0, //default is 4096
    };
    int ret = spi_bus_initialize(SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        return ret;
    }

    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = dc,
        .cs_gpio_num = cs,
        .pclk_hz = freq,
        .spi_mode = 3,
        .trans_queue_depth = 10,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .on_color_trans_done = NULL,
        .user_ctx = NULL,
    };
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SPI_HOST, &io_config, &io_handle);
    return ret;
}

int initLCD7789(int rst, int blk) {
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = rst,
        .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
        .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
        .bits_per_pixel = 16,
    };
    int ret = esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);
    if (ret != ESP_OK) {
        return ret;
    }
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_invert_color(panel_handle, true);
    esp_lcd_panel_set_gap(panel_handle, 0, 0);
    // turn on display
    esp_lcd_panel_disp_on_off(panel_handle, true);
    esp_lcd_panel_disp_sleep(panel_handle, false);
    return ESP_OK;
}

void lcdRotate(DIRECTION direction) {
    if (panel_handle) {
        // switch (direction)
        // {
        // case DIRECTION0:
        //     break;
        // case DIRECTION90:
        //     esp_lcd_panel_swap_xy(panel_handle, true);
        //     break;
        // case DIRECTION180:
        //     esp_lcd_panel_swap_xy(panel_handle, false);
        //     esp_lcd_panel_mirror(panel_handle, true, true);
        //     break;
        // case DIRECTION270:
        //     esp_lcd_panel_swap_xy(panel_handle, true);
        //     esp_lcd_panel_mirror(panel_handle, true, false);
        //     break;
        // }
        uint8_t madctl;
        switch (direction) {
            case DIRECTION0:   madctl = 0x00; break;   // 0°
            case DIRECTION90:  madctl = 0x60; break;   // 90°（XY交换+垂直镜像）
            case DIRECTION180: madctl = 0xA0; break;   // 180°（垂直镜像+水平镜像）
            case DIRECTION270: madctl = 0xC0; break;   // 270°（XY交换+双镜像）
        }
        esp_lcd_panel_io_tx_param(panel_handle, 0x36, &madctl, 1);
    }
}

void lcdPushPixels(int x1, int y1, int x2, int y2, uint16_t* colors) {
    int width = x2 - x1 + 1;
    int height = y2 - y1 + 1;
    int batchCount = TRANSFER_BATCH_SIZE / sizeof(uint16_t);
    int batchHeight = batchCount / width;
    uint16_t* addr = colors;
    int yStart = y1;
    while (height > 0) {
        int drawHeight = height > batchHeight? batchHeight:height;
        esp_lcd_panel_draw_bitmap(panel_handle, x1, yStart, x1 + width, yStart + drawHeight, addr);
        addr += drawHeight * width;
        height -= drawHeight;
        yStart += drawHeight;
    }
}