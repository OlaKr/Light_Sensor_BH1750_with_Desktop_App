from concurrent.futures import thread
from logging import exception
import sys
import os
from matplotlib.pyplot import box
import serial.tools.list_ports
import re
from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox, QPlainTextEdit, QPushButton
from PyQt5 import uic, QtCore, QtGui

import screen_brightness_control as screen
import threading
import serial
import signal

from threading import Timer

from PyQt5.QtCore import QSize
from PyQt5.QtGui import QImage, QPalette, QBrush

class MyWindow(QMainWindow):
    def __init__(self):
        super(MyWindow, self).__init__()
        uic.loadUi('GUI.ui', self)
        
        self.setWindowTitle("Screen Brightness Control")

        set_image = QImage("background.png")
        size = set_image.scaled(QSize(736,488))
        palette = QPalette()
        palette.setBrush(QPalette.Window, QBrush(size))                        
        self.setPalette(palette)

        self.value = 0
        self.flag = 0
        self.box_flag = 0
        self.freq = 1

        self.text_update('Connect your device to use this app',1)

        # ComboBox
        self.port_box = self.findChild(QtWidgets.QComboBox, "port_box")
        self.find_port()
        self.baud_box = self.findChild(QtWidgets.QComboBox, "baud_box")
        self.baud_box.addItems(["9600","14400","19200","28800","38400","56000","57600","115200"])

        # SpinBox
        self.spin = self.findChild(QtWidgets.QSpinBox, "spinBox")
        self.spin.setMinimum(1)
        self.spin.valueChanged.connect(self.freq_spin)

        # Buttons
        self.connect_button = self.findChild(QtWidgets.QPushButton, "connectbtn")
        self.connect_button.clicked.connect(self.connection)

        # self.led_button = self.findChild(QtWidgets.QPushButton, "ledbtn")
        # self.led_button.clicked.connect(self.toggleBtn)

        self.adjust_btn = self.findChild(QtWidgets.QPushButton, "adjustbtn")
        self.adjust_btn.clicked.connect(self.onetime_adjust)


        # Labels
        self.connect_label = self.findChild(QtWidgets.QLabel, "connect_your_device")
        self.values_label = self.findChild(QtWidgets.QLabel, "get_values")
        self.brightness_label = self.findChild(QtWidgets.QLabel, "set_brightness_label")
        self.calculate_label = self.findChild(QtWidgets.QLabel, "calculate_label")

        # Text
        self.info = self.findChild(QtWidgets.QPlainTextEdit, "info")

        # Check buttons
        self.check_thread = self.findChild(QtWidgets.QCheckBox, "threadbox")

        # Slider
        self.bright_slider = self.findChild(QtWidgets.QSlider, "brightslider")
        self.actual_brightness = screen.get_brightness()
        self.bright_slider.setValue(self.actual_brightness[0])
        self.bright_slider.valueChanged.connect(self.setBrightness) 

        # CLI
        self.command_text = self.findChild(QtWidgets.QPlainTextEdit, "command")
        self.command_text.setStyleSheet("QPlainTextEdit {color: white}")
        self.run_btn = self.findChild(QtWidgets.QPushButton, "run_btn")
        self.run_btn.clicked.connect(self.runCommand)
        
        self.output_text = self.findChild(QtWidgets.QPlainTextEdit, "output")
        self.output_text.setStyleSheet("QPlainTextEdit {color: white}")
        self.clean_btn = self.findChild(QtWidgets.QPushButton, "clean_btn")
        self.clean_btn.clicked.connect(lambda: self.output_text.clear())
        self.clean_btn.clicked.connect(lambda: self.command_text.clear())

        self.screen_b()


    def onetime_adjust(self):
        if self.flag==1:
            screen.set_brightness(self.calculate(self.value))
        else:
            self.text_update('To choose this option click the CONNECT button',1)

    def screen_b(self):
        threading.Timer(self.freq, self.screen_b).start()
        if self.flag==0:
            self.check_thread.setCheckable(False)
            if self.check_thread.isChecked():
                self.text_update('To choose this option click on CONNECT button',1)
        else:
            self.check_thread.setCheckable(True)
        if self.check_thread.isChecked() == True:
            screen.set_brightness(self.calculate(self.value))


    def freq_spin(self):
        self.freq = self.spin.value()
        return self.freq


    def runCommand(self):
        if self.flag == 1:
            self.flag = 2
            line = self.command_text.toPlainText()
            print(line)
            line_end = line + '\n'
            self.ser.write(line_end.encode())

            try:
                data = self.ser.readline()
                data_string = data.decode('utf-8')

                if line == 'help':
                    endl = data_string.replace(";", "\n")
                    self.text_update(endl,2)
                    print(endl)
                else:        
                    self.text_update(data_string,2)
                    # print("valuee: ", data_string)
                    self.flag = 1

            except Exception as exc:
                print(f"Exception: {exc}")


    def toggleBtn(self):
        pin_off = "on\n"
        self.ser.write(pin_off.encode())
        print("LED off")
        self.update()

    def getValues(self):
        if self.flag==1:
            pin_on = "readValue\n"
            self.ser.write(pin_on.encode())

            try: 
                data = self.ser.readline()
                dec_data = float(data.decode('utf-8'))
                self.value = dec_data
                # print("value: ", self.value)

                self.actual_brightness = screen.get_brightness()
                self.bright_slider.setValue(self.actual_brightness[0])
                # print("actual: ", self.actual_brightness[0])

            except Exception as exc:
                print(f"Exception: {exc}")


    def calculate(self, getvalues):
        max=800
        if getvalues>max:
            getvalues=max
        x = 100*getvalues/max
        return x

    def setBrightness(self, value):
        screen.set_brightness(str(value))
        get = screen.get_brightness()
        print(get)

                
    def find_port(self):
        ports = serial.tools.list_ports.comports()
        for device in ports:
            self.port_box.addItems([str(device)])


    def text_update(self, text, label):
        if label==1:
            self.info.clear()
            self.info.insertPlainText(text)
        elif label==2:
            self.output_text.clear()
            self.output_text.insertPlainText(text)


    def connection(self):
        try:
            
            clicked_com = str(self.port_box.currentText())
            clicked_baud = str(self.baud_box.currentText())
            por = re.search(r"\bCOM\d", clicked_com)
            print("App is connected with device")
            if clicked_baud=="115200" and clicked_com!='-':
                self.ser = serial.Serial(por.group(), clicked_baud, timeout=None)
                self.flag = 1
                self.text_update('Your device is connected!',1)
            else:
                self.text_update('Set the appropriate com and baud rate!',1)
                self.connect_button.clicked.connect(self.connection)
        except Exception as exc:
            print(f"Exception: {exc}")

    def restart():
        QtCore.QCoreApplication.quit()
        check_status = QtCore.QProcess.startDetached(sys.executable, sys.argv)
        print(check_status)
        

def custom_handler(signum, frame):
    print('ctrl+c was pressed')

signal.signal(signal.SIGINT, custom_handler)


def window():
    app = QApplication(sys.argv)
    win = MyWindow()
    win.show()
    
    timer = RepeatTimer(1,win.getValues)
    timer.start()

    os._exit(app.exec_())
    

class RepeatTimer(Timer):
    def run(self):
        while not self.finished.wait(self.interval):
            self.function()

window()