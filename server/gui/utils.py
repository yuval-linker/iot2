import json
import statistics
from src import ble_conn as ble
from src import tcp_conn as tcp
from src import udp_conn as udp
from db import db

from threading import Thread

config_mode_select_dict = {
    0: 0,
    1: 20
}

status_select_dict = {
    0: 21,
    1: 22,
    2: 23,
    3: 30,
    4: 31
}

id_protocol_select_dict = {
    0: 1,
    1: 2,
    2: 3,
    3: 4,
    4: 5
}

plot_select_var_dict = {
    0: 4,
    1: 6,
    2: 7,
    3: 5,
    4: 15,
    5: 16,
    6: 17,
    7: 8
}

def search_esp32(**kwargs):
    scanned_devices = kwargs["scanned_devices"]
    
    esp32_select = kwargs["esp32_select"]
    plot_1_device_name_select = kwargs["plot_1_device_name_select"]
    plot_2_device_name_select = kwargs["plot_2_device_name_select"]
    plot_3_device_name_select = kwargs["plot_3_device_name_select"]

    print("Scanning")

    scan_list = ble.scan_esp()
    scanned_devices.clear()
    device_names = []
    if scan_list:
        for device in scan_list:
            scanned_devices.append(device)

        device_names = [x["name"] for x in scan_list]

    esp32_select.clear()
    plot_1_device_name_select.clear()
    plot_2_device_name_select.clear()
    plot_3_device_name_select.clear()

    esp32_select.addItems(device_names)
    plot_1_device_name_select.addItems(device_names)
    plot_2_device_name_select.addItems(device_names)
    plot_3_device_name_select.addItems(device_names)

def config_btn(**kwargs):
    scanned_devices = kwargs["scanned_devices"]
    esp32_select = kwargs["esp32_select"]

    device_name = esp32_select.currentText()
    device = [d for d in scanned_devices if d["name"] == device_name][0]

    conn_threads = kwargs["conn_threads"]

    device_id = esp32_select.currentIndex()

    config_mode_select = kwargs["config_mode_select"]
    config_mode = config_mode_select_dict[int(config_mode_select.currentIndex())]

    status_select = kwargs["status_select"]
    status = status_select_dict[int(status_select.currentIndex())]

    id_protocol_box = kwargs["id_protocol_select"]
    id_protocol = id_protocol_select_dict[int(id_protocol_box.currentIndex())]

    print("Configuring")

    # we update the db with the new config, setting the status as 
    # config_mode to let the esp32 update its own status.
    db.set_device_config(
        id_device = device_id,
        id_protocol = id_protocol,
        status = config_mode,
        bmi270_sampling = int(kwargs["acc_samp_field"].toPlainText()),
        bmi270_acc_sensibility = int(kwargs["acc_sens_field"].toPlainText()),
        bmi270_gyro_sensibility = int(kwargs["gyro_sens_field"].toPlainText()),
        bme688_sampling = int(kwargs["bme_field"].toPlainText()),
        discontinuous_time = int(kwargs["disc_time_field"].toPlainText()),
        port_tcp = int(kwargs["port_tcp_field"].toPlainText()),
        port_udp = int(kwargs["port_udp_field"].toPlainText()),
        host_ip_addr = str(kwargs["host_ip_addr_field"].toPlainText()),
        ssid = str(kwargs["ssid_field"].toPlainText()),
        passwd = str(kwargs["pass_field"].toPlainText()),
    )

    ble.stop_BLE(device["address"])

    # we check if there is an ongoing connection between the
    # raspberry and the esp32, if true we wait until it stops 
    # due to the change of the status to config_mode
    thread = conn_threads.get(device_id)
    if thread is not None:
        thread.join()
        del conn_threads[device_id] # delete conn entry

    # we launch the config thread and join it instantly
    if config_mode == 0:
        conf_thread = Thread(target=ble.send_config_BLE, args=(device["address"], device_id))
    
    elif config_mode == 20:
        conf_thread = Thread(target=tcp.send_config_tcp, args=(
            str(kwargs["host_ip_addr_field"].toPlainText()),
            int(kwargs["port_tcp_field"].toPlainText()),
            device_id
        ))
    
    # before we send the config we must update the status from
    # config_mode to the desired status.
    db.set_new_status(device_id, status)
    conf_thread.start()
    conf_thread.join()
    


    # once the esp is successfully configured, we begin the connection.
    if status == 21:
        thread = Thread(target=tcp.init_tcp_continous_server, args=(
            str(kwargs["host_ip_addr_field"].toPlainText()),
            int(kwargs["port_tcp_field"].toPlainText()),
            device_id,
            id_protocol,
        ))
    elif status == 22:
        thread = Thread(target=tcp.init_tcp_discontinous_server, args=(
            str(kwargs["host_ip_addr_field"].toPlainText()),
            int(kwargs["port_tcp_field"].toPlainText()),
            device_id,
            id_protocol,
        ))
    elif status == 23:
        sv = udp.ServerUdp(
            host=str(kwargs["host_ip_addr_field"].toPlainText()),
            sv_port=int(kwargs["port_udp_field"].toPlainText()),
            id_device=device_id,
            id_protocol=id_protocol,
        )
        thread = Thread(target=udp.init_udp_server, args=(sv,))
    elif status == 30:
        thread = Thread(target=ble.recv_continous_BLE, args=(
            device["address"],
            device_id,
            id_protocol,
        ))
    elif status == 31:
        thread = Thread(target=ble.recv_discontinous_BLE, args=(
            device["address"],
            device_id,
            id_protocol,
        ))
    
    conn_threads[device_id] = thread
    thread.start()

    print("Configuring Done")


def reset_btn(**kwargs):
    esp32_select = kwargs["esp32_select"]
    conn_threads = kwargs["conn_threads"]
    device_id = esp32_select.currentIndex()

    db.set_new_status(device_id, 0)
    thread = conn_threads.get(device_id)
    
    if thread is not None:
        thread.join()
        del conn_threads[device_id]
    

def make_plot(**kwargs):
    # retrieve from gui the plot, device and var.
    plot = kwargs["plot"]
    var_id = plot_select_var_dict[int(kwargs["plot_select_var"].currentIndex())]
    device_id = int(kwargs["plot_select_device"].currentIndex())

    # clear the current plot    
    plot.clear()

    p = plot.addPlot(row=0, col=0)
    data = [d[var_id] for d in db.get_esp32_data(id_device=device_id)]
    if var_id >= 15 and var_id <= 17:   # acc_x, acc_y, acc_z are encoded arrays
        data = [statistics.mean(json.loads(d)) for d in data]   # we use the mean value.
    
    # x-axis
    x = range(len(data))

    # plot!
    p.plot(x, data)  
    