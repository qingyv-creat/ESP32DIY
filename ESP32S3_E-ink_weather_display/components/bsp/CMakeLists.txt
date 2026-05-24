idf_component_register(SRCS "lcd/st7789_i80.c" "lcd/ft6336u_driver.c" "xl9555/xl9555.c"
                    INCLUDE_DIRS "lcd" "xl9555"
                    REQUIRES    esp_lcd)
