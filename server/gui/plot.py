import db


"""
case 0: temperatura
case 1: humedad
case 2: co
case 3: presion 
case 4: acc_x
case 5: acc_y
case 6: acc_z
case 7: rms
"""
def make_plot(plot_box, type_case):
    #borrar toda la ventana
    plot_box.clear()

    #Agregar el plot
    p1 = plot_box.addPlot(row=0, col=0)

    #Buscar Datos
    data = []
    #eje x
    x = range(len(data))
 
    #Poner info en el plot
    p1.plot(x, data)

def clearPlot(plot_box):
    plot_box.clear()

    
