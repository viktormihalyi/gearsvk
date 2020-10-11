import os
import importlib.machinery
import time
import Gears as gears
import AppData


print("initializing appdata...")
AppData.initConfigParams()

try:
    print("initializing gears...")
    gears.InitializeEnvironment()

    print("loading modules...")
    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/gearsvk/src/UserInterface/Project/Sequences/stock.py").load_module()
    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/gearsvk/src/UserInterface/Project/Sequences/config.py").load_module()
    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/gearsvk/src/UserInterface/Project/Sequences/DefaultSequence.py").load_module()

    """
    my_module = importlib.machinery.SourceFileLoader("my_module", "./Project/Sequences/0_Utility/1_Spots/1_tiny_red.pyx").load_module()
    my_module = importlib.machinery.SourceFileLoader("my_module", "./Project/Sequences/1_Spots/3_Oscillations/1_sine_increasing_freqs.pyx").load_module()
    my_module = importlib.machinery.SourceFileLoader("my_module", "./Project/Sequences/9_Miscellaneous/FilteredSpots/1_box_red_spot_test.pyx").load_module()
    """
    my_module = importlib.machinery.SourceFileLoader("my_module", "./Project/Sequences/4_MovingShapes/1_Bars/04_velocity400.pyx").load_module()

    print("my_module.create")
    movingbar = my_module.create(None)

    print("createStimulusWindow")
    gears.createStimulusWindow()

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