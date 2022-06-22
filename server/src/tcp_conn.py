import socket
from utils.payload import MAX_PAYLOAD_SIZE, decode_payload
from utils.config import encode_config
from db import db

def send_config_tcp(host, port, id_device):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((host, port))
        s.listen() 
        conn, addr = s.accept()

        with conn:
            print(f"Connected by {addr} to an ESP32")
            conf = db.get_device_config(id_device)
            msg = encode_config(**conf)
            conn.send(msg)

def init_tcp_continous_server(host, port, device_id, id_protocol):
    """
    Starts TCP for data communication between the raspberry pi
    and a esp32.
    """

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((host, port))
        s.listen() 
        conn, addr = s.accept()

        with conn:
            print(f"Connected by {addr} to an ESP32")
            while True:
                if id_protocol == 5:
                    recv_len = MAX_PAYLOAD_SIZE
                    recv_msg = b''
                    while recv_len > 0:
                        encoded_msg = conn.recv(MAX_PAYLOAD_SIZE)
                        recv_msg += encoded_msg
                        recv_len -= len(encoded_msg)
                    encoded_payload = recv_msg
                else:
                    encoded_payload = conn.recv(MAX_PAYLOAD_SIZE)

                decoded_payload = decode_payload(encoded_payload)

                current_status = db.get_device_status(device_id)
                if decoded_payload['status'] != current_status:
                    # Enviar al client nuevo status y luego cerrar sv.
                    conn.send(int.to_bytes(current_status, byteorder="little", length=1))
                    break
                else:
                    # Enviar al cliente un ACK.
                    conn.send(b'\x06')
                
                    # Guardamos los datos enviados.
                    db.insert_esp32_data(**decoded_payload)

def init_tcp_discontinous_server(host, port, id_device, id_protocol):
    """
    Starts TCP for data communication between the raspberry pi
    and a esp32.
    """

    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((host, port))
        s.listen()
        while True:
            conn, addr = s.accept()

            with conn:
                print(f"Connected by {addr} to an ESP32")
                if id_protocol == 5:
                    recv_len = MAX_PAYLOAD_SIZE
                    recv_msg = b''
                    while recv_len > 0:
                        encoded_msg = conn.recv(MAX_PAYLOAD_SIZE)
                        recv_msg += encoded_msg
                        recv_len -= len(encoded_msg)
                    encoded_payload = recv_msg
                else:
                    encoded_payload = conn.recv(MAX_PAYLOAD_SIZE)

                decoded_payload = decode_payload(encoded_payload)

                current_status = db.get_device_status(id_device)
                if decoded_payload['status'] != current_status:
                    # Enviar al client nuevo status y luego cerrar sv.
                    conn.send(int.to_bytes(current_status, byteorder="little", length=1))
                    break
                else:
                    # Enviar al cliente un ACK.
                    conn.send(b'\x06')
                
                    # Guardamos los datos enviados.
                    db.insert_esp32_data(**decoded_payload)
