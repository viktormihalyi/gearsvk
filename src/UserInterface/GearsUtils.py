import sys
from platform import system
from numpy import matmul
from numpy import array
from numpy import linalg
import Gears as gears
from PyQt5.QtOpenGL import (QGLWidget, QGLFormat, QGLContext)

####################################################################
#                    Color conversion utilities                    #
####################################################################

RegisteredToRGBConverters   = {}
RegisteredFromRGBConverters = {}

def resetRegisteredConverters():
    global RegisteredToRGBConverters
    RegisteredToRGBConverters   = {}
    global RegisteredFromRGBConverters
    RegisteredFromRGBConverters = {}

def _hasNumberOfCols(m, c):
    return all (len (row) == c for row in m)

def _registerConverter(convStorage, name, converter, update, constraint):
    # Privte function to register converter into the specified storage
    if not isinstance(name, str):
        print("Error: Type of name argument for converter is not str!")
        return
    if name in convStorage and not update:
        print("Error: Color converter with name: " + name + " already exists!")
        return
    if isinstance(converter, list) and all (isinstance(item, list) for item in converter):
        if constraint(converter):
            convStorage[name] = converter
        else:
            print("Error: The dimension of color converter is invalid!")
        return
    print("Error: Type of converter or elements of it is not list!")

def registerToRGBConverter(name, converter, update = None):
    global RegisteredToRGBConverters
    _registerConverter(RegisteredToRGBConverters, name, converter, update, lambda c:len(c) == 3)

def registerFromRGBConverter(name, converter, update = None):
    global RegisteredFromRGBConverters
    _registerConverter(RegisteredFromRGBConverters, name, converter, update, lambda c:_hasNumberOfCols(c, 3))

def calculateInverseMatrix(conv):
    mtx = array(conv)
    result = mtx
    try:
        # try invert the matrix
        return linalg.inv(mtx)
    except linalg.LinAlgError:
        # inversion fails, try to calc pseudo inverse
        try:
            return linalg.pinv(mtx)
        except linalg.LinAlgError:
            print("Error: Cannot calculate inverse of matrix")

def _getConverter(storage, converterName):
    if converterName in storage:
        return storage[converterName]
    else:
        print("Error: Cannot find " + converterName + " in storage!")

def getConverterByName(converterName, toRGB = True):
    if toRGB:
        global RegisteredToRGBConverters
        return _getConverter(RegisteredToRGBConverters, converterName)
    else:
        global RegisteredFromRGBConverters
        return _getConverter(RegisteredFromRGBConverters, converterName)

def _convertColor(color, colorConverter, toRGB = True):
    # Check color argument
    if isinstance(color, tuple) or isinstance(color, list):
        colorArray = array(color)
    else:
        print("Error: Wrong color type! Type has to by tuple or list and not " + str(type(color)))
        return color

	# Check colorConverter argument 
    if isinstance(colorConverter, str):
        conv = getConverterByName(colorConverter, toRGB)
        if not conv:
            return color
    elif isinstance(colorConverter, list):
        conv = colorConverter
        rowNum = 3
        colNum = len(color)
        if not toRGB:
            rowNum, colNum = colNum, rowNum
        if not len(conv) == rowNum or not _hasNumberOfCols(conv, colNum):
            print("Error: Wrong converter dimension! Number of rows has to be " + str(rowNum) + " and number of columns has to be " + str(colNum) + "!")
            return color
    else:
        print("Error: Wrong converter type! Type has to be list and not " + str(type(colorConverter)))
        return color

	# Calculate result
    result = matmul(conv, colorArray.transpose())
    if isinstance(color, tuple):
        return tuple(result)
    if isinstance(color, list):
        return list(result)


def convertColorToRGB(color, colorConverter):
    return _convertColor(color, colorConverter)

def convertFromRGBColor(color, colorConverter):
    return _convertColor(color, colorConverter, False)

####################################################################
#    Utility for QGLWidgit initialization on multiple platforms    #
####################################################################

def initQGLWidget(self, base, parent, winId):
    format = QGLFormat()
    format.setSwapInterval(1)
    if system() == 'Windows':
        base.__init__(format, parent)
        self.makeCurrent()
        gears.shareCurrent()
    elif system() == 'Linux':
        gears.shareCurrent( int(winId) )
        base.__init__(QGLContext.currentContext(), parent)
        self.makeCurrent()
    else:
        print("Not supported platform: " + system())
        sys.exit(1)