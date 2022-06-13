idf_component_register(SRCS "src/wifi.c" "src/client_tcp.c" "src/client_udp.c" "src/ble_server.c" "src/status.c"
                    INCLUDE_DIRS "./include" "../payload/include" "../utils/include"
                    REQUIRES bt)