import GearsModule as gears
from .. import * 
import traceback
import inspect
import sys

class Base(gears.Pass) : 

    def __init__(self, **args):
        super().__init__( )
        self.args = args
        self.setJoiner(self.join)
        self.setPythonObject(self)
        self.tb = traceback.extract_stack()
        try :
            while not self.tb[-1][0].endswith('pyx') :
                self.tb.pop()
        except IndexError:
            raise SequenceError('Pass not created in a .pyx file!', traceback.extract_stack())

    def join(self) :
#        try:
            self.boot(**self.args)
#        except Exception as e:
#            message = str(e)
#            if message.startswith('boot() ') :
#                message = message[len('boot() '):]
#            raise SequenceError(message, self.tb)

    def argStatus(self, func):
        bootFunc = inspect.currentframe().f_back.f_code
        print(bootFunc.co_varnames)
        print( func.__kwdefaults__ )
        print( func.__annotations__ )
        return 1

