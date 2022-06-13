from typing import Dict, List
import pygatt

# The BGAPI backend will attempt to auto-discover the serial device name of the
# attached BGAPI-compatible USB adapter

adapter = pygatt.GATTToolBackend()


def scan_esp() -> List[Dict]:
    try:
        nearby_devices = adapter.scan()
        print(nearby_devices)
        nearby_esps = filter(lambda x: ("ESP32" in x["name"]), nearby_devices)
        print(list(nearby_esps))
        return list(nearby_esps)
    except Exception as err:
        print(err)

if __name__ == '__main__':
    try:
        adapter.start()
        ble_nearby_devices = adapter.scan()
        print(ble_nearby_devices)
        esp = None
        for device in ble_nearby_devices:
            if device["name"] == "ESP32-01":
                esp = device
        device = adapter.connect(esp["address"])
        value = device.char_read("0000FEF4-0000-1000-8000-00805F9B34FB")
        print(f"Received data {value.decode('utf-8')}")
        # device.subscribe("0000FEF4-0000-1000-8000-00805F9B34FB", callback=handle_data)
        # device.receive_notification(handle_data, )
    finally:
        adapter.stop()