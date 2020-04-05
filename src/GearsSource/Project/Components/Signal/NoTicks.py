import Gears as gears
from .. import * 

class NoTicks(Component) : 

    def applyWithArgs(
            self,
            stimulus,
            ) :
        stimulus.overrideTickSignals()
        stimulus.clearSignalOnTick(1, 'Tick')
        stimulus.clearSignalOnTick(1, 'Stim sync')

                                 

