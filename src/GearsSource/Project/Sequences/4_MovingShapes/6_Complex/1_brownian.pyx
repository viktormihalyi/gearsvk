from Project.Components import *
import random 

def create(mediaWindow):
	agenda = [
			#Stimulus.Blank( duration_s = 2  ),
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
											innerRadius = 5,
										) * Pif.SineWheel(),
										Pif.Spot(
											radius = 10,
											innerRadius = 5,
										) * Pif.Solid(color = 'red'),
										Pif.Spot(
											radius = 20,
											innerRadius = 10,
										) * Pif.Solid(color = 'blue'),
										Pif.Spot(
											radius = 20,
											innerRadius = 0,
										) * Pif.SineGrating(color1 = 'green', wavelength = 10),
										Pif.Spot(
											radius = 20,
											innerRadius = 15,
										) * Pif.Solid(color = 'yellow'),
										Pif.Spot(
											radius = 20,
											innerRadius = 5,
										) * Pif.Gradient(start=-30, end=+30, color1 = 'cyan', color2='magenta'),
										Pif.Spot(
											radius = 10,
											innerRadius = 5,
										) * Pif.Solid(color = 'magenta'),
										Pif.Rect(
											size_um = (20, 20),
										) * Pif.Solid(color = 'grey'),
										Pif.Rect(
											size_um = (20, 20),
										) * Pif.Solid(color = 'blue'),
									],
									),
							rasterizingMode = 'quads',
							polygonMask = quads,
							),
					prng = Prng.XorShift128(
						randomGridSize = (64, 64),
						),							
					particleSystem = ParticleSystem.Brownian(
						particleGridSize = (64, 64),
						),
					)
			]
	agenda += [
            EndMeasurement(),
            #Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Random moving shapes').setAgenda( agenda )

