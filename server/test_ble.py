import pygatt
import time
from db import db
from utils import config

BASE_UUID = "0000{bit_16_uuid}-0000-1000-8000-00805F9B34FB"

global received_data

def print_callback(handle, value):
  global received_data
  print(value)
  received_data = True

if __name__ == "__main__":
  received_data = False
  adapter = pygatt.GATTToolBackend()
  adapter.start()
  scanned = adapter.scan()
  esp = None
  for d in scanned:
    if d["name"] == "ESP32-01":
      esp = d
  # device = adapter.connect(esp["address"])
  # db.set_device_config(
  #   id_device = 1,
  #   id_protocol = 1,
  #   status = 31,
  #   bmi270_sampling = 1,
  #   bmi270_acc_sensibility = 1,
  #   bmi270_gyro_sensibility = 1,
  #   bme688_sampling = 1,
  #   discontinuous_time = 1,
  #   port_tcp = 1,
  #   port_udp = 1,
  #   host_ip_addr = "127.0.0.1",
  #   ssid = "AMOGUS",
  #   passwd = "AMOGUS",
  # )
  # c = db.get_device_config(1)
  # msg = config.encode_config(**c)
  # device.char_write(BASE_UUID.format(bit_16_uuid="ff01"), msg)
  # device.disconnect()
  # print("Disconnected")

  # time.sleep(5)

  i = 3
  while i > 0:
    device = adapter.connect(esp["address"])
    print("Connected")
    device.subscribe(BASE_UUID.format(bit_16_uuid="ff01"), callback=print_callback, wait_for_response=False)
    print("Waiting for data")
    while not received_data:
      time.sleep(1)
    print("Received data")
    received_data = False
    device.char_write(BASE_UUID.format(bit_16_uuid="ff01"), b'\x06')
    i -= 1
    print(i)
    time.sleep(60)
  
  device = adapter.connect(esp["address"])
  print("Connected")
  device.subscribe(BASE_UUID.format(bit_16_uuid="ff01"), callback=print_callback, wait_for_response=False)
  print("Waiting for data")
  while not received_data:
    time.sleep(1)
  received_data = False
  print("Received data")
  device.char_write(BASE_UUID.format(bit_16_uuid="ff01"), b'\x00')
  device.disconnect()


  
  # while i > 0:
  #   time.sleep(5)
  #   i -= 1
  #   print(i)
  
  # device.unsubscribe(BASE_UUID.format(bit_16_uuid="ff01"), wait_for_response=False)
  # print("Unsibscribed")
  # device.char_write(BASE_UUID.format(bit_16_uuid="ff01"), b'\x00')
  # print("Sent Stop")
  # device.disconnect()
  
  adapter.stop()
