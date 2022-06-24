# Servidor en Raspberry

Este servidor utiliza las librerías de Python `pygatt` para la conexión BLE y `PyQt5` para la interfaz gráfica.

## Funcionamiento de protocolos

En el servidor todos los protocolos funcionan en Threads separados para poder utilizar más de una ESP al mismo tiempo.

El funcionamiento básico de todas las comunicaciones es que la ESP envía un paquete y la Raspberry responde con una confirmación
`0x06` o con un nuevo ID de status al que cambiarse. Para cambiar de status siempre se pasa primero por un estado de enviar configuración (ya sea por BLE o TCP) y luego se va al nuevo estado.

Las únicas excepciónes a la comunicación ordenada que se explicó antes son BLE Continua y UDP. En estos protocolos existe un _stream_
de paquetes que se envían desde la ESP y cuando el servidor decide cambiar de status envía un paquete con el nuevo status al que cambiar.
En el caso de BLE esto es escribir en la característica. En el caso de UDP hay un servidor TCP en la ESP esperando conexiones, esto
para evitar la pérdida de paquetes de UDP y que así la comunicación quede en un estado inconsistente.

## Funcionamiento de la GUI

Las funciones que pueden quedarse atascadas por BLE (Mandar configuración y escanear dispositivos) se envuelven en una clase de PyQt llamada `Runner`
que sirve para partir un thread que corra la función. De esta manera no se bloquea la interfaz.

Por otro lado, los gráficos que se dibujan no son en tiempo real para minimizar la carga sobre la raspberry. Sin embargo, si se aprieta el botón de Iniciar
se tendrán todos los datos recopilados hasta el momento.

# Componentes

A continuación se explican las distintas carpetas de este directorio

- **utils**: Tiene funciones para poder codificar la configuración y decodificar los paquetes recibidos según el protocolo.
- **src**: Tiene todos los archivos con las funciones que se encargan de las distintas comunicaciones (TCP, UDP y BLE)
- **gui**: Tiene todos los componentes asociados a la interfaz gráfica
- **db**: Aquí se encuentra la base de datos y funciones que permiten accederla y relacionarse con ella
