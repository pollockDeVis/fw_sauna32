set(SOURCES 
"app.c"
"display_driver.c"
"MLX90614_SMBus_Driver.c"
"MLX90614_API.c"
"temp_driver.c"
"ble_manager.c"
"sensor_filter.c"
"custom_list_iterator.c"
"custom_list_node.c"
"custom_list.c"
"MessageQueue.c"
"buzzer.c"
"hmi_controller.c"
"sensor_manager.c"
"IEEE11073float.c"
)
idf_component_register(
        SRCS ${SOURCES}
        INCLUDE_DIRS ".")
        
include_directories(../inc)
