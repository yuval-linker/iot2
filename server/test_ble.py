from src import ble_conn as ble
from db import db
from threading import Thread

if __name__ == "__main__":
  ble.start_BLE()

  # db.set_device_config(
  #   id_device = 1,
  #   id_protocol = 3,
  #   status = 30,
  #   bmi270_sampling = 1,
  #   bmi270_acc_sensibility = 1,
  #   bmi270_gyro_sensibility = 1,
  #   bme688_sampling = 1,
  #   discontinuous_time = 1,
  #   port_tcp = 1,
  #   port_udp = 1,
  #   host_ip_addr = "127.0.0.1",
  #   ssid = "SUS",
  #   passwd = "SUS",
  # )

  scan_list = ble.scan_esp()
  esp = scan_list[0]
  # ble.send_config_BLE(esp["address"], 1)

  # print("Starting continous")

  # protocol = db.get_device_protocol(1)
  # thread = Thread(target=ble.recv_continous_BLE, args=(esp["address"], 1, protocol))
  # thread.start()
  # new_status = int(input("Enter new status"))
  # db.set_new_status(1, 31)
  # ble.stop_BLE(esp["address"])
  # thread.join()

  print("Starting discontinous")

  protocol = db.get_device_protocol(1)
  thread = Thread(target=ble.recv_discontinous_BLE, args=(esp["address"], 1, protocol))
  thread.start()
  new_status = int(input("Enter new status"))
  db.set_new_status(1, new_status)
  thread.join()