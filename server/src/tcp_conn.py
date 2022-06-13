import socket

from base64 import decode
from ..utils.payload import MAX_PAYLOAD_SIZE, decode_payload
from ..db import db

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
                decoded_payload = decode_payload(id_protocol, encoded_payload)

                device_id = decoded_payload['device_id']

                current_status = db.get_device_status(device_id)
                if decoded_payload['status'] != current_status:
                    # Enviar al client nuevo status y luego cerrar sv.
                    conn.send(int.to_bytes(current_status, byteorder="big", length=1))
                    break

                else:
                    # Enviar al cliente un ACK.
                    conn.send(b'\x06')
                
                    # Guardamos los datos enviados.
                    db.insert_esp32_data(**decoded_payload)