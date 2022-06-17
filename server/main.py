from PyQt5 import QtWidgets
from .gui.ui import Ui_Dialog
from .gui import button
from .src import ble_conn as ble


if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    Dialog = QtWidgets.QDialog()
    ui = Ui_Dialog()
    ui.setupUi(Dialog)
    ble.start_BLE()

    #Botones
    ui.boton_graficar.clicked.connect(lambda: button.buttonGraph(ui.plot1,ui.selec_plot1.currentIndex(),ui.plot2,ui.selec_plot2.currentIndex()))
    ui.boton_borrarBD.clicked.connect(lambda: button.buttonBorrarDB())
    ui.boton_configuracion.clicked.connect(lambda: button.buttonConfigESP())
    ui.boton_buscar_ESP.clicked.connect(lambda: ble.scan_esp())
    
    Dialog.show()
    sys.exit(app.exec_())
