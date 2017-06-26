'''
Monica Pineda 
CS 372
June 4, 2017
Program 2: This is a simple file transfer program where the server side is implemented in
C and the client side is implemented in Python.
ftclient.py: is the client side of this program. Once the server is started the client can connect
 to the server and either request a list of what is in the servers current directory or get a file from the 
 servers current directory using the filename. 
'''
'''
    Title: Computer netowrking Top-Down Approach 
    Author: James Kurose & Keith Ross 
    Date: 2017
    Code version: pgs 168-170 and pg 195 and the resource website 
    Availability: class book and https://media.pearsoncmg.com/aw/ecs_kurose_compnetwork_7/cw/index.php
'''


'''
    Title: StackOverflow:How can I get the IP address of eth0 in Python? 
    Author: Perer Mortensen
    Date: Mar 25, 2016
    Code version: pgs 168-170 and pg 195 and the resource website 
    Availability: http://stackoverflow.com/questions/24196932/how-can-i-get-the-ip-address-of-eth0-in-python
'''
from socket import *
import sys


def checks():
    if len(sys.argv) < 5 or len(sys.argv) > 6:# check arguments
        print "Invalid number of args"
        exit(1)
    elif (sys.argv[1] != "flip1" and sys.argv[1] != "flip2" and sys.argv[1] != "flip3"):# check that we are connected to flip1-3
        print "Invalid server name"
        exit(1)
    elif (int(sys.argv[2]) > 65535 or int(sys.argv[2]) < 1024):  # check valid port number
        print "Invalid control port number"
        exit(1)
    elif (sys.argv[3] != "-l" and sys.argv[3] != "-g"):#check that the option is correct
        print "Invalid option"
        exit(1)
    elif (sys.argv[3] == "-l" and (int(sys.argv[4]) > 65535 or int(sys.argv[4]) < 1024)):# check valid port number for when there are 5 args
        print "Invalid control port number"
        exit(1)
    elif (sys.argv[3] == "-g" and (int(sys.argv[5]) > 65535 or int(sys.argv[5]) < 1024)):  # check valid port number for when there are 6 args
    	print "Invalid control port number"
        exit(1)

def setup_data_socket():
    if sys.argv[3] == "-l": # if there are 5 args, port arg is 4, otherwise 5
        portarg = 4
    elif sys.argv[3] == "-g":
        portarg = 5
    serverport = int(sys.argv[portarg]) # create the server socket and accept a connection as data socket
    serversocket = socket(AF_INET, SOCK_STREAM)
    serversocket.bind(('', serverport))
    serversocket.listen(1)
    data_socket, addr = serversocket.accept()
    return data_socket

def get_file_list(data_socket):
    filename = data_socket.recv(100)# get the list of files and get the first filename
    while filename != "done":
        print filename# keep printing file names
        filename = data_socket.recv(100)

def get_file(data_socket):
    f = open(sys.argv[4],"w")# open a file for writing
    buffer = data_socket.recv(1000) # get the buffered output from the server
    while "__done__" not in buffer:# while we havent reached the end, write to the file
        f.write(buffer)
        buffer = data_socket.recv(1000)

def get_my_ip():
    s = socket(AF_INET, SOCK_DGRAM)# I was having touble getting the IP in c so I used a helper method to get IP address
    s.connect(("8.8.8.8", 80))
    return s.getsockname()[0]

def exchange_information(clientSocket):
    if sys.argv[3] == "-l": # if there are 5 arguments the port num is 4 otherwise the port number is five.
        print "Requesting file list"
        portnum = 4
    elif sys.argv[3] == "-g":
        print "Reqesting file {}".format(sys.argv[4])
        portnum = 5
    clientSocket.send(sys.argv[portnum])
    clientSocket.recv(1024) # there was a weird error werer python sends all pieces on same bufferer, this fixes it
    if sys.argv[3] == "-l":
        clientSocket.send("l")
    else:
        clientSocket.send("g")
    clientSocket.recv(1024)# there was a weird error werer python sends all pieces on same bufferer, this fixes it
    clientSocket.send(get_my_ip())# send my ip
    response = clientSocket.recv(1024)
    if response == "bad":# if the server didn't recognize the command, tell user
        print "Server received an invalid command"
        exit(1)
    if sys.argv[3] == "-g":# if we are getting a file, check if it exists
        clientSocket.send(sys.argv[4])
        response = clientSocket.recv(1024)
        if response != "File found":
            print "Client responded with 'File not found message'"
            return 
    data_socket = setup_data_socket() # set up the data socket
    if sys.argv[3] == "-l":#begin to receive data
       get_file_list(data_socket) 
    elif(sys.argv[3] == "-g"):
        get_file(data_socket) # otherwise get the file
        data_socket.close()# close the socket at the end


def connect_to_server():
    servername = sys.argv[1]+".engr.oregonstate.edu"# append the flip1-3 arg with the rest of the URL
    serverport = int(sys.argv[2])# transform the port into an int
    clientSocket = socket(AF_INET,SOCK_STREAM) # create a socket
    clientSocket.connect((servername, serverport)) # connect the socket
    return clientSocket

if __name__ == "__main__":
    checks() # check the args
    clientSocket = connect_to_server()# create a socket
    exchange_information(clientSocket)# communicate with server