from db import db

if __name__ == "__main__":
    db.set_device_config(
        id_device = 1,
        id_protocol = 1,
        status = 10,
        bmi270_sampling = 1,
        bmi270_acc_sensibility = 1,
        bmi270_gyro_sensibility = 1,
        bme688_sampling = 1,
        discontinuous_time = 1,
        port_tcp = 1,
        port_udp = 1,
        host_ip_addr = "AMOGUS",
        ssid = "AMOGUS",
        passwd = "AMOGUS",
    )

    print(db.get_device_status(id_device=1))

    db.set_device_config(
        id_device = 1,
        id_protocol = 1,
        status = 21,
        bmi270_sampling = 1,
        bmi270_acc_sensibility = 1,
        bmi270_gyro_sensibility = 1,
        bme688_sampling = 1,
        discontinuous_time = 1,
        port_tcp = 1,
        port_udp = 1,
        host_ip_addr = "SUS",
        ssid = "SUS",
        passwd = "SUS",
    )

    print(db.get_device_status(id_device=1))

    db.set_device_config(
        id_device = 1,
        id_protocol = 1,
        status = 31,
        bmi270_sampling = 1,
        bmi270_acc_sensibility = 1,
        bmi270_gyro_sensibility = 1,
        bme688_sampling = 1,
        discontinuous_time = 1,
        port_tcp = 1,
        port_udp = 1,
        host_ip_addr = "SUS",
        ssid = "SUS",
        passwd = "SUS",
    )

    print(db.get_device_status(id_device=1))
    