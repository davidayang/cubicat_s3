
file(GLOB_RECURSE SRCS "cubicat.cpp" "./*.cpp" "./*.c")
idf_component_register(SRCS ${SRCS}
                       REQUIRES esp_driver_gpio driver esp_wifi
                       PRIV_REQUIRES esp_driver_spi esp_psram nvs_flash heap spiffs fatfs esp_driver_sdmmc esp_timer soc
                       INCLUDE_DIRS "." "./graphic_engine")
target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions -Wno-error=reorder -DCONFIG_TFT_eSPI_ESPIDF -DST7789_DRIVER)