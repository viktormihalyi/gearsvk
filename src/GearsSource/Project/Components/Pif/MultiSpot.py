import Gears as gears
from .. import * 
import traceback
from .Base import *

class MultiSpot(Base) : 
    class Spot() :
        __gears_api_helper_class__ = True
        def __init__(self,
                *,
                radius      : 'Spot radius [um].'
                            = 30,
                innerRadius : 'Annulus hole radius [um].'
                            = 0,
                velocity    : '2D velocity vector as an (x,y) pair [(um/s,um/s)].'
                            = (0, 0),
                position    : '2D initial position vector as an (x,y) pair [(um,um)].'
                            = (0, 0),
                color       : 'Spot shape color mask as an (r,g,b) triplet, or a color name.'
                            = 'white'
                ) :
            self.tb = traceback.extract_stack()
            self.radius = radius
            self.innerRadius = innerRadius
            self.velocity = velocity
            self.position = position
            self.radius = radius
            self.color = processColor(color, self.tb)

    def applyWithArgs(
            self,
            spass,
            functionName,
            *,
            radius          : 'Spot radius [um].'
                            = 200,
            innerRadius     : 'Annulus inner radius [um] (or negative for solid disc).'
                            = -1000,
            spots           : 'A list of Shape.MultiSpot.Spot instances, specifying the spots.'
                            = [],
            background      : 'Color outside of spots, as an (r,g,b) triplet or a color name.'
                            = 'grey',
            filterRadius_um : 'Antialiasing filter size [um] (shape blur).'
                            = 0.1
            ) :
        background = processColor(background, self.tb)
        stimulus = spass.getStimulus()
        if max(background) - min(background) > 0.03:
            spass.enableColorMode()

        spass.setShaderVariable( name=functionName+'_filterRadius', value = filterRadius_um )
        spass.setShaderColor( name = functionName+'_backgroundColor',    red = background[0], green = background[1], blue = background[2] )
        i = 0
        s = """
                        vec3 @<X>@ (vec2 x, float time){ 
                            vec3 c = `backgroundColor;
                            vec2 diff = vec2(0,0);
                            float dist = 0;
                            float inOrOut = 0;
            """
        for spot in spots :
            if max(spot.color) - min(spot.color) > 0.03:
                spass.enableColorMode()

            spass.setShaderVector( name = functionName+'_spotRadius' + str(i),    x = spot.radius , y=spot.innerRadius )
            spass.setShaderVector( name = functionName+'_spotPos'+ str(i),    x = spot.position[0], y= spot.position[1] )
            spass.setShaderVector( name = functionName+'_spotVel'+ str(i),    x = spot.velocity[0], y= spot.velocity[1] )
            spass.setShaderColor( name = functionName+'_spotColor'+ str(i),    red = spot.color[0], green = spot.color[1], blue = spot.color[2] )
            s += """
                    //diff = length( mod(x - `spotPos{i} + time * `spotVel{i} + vec2(1, 1), 2) + vec2(1,1));
                    diff = x - `spotPos{i} + time * `spotVel{i};
                    diff = mod(diff + patternSizeOnRetina * 0.5, patternSizeOnRetina) - patternSizeOnRetina * 0.5;
                    dist = length(diff);
                    inOrOut = (1-smoothstep( -`filterRadius, +`filterRadius, dist - `spotRadius{i}.x )) * (1-smoothstep( -`filterRadius, +`filterRadius, `spotRadius{i}.y - dist ));
                    c = mix(c, `spotColor{i}, inOrOut);
                """.format( i=i)
            i+=1
        s += """
                return c;
                }
            """
        spass.setShaderFunction( name = functionName, src = self.glslEsc( s ).format( X=functionName )  )

