
file(GLOB_RECURSE SRCS  "cubicat.cpp"
                        "utils/*.cpp"
                        "core/*.c" "core/*.cpp"
                        "graphic_engine/*.c" "graphic_engine/*.cpp" 
                        "devices/*.c" "devices/*.cpp"
                        "esp32_impl/*.c")
if (CONFIG_REMOVE_GRAPHIC_ENGINE)
    file(GLOB_RECURSE EXCLUDED_FILES "./graphic_engine/*.cpp" "./graphic_engine/*.c")
    list(REMOVE_ITEM SRCS ${EXCLUDED_FILES})
endif()

if (CONFIG_JAVASCRIPT_ENABLE)
    file(GLOB_RECURSE JS_BINDING_SRCS "js_binding/*.cpp")
    list(APPEND SRCS ${JS_BINDING_SRCS})
    list(APPEND SRCS "3rd_party/mjs/mjs.c" "3rd_party/mjs/hashmap.c")
endif()
idf_component_register(SRCS ${SRCS}
                       REQUIRES esp_driver_gpio driver esp_wifi esp_timer
                       PRIV_REQUIRES esp_driver_spi esp_psram nvs_flash heap spiffs fatfs esp_driver_sdmmc soc
                       INCLUDE_DIRS "." "graphic_engine/3rd_party" "graphic_engine/3rd_party/zlib" "3rd_party/mjs" 
                       "3rd_party/mjs/src" "3rd_party/mjs/src/frozen")

target_compile_options(${COMPONENT_LIB} PRIVATE -fexceptions -Wno-error=reorder -Wno-maybe-uninitialized -Wno-error=format)
# target_compile_definitions(${COMPONENT_LIB} PRIVATE ST7789_DRIVER)
# libpng
target_compile_definitions(${COMPONENT_LIB} PRIVATE PPM_SUPPORTED)
target_compile_definitions(${COMPONENT_LIB} PRIVATE HAVE_MATH_H)
target_compile_definitions(${COMPONENT_LIB} PRIVATE HAVE_CTYPE_H)
target_compile_definitions(${COMPONENT_LIB} PRIVATE HAVE_UNISTD_H)
target_compile_definitions(${COMPONENT_LIB} PRIVATE HAVE_ERRNO_H)
# mjs
target_compile_definitions(${COMPONENT_LIB} PRIVATE CS_PLATFORM=CS_P_ESP32)
# audio
target_compile_definitions(${COMPONENT_LIB} PRIVATE PROGMEM=)