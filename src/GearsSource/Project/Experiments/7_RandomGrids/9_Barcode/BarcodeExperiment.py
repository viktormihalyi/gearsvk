class BarcodeSequence(DefaultSequence) : 
    def boot(self, frameRateDivisor=1 ):
        super().boot( frameRateDivisor=frameRateDivisor )
        self.exportRandomsWithHashmarkComments = True
        self.exportRandomsChannelCount = 1
        self.exportRandomsAsReal = False
        self.exportRandomsAsBinary = True

