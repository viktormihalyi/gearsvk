import Gears as gears
from .. import * 
import traceback
import inspect
from .Base import *

class Generic(Base) : 

    def __init__(self, **args):
        super().__init__(**args)

    def boot(
            self,
            *,
            name            : 'Pass name to display in sequence overview plot.'
                            = 'Generic',
            duration        : 'Pass time in frames. Defaults to stimulus duration. Superseded by duration_s is given.'
                            = 0,
            duration_s      : 'Pass time in seconds (takes precendece over duration given in frames).'
                            = 0,
            pif             : 'Root pif component. (Pif.* or Composition.*)'
                            = Pif.Solid(),
            alphaMask       : 'Pif component that provides the opacity when multiple passes or quads are combined. (Pif.* or Composition.*)'
                            = Pif.Solid(color=0.5),
            rasterizingMode : 'The type of geometry to be rasterized for the spass. (fullscreen/bezier/triangles/quads)'
                            = 'fullscreen',
            polygonMask     : 'Data defining the geometry to be rasterized.'
                            = [{'x':0, 'y':0}, {'x':0, 'y':1}, {'x':1, 'y':0}],
            polygonMotion   : 'Motion component for polygon animation. (Motion.*)'
                            = Motion.Linear()
            ):
        self.name                =      name
        self.duration            =      duration
        sequence = self.getSequence()
        stimulus = self.getStimulus()
        self.duration = duration
        if(duration == 0):
            self.duration = stimulus.getDuration()
        if(duration_s != 0):
            self.duration = int(duration_s // sequence.getFrameInterval_s() + 1)
        
        polygonMask = polygonMask["triangles"] if type(polygonMask) is dict else polygonMask
        self.setPolygonMask(rasterizingMode, polygonMask)
        if rasterizingMode == 'triangles':
            polygonMotion.applyForward(self, 'polygonMotionTransform')

        pif.apply(self, "fig")
        alphaMask.apply(self, "alphaMask")

        self.stimulusGeneratorShaderSource = """
    		in vec2 pos;
            in vec2 fTexCoord;
		    void main() { vec4 outcolor = vec4(
                    fig(pos, time), alphaMask(pos, time).x ); 
                outcolor.rgb = temporalProcess(outcolor.rgb, fTexCoord);
                outcolor.rgb = toneMap(outcolor.rgb);
                if(swizzleForFft == 0x00020000)
                    outcolor = vec4(outcolor.r, 0, outcolor.g, 0);
                    //outcolor = vec4(0, 0, 0, 0);
                else if(swizzleForFft == 0x00000406)
                    outcolor = vec4(outcolor.b, 0, outcolor.a, 0);
                                 
                gl_FragData[0] = outcolor;
            }
		"""

