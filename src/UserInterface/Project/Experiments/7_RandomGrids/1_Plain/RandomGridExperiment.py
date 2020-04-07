class RandomGridSequence(DefaultSequence) : 
    def boot(self, frameRateDivisor=1, fft_width_px=64, fft_height_px=64 ):
        super().boot( frameRateDivisor=frameRateDivisor )
        self.setSquareOptics()
        self.fft_width_px = fft_width_px
        self.fft_height_px = fft_height_px
        self.exportRandomsWithHashmarkComments = True
        self.exportRandomsChannelCount = 1
        self.exportRandomsAsReal = False
        self.exportRandomsAsBinary = True

