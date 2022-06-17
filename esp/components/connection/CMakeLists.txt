idf_component_register(SRCS "src/wifi.c" "src/client_tcp.c" "src/client_udp.c" "src/ble_server.c" "src/status.c" "src/system_utils.c"
                    INCLUDE_DIRS "./include" "../payload/include"
                    REQUIRES nvs_flash bt)
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error=uninitialized -Wno-uninitialized -Wno-error=maybe-uninitialized -Wno-maybe-uninitialized)