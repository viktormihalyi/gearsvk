import os
print(os.getcwd())
os.chdir("C:/Dev/gearsvk/src/UserInterface")
print(os.getcwd())

print("importing...")

import Gears as gears
import importlib.machinery
import AppData
import time

print("press enter to start...")
input()

print("initializing appdata...")
AppData.initConfigParams()

try:
    print("initializing gears...")
    gears.InitializeEnvironment()

    print("loading modules...")
    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/gearsvk/src/UserInterface/Project/Sequences/stock.py").load_module()
    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/gearsvk/src/UserInterface/Project/Sequences/DefaultSequence.py").load_module()
    my_module = importlib.machinery.SourceFileLoader("my_module", "./Project/Sequences/4_MovingShapes/1_Bars/01_velocity50.pyx").load_module()
    
    print("createStimulusWindow")
    gears.createStimulusWindow()

    print("my_module.create")
    movingbar = my_module.create(None)

    print("setSequence")
    gears.setSequence(movingbar)

    print("StartRendering")
    gears.StartRendering(lambda: False)

    print("success")
    gears.DestroyEnvironment()

except Exception as e:
    gears.DestroyEnvironment()
    print("EXCEPTION")
    print(e)