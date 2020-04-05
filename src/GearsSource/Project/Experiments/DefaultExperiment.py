import Gears as gears
import math
from PyQt5.QtWidgets import QApplication

class DefaultSequence(gears.Sequence) : 
    args = None
    name = None
    verboseLogging = False

    def __init__(self, name, **args):
        super().__init__( name )
        self.name = name
        self.args = args
        self.setPythonObject(self)
        self.boot(**args)

    def boot(self, frameRateDivisor = 1) :
        self.deviceFrameRate = 59.94
        self.frameRateDivisor = frameRateDivisor

        screenRes = QApplication.desktop().screenGeometry() 
        self.displayResolution = (screenRes.width(), screenRes.height()) #(1024, 768)
        self.display_width_px  = self.displayResolution[0]
        self.display_height_px = self.displayResolution[1]

        self.setFullfieldOptics()

        self.setDefaultSequenceChannels()
        #self.setBusyWaitingTickInterval(.01668335001668335001668335001668 * 0.25)

        self.electrodeDistance_um = (100, 100)
        self.electrodeOffset_um = (-750, -750)
        self.electrodeZone1 = (1, 0, 14, 15)
        self.electrodeZone2 = (0, 1, 15, 14)

        self.fft_width_px = 1024
        self.fft_height_px = 1024
        #self.electrodeDistance_um = (25, 25)
        #self.electrodeOffset_um = (-750, -750)
        #self.electrodeZone1 = (1, 0, 62, 63)
        #self.electrodeZone2 = (0, 1, 63, 62)

    def close(self):
        pass

    def setFullfieldOptics(self) :
        #self.field_width_um      =   2600
        #self.field_height_um     =   2000
        self.field_width_um      =   self.displayResolution[0] * 2
        self.field_height_um      =   self.displayResolution[1] * 2
        self.field_width_px      =   self.displayResolution[0]
        self.field_height_px     =   self.displayResolution[1]

    def setSquareOptics(self) :
        self.field_width_um      =   self.displayResolution[1] * 2
        self.field_height_um     =   self.displayResolution[1] * 2
        self.field_width_px      =   self.displayResolution[1]
        self.field_height_px     =   self.displayResolution[1]
        self.field_left_px       =   (self.displayResolution[0] - self.displayResolution[1]) // 2

    def setDefaultSequenceChannels(self) :
        self.addChannel('Msr stop', '\\\\.\\COM5', 'RTS')
        self.addChannel('Exp sync', '\\\\.\\COM5', 'BREAK')
        self.addChannel('Stim sync', '\\\\.\\COM3', 'RTS')
        self.addChannel('Tick', '\\\\.\\COM3', 'BREAK')
        self.onKeySpikeChannel = 'Stim sync'

    def abort(self) :
        gears.instantlyClearSignal('Exp sync')
        gears.instantlyRaiseSignal('Msr stop')
        gears.instantlyClearSignal('Msr stop')
