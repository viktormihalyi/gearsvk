import ctypes
import traceback



class PolychromeSequence(DefaultSequence) : 

    def boot(self, stimulusWindow):
        super().boot()
        self.stimulusWindow = stimulusWindow
        print('boot:' + str(stimulusWindow.handle))
        if stimulusWindow.handle == None :
            stimulusWindow.dllref = ctypes.WinDLL('TILLPolychrome.dll')
            stimulusWindow.handle = ctypes.c_void_p()
            print('TILLPolychrome_Open' + str(stimulusWindow.handle))
            stimulusWindow.dllref.TILLPolychrome_Open(ctypes.pointer(stimulusWindow.handle),ctypes.c_int(0))
            print(stimulusWindow.dllref)
        print('boot+:' + str(stimulusWindow.handle))

    def close(self) :
        print(self.stimulusWindow.handle)
        #if(stimulusWindow.handle.value != None) :
        #    stimulusWindow.dllref.TILLPolychrome_Close(ctypes.pointer(stimulusWindow.handle) )
        #print('TILLPolychrome_Close')
        #traceback.print_stack()

    def setWavelength(self, wavelength, duration = 0.01):
        print('TILLPolychrome_SetWavelength x')
        self.stimulusWindow.dllref.TILLPolychrome_SetWavelength(self.stimulusWindow.handle,ctypes.c_double(float(wavelength)), ctypes.c_double(float(duration*1000)), ctypes.c_bool(False))
        print('TILLPolychrome_SetWavelength')

    def setRestingWavelength(self, wavelength):
        self.stimulusWindow.dllref.TILLPolychrome_SetRestingWavelength(self.stimulusWindow.handle,ctypes.c_double(float(wavelength)))
        print('TILLPolychrome_SetRestingWavelength')

    def setBandwidth(self, bandwidth=15, intensity=1):
        self.stimulusWindow.dllref.TILLPolychrome_SetBandwidth(self.stimulusWindow.handle,ctypes.c_double(float(bandwidth)), ctypes.c_double(float(intensity)))
        print('TILLPolychrome_SetBandwidth')

