#!/usr/bin/python3

import socket
import os
import sys

from multiprocessing.pool import ThreadPool
from tkinter.font import BOLD
from PyQt5.QtGui import *
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *

measurement = ['0', '0', '0']
HOST = '128.141.94.197'  # Standard loopback interface address (localhost)
PORT = 65431        # Port to listen on (non-privileged ports are > 1023)

class Runnable(QRunnable):
       
    def __init__(self):
        super().__init__() 
        
    def send_msg(conn):  
        while(1):
            send_msg = input() # User input here
            if (send_msg == 'quit' ):
                print('Connection to client closed')
                os._exit(0)
            
            if (send_msg != ''): # Makes sure there is something typed before processing. 
                #Convert each letter send_msg an ASCII and saves it in an array.
                convert_to_ascii = [ord(c) for c in send_msg]
                
                my_bytes = bytearray() #Append each element to bytearray
                for x in convert_to_ascii:
                    my_bytes.append(x) 
                conn.send(my_bytes) #sends data to client

    def recv_msg(conn):
        global measurement
        while(1):
            data = conn.recv(1024)
            readableData = data.decode("utf-8")                  
            #ensure not getting spammed with empty empty strings
            if not readableData == "":
                measurement = readableData.split()
                measurement[0] = measurement[0][0:4] + u'\N{DEGREE SIGN}'
                measurement[1] = measurement[1][0:4] + u'\N{DEGREE SIGN}'
                measurement[2] = measurement[2][0:4] + u'\N{DEGREE SIGN}'

    
    def run(self):
        pool = ThreadPool(processes=3) 
        # creating threads
        pool.apply_async(Runnable.send_msg, args=(conn,))
        pool.apply_async(Runnable.recv_msg, args=(conn,))
        pool.apply_async(MyWindow.setupUI)
        
class MyWindow(QWidget):
    def __init__(self, parent=None):
        super(MyWindow, self).__init__(parent=parent)  # these values change where the main window is placed
        self.setWindowFlags(Qt.WindowStaysOnTopHint)
        self.title = 'This is my title'
        self.left = 400
        self.top = 400
        self.width = 300
        self.height = 200
        
        self.runTasks()
        self.setupUI()
        
    def runTasks(self):
        pool = QThreadPool.globalInstance()
        for i in range(1):
        # 2. Instantiate the subclass of QRunnable
            runnable = Runnable()
            pool.start(runnable) 
                
    def setupUI(self):
        self.setWindowTitle(self.title)
        self.setGeometry(self.left, self.top, self.width, self.height)
        
        #Grid layout
        self.createGridLayout()
        windowLayout = QVBoxLayout()
        windowLayout.addWidget(self.horizontalGroupBox)
        self.setLayout(windowLayout)
        self.show()
        
        self.setWindowTitle("Temperature Monitoring - Xilinx ZYNQ UltraScale+")
        custom_font = QFont('Arial')
        custom_font.setWeight(12);
        QApplication.setFont(custom_font, "QLabel")
        
        self.resize(270, 110)
        
    def createGridLayout(self):
          
        self.horizontalGroupBox = QGroupBox()
        self.Qlabel_0 = QLabel("%s" % measurement[0], self)
        self.Qlabel_1 = QLabel("%s" % measurement[1], self)
        self.Qlabel_2 = QLabel("%s" % measurement[2], self)
        
        layout = QGridLayout()
        layout.addWidget(QLabel("Device label: ZU7EV-1FBVB900E, 0.85V (nominal) "), 0, 0)
        layout.addWidget(QLabel("  0°C - 100°C"), 0, 1)
        layout.addWidget(QLabel("In the PS SYSMON near the RPU MPCore (LPD):"), 1, 0)
        layout.addWidget(self.Qlabel_0, 1, 1)
        layout.addWidget(QLabel("In the PS SYSMON Near the APU MPCore (FPD):"), 2, 0)
        layout.addWidget(self.Qlabel_1, 2, 1)
        layout.addWidget(QLabel("In the PL area near the PL SYSMON:"), 3, 0)
        layout.addWidget(self.Qlabel_2, 3, 1)
        self.horizontalGroupBox.setLayout(layout)

        
    def updateMeasurement(self):
        print('measurement: %s' % measurement)
        self.Qlabel_0.setText("      " + measurement[0])
        self.Qlabel_1.setText("      " + measurement[1])
        self.Qlabel_2.setText("      " + measurement[2])
        return measurement

## main ##
def main():
    app = QApplication(sys.argv)
    ex = MyWindow()

    timer = QTimer()
    timer.timeout.connect(ex.updateMeasurement)
    timer.start(1000)
    sys.exit(app.exec_())

if __name__ == '__main__':
    # creates socket object
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    s.bind((HOST, PORT))
    s.listen()
    print('Connecting to IP: {} Port: {}'.format(HOST,PORT))

    #seeking for the connection to be etablished
    conn, addr = s.accept()
    conn.send("Welcome to the server!".encode())
    print('Succes: Connection etablished')
    print("Send message to client or type 'quit' to exit")
    main()
