from concurrent.futures import thread
from logging import exception
from re import A
import sys
import os
from matplotlib.pyplot import box
import serial.tools.list_ports
import re
from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox, QPlainTextEdit
from PyQt5 import uic

import screen_brightness_control as screen

import serial
import signal

from threading import Timer


class MyWindow(QMainWindow):
    def __init__(self):
        super(MyWindow, self).__init__()
        uic.loadUi('myApp.ui', self)
        
        self.setWindowTitle("MyApp")

        self.value = 0
        self.flag = 0
        self.box_flag = 0

        # ComboBox
        self.port_box = self.findChild(QtWidgets.QComboBox, "port_box")
        self.set_port()
        self.baud_box = self.findChild(QtWidgets.QComboBox, "baud_box")
        self.baud_box.addItems(["9600","14400","19200","28800","38400","56000","57600","115200"])

        # Buttons
        self.connect_button = self.findChild(QtWidgets.QPushButton, "connectbtn")
        self.connect_button.clicked.connect(self.connection)

        self.led_button = self.findChild(QtWidgets.QPushButton, "ledbtn")
        self.led_button.clicked.connect(self.toggleBtn)

        self.calculate_btn = self.findChild(QtWidgets.QPushButton, "calculatebtn")
        # self.calculate_btn.clicked.connect(self.calculate)


        # Labels
        self.connect_label = self.findChild(QtWidgets.QLabel, "connect_your_device")
        self.values_label = self.findChild(QtWidgets.QLabel, "get_values")
        self.brightness_label = self.findChild(QtWidgets.QLabel, "set_brightness_label")
        self.calculate_label = self.findChild(QtWidgets.QLabel, "calculate_label")

        # Text
        self.values_text = self.findChild(QtWidgets.QTextEdit, "values")


        # Check buttons
        self.check_thread = self.findChild(QtWidgets.QCheckBox, "threadbox")
        self.check_message = self.findChild(QtWidgets.QCheckBox, "messagebox")
        # self.check_message.clicked.connect(self.message)
        self.check_message.stateChanged.connect(self.message)


        # Slider
        self.bright_slider = self.findChild(QtWidgets.QSlider, "brightslider")
        self.actual_brightness = screen.get_brightness()
        self.bright_slider.setValue(self.actual_brightness[0])
        self.bright_slider.valueChanged.connect(self.setBrightness)
        # self.bright_slider.setTickPosition(QSlider.TicksBelow)
        # self.bright_slider.setTickInterval(5)

        # CLI
        self.command_text = self.findChild(QtWidgets.QPlainTextEdit, "command")
        self.run_btn = self.findChild(QtWidgets.QPushButton, "run_btn")
        self.run_btn.clicked.connect(self.runCommand)
        
        self.output_text = self.findChild(QtWidgets.QPlainTextEdit, "output")
        self.clean_btn = self.findChild(QtWidgets.QPushButton, "clean_btn")
        self.clean_btn.clicked.connect(lambda: self.output_text.clear())

        self.output_text.insertPlainText('dir')
        # self.command_text.insertPlainText('>> ')
        # cwd = os.getcwd()
        
        # print("Jakie cwd:", cwd)
        # threading.Timer(3.0, self.getValues).start()

    def runCommand(self):
        line = self.command_text.toPlainText()      #.strip()
        print(line)

        # p = os.popen(line)
        # if p:
        #     self.output_text.clear()
        #     outputt = p.read()
        #     self.output_text.insertPlainTezt(outputt)

        if line=="cwd":
            self.output_text.clear()
            self.output_text.insertPlainText(os.getcwd())
            print("Jakie cwd:", os.getcwd())
        elif line=="help":
            self.output_text.clear()
            show_help = "Available commands: \n - cwd \n - on"
            self.output_text.insertPlainText(show_help)
        elif line=="on":
            pin_on = "on\n"
            self.ser.write(pin_on.encode())
        elif line=="off":
            pin_off = "off\n"
            self.ser.write(pin_off.encode())
    

    def toggleBtn(self):
        pin_off = "on\n"
        self.ser.write(pin_off.encode())
        print("LED off")
        self.update()

    def getValues(self):
        if self.flag==1:
            self.values_label.setText("Sensor results:")
            pin_on = "on\n"
            self.ser.write(pin_on.encode())

            try: 
        
                data = self.ser.readline()
                dec_data = float(data.decode('utf-8'))
                self.value = dec_data
                print("value: ", self.value)

                if self.check_thread.isChecked() == True:
                    screen.set_brightness(self.calculate(self.value))

                self.actual_brightness = screen.get_brightness()
                self.bright_slider.setValue(self.actual_brightness[0])
                print("AKTUALNA: ", self.actual_brightness[0])
                # self.values_text.setText(dec_data)

            except Exception as exc:
                print(f"Exception: {exc}")


    def calculate(self, getvalues):
        print(getvalues)
        max=800
        if getvalues>max:
            getvalues=max
        x = 100*getvalues/max
        return x


    def setBrightness(self, value):
        screen.set_brightness(str(value))
        get = screen.get_brightness()
        print(get)

    def message(self):
        # if self.check_message.isChecked() == True:
        if self.box_flag==0:
            print("jasnossc: ", screen.get_brightness())
            self.actual_brightness = screen.get_brightness()
            if self.actual_brightness[0]-20>self.calculate(self.value) or self.actual_brightness[0]+20<self.calculate(self.value):
                print("ale2:", self.value)
                self.box_flag = 1
                answer = QMessageBox.question(self, "MessageBox", "Do you want to adjust screen brightness?", QMessageBox.Yes | QMessageBox.Ignore)
                if answer == QMessageBox.Yes:
                    screen.set_brightness(self.calculate(self.value))
                    self.bright_slider.setValue(self.actual_brightness[0])
                    self.box_flag = 0
                
    def set_port(self):
        ports = serial.tools.list_ports.comports()
        for device in ports:
            self.port_box.addItems([str(device)])


    def connection(self):
        self.connect_label.setText("Device is connected")
        clicked_com = str(self.port_box.currentText())
        clicked_baud = str(self.baud_box.currentText())
        por = re.search(r"\bCOM\d", clicked_com)
        # self.ser = serial.Serial("COM6", "115200", timeout=None)
        self.ser = serial.Serial(por.group(), clicked_baud, timeout=None)
        print("App is connected with device")
        self.flag = 1 # device is connected
        

def custom_handler(signum, frame):
    print('ctrl+c was pressed')

signal.signal(signal.SIGINT, custom_handler)


def window():
    app = QApplication(sys.argv)
    win = MyWindow()
    win.show()
    
    timer = RepeatTimer(1,win.getValues)
    timer.start()

    # timer2 = RepeatTimer(20,win.message)
    # timer2.start()

    sys.exit(app.exec_())

class RepeatTimer(Timer):
    def run(self):
        while not self.finished.wait(self.interval):
            self.function()

window()