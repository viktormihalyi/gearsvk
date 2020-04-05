from Project.Components import *

class BlackoutWithPolychromePreparationStimulus(Stimulus.Fullfield) : 
    def boot(self, 
            duration=1,
            duration_s     =   0,
            wavelength = 480.0,
            bandwidth = 15.0,
            intensity = 1
            ):
        super().boot( name='Blackout/Polychrome', duration=duration, duration_s=duration_s )
        self.wavelength = wavelength
        self.bandwidth = bandwidth
        self.intensity = intensity
        stimulus.registerCallback(gears.BeginStimulus(), self.play)


    def play(self):
        print('polychrome prepare')
        sequence = self.getSequence()
        #sequence.getPythonObject().setBandwidth(self.bandwidth, self.intensity)
        #sequence.getPythonObject().setWavelength(self.wavelength, self.duration )
        sequence.getPythonObject().setRestingWavelength(self.wavelength)

