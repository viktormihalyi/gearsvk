import sys
import os
import atexit
import pathlib
import importlib.machinery

import GearsModule as gears
import AppData
import SequenceLoaderCore

atexit.register (gears.DestroyEnvironment)

AppData.initConfigParams ()

filePath = str(pathlib.Path (sys.argv[1]))
fileParentPath = str(pathlib.Path (sys.argv[1]).parent)
rootPath = str(pathlib.Path.cwd ().joinpath ('Project'))

SequenceLoaderCore.loadParents (fileParentPath, rootPath)

print ('Loading SequenceCreator... ', end='')
sequenceCreator = importlib.machinery.SourceFileLoader ('my_module', filePath).load_module ()
print ('Done!')

print ('Running Create... ', end='')
sequence = sequenceCreator.create (None)
print ('Done!')

print ('SetSequence... ', end='')
gears.setSequence (sequence, os.path.basename (filePath))
print ('Done!')

print ('StartRendering...')
gears.StartRendering ()
