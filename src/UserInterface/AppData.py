import Gears
from PyQt5.QtWidgets import QApplication
import traceback
import os

appDataDir = 'not set'

cursors = {}

configParams = {}

def update(**args):
    global configParams
    tb = traceback.extract_stack()
    tb.pop()
    for key in args:
        if key in configParams:
            configParams[key] += [(args[key], os.path.normpath(tb[-1][0]))]
        else:
            configParams[key] = [(args[key], os.path.normpath(tb[-1][0]))]

def initConfigParams():
    global configParams
    configParams = {}
    screen = QApplication.screens()[0]
    display_width_px  = screen.size().width()
    display_height_px = screen.size().height()

    update(
        square_field            =  False                            ,
        monitorIndex            =  len(QApplication.screens())-1    ,
        deviceFrameRate         =  screen.refreshRate()             ,
        frameRateDivisor        =  1                                ,
        useHighFreqDevice       =  False                            ,
        useOpenCL               =  False                            ,
        field_width_um          =  display_width_px * 2             ,
        field_height_um         =  display_height_px * 2            ,
        field_width_px          =  display_width_px                 ,
        field_height_px         =  display_height_px                ,
        field_left_px           =  0                                ,
        field_top_px            =  0                                ,
        electrodeDistance_um_X  =  100                              ,
        electrodeDistance_um_Y  =  100                              ,
        electrodeOffset_um_X    =  -750                             ,
        electrodeOffset_um_Y    =  -750                             ,
        electrodeZone1_left     =  1                                ,
        electrodeZone1_top      =  0                                ,
        electrodeZone1_right    =  14                               ,
        electrodeZone1_bottom   =  15                               ,
        electrodeZone2_left     =  0                                ,
        electrodeZone2_top      =  1                                ,
        electrodeZone2_right    =  15                               ,
        electrodeZone2_bottom   =  14                               ,
        fft_width_px            =  1024                             ,
        fft_height_px           =  1024                             ,
        msr_stop_port           =  ('COM5', 'RTS')                  ,
        exp_sync_port           =  ('COM5', 'BREAK')                ,
        stim_sync_port          =  ('COM3', 'RTS')                  ,
        tick_port               =  ('COM3', 'BREAK')                ,
        )

