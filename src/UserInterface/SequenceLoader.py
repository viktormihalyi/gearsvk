import sys
import traceback
import Gears as gears
import importlib.machinery
import warnings
import os
import glob
import AppData
from SequenceError import SequenceError

from PyQt5.QtCore import (Qt, QCoreApplication, QTimer)
from PyQt5.QtWidgets import (QWidget, QMessageBox, QApplication, QTreeWidget, QTreeWidgetItem, QGridLayout, QLabel, QSpacerItem, QSizePolicy)
from PyQt5.QtGui import (QFont, QPalette )
from SequenceError import SequenceError

def loadParents( fullpath, rootPath ) :
    if os.path.realpath(fullpath) != os.path.realpath(rootPath) :
        loadParents( fullpath + "/..", rootPath )
    for module in os.listdir( fullpath ):
        if module == '__init__.py' or module[-3:] != '.py':
            continue
        loader = importlib.machinery.SourceFileLoader('my_module', fullpath + "/" + module)
        my_module = loader.load_module()
        module_dict = my_module.__dict__
        try:
            to_import = my_module.__all__
        except AttributeError:
            to_import = [name for name in module_dict if not name.startswith('_')]
        globals().update({name: module_dict[name] for name in to_import})


def loadSequence(fullpath, browser, openIde):
    from Ide import Ide
    try:
        with warnings.catch_warnings(record=True) as w:
            # Cause all warnings to always be triggered.
            warnings.simplefilter("always")

            AppData.initConfigParams()
            
            loadParents(os.path.dirname(fullpath), browser.rootPath)

            loader = importlib.machinery.SourceFileLoader("my_module", fullpath)

            my_module = loader.load_module()
            gears.makeCurrent()
            gears.setSequence( my_module.create(None) )

            for e in w:
                browser.launcherWindow.warn( e.message )

    except SequenceError as e:
        if not openIde:
            box = QMessageBox(browser)
            horizontalSpacer = QSpacerItem(1000, 0, QSizePolicy.Minimum, QSizePolicy.Expanding)
            box.setText('Error in sequence file!\n' 
                        + e.tb[-1][0] + '(' + str(e.tb[-1][1]) + '):\n in function "' + e.tb[-1][2] + '":\n ' + e.tb[-1][3] + '\n\n' + str(e)
                        )
            if e.deepertb :
                box.setDetailedText(''.join(traceback.format_list(e.deepertb)))
            box.setWindowTitle('Error in sequence file ' + fullpath + '!')
            box.setWindowFlags(Qt.Dialog);
            box.addButton("Abort sequence", QMessageBox.RejectRole)
            box.addButton("Open script editor", QMessageBox.AcceptRole)
            box.setDefaultButton(QMessageBox.Abort)
            layout = box.layout()
            layout.addItem(horizontalSpacer, layout.rowCount(), 0, 1, layout.columnCount())
            if box.exec() == QMessageBox.RejectRole :
                openIde = True
        if openIde :
            browser.launcherWindow.ide = Ide(fullpath, browser, e.tb[-1][1])
            browser.launcherWindow.ide.show()
        return False
    except:
        if not openIde:
            exc_type, exc_value, exc_traceback = sys.exc_info()
            formatted_lines = traceback.format_exc().splitlines()
            box = QMessageBox(browser)
            horizontalSpacer = QSpacerItem(1000, 0, QSizePolicy.Minimum, QSizePolicy.Expanding)
            message = ''
            for l in formatted_lines :
                message += l + '\n'
            box.setText( message )
            box.setWindowTitle('Sequence error!')
            box.setWindowFlags(Qt.Dialog);
            box.addButton("Abort sequence", QMessageBox.RejectRole)
            box.addButton("Open script editor", QMessageBox.AcceptRole)
            box.setDefaultButton(QMessageBox.Abort)
            layout = box.layout()
            layout.addItem(horizontalSpacer, layout.rowCount(), 0, 1, layout.columnCount())
            if box.exec() == QMessageBox.RejectRole:
                openIde = True
        if openIde :
            browser.launcherWindow.ide = Ide(fullpath, browser)
            browser.launcherWindow.ide.show()
        return False

    pathInCalibs = fullpath.replace('Sequences', 'Tonemapping', 1)
    calibFilePattern = pathInCalibs.replace('.pyx', '*_measurement.py', 1)
    calibFiles = glob.glob(calibFilePattern)
    for f in calibFiles :
        cloader = importlib.machinery.SourceFileLoader("c_module", f)
        c_module = cloader.load_module()
        try:
            c_module.apply(gears.getSequence())
        except:
            browser.launcherWindow.warn('Measurement file ' + f + ' is no longer valid, as the sequence has been edited. File skipped. Please recalibrate the sequence.')
    calibFilePattern = pathInCalibs.replace('.pyx', '*_tonemap.py', 1)
    calibFiles = glob.glob(calibFilePattern)
    for f in calibFiles :
        cloader = importlib.machinery.SourceFileLoader("c_module", f)
        c_module = cloader.load_module()
        try:
            c_module.apply(gears.getSequence())
        except:
            browser.launcherWindow.warn('Tone mapping file ' + f + ' is no longer valid, as the sequence has been edited. File skipped.  Please recalibrate the sequence.')

    if openIde :
        browser.launcherWindow.ide = Ide(fullpath, browser)
        browser.launcherWindow.ide.show()
        return False

    return True
    