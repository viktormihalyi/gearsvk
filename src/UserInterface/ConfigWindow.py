import sys
import Gears as gears
import importlib.machinery
import os
from PyQt5.QtCore import (Qt, QCoreApplication, QTimer)
from PyQt5.QtWidgets import (QWidget, QDialog, QGridLayout, QLabel, QPushButton, QLineEdit, QGroupBox, QRadioButton, QButtonGroup, QTextEdit, QSpinBox, QDoubleSpinBox, QCheckBox, QComboBox, QApplication, QTabWidget)
from PyQt5.QtGui import (QFont, QPalette, QTextCursor )

import AppData
import serial.tools.list_ports as list_ports

class ConfigWindow(QDialog):

    def __init__(self, filename):
        super().__init__()
        self.setCursor(AppData.cursors['arrow'])
        self.filename = filename + '\\config.py'
        self.settings = []
        self.comports = list_ports.comports()
        self.setWindowFlags(Qt.FramelessWindowHint)
        self.initUI()
                
    def initUI(self):
        self.setStyleSheet("""
            QWidget{
                border: none;
                background-color: black;
                font: bold 14px;
                color: red;
                font-family: "Candara";
                font-size: 14px;
            }
            QWidget:disabled{
                border: none;
                background-color: black;
                font: bold 14px;
                color: #880000;
                font-family: "Candara";
                font-size: 14px;
            }
            QGroupBox{
                padding : 16px;
                border : 1px solid #660000;
            }
            QLabel{
                border: none;
            }
            QLabel:disabled{
                border: none;
                color : #880000
            }
            QLabel::tooltip{
                background: black;
            }
            QPushButton:hover{
                text-decoration: underline;
            }
            QRadioButton{
                border : 0px solid green;
            }
            QRadioButton::indicator::unchecked{ 
                border: 1px solid red;
                border-radius: 6px;
                background-color: black;
                width: 10px;
                height: 10px;
                margin-left: 5px;
            }
            QRadioButton::indicator::checked{
                border: 1px solid red;
                border-radius: 6px;
                background-color: #880000;
                width: 10px;
                height: 10px;
                margin-left: 5px;
            }
            QSpinBox{
                selection-color: black;
                selection-background-color: red;
            }
            QSpinBox:disabled{
                border: #880000;
                color: #880000;
            }
            QDoubleSpinBox{
                selection-color: black;
                selection-background-color: red;
            }
            QDoubleSpinBox:disabled{
                border: #880000;
                color: #880000;
            }
            QSpinBox::up-button {
                subcontrol-position: top left;
            }
            QSpinBox::down-button {
                subcontrol-position: bottom left;
            }
            QDoubleSpinBox::up-button {
                subcontrol-position: top left;
            }
            QDoubleSpinBox::down-button {
                subcontrol-position: bottom left;
            }
            QTextEdit { 
                selection-color: #000000;
                selection-background-color: #880000;
            }
            QCheckBox{
                border : 0px solid green;
            }
            QCheckBox::indicator::unchecked{ 
                border: 1px solid red;
                border-radius: 1px;
                background-color: black;
                width: 10px;
                height: 10px;
                margin-left: 5px;
            }
            QCheckBox::indicator::checked{
                border: 1px solid red;
                border-radius: 1px;
                background-color: #880000;
                width: 10px;
                height: 10px;
                margin-left: 5px;
            }
            QComboBox{
                padding-left: 10px;
            }
            QComboBox QAbstractItemView{
                border: 1px solid #ff0000;
                border-left-color = #00ff00;
                selection-background-color: #ff0000;
                selection-color: #000000;
            }
            QComboBox::drop-down {
                subcontrol-position: left;
                border: 1px solid red;
                image: url(Gui/stylesheet-branch-open.png);
            }
            QWidget::item:hover {
                background: black;
                border: 1px solid red;
            }
            QWidget::item:selected {
                border: 1px solid red;
                color: red;
            }
            QWidget::item:selected:active{
                background: black;
                border: 1px solid red;
            }
            QWidget::item:selected:!active {
                border: 1px solid red;
                color: red;
            }
            QTabBar::tab {
                background: black;
                border: 2px solid red;
                border-bottom-color: black;
                border-top-left-radius: 4px;
                border-top-right-radius: 4px;
                min-width: 8ex;
                padding: 2px;
            }
            QTabBar::tab:selected, QTabBar::tab:hover {
                background: red;
                color: black;
            }
            QTabWidget::pane {
                background: black;
            }
            QDialog{
                border: 1 px red;
            }
            """)
        grid = QGridLayout()
        self.optionsTabber = QTabWidget(self)

        self.displayBox = QWidget()
        displayBoxgl = QGridLayout()

        rows = 1

        self.addParam(
            layout = displayBoxgl,
            label = 'Stimulus monitor',
            box = self.displayBox,
            paramName = 'monitorIndex',
            rowIndex = rows,
            listmonitor = True,
            )
        rows += 1

        self.addParam(
                    layout = displayBoxgl,
                    label = 'Refresh rate',
                    box = self.displayBox,
                    paramName = 'deviceFrameRate',
                    rowIndex = rows,
                    decimals = 2,
                    minimum = 1,
                    maximum = 1000,
                    unit = 'Hz',
                    float = True,
                 )
        rows += 1

        self.addParam(
                    layout = displayBoxgl,
                    label = 'Use High Frequence Device',
                    box = self.displayBox,
                    paramName = 'useHighFreqDevice',
                    rowIndex = rows,
                    unit = '',
                    boolean = True,
                 )
        rows += 1

        self.addParam(
                    layout = displayBoxgl,
                    label = 'Use OpenCL for FFT',
                    box = self.displayBox,
                    paramName = 'useOpenCL',
                    rowIndex = rows,
                    unit = '',
                    boolean = True,
                 )
        rows += 1

        self.addParam(
                    layout = displayBoxgl,
                    label = 'Field width',
                    box = self.displayBox,
                    paramName = 'field_width_um',
                    rowIndex = rows,
                    minimum = 1,
                    maximum = 10000,
                    unit = 'μm',
                    float = False,
                 )
        rows += 1
        self.addParam(
                    layout = displayBoxgl,
                    label = 'Field height',
                    box = self.displayBox,
                    paramName = 'field_height_um',
                    rowIndex = rows,
                    minimum = 1,
                    maximum = 10000,
                    unit = 'μm',
                    float = False,
                 )
        rows += 1

        self.addParam(
                    layout = displayBoxgl,
                    label = 'Force square field',
                    box = self.displayBox,
                    paramName = 'square_field',
                    rowIndex = rows,
                    unit = '',
                    boolean = True,
                 )
        rows += 1

        self.addParam(
                    layout = displayBoxgl,
                    label = 'Field horizontal resolution',
                    box = self.displayBox,
                    paramName = 'field_width_px',
                    rowIndex = rows,
                    minimum = 1,
                    maximum = 10000,
                    unit = 'px',
                    float = False,
                 )
        rows += 1
        self.addParam(
                    layout = displayBoxgl,
                    label = 'Field vertical resolution',
                    box = self.displayBox,
                    paramName = 'field_height_px',
                    rowIndex = rows,
                    minimum = 1,
                    maximum = 10000,
                    unit = 'px',
                    float = False,
                 )
        rows += 1
        self.addParam(
                    layout = displayBoxgl,
                    label = 'Field left margin',
                    box = self.displayBox,
                    paramName = 'field_left_px',
                    rowIndex = rows,
                    minimum = 0,
                    maximum = 10000,
                    unit = 'px',
                    float = False,
                 )
        rows += 1
        self.addParam(
                    layout = displayBoxgl,
                    label = 'Field top margin',
                    box = self.displayBox,
                    paramName = 'field_top_px',
                    rowIndex = rows,
                    minimum = 0,
                    maximum = 10000,
                    unit = 'px',
                    float = False,
                 )
        rows += 1

        self.displayBox.setLayout(displayBoxgl)
        self.optionsTabber.addTab(self.displayBox, 'Display')

        self.channelBox = QWidget()

        rows = 1
        channelBoxgl = QGridLayout()

        self.addParam(
            layout = channelBoxgl,
            label = 'Measurement stop',
            box = self.channelBox,
            paramName = 'msr_stop_port',
            rowIndex = rows,
            listport = True,
            )
        rows += 1

        self.addParam(
            layout = channelBoxgl,
            label = 'Experiment sync',
            box = self.channelBox,
            paramName = 'exp_sync_port',
            rowIndex = rows,
            listport = True,
            )
        rows += 1

        self.addParam(
            layout = channelBoxgl,
            label = 'Stimulus sync',
            box = self.channelBox,
            paramName = 'stim_sync_port',
            rowIndex = rows,
            listport = True,
            )
        rows += 1

        self.addParam(
            layout = channelBoxgl,
            label = 'Per frame spikes',
            box = self.channelBox,
            paramName = 'tick_port',
            rowIndex = rows,
            listport = True,
            )
        rows += 1
        self.channelBox.setLayout(channelBoxgl)

        self.optionsTabber.addTab(self.channelBox, 'Ports')

        rows = 1
        self.fftBox = QWidget()
        fftBoxgl = QGridLayout()

        self.addParam(
            layout = fftBoxgl,
            label = 'FFT horizontal resolution',
            box = self.fftBox,
            paramName = 'fft_width_px',
            minimum = 16,
            maximum = 4096,
            unit = 'px',
            rowIndex = rows,
            float = True,
            )
        rows += 1

        self.addParam(
            layout = fftBoxgl,
            label = 'FFT vertical resolution',
            box = self.fftBox,
            paramName = 'fft_height_px',
            minimum = 16,
            maximum = 4096,
            unit = 'px',
            rowIndex = rows,
            float = True,
            )
        rows += 1

        self.fftBox.setLayout(fftBoxgl)
        self.optionsTabber.addTab(self.fftBox, 'FFT')

        rows = 1
        self.electrodeBox = QWidget()
        electrodeBoxgl = QGridLayout()

        self.addParam(
            layout = electrodeBoxgl,
            label = 'Distance between electrodes (axis x)',
            box = self.electrodeBox,
            paramName = 'electrodeDistance_um_X',
            minimum = 0,
            maximum = 10000,
            unit = 'μm',
            rowIndex = rows,
            float = False,
            )
        rows += 1

        self.addParam(
            layout = electrodeBoxgl,
            label = 'Distance between electrodes (axis y)',
            box = self.electrodeBox,
            paramName = 'electrodeDistance_um_Y',
            minimum = 0,
            maximum = 10000,
            unit = 'μm',
            rowIndex = rows,
            float = False,
            )
        rows += 1

        self.addParam(
            layout = electrodeBoxgl,
            label = 'Position of column A (on axis x)',
            box = self.electrodeBox,
            paramName = 'electrodeOffset_um_X',
            minimum = -10000,
            maximum = 10000,
            unit = 'μm',
            rowIndex = rows,
            float = False,
            )
        rows += 1

        self.addParam(
            layout = electrodeBoxgl,
            label = 'Position of row 1 (on axis y)',
            box = self.electrodeBox,
            paramName = 'electrodeOffset_um_Y',
            minimum = -10000,
            maximum = 10000,
            unit = 'μm',
            rowIndex = rows,
            float = False,
            )
        rows += 1

        self.addParam(
            layout = electrodeBoxgl,
            label = 'Zone 1 leftmost index',
            box = self.electrodeBox,
            paramName = 'electrodeZone1_left',
            minimum = 0,
            maximum = 34,
            unit = '',
            rowIndex = rows,
            float = False,
            )
        rows += 1

        self.addParam(
            layout = electrodeBoxgl,
            label = 'Zone 1 topmost index',
            box = self.electrodeBox,
            paramName = 'electrodeZone1_top',
            minimum = 0,
            maximum = 34,
            unit = '',
            rowIndex = rows,
            float = False,
            )
        rows += 1

        self.addParam(
            layout = electrodeBoxgl,
            label = 'Zone 1 rightmost index',
            box = self.electrodeBox,
            paramName = 'electrodeZone1_right',
            minimum = 0,
            maximum = 34,
            unit = '',
            rowIndex = rows,
            float = False,
            )
        rows += 1

        self.addParam(
            layout = electrodeBoxgl,
            label = 'Zone 1 bottommost index',
            box = self.electrodeBox,
            paramName = 'electrodeZone1_bottom',
            minimum = 0,
            maximum = 34,
            unit = '',
            rowIndex = rows,
            float = False,
            )
        rows += 1

        self.addParam(
            layout = electrodeBoxgl,
            label = 'Zone 2 leftmost index',
            box = self.electrodeBox,
            paramName = 'electrodeZone2_left',
            minimum = 0,
            maximum = 34,
            unit = '',
            rowIndex = rows,
            float = False,
            )
        rows += 1

        self.addParam(
            layout = electrodeBoxgl,
            label = 'Zone 2 topmost index',
            box = self.electrodeBox,
            paramName = 'electrodeZone2_top',
            minimum = 0,
            maximum = 34,
            unit = '',
            rowIndex = rows,
            float = False,
            )
        rows += 1

        self.addParam(
            layout = electrodeBoxgl,
            label = 'Zone 2 rightmost index',
            box = self.electrodeBox,
            paramName = 'electrodeZone2_right',
            minimum = 0,
            maximum = 34,
            unit = '',
            rowIndex = rows,
            float = False,
            )
        rows += 1

        self.addParam(
            layout = electrodeBoxgl,
            label = 'Zone 2 bottommost index',
            box = self.electrodeBox,
            paramName = 'electrodeZone2_bottom',
            minimum = 0,
            maximum = 34,
            unit = '',
            rowIndex = rows,
            float = False,
            )
        rows += 1
        self.electrodeBox.setLayout(electrodeBoxgl)
        self.optionsTabber.addTab(self.electrodeBox, 'Electrode grid')

        grid.addWidget(self.optionsTabber, 1, 1, 1, 2)
        self.cancelButton = QPushButton('Cancel')
        self.cancelButton.setStyleSheet('QPushButton{border: 1px solid red;}')
        self.cancelButton.pressed.connect(self.cancel)
        grid.addWidget(self.cancelButton, 2, 1, 1, 1)
        self.okButton = QPushButton('OK')
        self.okButton.setStyleSheet('QPushButton{border: 1px solid red;}')
        self.okButton.pressed.connect(self.ok)
        grid.addWidget(self.okButton, 2, 2, 1, 1)
        self.setLayout(grid)

    def addParam(self, *, 
                    layout = None,
                    label = 'some param name',
                    box = None,
                    paramName = 'n/a',
                    rowIndex = 1,
                    decimals = 2,
                    minimum = 0,
                    maximum = 10000,
                    unit = '',
                    float = True,
                    listport = False,
                    listmonitor = False,
                    boolean = False
                 ):
        self.settings += [paramName]
        layout.addWidget(QLabel(label, box), rowIndex, 1, 1, 1)
        if listmonitor:
            valc = QComboBox()
            i = 0 
            for monitor in QApplication.screens():
                valc.insertItem(i, str(i) + ': ' + monitor.name(), i)
                i += 1
            sindex = valc.findData( AppData.configParams[paramName][-1][0] )
            if sindex == -1:
                sindex = 0
            valc.setCurrentIndex(sindex)
        elif listport:
            valc = QComboBox()
            i = 0 
            for port in self.comports:
                valc.insertItem(i, 'RTS on ' + port[0] + ': ' + port[1], (port[0], 'RTS') )
                valc.insertItem(i, 'BREAK on ' + port[0] + ': ' + port[1], (port[0], 'BREAK') )
                i += 1
            sindex = valc.findData( AppData.configParams[paramName][-1][0][1] )
            if sindex == -1:
                valc.insertItem(i, AppData.configParams[paramName][-1][0][1] + ' on ' + AppData.configParams[paramName][-1][0][0] + ': no device detected', AppData.configParams[paramName][-1][0])
            else:
                valc.setCurrentIndex(sindex)
        elif boolean:
            valc = QComboBox()
            valc.insertItem(0, 'True', True)
            valc.insertItem(1, 'False', False)
            sindex = valc.findData( AppData.configParams[paramName][-1][0] )
            valc.setCurrentIndex(sindex)
        else:
            if float:
                valc = QDoubleSpinBox()
                valc.setDecimals(decimals)
            else:
                valc = QSpinBox()
            valc.lineEdit().setCursor(AppData.cursors['text'])
            valc.setMinimum(minimum)
            valc.setMaximum(maximum)
            valc.setValue(AppData.configParams[paramName][-1][0])
        setattr(self, paramName, valc)
        dowc = QLabel()
        checkc = QPushButton()
        filec = QLabel()
        setattr(self, paramName + 'Default', checkc)
        def resetParam():
            setattr(self, paramName + 'Override', False)
            if AppData.configParams[paramName][-1][1] == self.filename:
                dowc.setText('')
                checkc.setText('')
                filec.setText(unit + ' from ' + AppData.configParams[paramName][-2][1])
                dowc.setEnabled(False)
                checkc.setEnabled(False)
                filec.setEnabled(False)
                valc.blockSignals(True)
                if listmonitor or listport or boolean:
                    sindex = valc.findData( AppData.configParams[paramName][-2][0] )
                    valc.setCurrentIndex(sindex)
                else:
                    valc.setValue(AppData.configParams[paramName][-2][0])
                valc.blockSignals(False)
            else:
                dowc.setText('')
                checkc.setText('')
                filec.setText(unit + ' from ' + AppData.configParams[paramName][-1][1])
                dowc.setEnabled(False)
                checkc.setEnabled(False)
                filec.setEnabled(False)
                valc.blockSignals(True)
                if listmonitor or listport or boolean:
                    sindex = valc.findData( AppData.configParams[paramName][-1][0] )
                    valc.setCurrentIndex(sindex)
                else:
                    valc.setValue(AppData.configParams[paramName][-1][0])
                valc.blockSignals(False)
        def trumpParam():
            setattr(self, paramName + 'Override', True)
            if AppData.configParams[paramName][-1][1] == self.filename:
                dowc.setText(unit + ' overriding')
                checkc.setText(str(AppData.configParams[paramName][-2][0]) + ' ' + unit)
                filec.setText( 'specified in ' + AppData.configParams[paramName][-2][1])
                dowc.setEnabled(True)
                checkc.setEnabled(True)
                filec.setEnabled(True)
            else:
                dowc.setText(unit + ' overriding')
                checkc.setText(str(AppData.configParams[paramName][-1][0]) + ' ' + unit)
                filec.setText( '\tspecified in ' + AppData.configParams[paramName][-1][1])
                dowc.setEnabled(True)
                checkc.setEnabled(True)
                filec.setEnabled(True)

        checkc.pressed.connect(resetParam)
        if listport or listmonitor or boolean:
            valc.currentIndexChanged.connect(trumpParam)
        else:
            valc.valueChanged.connect(trumpParam)
        overridesParam = AppData.configParams[paramName][-1][1] == self.filename
        setattr(self, paramName + 'Override', overridesParam)
        if overridesParam:
            trumpParam()
        else:
            resetParam()
        layout.addWidget(valc, rowIndex, 2, 1, 1)
        layout.addWidget(dowc, rowIndex, 3, 1, 1)
        layout.addWidget(checkc, rowIndex, 4, 1, 1)
        layout.addWidget(filec, rowIndex, 5, 1, 1)

    def cancel(self):
        self.done(0)

    def ok(self):
        file = open( self.filename, 'w')
        print('import AppData', file=file)
        print('AppData.update(', file=file)
        for s in self.settings:
            if getattr(self, s + 'Override'):
                try:
                    value = getattr(self, s).value()
                except:
                    c = getattr(self, s)
                    value = c.itemData( c.currentIndex() )
                print('  {paramname} = {paramval},'.format(paramname=s, paramval=value), file=file)
        print('  )', file=file)
        file.close()
        self.done(1)