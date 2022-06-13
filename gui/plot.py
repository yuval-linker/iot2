import db


"""
case 0: temperatura
case 1: humedad
case 2: acc_x
case 3: acc_y
case 4: acc_z
case 5: rms
"""
def make_plot(plotBox, typeCase):
    #borrar toda la ventana
    plotBox.clear()

    #Agregar el plot
    p1 = plotBox.addPlot(row=0, col=0)

    #Buscar Datos
    data = []

    if(typeCase == 0):
        data = db.extractData(2)
    elif(typeCase == 1):
        data = db.extractData(4)
    elif(typeCase == 2):
        data = db.extractData(13)
    elif(typeCase == 3):
        data = db.extractData(14)
    elif(typeCase == 4):
        data = db.extractData(15)
    elif(typeCase == 5):
        data = db.extractData(6)

    #eje x
    x = range(len(data))
 
    #Poner info en el plot
    p1.plot(x, data)

def clearPlot(plotBox):
    plotBox.clear()

    
