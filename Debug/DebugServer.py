from socket import *
from datetime import datetime
import threading
import traceback
import sys

buff = []
def createServer():
    now = datetime.now()
    with socket(AF_INET, SOCK_STREAM) as serversocket:
        serversocket.bind(('192.168.1.6',25565))
        serversocket.listen(5)
        while True:
            try:
                (client, address) = serversocket.accept()

                client.recv(5000).decode('cp1252')
                dt = now.strftime('%d/%m/%Y %H:%M:%S')
                rd = dt + ': ' + client.recv(5000).decode('cp1252')
                print(rd)
                buff.append(rd)

                data = 'HTTP/1.1 200 OK\r\n'
                data += 'Content-Type: text/html; charset=utf-8\r\n'
                data += '\r\n'
                data += 'OK\r\n\r\n'

                client.sendall(data.encode())
                client.shutdown(SHUT_WR)
            except Exception as e:
                print('server thread closed')
                track = traceback.format_exc()
                print(track)
                #I know I am the most lazy person in the world.
                #I hope u like it
                createServer()


sthread = threading.Thread(target=createServer, daemon=True)
sthread.start()

while True:
    stop = input('input y to stop anytime\n\n\n')
    if stop.lower() == 'y':
        with open('data.txt', 'a') as file:
            file.writelines(buff)
        sys.exit(0)
