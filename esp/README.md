# Cleinte en ESP32

Este directorio tiene todo el código asociado a la ESP. En `main` se encuentra el punto de inicio de la aplicación donde simplemente
se inician controladores y comienza la función `switch_status` que se encarga de comenzar las funciones o _Tasks_ que correspondan
al status que está guardado en NVS.

Por otro lado, en `components` estan todos los componentes que hacen la aplicación.

Todas las funciones se encuentran documentadas con una breve explicación de lo que hacen y cuáles son sus parámetros.

## Componentes

- **connection**: Aquí se encuentran todos los archivos que se encargan de las comunicaciones con el servidor. Esto es, conectarse a wifi con `wifi.c`, hacer conexiones TCP con `client_tcp.c`, hacer conexiones UDP con `udp_client.c`, hacer conexiones BLE con `ble_server.c`, poder guardar valores en memoria no volátil
  y otras utilidades con `system_utils.c` y, finalmente, las funciones que usan las conexiones para poder enviar y recibir paquetes según el status dado en `status.c`

- **payload**: Aquí se encuentran todas las utilidades para poder codificar valores, decodificar un buffer de bytes y crear valores aleatorios asociados a sensores.
