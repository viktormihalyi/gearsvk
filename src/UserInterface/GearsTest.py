import os
print(os.getcwd())
os.chdir("C:/Dev/vulkantest/src/UserInterface")
print(os.getcwd())

print("importing...")
#import Gears as gears
import importlib.machinery
#import AppData
#from MediaWindow import MediaWindow
#import SequenceLoader
print("initializing...")
#AppData.initConfigParams()


try:
    print("initializing...")
    #gears.InitializeEnvironment()

    print("press enter to start...")
    input()

    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/vulkantest/src/UserInterface/Project/Sequences/stock.py").load_module()
    importlib.machinery.SourceFileLoader("my_module", "C:/Dev/vulkantest/src/UserInterface/Project/Sequences/DefaultSequence.py").load_module()
    my_module = importlib.machinery.SourceFiPyGILState_EnsureleLoader("my_module", "./Project/Sequences/4_MovingShapes/1_Bars/01_velocity50.pyx").load_module()
    
    #gears.createStimulusWindow()
    #gears.makeCurrent()
    movingbar = my_module.create(None)
    #gears.setSequence(movingbar)
    
    print("set seq succeeded")

except Exception as e:
    print("EXCEPTION")
    print(e)