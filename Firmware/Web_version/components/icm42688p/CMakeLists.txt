idf_component_register(SRCS "icm42688p.c"
                INCLUDE_DIRS "include"
                REQUIRES driver
                freertos
                esp_timer
)
