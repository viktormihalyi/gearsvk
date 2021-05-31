class SquareScanSequence(DefaultSequence) : 
    def boot(self, frameRateDivisor=1):
        super().boot( frameRateDivisor=frameRateDivisor )
        self.setSquareOptics()