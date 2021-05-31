
class SequenceError(Exception) : 
    def __init__(self, desc, tb, deepertb):
        super().__init__(desc)
        self.tb = tb
        self.deepertb = deepertb
