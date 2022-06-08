from base64 import decode
import socket
from ..utils.payload import MAX_PAYLOAD_SIZE, decode_payload

def init_tcp_sever(host, port, id_protocol):
    """
    Starts TCP for data communication between the raspberry pi
    and a esp32.
    """

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((host, port))
        s.listen() 
        conn, addr = s.accept()

        with conn:
            print(f"Connected by {addr} to an ESP32")
            while True:
                encoded_payload = conn.recv(MAX_PAYLOAD_SIZE)
                decoded_paylodad = decode_payload(id_protocol, encoded_payload)

                device_id = decoded_paylodad['device_id']
                if decoded_paylodad['status'] != CURRENT_STATUS[device_id]:
                    # Enviar al client nueva config y luego cerrar sv.
                    break
                
                # Hacer cosas con el decoded_payload.