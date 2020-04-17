print("press enter to start...")
input()

import os
print(os.getcwd())
os.chdir("C:/Dev/vulkantest/src/UserInterface")
print(os.getcwd())

print("importing...")

import Gears as gears
import importlib.machinery
import AppData
from MediaWindow import MediaWindow
import SequenceLoader
import time

print("initializing appdata...")
AppData.initConfigParams()

try:
    print("initializing gears...")
    gears.InitializeEnvironment()

    print("loading modules...")
    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/vulkantest/src/UserInterface/Project/Sequences/stock.py").load_module()
    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/vulkantest/src/UserInterface/Project/Sequences/DefaultSequence.py").load_module()
    my_module = importlib.machinery.SourceFileLoader("my_module", "./Project/Sequences/4_MovingShapes/1_Bars/01_velocity50.pyx").load_module()
    
    print("createStimulusWindow")
    gears.createStimulusWindow()

    print("my_module.create")
    movingbar = my_module.create(None)

    print("setSequence")
    gears.setSequence(movingbar)

    print("StartRendering")
    gears.StartRendering(lambda: False)
    gears.StartRendering(lambda: False)
    gears.StartRendering(lambda: False)
    gears.StartRendering(lambda: False)
    
    print("success")

except Exception as e:
    print("EXCEPTION")
    print(e)