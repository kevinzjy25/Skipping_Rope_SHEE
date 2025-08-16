idf_component_register(SRCS "SiYuanHeiTiGoogleBan_16.c" "SiYuanHeiTiGoogleBan_68.c" "SiYuanHeiTiGoogleBan_14.c" "my_ui.c" "main.c"
                    PRIV_REQUIRES spi_flash
                    INCLUDE_DIRS ""
                    REQUIRES lvgl
                             lvgl_porting
                             esp_wifi
                             nvs_flash
                             wpa_supplicant)
