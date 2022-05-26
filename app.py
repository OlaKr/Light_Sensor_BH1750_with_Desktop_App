from concurrent.futures import thread
from logging import exception
import sys
from PyQt5 import QtWidgets
from PyQt5.QtWidgets import QApplication, QMainWindow, QMessageBox
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
        # self.setGeometry(500, 500, 500, 500)

        # Buttons
        self.connect_button = self.findChild(QtWidgets.QPushButton, "connectbtn")
        self.connect_button.clicked.connect(self.connection)

        self.led_button = self.findChild(QtWidgets.QPushButton, "ledbtn")
        self.led_button.clicked.connect(self.toggleBtn)

        self.bh_button = self.findChild(QtWidgets.QPushButton, "bhbtn")
        self.bh_button.clicked.connect(self.getValues)

        self.calculate_btn = self.findChild(QtWidgets.QPushButton, "calculatebtn")
        # self.calculate_btn.clicked.connect(self.calculate)

        self.send_btn = self.findChild(QtWidgets.QPushButton, "sendbtn")
        # self.send_btn.clicked.connect(self.anotherMeasure)
        # self.send_btn.clicked.connect(self.message)

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
        # self.check_message.stateChanged.connect(self.message)


        # Slider
        self.bright_slider = self.findChild(QtWidgets.QSlider, "brightslider")
        self.bright_slider.setValue(50)
        self.bright_slider.valueChanged.connect(self.setBrightness)
        # self.bright_slider.setTickPosition(QSlider.TicksBelow)
        # self.bright_slider.setTickInterval(5)
        
        # threading.Timer(3.0, self.getValues).start()

    def toggleBtn(self):
        pin_off = "off\n"
        self.ser.write(pin_off.encode())
        print("LED off")
        self.update()

    def getValues(self):
        self.values_label.setText("Sensor results:")
        pin_on = "on\n"
        self.ser.write(pin_on.encode())

        try: 

            data = self.ser.readline()
            dec_data = float(data.decode('utf-8'))
            # dec_data = data.decode('utf-8')
            self.check_function(dec_data)

            # print(dec_data)
            # return dec_data
            # self.values_text.setText(dec_data)


        except Exception as exc:
            print(f"Exception: {exc}")

        # self.update()

    def calculate(self, getvalues):
        print(getvalues)
        max=800
        if getvalues>max:
            getvalues=max
        x = 100*getvalues/max
        # screen.set_brightness(x)
        return x

    def check_function(self, getvalues):
        if self.check_thread.isChecked() == True:
            # self.calculate(getvalues)
            screen.set_brightness(self.calculate(getvalues))


    def setBrightness(self, value):
        screen.set_brightness(str(value))
        get = screen.get_brightness()
        print(get)

    def message(self):
        if self.check_message.isChecked() == True:
            answer = QMessageBox.question(self, "MessageBox", "Do you want to adjust screen brightness?", QMessageBox.Yes | QMessageBox.Ignore)
            if answer == QMessageBox.Yes:
                screen.set_brightness(10)
                


    def connection(self):
        self.connect_label.setText("Your device is connected")
        self.ser = serial.Serial("COM6", 115200, timeout=None)
        print("App is connected with device")

    # def update(self):
    #     self.conLabel.adjustSize()

def custom_handler(signum, frame):
    print('ctrl+c was pressed')

signal.signal(signal.SIGINT, custom_handler)


def window():
    app = QApplication(sys.argv)
    win = MyWindow()
    win.show()
    
    timer = RepeatTimer(4,win.getValues)
    timer.start()

    # timer2 = RepeatTimer(20,win.message)
    # timer2.start()

    sys.exit(app.exec_())

class RepeatTimer(Timer):
    def run(self):
        while not self.finished.wait(self.interval):
            self.function()

window()