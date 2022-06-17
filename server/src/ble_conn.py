import time
from typing import Dict, List
from db import db
from utils import config, payload
import pygatt

CHAR_UUID = "0000ff01-0000-1000-8000-00805F9B34FB"

# The BGAPI backend will attempt to auto-discover the serial device name of the
# attached BGAPI-compatible USB adapter

adapter = pygatt.GATTToolBackend()
connections = {}
recv_data = {}

def continous_callback(handle, value):
    payload_dict = payload.decode_payload(value)
    db.insert_esp32_data(**payload_dict)

def discontinous_callback(handle, value):
    global recv_data
    payload_dict = payload.decode_payload(value)
    db.insert_esp32_data(**payload_dict)
    recv_data[payload_dict["mac"]] = True


def start_BLE():
    adapter.start()


def scan_esp() -> List[Dict]:
    try:
        nearby_devices = adapter.scan()
        print(nearby_devices)
        nearby_esps = filter(lambda x: ("ESP32-01" == x["name"]), nearby_devices)
        print(list(nearby_esps))
        return list(nearby_esps)
    except Exception as err:
        print(err)

def send_config_BLE(address: str, device_id: int) -> None:
    device = adapter.connect(address)
    c = db.get_device_config(device_id)
    msg = config.encode_config(**c)
    device.char_write(CHAR_UUID, msg)
    device.disconnect()

def recv_continous_BLE(address: str, id_device: int) -> None:
    global connections
    device = adapter.connect(address)
    connections[address] = True 
    device.subscribe(CHAR_UUID, callback=continous_callback, wait_for_response=False)

    while connections[address]:
        time.sleep(2)
    
    status = db.get_device_status(id_device)
    device.char_write(CHAR_UUID, config.encode_status(status))
    device.disconnect()

def recv_discontinous_BLE(address: str, id_device: int) -> None:
    global connections, recv_data
    connections[address] = True
    while connections[address]:
        recv_data[address] = False
        device = adapter.connect(address)
        device.subscribe(CHAR_UUID, callback=discontinous_callback, wait_for_response=False)
        while not recv_data[address]:
            time.sleep(2)
        status = db.get_device_status(id_device)
        if status == 31:
            status = 6
        device.char_write(CHAR_UUID, config.encode_status(status))
        if status == 6:
            sleep_time = db.get_device_disc_time(id_device)
            time.sleep(sleep_time*60)

    device.disconnect()

