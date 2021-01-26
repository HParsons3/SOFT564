import socket            
 
sock = socket.socket()
 
host = "192.168.1.237" #ESP32 IP in local network
port = 80             #ESP32 Server Port    
 
sock.connect((host, port))
 
message = input()
message = message.encode('utf-8')
sock.send(message)
 
data = ""   
datafinal = ""   
 
while len(datafinal) < 21:
    data = sock.recv(1)
    datafinal += data.decode()

 
print(datafinal)
 
sock.close()