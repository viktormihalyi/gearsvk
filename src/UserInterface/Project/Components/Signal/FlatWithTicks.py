import Gears as gears
from .. import * 

class FlatWithTicks(Component) : 

    def applyWithArgs(
            self,
            stimulus,
            ) :
        stimulus.raiseSignalOnTick(0, 'Tick')
        stimulus.clearSignalOnTick(0, 'Tick')
        stimulus.clearSignalOnTick(1, 'Stim sync')
        stimulus.raiseSignalOnTick(1, 'Stim sync')
                                 

