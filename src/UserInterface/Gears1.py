import sys
import importlib.machinery
import os
import AppData

AppData.appDataDir = os.getcwd()

import Gears as gears
import atexit
import traceback
from PyQt5.QtCore import (Qt, QCoreApplication, QUrl)
from PyQt5.QtGui import (QSurfaceFormat, QCursor, QPixmap)
from PyQt5.QtWidgets import (QApplication, QMessageBox, QSpacerItem, QSizePolicy)
from PyQt5.QtMultimediaWidgets import QVideoWidget
from PyQt5.QtMultimedia import (QMediaPlayer, QMediaPlaylist)
from PyQt5.QtNetwork import QAuthenticator

#from StimulusWindow import StimulusWindow
#from MediaWindow import MediaWindow
from LauncherWindow import LauncherWindow
from BrowserWindow import BrowserWindow

try:
    #Qt.AA_ShareOpenGLContexts = True
    app = QApplication(sys.argv)
    app.setKeyboardInputInterval(0)

    AppData.initConfigParams()

    AppData.cursors['arrow'] = QCursor(QPixmap("Gui/redpointer.png"), 0, 0)
    AppData.cursors['text'] = QCursor(QPixmap("Gui/redtext.png"), 0, 0)

    #glFormat = QSurfaceFormat()
    #glFormat.setMajorVersion(4)
    #glFormat.setMinorVersion(4)
    #QSurfaceFormat.setDefaultFormat( glFormat )

#    videoWidget = None
#    mediaPlayer= QMediaPlayer(None, QMediaPlayer.VideoSurface)

#    stimulusWindow = StimulusWindow()
#    mediaWindow = MediaWindow(stimulusWindow)
    gears.createStimulusWindow()
    
    launcherWindow = LauncherWindow()
    browserWindow = BrowserWindow(launcherWindow, app)
    
    hwnd = browserWindow.winId()
    print (hwnd.__int__ ())

    gears.CreateSurface (hwnd.__int__ ())

    atexit.register(gears.cleanup)
    sys.exit(app.exec_())
except SystemExit:
    pass
except:
    exc_type, exc_value, exc_traceback = sys.exc_info()
    formatted_lines = traceback.format_exc().splitlines()
    box = QMessageBox(None)
    box.setCursor(AppData.cursors['arrow'])
    box.setStyleSheet("""
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
            """)
    horizontalSpacer = QSpacerItem(1000, 0, QSizePolicy.Minimum, QSizePolicy.Expanding)
    message = ''
    for l in formatted_lines :
        message += l + '\n'
    box.setText(message)
    box.setWindowTitle('Python error!')
    box.setWindowFlags(Qt.Dialog)
    box.setStandardButtons(QMessageBox.Abort)
    box.setDefaultButton(QMessageBox.Abort)
    layout = box.layout()
    layout.addItem(horizontalSpacer, layout.rowCount(), 0, 1, layout.columnCount())
    box.exec()

