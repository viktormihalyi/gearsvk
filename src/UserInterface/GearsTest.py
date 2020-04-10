import Gears as gears
import importlib.machinery
import AppData
from StimulusWindow import StimulusWindow
#from MediaWindow import MediaWindow
import SequenceLoader

AppData.initConfigParams()
 
    
fullpath = "./Project/Sequences/4_MovingShapes/1_Bars/01_velocity50.pyx"

loader = importlib.machinery.SourceFileLoader ("my_module", fullpath)

my_module = loader.load_module ()
gears.createStimulusWindow ()
gears.makeCurrent ()
gears.setSequence (my_module.create (None))