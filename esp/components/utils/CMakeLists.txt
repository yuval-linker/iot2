idf_component_register(SRCS "src/system_utils.c"
                    INCLUDE_DIRS "./include" "../payload/include"
                    REQUIRES nvs_flash)