from Project.Components import *
import random 

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			]
	quads = []
	for i in range(0, 5000):
		quads += [{'x':random.uniform(-1000,+1000), 'y':random.uniform(-1000,+1000), 'width':40, 'height':40, 'pif':random.uniform(0, 1), 'motion':random.uniform(0, 1)}]
	agenda += [
			Stimulus.SinglePass    (
					duration_s = 10,
					spass = Pass.Generic(
							pif = Composition.Instanced(
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
									motions = [
										Motion.Linear(
											velocity = (100, 0),
											startScale = (1, 1.3),
											startAngle = 0.7,
											),
										Motion.Linear(
											velocity = (0, 100),
											startScale = (0.7, 0.7),
											),
										Motion.Linear(
											velocity = (0, 100),
											startAngle = 30,
											startScale = (1.2, 1.6),
											),
										Motion.Linear(
											velocity = (0, 100),
											startAngle = 1,
											),
									]
									),
							rasterizingMode = 'quads',
							polygonMask = quads,
								#[
								#	{'x':0, 'y':0, 'width':60, 'height':60, 'pif':0, 'motion':0},
								#	{'x':0, 'y':0, 'width':30, 'height':30, 'pif':1, 'motion':1},
								#	],
							),
					)
			]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Spots with increasing modulation frequencies with shaking').setAgenda( agenda )

