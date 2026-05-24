file(GLOB_RECURSE MYGUI_SRCS
    "*.c"
)

idf_component_register(
    SRCS ${MYGUI_SRCS}
    INCLUDE_DIRS "." "guider_customer_fonts" "images"
    REQUIRES lvgl__lvgl  #这里要写自己的lvgl组件名称不然会出现找不到lvgl头文件的报错
)
