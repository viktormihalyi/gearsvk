import os
import importlib.machinery


def loadParents(fullpath, rootPath):
    if os.path.realpath(fullpath) != os.path.realpath(rootPath):
        loadParents(fullpath + "/..", rootPath)
    
    for module in os.listdir(fullpath):
        if module == '__init__.py' or module[-3:] != '.py':
            continue

        sourceFileName = fullpath + "/" + module

        print ('Loading {}...'.format (sourceFileName))

        loader = importlib.machinery.SourceFileLoader('my_module', sourceFileName)
        my_module = loader.load_module()

        module_dict = my_module.__dict__
        try:
            to_import = my_module.__all__
        except AttributeError:
            to_import = [name for name in module_dict if not name.startswith('_')]
        
        globals().update({name: module_dict[name] for name in to_import})
