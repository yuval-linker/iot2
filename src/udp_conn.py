import socket
from ..utils.payload import MAX_PAYLOAD_SIZE, decode_payload

def init_udp_sever(host, port, id_protocol):
    """
    Starts UDP for data communication between the raspberry pi
    and a esp32.
    """

    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
        s.bind((host, port))

        while True:
            encoded_payload, addr = s.recvfrom(MAX_PAYLOAD_SIZE)
            print(f"Recieved data of ESP32 by {addr}")
            decoded_paylodad = decode_payload(id_protocol, encoded_payload)

            # Hacer cosas con el decoded_payload.