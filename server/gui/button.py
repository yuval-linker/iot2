from . import plot
import db
from PyQt5 import QtWidgets

def button_graph(plotBox1, typeCase1,plotBox2, typeCase2):

    plot.make_plot(plotBox1,typeCase1)
    plot.make_plot(plotBox2,typeCase2)

def buttonBorrarDB():
    db.borrar_db()

def button_config_ESP():

    #codigo para configurar

    msg = QtWidgets.QMessageBox()
    msg.setWindowTitle("Config")
    msg.setText("Configuración Completada")

    x = msg.exec_()
    
