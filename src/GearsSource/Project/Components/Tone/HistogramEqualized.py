import Gears as gears
from .. import * 

class HistogramEqualized(Component) : 

    def applyWithArgs(
            self,
            stimulus,
            *,
            dynamic = True
            ) :

        self.stimulus = stimulus;
        stimulus.setToneMappingEqualized(dynamic)

