import Gears as gears
import math
from PyQt5.QtWidgets import QApplication
import AppData

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

    def boot(self, **args) :
        AppData.update(**args)
        self.monitorIndex = AppData.configParams['monitorIndex'][-1][0]
        self.deviceFrameRate = AppData.configParams['deviceFrameRate'][-1][0]
        self.frameRateDivisor = AppData.configParams['frameRateDivisor'][-1][0]
        self.useHighFreqRender = AppData.configParams['useHighFreqDevice'][-1][0]
        self.useOpenCL = AppData.configParams['useOpenCL'][-1][0]
        self.square_field = AppData.configParams['square_field'][-1][0]
        self.field_width_um = AppData.configParams['field_width_um'][-1][0]
        self.field_height_um = AppData.configParams['field_height_um'][-1][0]
        self.field_width_px = AppData.configParams['field_width_px'][-1][0]
        self.field_height_px = AppData.configParams['field_height_px'][-1][0]
        self.field_left_px = AppData.configParams['field_left_px'][-1][0]
        self.field_top_px = AppData.configParams['field_top_px'][-1][0]
        self.electrodeDistance_um = (AppData.configParams['electrodeDistance_um_X'][-1][0], AppData.configParams['electrodeDistance_um_Y'][-1][0])
        self.electrodeOffset_um = (AppData.configParams['electrodeOffset_um_X'][-1][0], AppData.configParams['electrodeOffset_um_Y'][-1][0])
        self.electrodeZone1 = (
            AppData.configParams['electrodeZone1_left'][-1][0],
            AppData.configParams['electrodeZone1_top'][-1][0],
            AppData.configParams['electrodeZone1_right'][-1][0],
            AppData.configParams['electrodeZone1_bottom'][-1][0])
        self.electrodeZone2 = (
            AppData.configParams['electrodeZone2_left'][-1][0],
            AppData.configParams['electrodeZone2_top'][-1][0],
            AppData.configParams['electrodeZone2_right'][-1][0],
            AppData.configParams['electrodeZone2_bottom'][-1][0])
        self.fft_width_px = AppData.configParams['fft_width_px'][-1][0]
        self.fft_height_px = AppData.configParams['fft_height_px'][-1][0]

        self.addChannel('Msr stop',   '\\\\.\\' +  AppData.configParams['msr_stop_port'][-1][0][0], AppData.configParams['msr_stop_port'][-1][0][1])
        self.addChannel('Exp sync',   '\\\\.\\' +  AppData.configParams['exp_sync_port'][-1][0][0], AppData.configParams['exp_sync_port'][-1][0][1])
        self.addChannel('Stim sync',  '\\\\.\\' +  AppData.configParams['stim_sync_port'][-1][0][0], AppData.configParams['stim_sync_port'][-1][0][1])
        self.addChannel('Tick',       '\\\\.\\' +  AppData.configParams['tick_port'][-1][0][0], AppData.configParams['tick_port'][-1][0][1])
        self.onKeySpikeChannel = 'Stim sync'

        if self.square_field :
            self.setSquareOptics()
        #self.setDefaultSequenceChannels()
        #self.setBusyWaitingTickInterval(.01668335001668335001668335001668 * 0.25)

    def close(self):
        pass

    def setFullfieldOptics(self) :
        #self.field_width_um      =   2600
        #self.field_height_um     =   2000
        self.field_width_um      =   self.displayResolution[0] * 2
        self.field_height_um     =   self.displayResolution[1] * 2
        self.field_width_px      =   self.displayResolution[0]
        self.field_height_px     =   self.displayResolution[1]

    def setSquareOptics(self) :
        self.field_left_px       =   (self.field_width_px - self.field_height_px) // 2
        self.field_width_um      =   self.field_height_um
        self.field_width_px      =   self.field_height_px

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
