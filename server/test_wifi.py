from src import ble_conn as ble
from src import tcp_conn as tcp
from src import udp_conn as udp
from db import db
from threading import Thread
import time

def ask_for_status():
  new_status = int(input("Enter new status"))
  db.set_new_status(1, 0)

if __name__ == "__main__":
  ble.start_BLE()

  db.set_device_config(
    id_device = 1,
    id_protocol = 3,
    status = 20,
    bmi270_sampling = 1,
    bmi270_acc_sensibility = 1,
    bmi270_gyro_sensibility = 1,
    bme688_sampling = 1,
    discontinuous_time = 1,
    port_tcp = 25564,
    port_udp = 256565,
    host_ip_addr = "192.168.4.1",
    ssid = "raspberry_wifi",
    passwd = "wifigrupo3",
  )

  scan_list = ble.scan_esp()
  esp = scan_list[0]
  ble.send_config_BLE(esp["address"], 1)

  # Send config through TCP
  db.set_device_config(
    id_device = 1,
    id_protocol = 4,
    status = 21,
    bmi270_sampling = 1,
    bmi270_acc_sensibility = 1,
    bmi270_gyro_sensibility = 1,
    bme688_sampling = 1,
    discontinuous_time = 1,
    port_tcp = 25564,
    port_udp = 25565,
    host_ip_addr = "192.168.4.1",
    ssid = "raspberry_wifi",
    passwd = "wifigrupo3",
  )
  conf = db.get_device_config(1)
  tcp.send_config_tcp(conf["host_ip_addr"], conf["port_tcp"], 1)
  print("Sent Config through TCP")

  # Continous TCP
  t = Thread(target=tcp.init_tcp_continous_server, args=(conf["host_ip_addr"], conf["port_tcp"], 1, conf["id_protocol"]))
  t.start()
  new_status = int(input("Enter new status"))
  db.set_new_status(1, 23)
  t.join()
  
  time.sleep(5)
  
  print("Discontinous")

  # Discontinous TCP
  t = Thread(target=tcp.init_tcp_discontinous_server, args=(conf["host_ip_addr"], conf["port_tcp"], 1, conf["id_protocol"]))
  t.start()
  new_status = int(input("Enter new status"))
  db.set_new_status(1, 23)
  t.join()
  
  # Continous UDP
  t = Thread(target=ask_for_status)
  t.start()
  udp_conn = udp.ServerUdp(conf["host_ip_addr"], conf["port_udp"], 1)
  udp_conn.run()
  t.join()


