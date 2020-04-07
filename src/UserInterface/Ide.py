from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import QApplication, QLabel, QDialog, QWidget, QGridLayout, QPushButton, QSplitter, QHBoxLayout, QSlider
from PyQt5.Qsci import QsciScintilla, QsciScintillaBase, QsciLexerPython, QsciAPIs

from Editor import Editor
from Preview import Preview
from SequenceLoader import *
from PolymaskGenerator.PolymaskGeneratorWindow import *

class Ide(QWidget):

    def __init__(self, sequencePath, browser, errline=0, parent=None):
        super().__init__(parent)
        self.sequencePath = sequencePath
        self.browser = browser
        self.polyMaskGenWnd = PolymaskGeneratorWindow()

        self.playSpeed = 1

        hbox = QHBoxLayout(self)

        self.splitter = QSplitter()
        self.editor = Editor(sequencePath, errline, None)
        self.splitter.addWidget(self.editor)

        self.rpanel = QWidget(None)
        grid = QGridLayout()

        self.reloadButton = QPushButton('Save script and load sequence', self.rpanel)
        self.reloadButton.clicked.connect(self.reload)
        grid.addWidget(self.reloadButton, 1, 2, 1, 8)

        self.discardButton = QPushButton('Discard changes', self.rpanel)
        self.discardButton.clicked.connect(self.discard)
        grid.addWidget(self.discardButton, 5, 2, 1, 8)

        self.preview = Preview(self.rpanel, self.editor, self.winId())
        grid.addWidget(self.preview, 2, 2, 1, 8)

        self.seeker = QSlider(Qt.Horizontal, self.rpanel)
        self.seeker.setTickPosition(QSlider.TicksBelow)
        self.seeker.setMinimum(0)
        seq = gears.getSequence()
        if seq :
            self.seeker.setMaximum(seq.getDuration())
            self.seeker.sliderPressed.connect(self.seekerPressed)
            self.seeker.sliderReleased.connect(self.seekerReleased)
            self.seeker.valueChanged.connect(self.seekerChanged)

        self.seeking = False

        grid.addWidget(self.seeker, 3, 2, 1, 8)

        self.pauseButton = QPushButton('\u275a\u275a', self.rpanel)
        self.pauseButton.clicked.connect( self.pause )
        grid.addWidget(self.pauseButton, 4, 2, 1, 1)

        self.play1button = QPushButton('\u25b6', self.rpanel)
        self.play1button.clicked.connect(self.play1)
        grid.addWidget(self.play1button, 4, 3, 1, 1)
        self.play2button = QPushButton('\u25b6\u25b6', self.rpanel)
        self.play2button.clicked.connect(self.play2)
        grid.addWidget(self.play2button, 4, 4, 1, 1)

   
        self.rpanel.setLayout(grid)
        self.splitter.addWidget(self.rpanel)

        hbox.addWidget(self.splitter)
        self.setLayout(hbox)
        self.setGeometry(100, 100, 1600, 900)

        self.timer = QTimer(self)
        self.timer.setInterval(16)
        self.timer.timeout.connect(self.onTimer)
        self.timer.start()

    def onTimer(self):
        self.seeker.setValue(self.preview.sFrame)
        if not self.seeking :
            self.preview.sFrame += self.playSpeed
        self.preview.update()

    def reload(self, e):
        self.editor.save()
        self.close()
        if loadSequence(self.sequencePath, self.browser, False):
            self.browser.launcherWindow.start(self.browser.browserWindow, self.sequencePath)
            QApplication.instance().processEvents()
            self.browser.browserWindow.hide()

    def discard(self, e):
        self.close()

    def seekerPressed(self):
        self.seeking = True

    def seekerReleased(self):
        self.seeking = False

    def seekerChanged(self):
        self.preview.sFrame = self.seeker.value()

    def pause(self, e):
        self.playSpeed = 0

    def play1(self, e):
        self.playSpeed = 1

    def play2(self, e):
        self.playSpeed = 2

    def openPolyGenWindow(self):
        self.polyMaskGenWnd.show()