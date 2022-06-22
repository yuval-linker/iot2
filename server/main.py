from PyQt5 import QtWidgets
from gui.ui import Ui_Dialog
from gui import utils
from src import ble_conn as ble


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    Dialog = QtWidgets.QDialog()
    ui = Ui_Dialog()
    ui.setupUi(Dialog)
    ble.start_BLE()

    conn_threads = {}
    scanned_devices = []

    #Botones
    ui.search_esp32.clicked.connect(lambda : utils.search_esp32(
        scanned_devices=scanned_devices,
        esp32_select=ui.esp32_select,
        plot_1_device_name_select=ui.plot_1_device_name_select,
        plot_2_device_name_select=ui.plot_2_device_name_select,
        plot_3_device_name_select=ui.plot_3_device_name_select
    ))

    ui.config_btn.clicked.connect(lambda : utils.config_btn(
        scanned_devices=scanned_devices,
        conn_threads=conn_threads,
        esp32_select=ui.esp32_select,
        config_mode_select=ui.config_mode_select,
        status_select=ui.status_select,
        id_protocol_select = ui.id_protocol_select,
        acc_samp_field = ui.acc_samp_field,
        acc_sens_field = ui.acc_sens_field,
        gyro_sens_field = ui.gyro_sens_field,
        bme_field = ui.bme_field,
        disc_time_field = ui.disc_time_field,
        port_tcp_field = ui.port_tcp_field,
        port_udp_field = ui.port_udp_field,
        host_ip_addr_field = ui.host_ip_addr_field,
        ssid_field = ui.ssid_field,
        pass_field = ui.pass_field,

    ))


    Dialog.show()
    sys.exit(app.exec_())
