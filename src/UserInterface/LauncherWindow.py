import sys
import Gears as gears
import importlib.machinery
import os
from PyQt5.QtCore import (Qt, QCoreApplication, QTimer)
from PyQt5.QtWidgets import (QWidget, QToolTip, QPushButton, QLineEdit, QFileDialog, QMessageBox, QApplication, QTreeWidget, QTreeWidgetItem, QGridLayout, QGroupBox, QTabWidget, QDoubleSpinBox, QLabel, QRadioButton, QButtonGroup, QScrollBar, QSlider, QTextEdit)
from PyQt5.QtGui import (QFont, QPalette, QTextCursor )
#from PyQt5.QtOpenGL import (, QGLFormat)

import datetime as datetime
import time
import warnings

import ILog

from SequenceTimeline import SequenceTimeline
from StimulusTimeline import StimulusTimeline
from SpatialKernelPlot import SpatialKernelPlot
from TemporalKernelPlot import TemporalKernelPlot
from SpatialProfilePlot import SpatialProfilePlot
from Ide import Ide
import AppData

class LauncherWindow(QWidget):
    browserWindow = None
#    stimulusWindow = None
    sequenceTimeline = None
    stimulusTimeline = None
    stimulusLabel = None
    sequenceTimelineBox = None
    sequenceTimelineZoom = 512
    stimulusTimelineZoom = 512
    timer = None
    spatialPlotMin = -1
    spatialPlotMax = 1
    spatialPlotSize = 1000
    aspect = 1
    midFrame = None
    mousePos = 1.0

    def __init__(self):
        super().__init__()
        self.calibrating = False
        self.setCursor(AppData.cursors['arrow'])
        self.initUI()
        gears.onHideStimulusWindow(self.wake)
                
    def initUI(self):
##        pal = QPalette();
##        pal.setColor(QPalette.Background, Qt.black);
##        self.setPalette(pal);
        self.setStyleSheet("""
            QWidget{
                border-style: solid;
                border-width: 1px;
                border-radius: 0px;
                border-color: red;
                background-color: black;
                font: bold 14px;
                color: red;
                font-family: "Candara";
                font-size: 14px;
            }

            QToolTip{
                color: red;
                padding: 0px;
                background-image: url(Media/lolcat1.jpg);
                margin: 0 0 0 0;
            }

            QScrollBar:horizontal {
                 border: 2px solid red;
                 background: black;
                 height: 15px;
                 margin: 0 20px 0 20px;
             }
             QScrollBar::handle:horizontal {
                 background: red;
                 min-width: 20px;
             }
              QScrollBar::add-line:horizontal {
                 border: 2px red;
                 background: black;
                 width: 20px;
                 subcontrol-position: right;
                 subcontrol-origin: margin;
             }
             QScrollBar::sub-line:horizontal {
                 border: 2px red;
                 background: black;
                 width: 20px;
                 subcontrol-position: left;
                 subcontrol-origin: margin;
             }

             QScrollBar::left-arrow:horizontal, QScrollBar::right-arrow:horizontal {
                 border: 2px solid black;
                 width: 3px;
                 height: 3px;
                 background: red;
             }

             QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {
                 background: none;
             }

            QScrollBar:vertical {
                 border: 2px solid red;
                 background: black;
                 margin: 20px 0 20px 0;
             }
             QScrollBar::handle:vertical {
                 background: red;
                 min-height: 20px;
             }
             QScrollBar::add-line:vertical {
                 border: 2px red;
                 background: black;
                 height: 20px;
                 subcontrol-position: bottom;
                 subcontrol-origin: margin;
             }
             QScrollBar::sub-line:vertical {
                 border: 2px red;
                 background: black;
                 height: 20px;
                 subcontrol-position: top;
                 subcontrol-origin: margin;
             }
             QScrollBar::down-arrow:vertical, QScrollBar::up-arrow:vertical {
                 border: 2px solid black;
                 width: 3px;
                 height: 3px;
                 background: red;
             }

             QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {
                 background: none;
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
            QSlider {
                border : none;
            }
            QSlider::groove:horizontal {
                border: 1px solid red;
                background: black;
                height: 6px;
                border-radius: 0px;
            }

            QSlider::sub-page:horizontal {
                background: black;
                border: 1px solid red;
                height: 10px;
                border-radius: 0px;
            }

            QSlider::add-page:horizontal {
                background: black;
                border: 1px solid red;
                height: 10px;
                border-radius: 0px;
            }

            QSlider::handle:horizontal {
            background: black;
            border: 1px solid red;
            width: 10px;
            margin-top: -6px;
            margin-bottom: -6px;
            border-radius: 0px;
            }

            QSlider::handle:horizontal:hover {
            background: red;
            border: 1px solid red;
            border-radius: 0px;
            }

            QSlider::sub-page:horizontal:disabled {
            background: black;
            border-color: red;
            }

            QSlider::add-page:horizontal:disabled {
            background: black;
            border-color: red;
            }

            QSlider::handle:horizontal:disabled {
            background: black;
            border: 1px solid red;
            border-radius: 0px;
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
                color : #440000
            }
            QLabel::tooltip{
                background: black;
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
            QDoubleSpinBox{
                selection-color: black;
                selection-background-color: red;
            }
            QDoubleSpinBox:disabled{
                border: #440000;
                color: #440000;
            }
            QTextEdit { 
                selection-color: #000000;
                selection-background-color: #880000;
            }
            """)

        grid = QGridLayout()

        self.sequenceTimelineBox = QGroupBox('Sequence timeline', self)
        self.sequenceTimelineBox.setStyleSheet("QGroupBox{ font-size: 30px; padding : 32px; }")
        #font = self.sequenceTimelineBox.font()
        #font.setPointSizeF(font.pointSizeF() * 2)
        #self.sequenceTimelineBox.setFont(font)

        self.sequenceTimeline = SequenceTimeline(self.sequenceTimelineBox, self, self.winId())
        grid.addWidget(self.sequenceTimelineBox, 1, 1, 2, 8)
        etlbgl = QGridLayout()
        etlbgl.addWidget(self.sequenceTimeline, 1, 1, 1, 10)
        self.sequenceLabel = QLabel(
            'Device refresh rate: {devfps:.2f},   Sequence frame rate: {fps:.2f}'.format( devfps=59.94, fps=59.94)
            , self.sequenceTimelineBox)
        etlbgl.addWidget(self.sequenceLabel, 0, 1, 1, 10)

        self.sequenceTimelineScrollbar = QScrollBar(Qt.Horizontal, self.sequenceTimelineBox)
        etlbgl.addWidget(self.sequenceTimelineScrollbar, 2, 1, 1, 8)
        self.sequenceTimelineScrollbar.valueChanged.connect(self.onScroll)
        #self.sequenceTimelineZoomExtents = QPushButton('F', self.sequenceTimelineBox)
        #etlbgl.addWidget(self.sequenceTimelineZoomExtents, 2, 7, 1, 1)
        self.sequenceTimelineZoomSlider = QSlider(Qt.Horizontal, self.sequenceTimelineBox)
        etlbgl.addWidget(self.sequenceTimelineZoomSlider, 2, 9, 1, 2)
        self.sequenceTimelineZoomSlider.valueChanged.connect(self.onZoomSlider)
        #self.sequenceTimelineZoomMax = QPushButton('M', self.sequenceTimelineBox)
        #etlbgl.addWidget(self.sequenceTimelineZoomMax, 2, 10, 1, 1)
        etlbgl.setSpacing(0)
        etlbgl.setContentsMargins(0, 0, 0, 0)
        self.sequenceTimelineBox.setLayout(etlbgl)
        
        self.optionsTabber = QTabWidget(self)
        self.mainPanel = QWidget()
        ##self.stimulusTimelineBox = QGroupBox('Click in sequence timeline to select stimulus', self)
        self.stimulusTimeline = StimulusTimeline(self.mainPanel, self, self.winId())
        self.stimulusLabel = QLabel('Click in sequence timeline to select stimulus!', self.mainPanel)
        mly = QGridLayout()
        mly.addWidget(self.stimulusTimeline, 1, 1, 1, 10)
        mly.addWidget(self.stimulusLabel, 0, 1, 1, 10)
        self.stimulusTimelineScrollbar = QScrollBar(Qt.Horizontal, self.mainPanel)
        mly.addWidget(self.stimulusTimelineScrollbar, 2, 1, 1, 8)
        self.stimulusTimelineScrollbar.valueChanged.connect(self.onStimScroll)
        self.stimulusTimelineZoomSlider = QSlider(Qt.Horizontal, self.mainPanel)
        mly.addWidget(self.stimulusTimelineZoomSlider, 2, 9, 1, 2)
        self.stimulusTimelineZoomSlider.valueChanged.connect(self.onStimZoomSlider)
        self.mainPanel.setLayout(mly)

        self.optionsTabber.addTab(self.mainPanel, 'Timeline')

        self.spatialPanel = QWidget()
        self.spatialKernelPlot = SpatialKernelPlot(self.spatialPanel, self, self.winId())
        self.spatialProfilePlot =  SpatialProfilePlot(self.spatialPanel, self, self.winId())
        spapag = QGridLayout()
        spapag.addWidget(self.spatialKernelPlot, 1, 1, 3, 4)
        spapag.addWidget(self.spatialProfilePlot, 1, 7, 3, 4)
       
        spapag.addWidget(QLabel("Plot min", self.spatialPanel), 3, 5)
        self.minSpinBox = QDoubleSpinBox(self.spatialPanel)
        self.minSpinBox.lineEdit().setCursor(AppData.cursors['text'])
        self.minSpinBox.setDecimals(6)
        self.minSpinBox.setMinimum(-10)
        self.minSpinBox.setMaximum(0)
        self.minSpinBox.setValue(self.spatialPlotMin)
        self.minSpinBox.valueChanged.connect(self.setSpatialPlotMin)
        spapag.addWidget(self.minSpinBox, 3, 6)

        spapag.addWidget(QLabel("Plot max", self.spatialPanel), 1, 5)
        self.maxSpinBox = QDoubleSpinBox(self.spatialPanel)
        self.maxSpinBox.lineEdit().setCursor(AppData.cursors['text'])
        self.maxSpinBox.setDecimals(6)
        self.maxSpinBox.setMinimum(0)
        self.maxSpinBox.setMaximum(10.0)
        self.maxSpinBox.setValue(self.spatialPlotMax)
        self.maxSpinBox.valueChanged.connect(self.setSpatialPlotMax)
        spapag.addWidget(self.maxSpinBox, 1, 6)

        spapag.addWidget(QLabel("Plot zoom (µm)", self.spatialPanel), 2, 5)
        self.sizeSpinBox = QDoubleSpinBox(self.spatialPanel)
        self.sizeSpinBox.lineEdit().setCursor(AppData.cursors['text'])
        self.sizeSpinBox.setDecimals(1)
        self.sizeSpinBox.setMinimum(10)
        self.sizeSpinBox.setMaximum(1000.0)
        self.sizeSpinBox.setValue(self.spatialPlotSize)
        self.sizeSpinBox.valueChanged.connect(self.setspatialPlotSize)
        spapag.addWidget(self.sizeSpinBox, 2, 6)

        #spapag.addWidget(QLabel("Height", self.spatialPanel), 4, 1)
        #self.heightSpinBox = QDoubleSpinBox(self.spatialPanel)
        #self.heightSpinBox.setDecimals(4)
        #self.heightSpinBox.setMinimum(10)
        #self.heightSpinBox.setMaximum(1000.0)
        #self.heightSpinBox.setValue(self.spatialPlotHeight)
        #self.heightSpinBox.valueChanged.connect(self.setSpatialPlotHeight)
        #spapag.addWidget(self.heightSpinBox, 4, 2)

        self.spatialPanel.setLayout(spapag)

        self.managePanel = QWidget()
        mpapag = QGridLayout()

        exportCommentRadioGroup = QGroupBox('Comments in data export', self.managePanel)
        mpapag.addWidget(exportCommentRadioGroup, 1, 1)
        ecrl = QGridLayout()
        self.ecrHashmark = QRadioButton("Use # for comments", exportCommentRadioGroup)
        ecrl.addWidget(self.ecrHashmark, 1, 1)
        self.ecrNone =   QRadioButton("None", exportCommentRadioGroup)
        ecrl.addWidget(self.ecrNone, 1, 2)
        exportCommentRadioGroup.setLayout(ecrl)
        self.ecrHashmark.clicked.connect(self.exportRandomChanged)
        self.ecrNone.clicked.connect(self.exportRandomChanged)

        exportChannelsRadioGroup = QGroupBox('Number of colors to export', self.managePanel)
        mpapag.addWidget(exportChannelsRadioGroup, 2, 1)
        echrl = QGridLayout()
        self.echr1 = QRadioButton("1", exportChannelsRadioGroup)
        echrl.addWidget(self.echr1, 1, 1)
        self.echr2 = QRadioButton("2", exportChannelsRadioGroup)
        echrl.addWidget(self.echr2, 1, 2)
        self.echr3 = QRadioButton("3", exportChannelsRadioGroup)
        echrl.addWidget(self.echr3, 1, 3)
        self.echr4 = QRadioButton("4", exportChannelsRadioGroup)
        echrl.addWidget(self.echr4, 1, 4)
        exportChannelsRadioGroup.setLayout(echrl)
        self.echr1.clicked.connect( self.exportRandomChanged ) 
        self.echr2.clicked.connect( self.exportRandomChanged )
        self.echr3.clicked.connect( self.exportRandomChanged ) 
        self.echr4.clicked.connect( self.exportRandomChanged ) 


        exportFormatRadioGroup = QGroupBox('Export randoms as type', self.managePanel)
        mpapag.addWidget(exportFormatRadioGroup, 3, 1)
        efrl = QGridLayout()
        self.efrBinary =     QRadioButton("Binary", exportFormatRadioGroup)
        efrl.addWidget(self.efrBinary, 1, 1)
        self.efrReal =       QRadioButton("Real", exportFormatRadioGroup)
        efrl.addWidget(self.efrReal, 1, 2)
        self.efrInteger =    QRadioButton("Integer", exportFormatRadioGroup)
        efrl.addWidget(self.efrInteger, 1, 3)
        exportFormatRadioGroup.setLayout(efrl)
        self.efrBinary.clicked.connect( self.exportRandomChanged ) 
        self.efrReal.clicked.connect( self.exportRandomChanged ) 
        self.efrInteger.clicked.connect( self.exportRandomChanged ) 

        exportButton = QPushButton('Save randoms', self.managePanel)
        exportButton.clicked.connect(self.exportRandoms)
        mpapag.addWidget(exportButton, 4, 1)

        self.managePanel.setLayout(mpapag)

        self.temporalPanel = QWidget()
        self.temporalKernelPlot = TemporalKernelPlot(self.temporalPanel, self, self.winId())
        tpapag = QGridLayout()
        tpapag.addWidget(self.temporalKernelPlot, 1, 1)
        self.temporalPanel.setLayout(tpapag)

        self.calibrationPanel = QWidget()
        cpapag = QGridLayout()
        calibMeasuredMinLabel = QLabel('Measured min: ', self.calibrationPanel)
        cpapag.addWidget(calibMeasuredMinLabel, 1, 1, 1, 1)
        self.calibMeasuredMinValue = QLabel('N/A', self.calibrationPanel)
        cpapag.addWidget(self.calibMeasuredMinValue, 1, 2, 1, 1)
        calibMeasuredMaxLabel = QLabel('Measured max: ', self.calibrationPanel)
        cpapag.addWidget(calibMeasuredMaxLabel, 2, 1, 1, 1)
        self.calibMeasuredMaxValue = QLabel('N/A', self.calibrationPanel)
        cpapag.addWidget(self.calibMeasuredMaxValue, 2, 2, 1, 1)

        calibApplyMinButton = QPushButton('->')
        calibApplyMinButton.clicked.connect(self.applyMeasuredToneMin)
        cpapag.addWidget(calibApplyMinButton, 1, 3, 1, 1)
        self.calibToneMinLabel = QLabel('Intensity min: ', self.calibrationPanel)
        cpapag.addWidget(self.calibToneMinLabel, 1, 4, 1, 1)
        self.calibToneMinValue = QDoubleSpinBox(self.calibrationPanel)
        self.calibToneMinValue.lineEdit().setCursor(AppData.cursors['text'])
        self.calibToneMinValue.setDecimals(2)
        self.calibToneMinValue.setMinimum(-500)
        self.calibToneMinValue.setMaximum(500)
        self.calibToneMinValue.setSingleStep(0.05)
        self.calibToneMinValue.setValue(0)
        cpapag.addWidget(self.calibToneMinValue, 1, 5, 1, 1)
        calibApplyMaxButton = QPushButton('->')
        calibApplyMaxButton.clicked.connect(self.applyMeasuredToneMax)
        cpapag.addWidget(calibApplyMaxButton, 2, 3, 1, 1)
        self.calibToneMaxLabel = QLabel('Intensity max: ', self.calibrationPanel)
        cpapag.addWidget(self.calibToneMaxLabel, 2, 4, 1, 1)
        self.calibToneMaxValue = QDoubleSpinBox(self.calibrationPanel)
        self.calibToneMaxValue.lineEdit().setCursor(AppData.cursors['text'])
        self.calibToneMaxValue.setDecimals(2)
        self.calibToneMaxValue.setMinimum(-500)
        self.calibToneMaxValue.setMaximum(500)
        self.calibToneMaxValue.setSingleStep(0.05)
        self.calibToneMaxValue.setValue(1)
        cpapag.addWidget(self.calibToneMaxValue, 2, 5, 1, 1)

        calibApplyMeanButton = QPushButton('->')
        calibApplyMeanButton.clicked.connect(self.applyMeasuredToneMean)
        cpapag.addWidget(calibApplyMeanButton, 3, 3, 1, 1)
        self.calibToneMeanLabel = QLabel('Intensity mean: ', self.calibrationPanel)
        cpapag.addWidget(self.calibToneMeanLabel, 3, 4, 1, 1)
        self.calibToneMeanValue = QDoubleSpinBox(self.calibrationPanel)
        self.calibToneMeanValue.lineEdit().setCursor(AppData.cursors['text'])
        self.calibToneMeanValue.setDecimals(2)
        self.calibToneMeanValue.setMinimum(-4)
        self.calibToneMeanValue.setMaximum(5)
        self.calibToneMeanValue.setSingleStep(0.05)
        self.calibToneMeanValue.setValue(0)
        cpapag.addWidget(self.calibToneMeanValue, 3, 5, 1, 1)
        calibApplyVarButton = QPushButton('->')
        calibApplyVarButton.clicked.connect(self.applyMeasuredToneVar)
        cpapag.addWidget(calibApplyVarButton, 4, 3, 1, 1)
        self.calibToneVarLabel = QLabel('Intensity var: ', self.calibrationPanel)
        cpapag.addWidget(self.calibToneVarLabel, 4, 4, 1, 1)
        self.calibToneVarValue = QDoubleSpinBox(self.calibrationPanel)
        self.calibToneVarValue.lineEdit().setCursor(AppData.cursors['text'])
        self.calibToneVarValue.setDecimals(2)
        self.calibToneVarValue.setMinimum(-4)
        self.calibToneVarValue.setMaximum(5)
        self.calibToneVarValue.setSingleStep(0.05)
        self.calibToneVarValue.setValue(1)
        cpapag.addWidget(self.calibToneVarValue, 4, 5, 1, 1)

        calibMeasuredMeanLabel = QLabel('Measured mean: ', self.calibrationPanel)
        cpapag.addWidget(calibMeasuredMeanLabel, 3, 1, 1, 1)
        self.calibMeasuredMeanValue = QLabel('N/A', self.calibrationPanel)
        cpapag.addWidget(self.calibMeasuredMeanValue, 3, 2, 1, 1)
        calibMeasuredVarLabel = QLabel('Measured variance: ', self.calibrationPanel)
        cpapag.addWidget(calibMeasuredVarLabel, 4, 1, 1, 1)
        self.calibMeasuredVarValue = QLabel('N/A', self.calibrationPanel)
        cpapag.addWidget(self.calibMeasuredVarValue, 4, 2, 1, 1)


        toneMappingRadioGroup = QGroupBox('Tone mapping', self.calibrationPanel)
        cpapag.addWidget(toneMappingRadioGroup, 5, 3, 2, 3)
        tmrl = QGridLayout()
        self.toneMappingRadioNone =     QRadioButton("None", toneMappingRadioGroup)
        tmrl.addWidget(self.toneMappingRadioNone, 1, 1)
        self.toneMappingRadioLinear =       QRadioButton("Linear", toneMappingRadioGroup)
        tmrl.addWidget(self.toneMappingRadioLinear, 1, 2)
        self.toneMappingRadioErf =    QRadioButton("Sigmoid (erf)", toneMappingRadioGroup)
        tmrl.addWidget(self.toneMappingRadioErf, 1, 3)
        self.toneMappingRadioEq =    QRadioButton("Hist. equalized", toneMappingRadioGroup)
        tmrl.addWidget(self.toneMappingRadioEq, 1, 4)
        toneMappingRadioGroup.setLayout(tmrl)
        self.toneMappingRadioNone.clicked.connect( self.toneMappingChanged ) 
        self.toneMappingRadioLinear.clicked.connect( self.toneMappingChanged ) 
        self.toneMappingRadioErf.clicked.connect( self.toneMappingChanged ) 
        self.toneMappingRadioEq.clicked.connect( self.toneMappingChanged ) 

        self.saveToneMapSettingsButton = QPushButton('Save tone mapping')
        cpapag.addWidget(self.saveToneMapSettingsButton, 7, 3, 1, 3)
        self.saveToneMapSettingsButton.clicked.connect(self.saveToneMapSettings)

        calibHistoMinLabel = QLabel('Histogram min: ', self.calibrationPanel)
        cpapag.addWidget(calibHistoMinLabel, 6, 1, 1, 1)
        self.calibHistoMinValue = QDoubleSpinBox(self.calibrationPanel)
        self.calibHistoMinValue.lineEdit().setCursor(AppData.cursors['text'])
        self.calibHistoMinValue.setDecimals(2)
        self.calibHistoMinValue.setMinimum(-4)
        self.calibHistoMinValue.setMaximum(5)
        self.calibHistoMinValue.setSingleStep(0.05)
        self.calibHistoMinValue.setValue(-1)
        cpapag.addWidget(self.calibHistoMinValue, 6, 2, 1, 1)
        calibHistoMaxLabel = QLabel('Histogram max: ', self.calibrationPanel)
        cpapag.addWidget(calibHistoMaxLabel, 7, 1, 1, 1)
        self.calibHistoMaxValue = QDoubleSpinBox(self.calibrationPanel)
        self.calibHistoMaxValue.lineEdit().setCursor(AppData.cursors['text'])
        self.calibHistoMaxValue.setDecimals(2)
        self.calibHistoMaxValue.setMinimum(-4)
        self.calibHistoMaxValue.setMaximum(5)
        self.calibHistoMaxValue.setSingleStep(0.05)
        self.calibHistoMaxValue.setValue(2)
        cpapag.addWidget(self.calibHistoMaxValue, 7, 2, 1, 1)

        caliButton = QPushButton('&Measure', self.calibrationPanel)
        caliButton.clicked.connect(self.launchCalibration)
        cpapag.addWidget(caliButton, 5, 1, 1, 2)
        self.calibrationPanel.setLayout(cpapag)

        self.outputVideoPanel = QWidget()
        vpapag = QGridLayout()

        
        
        self.videoPath = QLabel('Output video file: ', self.outputVideoPanel)
        vpapag.addWidget(self.videoPath, 1, 1, 1, 1)
               
        
        self.videoFileName = QLineEdit(self.outputVideoPanel)
        self.videoFileName.setCursor(AppData.cursors['text'])
        vpapag.addWidget(self.videoFileName, 1,4,1,5)

        videoBrowse = QPushButton('Browse', self.outputVideoPanel)
        videoBrowse.clicked.connect(self.selectfile)
        vpapag.addWidget(videoBrowse, 1, 10, 1, 1)

        
        

        self.videoResolution = QLabel('Resolution (pixels): ', self.outputVideoPanel)
        vpapag.addWidget(self.videoResolution, 2, 1, 1, 1)

        self.videoWidth = QLabel('Width: ', self.outputVideoPanel)
        vpapag.addWidget(self.videoWidth, 3, 2, 1, 1)
        self.videoWidthValue = QDoubleSpinBox(self.outputVideoPanel)
        self.videoWidthValue.lineEdit().setCursor(AppData.cursors['text'])
        self.videoWidthValue.setDecimals(0)
        self.videoWidthValue.setMinimum(32)
        self.videoWidthValue.setMaximum(2048)
        self.videoWidthValue.setSingleStep(1)
        vpapag.addWidget(self.videoWidthValue,3, 4, 1, 1)

        self.videoHeight = QLabel('Height: ', self.outputVideoPanel)
        vpapag.addWidget(self.videoHeight, 4, 2, 2, 1)
        self.videoHeightValue = QDoubleSpinBox(self.outputVideoPanel)
        self.videoHeightValue.lineEdit().setCursor(AppData.cursors['text'])
        self.videoHeightValue.setDecimals(0)
        self.videoHeightValue.setMinimum(32)
        self.videoHeightValue.setMaximum(2048)
        self.videoHeightValue.setSingleStep(1)
        vpapag.addWidget(self.videoHeightValue, 4, 4, 2, 1)

        videoButton = QPushButton('Export Video', self.outputVideoPanel)
        videoButton.clicked.connect(self.exportVideo)
        vpapag.addWidget(videoButton, 10, 4, 2, 8)

        
        self.outputVideoPanel.setLayout(vpapag)


        self.outputStream = QTextEdit()
        self.outputStream .setCursor(AppData.cursors['text'])
        self.outputStream.setReadOnly(True)
        self.outputStream.setUndoRedoEnabled(False)
        self.outputStream.setContextMenuPolicy(Qt.NoContextMenu)
        self.outputStream.acceptRichText()
        
        self.optionsTabber.addTab(self.managePanel, 'Randoms')
        self.optionsTabber.addTab(self.spatialPanel , 'Spatial')
        self.optionsTabber.addTab(self.temporalPanel, 'Temporal')
        self.optionsTabber.addTab(self.calibrationPanel, 'Tone mapping')
        self.optionsTabber.addTab(self.outputStream, 'Report')
        self.optionsTabber.addTab(self.outputVideoPanel, 'Video')
        grid.addWidget(self.optionsTabber, 3, 1, 4, 7)

        self.sequenceEditButton = QPushButton('\nEdi&t\n', self.sequenceTimelineBox)
        self.sequenceEditButton.clicked.connect(self.openIde)
        grid.addWidget(self.sequenceEditButton, 3, 8, 1, 1)

        qbtn = QPushButton('\n&Back\n', self)
        qbtn.clicked.connect(self.stop)
        grid.addWidget(qbtn, 4, 8, 1, 1)
        #
        #btn = QPushButton('Save randoms', self)
        ##btn.clicked.connect(self.saveRandoms())
        #grid.addWidget(btn, 6, 0, 1, 2)
        #
        #cbtn = QPushButton('Calibrate', self)
        ##cbtn.clicked.connect(self.saveRandoms())
        #grid.addWidget(cbtn, 7, 0, 1, 2)

        hintLabel = QLabel('''
                <p>During sequence:</p>
                <p><b>Q</b> or <b>Esc</b> : abort</p>
                <p><b>Space</b> : show stats/controls</p>
                <p><b>&lt;-</b> / <b>-&gt;</b> : back/skip</p>
                <p><b>P</b> : pause </p>
                ''')
        hintLabel.setTextFormat( Qt.RichText )
        grid.addWidget(hintLabel, 5, 8, 1, 1)

        lbtn = QPushButton('\n\n&Execute\n\n', self)
        lbtn.clicked.connect(self.launch)
        grid.addWidget(lbtn, 6, 8, 1, 1)

        self.setLayout(grid)

        self.setWindowTitle('Gears')
#        self.setWindowIcon(QIcon('web.png'))

    def keyPressEvent(self, e):
        if e.key() == Qt.Key_Escape or e.text() == 'q' or e.text() == 'b':
            self.stop()
        if e.key() == Qt.Key_Enter or e.text() == 'e':
            self.launch()

    def initSequence(self, browserWindow, path):
        self.path = path
        self.browserWindow = browserWindow
        self.timer = QTimer(self)
        self.timer.setInterval(100)
        self.timer.timeout.connect(self.onTimer)
        self.timer.start()

        if self.calibrating :
            stim = gears.getSelectedStimulus().getPythonObject()
            print('''
    stim.setMeasuredDynamics( {dmin}, {dmax}, {dmean}, {dvar}, {histogramList})
                '''.format(
                        dmin  = stim.measuredToneRangeMin,
                        dmax  = stim.measuredToneRangeMax,
                        dmean = stim.measuredToneRangeMean,
                        dvar  = stim.measuredToneRangeVar,
                       histogramList = stim.getMeasuredHistogramAsPythonList() )
                , file=self.calibrationSettingsFile)
            self.calibrationSettingsFile.close()
            self.stimulusChanged()
            self.calibrating = False
        
    def start(self, browserWindow, path):
        self.exp = gears.getSequence().getPythonObject()
        gears.pickStimulus( 0, 10 )
        self.sequencePath = path
        self.initSequence(browserWindow, path)

        self.stimulusChanged()

        self.sequenceTimelineBox.setTitle(self.exp.name) # + '   ' + path)
        if(self.exp.exportRandomsWithHashmarkComments) :
            self.ecrHashmark.setChecked(True)
        else :
            self.ecrNone.setChecked(True)
        if(self.exp.exportRandomsChannelCount < 2) :
            self.echr1.setChecked(True)
        if(self.exp.exportRandomsChannelCount == 2) :
            self.echr2.setChecked(True)
        if(self.exp.exportRandomsChannelCount == 3) :
            self.echr3.setChecked(True)
        if(self.exp.exportRandomsChannelCount > 3) :
            self.echr4.setChecked(True)

        if(self.exp.exportRandomsAsReal) :
            self.efrReal.setChecked(True)
        elif(self.exp.exportRandomsAsBinary) :
            self.efrBinary.setChecked(True)
        else :
            self.efrInteger.setChecked(True)

        if self.exp.usesRandoms() :
            self.optionsTabber.addTab( self.managePanel, 'Randoms' )
        else :
            self.optionsTabber.removeTab( self.optionsTabber.indexOf(self.managePanel) )

        self.optionsTabber.setCurrentIndex( self.optionsTabber.indexOf(self.outputStream) )

        self.videoHeightValue.setValue(self.exp.field_height_px)
        self.videoWidthValue.setValue(self.exp.field_width_px)

        pathInVideos = self.path.replace('Sequences', 'OutputVideos', 1)
        videoFilePath = pathInVideos.replace('.pyx', '.mpg', 1)


        self.videoFileName.setText(videoFilePath)              

        gammaTypes = []
        stimuli = self.exp.getStimuli()
        for stim in stimuli:
            stimo = stim.data().getPythonObject()
            if hasattr(stimo, 'gammaLabel') :
                gname = stimo.gammaLabel
                if gname not in gammaTypes :
                    gammaTypes += [gname]
            else :
                gammaTypes += ['N/A']

        #a = ''
        #for i in dir(self.exp) :
        #   if not i.startswith('__') :
        #       a += i
        self.sequenceLabel.setText(
            'Gamma: {gammaType},   Device refresh rate: {devfps:.2f},    Sequence frame rate: {fps:.2f},    Field res.: {w}x{h}'.format( 
                    gammaType=str(gammaTypes),
                    devfps=self.exp.deviceFrameRate,
                    fps=self.exp.deviceFrameRate/self.exp.frameRateDivisor,
                    w=self.exp.field_width_px,
                    h=self.exp.field_height_px,
                    ))

        totalExpDuration = self.exp.getDuration()
        minStimulusDuration = self.exp.getShortestStimulusDuration()
        self.sequenceTimelineScrollbar.setMinimum(0)
        self.sequenceTimelineScrollbar.setMaximum(0)
        self.sequenceTimelineScrollbar.setValue(0)
        self.sequenceTimelineScrollbar.setPageStep( totalExpDuration )

        self.sequenceTimelineZoomSlider.setMinimum(  min(totalExpDuration, 60 ) ) # minStimulusDuration * 40) )
        self.sequenceTimelineZoomSlider.setMaximum( totalExpDuration )
        self.sequenceTimelineZoomSlider.setValue( totalExpDuration ) #min(totalExpDuration, minStimulusDuration * 600) )

        self.stimulusTimelineScrollbar.setMinimum(0)
        self.stimulusTimelineScrollbar.setMaximum(0)
        self.stimulusTimelineScrollbar.setValue(0)
        self.stimulusTimelineScrollbar.setPageStep( minStimulusDuration )

        self.stimulusTimelineZoom = minStimulusDuration
        self.stimulusTimelineZoomSlider.setMinimum( 1 )
        self.stimulusTimelineZoomSlider.setMaximum( minStimulusDuration )
        self.stimulusTimelineZoomSlider.setValue( minStimulusDuration )

        #self.outputStream.clear()
        self.outputStream.cursor().setPos(QTextCursor.End, QTextCursor.MoveAnchor)
        self.outputStream.insertHtml('Sequence open.<BR>')

        self.update()
        self.showFullScreen()

    def wake(self):
        nSkipped = gears.getSkippedFrameCount()
        self.outputStream.cursor().setPos(QTextCursor.End, QTextCursor.MoveAnchor)
        self.outputStream.insertHtml('Sequence finished!<BR>')
        ILog.log.close()
        #TODO: report skips
        #if(nSkipped > 1 and nSkipped < 9999999999):
        #    self.warn('WARNING! {n} frames have been skipped!'.format(n=nSkipped))
        #    if(nSkipped > 15):
        #        self.outputStream.insertHtml('Revise performance-critial settings!<BR>')
        ##else :
        ##    self.outputStream.insertHtml('No frames skipped.<BR>')
        self.outputStream.insertHtml( gears.getSequenceTimingReport() )
        self.initSequence(self.browserWindow, self.path)
        self.showFullScreen()

    def sleep(self):
        pass
        #QApplication.instance().processEvents()
        #self.hide()
        #self.lower()

    def stop(self):
        self.timer.stop()
        sequence = gears.getSequence().getPythonObject()
        sequence.close()
        self.browserWindow.showFullScreen()
        QApplication.instance().processEvents()
        time.sleep(1)
        self.hide()
        self.lower()

    def onTimer(self):
        self.sequenceTimeline.update()
        self.stimulusTimeline.update()
        self.temporalKernelPlot.update()
        self.spatialKernelPlot.update()
        self.spatialProfilePlot.update()

    def launch(self):
        #print("Launch initiated.")
        #print(datetime.datetime.now().time())
        sequence = gears.getSequence()
        timestr = datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
        pathInLogs = self.path.replace('Sequences', 'Logs', 1)
        logFilePath = pathInLogs.replace('.pyx', '_' + timestr + '.log', 1)
        gears.makePath(logFilePath)
        log = open( logFilePath, 'w')
        ilogFilePath = pathInLogs.replace('.pyx', '_' + timestr + '_interactions.log', 1)
        ILog.log.open(ilogFilePath)
        print('#Sequence launch at ' + timestr, file=log) 
        print('----------------------EXPERIMENT-----------------------', file=log )
        pex = sequence.getPythonObject()
        print( type(pex).__name__ , file=log )
        print( " args = " + str(pex.args), file=log )
        if pex.verboseLogging :
            for property in dir(pex):
                member = getattr(pex, property)
                if not callable(member) and not property.startswith('__') :
                    print( property + " = " + str(member), file=log )
        stimuli = sequence.getStimuli()
        for stimulus in stimuli :
            sex = stimulus.data().getPythonObject()
            print('----------------------STIMULUS-----------------------', file=log )
            print( type(sex).__name__ , file=log )
            print( " args = " + str(sex.args), file=log )
            if pex.verboseLogging :
                for property in dir(sex):
                    member = getattr(sex, property)
                    if not callable(member) and not property.startswith('__') :
                        print( property + " = " + str(member), file=log )

        #print("Logs printed.")
        #print(datetime.datetime.now().time())

        self.outputStream.cursor().setPos(QTextCursor.End, QTextCursor.MoveAnchor)
        self.outputStream.insertHtml('Launching sequence! (log:' + logFilePath + ')<BR>')

        #TODO: format log as .pyx, insert signal info
        self.timer.stop()

        #print("Report printed. Timer stopped.")
        #print(datetime.datetime.now().time())

        with warnings.catch_warnings(record=True) as w:
            # Cause all warnings to always be triggered.
            warnings.simplefilter("always")
            gears.run()
            for e in w:
                self.warn( e.message )
        #self.mediaWindow.start(self)
        self.sleep()

    def launchCalibration(self):
        sequence = gears.getSequence()
        stim = gears.getSelectedStimulus().getPythonObject()
        gears.enableCalibration(stim.getStartingFrame(), stim.getDuration(), self.calibHistoMinValue.value(), self.calibHistoMaxValue.value())
        timestr = datetime.datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
        pathInCalibs = self.path.replace('Sequences', 'Tonemapping', 1)
        calibFilePath = pathInCalibs.replace('.pyx', '_@frame' + str(stim.getStartingFrame()) + '_measurement.py', 1)
        gears.makePath(calibFilePath)
        self.calibrationSettingsFile = open( calibFilePath, 'w')
        print('#Tone mapping set at ' + timestr, file=self.calibrationSettingsFile) 
        print('def apply(sequence) :', file=self.calibrationSettingsFile) 
        print('    stim = sequence.getStimulusAtFrame({startingFrame})'.format(startingFrame = stim.getStartingFrame()), file=self.calibrationSettingsFile) 
        self.calibrating = True

        self.timer.stop()
        gears.run()
        self.sleep()

    def exportVideo(self):
        
        gears.makePath(self.videoFileName.text())
        gears.enableVideoExport(self.videoFileName.text(), 60, int(self.videoWidthValue.value()), int(self.videoHeightValue.value()) )

        self.timer.stop()
        gears.run()
        self.sleep()

    def selectfile(self):
        
        videoFileName = str(QFileDialog.getSaveFileName(self, 'Save file',self.videoFileName.text(), filter ='*.mpg'))
        self.videoFileName.setText(videoFileName[2:-11] + videoFileName[-6:-2])


    def exportRandoms(self):
        gears.enableExport(self.path.replace('Sequences', 'Randoms', 1))
        self.timer.stop()
        gears.run()
        #self.mediaWindow.start(self)
        self.sleep()

    def setSpatialPlotMin(self, value):
        self.spatialPlotMin = value

    def setSpatialPlotMax(self, value):
        self.spatialPlotMax = value

    def setspatialPlotSize(self, value):
        self.spatialPlotSize = value
        #if(self.spatialPlotSize != aspect * self.spatialPlotHeight):
        #    self.heightSpinBox.setValue(value / aspect)

    def stimulusChanged(self):
        stim = gears.getSelectedStimulus().getPythonObject()

        if stim.doesErfToneMapping() :
            self.toneMappingRadioErf.setChecked(True)
            self.calibToneMinValue.setDisabled(True)
            self.calibToneMaxValue.setDisabled(True)
            self.calibToneMeanValue.setDisabled(False)
            self.calibToneVarValue.setDisabled(False)
            self.calibToneMinLabel.setDisabled(True)
            self.calibToneMaxLabel.setDisabled(True)
            self.calibToneMeanLabel.setDisabled(False)
            self.calibToneVarLabel.setDisabled(False)
        elif stim.toneRangeMin == 0 and stim.toneRangeMax == 1 :
            self.toneMappingRadioNone.setChecked(True)
            self.calibToneMinValue.setDisabled(True)
            self.calibToneMaxValue.setDisabled(True)
            self.calibToneMeanValue.setDisabled(True)
            self.calibToneVarValue.setDisabled(True)
            self.calibToneMinLabel.setDisabled(True)
            self.calibToneMaxLabel.setDisabled(True)
            self.calibToneMeanLabel.setDisabled(True)
            self.calibToneVarLabel.setDisabled(True)
        else :
            self.toneMappingRadioLinear.setChecked(True)
            self.calibToneMinValue.setDisabled(False)
            self.calibToneMaxValue.setDisabled(False)
            self.calibToneMeanValue.setDisabled(True)
            self.calibToneVarValue.setDisabled(True)
            self.calibToneMinLabel.setDisabled(False)
            self.calibToneMaxLabel.setDisabled(False)
            self.calibToneMeanLabel.setDisabled(True)
            self.calibToneVarLabel.setDisabled(True)


        self.calibToneMinValue. setValue( stim.toneRangeMin )
        self.calibToneMaxValue. setValue( stim.toneRangeMax )
        self.calibToneMeanValue.setValue( stim.toneRangeMean)
        self.calibToneVarValue. setValue( stim.toneRangeVar )

        self.calibMeasuredMinValue.setText(  "{v:.2f}".format(v=stim.measuredToneRangeMin ));
        self.calibMeasuredMaxValue.setText(  "{v:.2f}".format(v=stim.measuredToneRangeMax ));
        self.calibMeasuredMeanValue.setText( "{v:.2f}".format(v=stim.measuredToneRangeMean));
        self.calibMeasuredVarValue.setText(  "{v:.2f}".format(v=stim.measuredToneRangeVar ));

        spatialPlotMin = stim.getSpatialPlotMin()
        spatialPlotMax = stim.getSpatialPlotMax()
        spatialPlotSize = stim.getSpatialPlotWidth()
        self.aspect = 1
        self.minSpinBox.setMinimum(spatialPlotMin - (spatialPlotMax - spatialPlotMin) * 10)
        self.minSpinBox.setMaximum(spatialPlotMax + (spatialPlotMax - spatialPlotMin) * 10)
        self.minSpinBox.setSingleStep( (spatialPlotMax - spatialPlotMin) / 10 )
        self.minSpinBox.setValue(spatialPlotMin)
        
        self.maxSpinBox.setMinimum(spatialPlotMin - (spatialPlotMax - spatialPlotMin) * 10)
        self.maxSpinBox.setMaximum(spatialPlotMax + (spatialPlotMax - spatialPlotMin) * 10)
        self.maxSpinBox.setSingleStep( (spatialPlotMax - spatialPlotMin) / 10 )
        self.maxSpinBox.setValue(spatialPlotMax)
        
        self.sizeSpinBox.setMinimum(spatialPlotSize * 0.01)
        self.sizeSpinBox.setMaximum(spatialPlotSize * 100.0)
        self.sizeSpinBox.setValue(spatialPlotSize)
        self.sizeSpinBox.setSingleStep(spatialPlotSize / 10)

        self.spatialPlotMin = spatialPlotMin
        self.spatialPlotMax = spatialPlotMax
        self.spatialPlotSize = spatialPlotSize

        if stim.hasSpatialFiltering() :
            self.optionsTabber.addTab( self.spatialPanel, 'Spatial' )
        else :
            self.optionsTabber.removeTab( self.optionsTabber.indexOf(self.spatialPanel) )

        if stim.hasTemporalFiltering() :
            self.optionsTabber.addTab( self.temporalPanel, 'Temporal' )
        else :
            self.optionsTabber.removeTab( self.optionsTabber.indexOf(self.temporalPanel) )

        self.stimulusLabel.setText(
            stim.name + ' : ' + type(stim).__name__ + str(stim.args) 
            #+ ' ' 
            #+ 'Gamma: {gammaType},   Device refresh rate: {devfps:.2f},    Sequence frame rate: {fps:.2f},    Field res.: {w}x{h}'.format(
            #'{name} duration: {dur} frames, {dursec:.2f} s,  gamma: {gamma}'.format(
            #        name=stim.name, dur=stim.duration, dursec=stim.getDuration_s(), gamma=stim.gammaLabel) 
            )
        self.stimulusLabel.setWordWrap(True)
        #self.stimulusLabel.setToolTip( '<div style=\"background-color:black;\">' + self.stimulusLabel.text() + '</div>' )

        self.stimulusTimelineScrollbar.setMinimum(0)
        self.stimulusTimelineScrollbar.setMaximum(0)
        self.stimulusTimelineScrollbar.setValue(0)
        self.stimulusTimelineScrollbar.setPageStep( stim.duration )
        self.stimulusTimelineZoomSlider.setMinimum(  1 )
        self.stimulusTimelineZoomSlider.setMaximum( stim.duration )
        self.stimulusTimelineZoomSlider.setValue( stim.duration )

        self.update()

    def onZoomSlider(self, value) :
        totalExpDuration = self.exp.getDuration()
        if self.midFrame == None :
            self.midFrame = self.sequenceTimeline.sequenceTimelineStartFrame + self.sequenceTimelineZoom // 2
            self.mousePos = 0.5
        self.sequenceTimelineZoom = value
        self.sequenceTimelineScrollbar.setMaximum(totalExpDuration-self.sequenceTimelineZoom)
        self.sequenceTimelineScrollbar.setPageStep( self.sequenceTimelineZoom )
        self.sequenceTimelineScrollbar.setValue( self.midFrame - value * self.mousePos )
        self.sequenceTimelineScrollbar.update()
        gears.setSequenceTimelineZoom(value)
        self.sequenceTimeline.sequenceTimelineZoom = value
        self.midFrame = None

    def onScroll(self, value):
        #value = self.sequenceTimelineZoomSlider.maximum() - value + self.sequenceTimelineZoomSlider.minimum()
        gears.setSequenceTimelineStart(value)
        self.sequenceTimeline.sequenceTimelineStartFrame = value

    def onStimZoomSlider(self, value) :
        stim = gears.getSelectedStimulus().getPythonObject()
        if self.midFrame == None :
            self.midFrame = self.stimulusTimeline.stimulusTimelineStartFrame + self.stimulusTimelineZoom // 2
            self.mousePos = 0.5
        self.stimulusTimelineZoom = value
        self.stimulusTimelineScrollbar.setMaximum(stim.duration - self.stimulusTimelineZoom)
        self.stimulusTimelineScrollbar.setPageStep( self.stimulusTimelineZoom )
        self.stimulusTimelineScrollbar.setValue( self.midFrame - value * self.mousePos )
        self.stimulusTimelineScrollbar.update()
        gears.setStimulusTimelineZoom(value)
        self.stimulusTimeline.stimulusTimelineZoom = value
        self.midFrame = None

    def onStimScroll(self, value):
        #value = self.stimulusTimelineZoomSlider.maximum() - value + self.stimulusTimelineZoomSlider.minimum()
        #print(value)
        gears.setStimulusTimelineStart(value)
        self.stimulusTimeline.stimulusTimelineStartFrame = value

    def zoom(self, e):
        self.mousePos = e.x() / self.width()
        self.midFrame = self.mousePos * self.sequenceTimelineZoom + self.sequenceTimeline.sequenceTimelineStartFrame
        self.sequenceTimelineZoomSlider.setValue( 
                self.sequenceTimelineZoom 
                - e.angleDelta().y() * ( self.sequenceTimelineZoomSlider.maximum() - self.sequenceTimelineZoomSlider.minimum() ) // 1920 )
        
    def pan(self, d):
        self.sequenceTimelineScrollbar.setValue( self.sequenceTimeline.sequenceTimelineStartFrame - d / self.width() * self.sequenceTimelineZoom )

    def stimZoom(self, e):
        self.mousePos = e.x() / self.width()
        self.midFrame = self.mousePos * self.stimulusTimelineZoom + self.stimulusTimeline.stimulusTimelineStartFrame
        self.stimulusTimelineZoomSlider.setValue( 
                self.stimulusTimelineZoom 
                - e.angleDelta().y() * ( self.stimulusTimelineZoomSlider.maximum() - self.stimulusTimelineZoomSlider.minimum() ) // 1920 )
        
    def stimPan(self, d):
        self.stimulusTimelineScrollbar.setValue( self.stimulusTimeline.stimulusTimelineStartFrame - d / self.width() * self.stimulusTimelineZoom )

    def applyMeasuredToneMin(self, d):
        stim = gears.getSelectedStimulus().getPythonObject()
        stim.toneRangeMin = stim.measuredToneRangeMin 
        self.calibToneMinValue.setValue(stim.toneRangeMin)
        self.update()

    def applyMeasuredToneMax(self, d):
        stim = gears.getSelectedStimulus().getPythonObject()
        stim.toneRangeMax = stim.measuredToneRangeMax 
        self.calibToneMaxValue.setValue(stim.toneRangeMax)
        self.update()

    def applyMeasuredToneMean(self, d):
        stim = gears.getSelectedStimulus().getPythonObject()
        stim.toneRangeMean = stim.measuredToneRangeMean 
        self.calibToneMeanValue.setValue(stim.toneRangeMean)
        self.update()


    def applyMeasuredToneVar(self, d):
        stim = gears.getSelectedStimulus().getPythonObject()
        stim.toneRangeVar = stim.measuredToneRangeVar 
        self.calibToneVarValue.setValue(stim.toneRangeVar)
        self.update()
    
    def exportRandomChanged(self):
        if self.ecrHashmark.isChecked() :
            self.exp.exportRandomsWithHashmarkComments = True
        elif self.ecrNone.isChecked():
            self.exp.exportRandomsWithHashmarkComments = False

        if self.echr1.isChecked():
            self.exp.exportRandomsChannelCount = 1
        elif self.echr2.isChecked():
            self.exp.exportRandomsChannelCount = 2
        elif self.echr3.isChecked():
            self.exp.exportRandomsChannelCount = 3
        elif self.echr4.isChecked():
            self.exp.exportRandomsChannelCount = 4

        if self.efrReal.isChecked():
            self.exp.exportRandomsAsReal = True
            self.exp.exportRandomsAsBinary = False
            self.exp.exportRandomsAsInteger = False
        elif self.efrBinary.isChecked():
            self.exp.exportRandomsAsReal = False
            self.exp.exportRandomsAsBinary = True
            self.exp.exportRandomsAsInteger = False
        elif self.efrInteger.isChecked():
            self.exp.exportRandomsAsReal = False
            self.exp.exportRandomsAsBinary = False
            self.exp.exportRandomsAsInteger = True

        self.update()

    def toneMappingChanged(self, e) :
        if self.toneMappingRadioErf.isChecked() :
            self.calibToneMinValue.setDisabled(True)
            self.calibToneMaxValue.setDisabled(True)
            self.calibToneMeanValue.setDisabled(False)
            self.calibToneVarValue.setDisabled(False)
            self.calibToneMinLabel.setDisabled(True)
            self.calibToneMaxLabel.setDisabled(True)
            self.calibToneMeanLabel.setDisabled(False)
            self.calibToneVarLabel.setDisabled(False)

        elif self.toneMappingRadioNone.isChecked() :
            self.calibToneMinValue.setDisabled(True)
            self.calibToneMaxValue.setDisabled(True)
            self.calibToneMeanValue.setDisabled(True)
            self.calibToneVarValue.setDisabled(True)
            self.calibToneMinLabel.setDisabled(True)
            self.calibToneMaxLabel.setDisabled(True)
            self.calibToneMeanLabel.setDisabled(True)
            self.calibToneVarLabel.setDisabled(True)

        elif self.toneMappingRadioEq.isChecked() :
            self.calibToneMinValue.setDisabled(True)
            self.calibToneMaxValue.setDisabled(True)
            self.calibToneMeanValue.setDisabled(True)
            self.calibToneVarValue.setDisabled(True)
            self.calibToneMinLabel.setDisabled(True)
            self.calibToneMaxLabel.setDisabled(True)
            self.calibToneMeanLabel.setDisabled(True)
            self.calibToneVarLabel.setDisabled(True)

        else :
            self.toneMappingRadioLinear.setChecked(True)
            self.calibToneMinValue.setDisabled(False)
            self.calibToneMaxValue.setDisabled(False)
            self.calibToneMeanValue.setDisabled(True)
            self.calibToneVarValue.setDisabled(True)
            self.calibToneMinLabel.setDisabled(False)
            self.calibToneMaxLabel.setDisabled(False)
            self.calibToneMeanLabel.setDisabled(True)
            self.calibToneVarLabel.setDisabled(True)

        self.update()

    def saveToneMapSettings(self) :
        stim = gears.getSelectedStimulus().getPythonObject()
        pathInCalibs = self.path.replace('Sequences', 'Tonemapping', 1)
        calibFilePath = pathInCalibs.replace('.pyx', '_@frame' + str(stim.getStartingFrame()) + '_tonemap.py', 1)
        gears.makePath(calibFilePath)
        toneMapSettingsFile = open( calibFilePath, 'w')
        print('def apply(sequence) :', file=toneMapSettingsFile) 
        print('    stim = sequence.getStimulusAtFrame({startingFrame})'.format(startingFrame = stim.getStartingFrame()), file=toneMapSettingsFile) 
        if self.toneMappingRadioErf.isChecked() :
            print('    stim.setToneMappingErf({dmean},{dvar}, False)'.format(dmean=self.calibToneMeanValue.value(), dvar=self.calibToneVarValue.value()), file=toneMapSettingsFile) 
            stim.setToneMappingErf(self.calibToneMeanValue.value(), self.calibToneVarValue.value(), False)
        elif self.toneMappingRadioNone.isChecked() :
            print('    pass', file=toneMapSettingsFile) 
            stim.setToneMappingLinear(0, 1, False)
        elif self.toneMappingRadioEq.isChecked() :
            print('    stim.setToneMappingEqualized(False)', file=toneMapSettingsFile) 
            stim.setToneMappingEqualized(False)
        else :
            print('    stim.setToneMappingLinear({dmin},{dmax}, False)'.format(dmin=self.calibToneMinValue.value(), dmax=self.calibToneMaxValue.value()), file=toneMapSettingsFile) 
            stim.setToneMappingLinear(self.calibToneMinValue.value(), self.calibToneMaxValue.value(), False)
        toneMapSettingsFile.close()
        timestr = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
        self.saveToneMapSettingsButton.setText('Save (last saved ' + timestr + ' )')

    def warn(self, message):
        self.outputStream.cursor().setPos(QTextCursor.End, QTextCursor.MoveAnchor)
        self.outputStream.insertHtml('<table bgcolor=#ff0000><td bgcolor=#ff0000><font size=20 color=#000000>{message}</font></td></table>'.format(
                message = message)
                                     )

    def openIde(self):
        self.ide = Ide(self.sequencePath, self.browserWindow.tree)
        self.ide.show()
        #self.editor.setText('code')