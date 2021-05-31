from Project.Components import *
import random 

def create(mediaWindow):
	agenda = [
			Stimulus.Blank( duration_s = 2  ),
			StartMeasurement()             ,
			]
	quads = []
	for i in range(-2,2):
		for j in range(-4,4):
			quads += [{'x':i*400, 'y':j*200, 'width':300, 'height':300, 'pif':((j+4) if i%2==0 else (4-j))/9, 'motion':0}]
	agenda += [
			Stimulus.SinglePass    (
					duration_s = 30,
					spass = Pass.Generic(
							alphaMask = Pif.Gaussian(variance = Interactive.MouseWheel(
																	label = 'Gaussian variance',
																	key = 'G',
																	initialValue =20,
																	minimum = 0,
																	maximum = 100,
							),),
							pif = Composition.Instanced(
									pifs = [
										Pif.SineGrating(wavelength=40) << Motion.Linear(velocity=(-40, 0)),
										Pif.SineGrating(wavelength=40) << Motion.Linear(velocity=(-30, 0)),
										Pif.SineGrating(wavelength=40) << Motion.Linear(velocity=(-20, 0)),
										Pif.SineGrating(wavelength=40) << Motion.Linear(velocity=(-10, 0)),
										Pif.SineGrating(wavelength=40) << Motion.Linear(velocity=(0, 0)),
										Pif.SineGrating(wavelength=40) << Motion.Linear(velocity=(10, 0)),
										Pif.SineGrating(wavelength=40) << Motion.Linear(velocity=(20, 0)),
										Pif.SineGrating(wavelength=40) << Motion.Linear(velocity=(30, 0)),
										Pif.SineGrating(wavelength=40) << Motion.Linear(velocity=(40, 0)),
									],
									motions = [Motion.Linear()],
									),
							rasterizingMode = 'quads',
							polygonMask = quads,
							),
					)
			]
	agenda += [
            EndMeasurement(),
            Stimulus.Blank( duration_s = 2  ),
			]
	return DefaultSequence('Motion illusion on vertical lineas made of drifting Gabor filters').setAgenda( agenda )

