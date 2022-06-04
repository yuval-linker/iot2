import socket

BUFFER = 1024

def connectWithTCP(host,port,idProtocol):
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind((host, port))
        s.listen()
        conn, addr = s.accept()
        with conn:
            while True:
                data = conn.recv(BUFFER)
                print(data)
                if not data:
                    break

#Test
# connectWithTCP("localhost",1212,1)