import sys
import os
import Gears as gears
import subprocess
from platform import system

from PyQt5.QtCore import (Qt, QCoreApplication, QTimer, QSize)
from PyQt5.QtWidgets import (QWidget, QMessageBox, QApplication, QTreeWidget, QTreeWidgetItem, QTreeWidgetItemIterator, QGridLayout, QLabel, QSpacerItem, QSizePolicy, QMenu, QAction)
from PyQt5.QtGui import (QFont, QPalette )
from SequenceLoader import *
from Ide import Ide
from ConfigWindow import ConfigWindow
import AppData

class BrowserTree(QTreeWidget):
    launcherWindow = None
    browserWindow = None

    def __init__(self, launcherWindow, browserWindow):
        super().__init__()
        self.rootPath = "./Project"
        self.launcherWindow = launcherWindow
        self.browserWindow = browserWindow
        self.itemClicked.connect(self.onClick)
        self.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed )
        self.setHorizontalScrollBarPolicy(Qt.ScrollBarAlwaysOff)
        self.setContextMenuPolicy(Qt.CustomContextMenu)
        self.customContextMenuRequested.connect(self.onContext)
        self.itemCollapsed.connect(self.onCollapse)
        self.itemExpanded.connect(self.onExpand)
        self.isHidingEmptyFolders = True
        self.setColumnCount(2)
        self.setColumnHidden(1, True)

    def pathFromItem(self, item):
        fullpath = ''
        p = item
        while p:
            fullpath = '/' + p.text(0) + fullpath
            p = p.parent()
        fullpath = self.rootPath + fullpath
        return fullpath


    def open(self, item, openIde):
        if(item.text(0).endswith('.pyx')):
            fullpath = self.pathFromItem(item)

            if loadSequence(fullpath, self, openIde):
                self.launcherWindow.start(self.browserWindow, fullpath)
                QApplication.instance().processEvents()
                self.browserWindow.hide()
        else:
            if item.isExpanded() :
                self.collapseItem(item)
            else :
                self.expandItem(item)

    def onExpand(self, item):
        self.resizeColumnToContents(0)
        self.updateGeometry()

    def onCollapse(self, item):
        self.resizeColumnToContents(0)
        self.updateGeometry()

    def onClick(self, item, int):
        self.open(item, False)

#    def sizeHintForColumn(self, iCol):
#        width = 0
#        itemit = QTreeWidgetItemIterator(self, QTreeWidgetItemIterator.NotHidden)
#        while itemit.value() :
#            rect = self.visualItemRect( itemit.value() )
#            width = max(width, rect.right())
#            itemit += 1
#        return width

    def sizeHint(self):
        height = self.header().height() + 2
        itemit = QTreeWidgetItemIterator(self, QTreeWidgetItemIterator.NotHidden)
        while itemit.value() :
            rect = self.visualItemRect( itemit.value() )
            height += rect.height()
            itemit += 1
        width = self.columnWidth(0) 
        scrollb = self.scrollBarWidgets(Qt.AlignRight)
        if scrollb :
            width += scrollb[0].size().width()
        return QSize(width, height) 

    def onContext(self, point):
        globalPos = self.mapToGlobal(point)
        item = self.itemAt(point)
        if(item.text(0).endswith('.pyx')):
            menu = QMenu(self)
            menu.setCursor(AppData.cursors['arrow'])
            a = QAction("Open sequence", menu)
            a.triggered.connect( lambda action: self.open(item, False) )
            menu.addAction(a)

            a = QAction("Edit script", menu)
            a.triggered.connect( lambda action: self.open(item, True) )
            menu.addAction(a)

            a = QAction("Explore in folder", menu)
            a.triggered.connect( lambda action: subprocess.Popen(r'explorer /select,"' + os.path.abspath(self.pathFromItem(item)) + '"'))
            menu.addAction(a)

            action = menu.exec(globalPos)
        else:
            menu = QMenu(self)
            menu.setCursor(AppData.cursors['arrow'])
            a = QAction("Configure", menu)
            a.triggered.connect( lambda action: self.configure(item))
            menu.addAction(a)

            #if self.isHidingEmptyFolders :
            #    a = QAction("Show empty folders", menu)
            #    a.triggered.connect( lambda action: self.hideEmptyFolders(False))
            #    menu.addAction(a)
            #else:
            #    a = QAction("Hide empty folders", menu)
            #    a.triggered.connect( lambda action: self.hideEmptyFolders(True))
            #    menu.addAction(a)
            if system() == 'Windows':
                a = QAction("Explore folder", menu)
                a.triggered.connect( lambda action: subprocess.Popen(r'explorer "' + os.path.abspath(self.pathFromItem(item)) + '"'))
                menu.addAction(a)

            action = menu.exec(globalPos)

    def hideEmptyFolders(self, shide):
        self.isHidingEmptyFolders = shide
        emptyFolderItems = self.findItems("gears_folder_not_holding_any_sequence_scripts", Qt.MatchContains | Qt.MatchRecursive, 1)
        for item in emptyFolderItems:
            item.setHidden(shide)

    def configure(self, item):
        path = self.pathFromItem(item)
        AppData.initConfigParams()
        loadParents(path, self.rootPath)
        configDialog = ConfigWindow(os.path.normpath(path))
        configDialog.exec()

