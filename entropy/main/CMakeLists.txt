set(SOURCES 
"app.c"
"display_driver.c"
"MLX90614_SMBus_Driver.c"
"MLX90614_API.c"
"temp_driver.c"
)
idf_component_register(
        SRCS ${SOURCES}
        INCLUDE_DIRS ".")
        
include_directories(../inc)
