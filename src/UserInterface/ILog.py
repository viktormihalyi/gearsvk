import os
import Gears as gears


class Log:
    def __init__(self):
        self.file = None

    def open(self, logFilePath):
        gears.makePath(logFilePath)
        self.file = open( logFilePath, 'w')

    def close(self):
        if self.file != None:
            self.file.close()
        self.file = None

    def put(self, text):
        print(text, file=self.file)

log = Log()

    

