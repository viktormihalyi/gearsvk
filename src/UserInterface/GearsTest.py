import os
print(os.getcwd())
os.chdir("C:/Dev/vulkantest/src/UserInterface")
print(os.getcwd())

import Gears as gears
import importlib.machinery
import AppData
#from MediaWindow import MediaWindow
#import SequenceLoader
AppData.initConfigParams()


try:
    print("press enter to start...")
    input()

    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/vulkantest/src/UserInterface/Project/Sequences/stock.py").load_module()
    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/vulkantest/src/UserInterface/Project/Sequences/DefaultSequence.py").load_module()
    my_module = importlib.machinery.SourceFileLoader("my_module", "./Project/Sequences/4_MovingShapes/1_Bars/01_velocity50.pyx").load_module()
    
    gears.createStimulusWindow()
    gears.makeCurrent()
    movingbar = my_module.create(None)
    gears.setSequence(movingbar)
    
    print("set seq succeeded")

except Exception as e:
    print("EXCEPTION")
    print(e)