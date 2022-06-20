from audioop import add
from re import A
import socket
import threading

from utils.payload import MAX_PAYLOAD_SIZE, decode_payload
from db import db

class ServerUdp():
    def __init__(self, host, sv_port, id_device, esp_port=25564) -> None:
        # constants of the problem
        self.host = host
        self.sv_port = sv_port
        self.esp_port = esp_port
        self.id_device = id_device

        # global variables, mutual exclusion
        self.wait_notifier = True
        self.esp_addr = None
        self.device_id = None
        self.condition = threading.Condition()
    
    def run(self):
        """
        Starts UDP for data communication between the raspberry pi
        and a esp32.
        """

        # defining the threads
        threads = [
            threading.Thread(target=self.udp_listener, args=()),
            threading.Thread(target=self.tcp_notifier, args=())
        ]

        # starting the threads
        for t in threads:
            t.start()
        
        # joining the threads
        for t in threads:
            t.join()


    def udp_listener(self):
        """
        In charge of recieving all the data sent by the esp32
        and storing them in the db if it corresponds.
        """

        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.bind((self.host, self.sv_port))

            while True:
                encoded_payload, addr = s.recvfrom(MAX_PAYLOAD_SIZE)
                print(f"Recieved data of ESP32 by {addr}")
                decoded_payload = decode_payload(encoded_payload)
                
                current_status = db.get_device_status(self.id_device)
                if decoded_payload['status'] == current_status:
                    s.sendto(b'\x06', addr)                 # the sv sends an ACK.
                    db.insert_esp32_data(**decoded_payload)
                else:
                    # the current_status of the esp32 is outdated.     
                    with self.condition:
                        self.wait_notifier = False
                        self.esp_addr = addr
                        self.condition.notifyAll()

    def tcp_notifier(self):
        """
        In charge of notifying the esp32 that his current_status
        is outdated. The new status is sent by a TCP connection 
        to avoid packet loss.
        """
        while self.wait_notifier:
            with self.condition:
                self.condition.wait()

        assert (self.esp_addr or self.id_device) is not None

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect((self.esp_addr[0], self.esp_port))
            current_status = db.get_device_status(self.id_device)
            s.sendall(int.to_bytes(current_status, byteorder="little", length=1))
            print("Sent new status")
        


        




    

