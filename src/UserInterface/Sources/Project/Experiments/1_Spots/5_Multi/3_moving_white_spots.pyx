from Project.Components import *
import random 

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			]
	quads = []
	for i in range(0, 4096):
		quads += [{'x':random.uniform(-1000,+1000), 'y':random.uniform(-1000,+1000), 'width':40, 'height':40, 'pif':random.uniform(0, 1), 'motion':random.uniform(0, 1)}]
	agenda += [
			Stimulus.SinglePass    (
					duration_s = 10,
					spass = Pass.Generic(
							pif = Composition.InstancedParticles(
									pifs = [
										Pif.Spot(
											radius = 20,
										) * Pif.Solid(color = -1),
										Pif.Spot(
											radius = 20,
										) * Pif.Solid(color = 'white'),
									],
									),
							rasterizingMode = 'quads',
							polygonMask = quads,
							),
					prng = Prng.StaticHash(
						randomGridSize = (64, 64),
						),							
					particleSystem = ParticleSystem.Linear(
						particleGridSize = (64, 64),
						),
					)
			]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return SpotSequence('Random moving shapes').setAgenda( agenda )


#'''
#from Project.Components import *
#import random
#
#def addSpots(sequence, spots, radius, level) :
#	if level == 0 :
#		return
#	spots += [
#		Shape.MultiSpot.Spot(
#				position=(random.uniform(a=-sequence.field_width_um / 2, b=sequence.field_width_um / 2), random.uniform(a= -sequence.field_height_um / 2, b=sequence.field_height_um / 2)),
#				velocity=(random.uniform(a=-200, b=200), random.uniform(a=-200, b=200)),
#				radius = radius,
#				color = 'white',
#				),
#		Shape.MultiSpot.Spot(
#				position=(random.uniform(a=-sequence.field_width_um / 2, b=sequence.field_width_um / 2), random.uniform(a= -sequence.field_height_um / 2, b=sequence.field_height_um / 2)),
#				velocity=(random.uniform(a=-200, b=200), random.uniform(a=-200, b=200)),
#				radius = radius,
#				color = 'black',
#				),
#		]
#	addSpots(sequence, spots, radius / 1.41, level - 1)
#	addSpots(sequence, spots, radius / 1.41, level - 1)
#
#def create(mediaWindow):
#	spots = []
#	sequence = SpotSequence('Component tester')
#	addSpots(sequence, spots, 100, 4)
#	sequence.setAgenda( [
#			Stimulus.Blank( duration_s = 1  ),
#            StartMeasurement()             ,
#            Stimulus.Blank( duration_s = 1  ),
#            Stimulus.Generic(
#					duration_s = 20,
#					shape= Shape.MultiSpot( spots=spots )
#				),
#            Stimulus.Blank( duration_s = 1  ),
#            EndMeasurement(),
#            Stimulus.Blank( duration_s = 1  ),
#		] )
#	return sequence
#
#'''