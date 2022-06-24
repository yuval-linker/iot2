import time
from typing import Dict, List
from db import db
from utils import config, payload
from functools import partial
import pygatt

CHAR_UUID = "0000ff01-0000-1000-8000-00805F9B34FB"
BAD_NOTIFICATION = bytearray([x for x in range(15)])

# The BGAPI backend will attempt to auto-discover the serial device name of the
# attached BGAPI-compatible USB adapter

adapter = pygatt.GATTToolBackend()
connections = {}
esp_data = {}
recv_data = {}

def continous_callback(handle, value, address, id_device, msg_len):
    global esp_data
    if value == BAD_NOTIFICATION:
        return
    if len(esp_data[address]) < msg_len:
        esp_data[address] += value
        return
    payload_dict = payload.decode_payload(esp_data[address], id_device)
    esp_data[address] = b''
    db.insert_esp32_data(**payload_dict)

def discontinous_callback(handle, value, address, id_device, msg_len):
    global recv_data, esp_data
    if value == BAD_NOTIFICATION:
        return
    if len(esp_data[address]) < msg_len:
        esp_data[address] += value
        return  
    payload_dict = payload.decode_payload(esp_data[address], id_device)
    esp_data[address] = b''
    db.insert_esp32_data(**payload_dict)
    recv_data[address] = True


def start_BLE():
    adapter.start()


def scan_esp() -> List[Dict]:
    try:
        nearby_devices = adapter.scan()
        nearby_esps = filter(lambda x: ("ESP32-" in x["name"] if x["name"] else False), nearby_devices)
        list_esps = list(nearby_esps)
        list_esps.sort(key=lambda x: x["name"])
        print(list_esps)
        return list_esps
    except Exception as err:
        print(err)

def connect_device(address):
    device = None
    while not device:
        try:
            device = adapter.connect(address)
        except (pygatt.exceptions.NotificationTimeout, pygatt.exceptions.NotConnectedError):
            print("Trying to connect again")
            device = None
    return device

def send_config_BLE(address: str, device_id: int) -> None:
    completed = False
    while not completed:
        device = connect_device(address)
        try:
            c = db.get_device_config(device_id)
            msg = config.encode_config(**c)
            device.char_write(CHAR_UUID, msg)
            completed = True
        except Exception as err:
            print(err)
        finally:
            try:
                device.disconnect()
            except Exception:
                pass
        

def recv_continous_BLE(address: str, id_device: int, id_protocol: int) -> None:
    global connections, esp_data
    while True:
        try:
            connected = False
            connections[address] = True
            esp_data[address] = b''
            con_callback = partial(
                continous_callback,
                address=address,
                id_device=id_device,
                msg_len=payload.PROTOCOL_LENGTH[id_protocol],
            )
            while not connected:
                try:
                    device = connect_device(address)
                    device.subscribe(CHAR_UUID, callback=con_callback, wait_for_response=False)
                    connected = True
                except pygatt.exceptions.BLEError:
                    connected = False

            while connections[address]:
                time.sleep(2)
            
            status = db.get_device_status(id_device)
            device.char_write(CHAR_UUID, config.encode_status(status))
            device.disconnect()
            break
        except Exception:
            pass

def recv_discontinous_BLE(address: str, id_device: int, id_protocol: int) -> None:
    global connections, recv_data, esp_data
    connections[address] = True
    esp_data[address] = b''
    status = db.get_device_status(id_device)
    sleep_time = db.get_device_disc_time(id_device)
    disc_callback = partial(
        discontinous_callback,
        address=address,
        id_device=id_device,
        msg_len=payload.PROTOCOL_LENGTH[id_protocol],
    )
    while True:
        recv_data[address] = False
        connected = False
        try:
            while not connected:
                try:
                    device = connect_device(address)
                    device.subscribe(CHAR_UUID, callback=disc_callback, wait_for_response=False)
                    connected = True
                except pygatt.exceptions.BLEError:
                    connected = False

            while not recv_data[address]:
                time.sleep(2)

            status = db.get_device_status(id_device)
            if status == 31:
                device.char_write(CHAR_UUID, config.encode_status(6))
                time.sleep(sleep_time*60)
            else:
                device.char_write(CHAR_UUID, config.encode_status(status))
                break
        except Exception:
            pass

    device.disconnect()

def stop_BLE(address):
    global connections
    if address in connections.keys():
        connections[address] = False
