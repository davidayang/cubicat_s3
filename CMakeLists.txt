
file(GLOB_RECURSE SRCS "cubicat.cpp" "./*.cpp" "./*.c")
if (CONFIG_REMOVE_GRAPHIC_ENGINE)
    file(GLOB_RECURSE EXCLUDED_FILES "./graphic_engine/*.cpp" "./graphic_engine/*.c")
    list(REMOVE_ITEM SRCS ${EXCLUDED_FILES})
endif()
idf_component_register(SRCS ${SRCS}
                       REQUIRES esp_driver_gpio driver esp_wifi esp_timer
                       PRIV_REQUIRES esp_driver_spi esp_psram nvs_flash heap spiffs fatfs esp_driver_sdmmc soc
                       INCLUDE_DIRS ".")

target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions -Wno-error=reorder -DCONFIG_TFT_eSPI_ESPIDF -DST7789_DRIVER)