import Gears as gears
from .. import * 
import traceback
import inspect
from .Base import *
import ILog


class Start(Base) : 

    def __init__(self, **args):
        super().__init__(**args)

    def boot(
            self,
            *,
            question        : 'Question to display.'
                            = 'Please respond.',
            buttons         :'[button label, x coordinate of the button center (um), y coordinate of the button center (um), width, height, key, visible]'
                            = [ ('No', -500, -600, 400, 100, 'N', True),
                                ('Yes', 500, -600, 400, 100, 'Y', True),],
            loop            : 'Repeat the stimulus/stimuli until the response is received. (True/False)'
                            = True
           ):
        self.question                =      question
        self.loop =loop
        self.buttons = buttons
        for button in  buttons:
            self.addButton(button[0], button[1], button[2], button[3], button[4], ord(button[5]), button[6])

        self.registerCallback(gears.MouseReleasedLeftEvent.typeId, self.onMouseReleasedLeft )
        self.registerCallback(gears.KeyPressedEvent.typeId, self.onKey )

    def onMouseReleasedLeft(self, event) :
        seq = self.getSequence()
        x = event.globalPercentX()*seq.field_width_um - seq.field_width_um/2
        y = -(event.globalPercentY()*seq.field_height_um - seq.field_height_um/2)
        for button in self.buttons:
            if (button[1]-button[3]/2< x <button[1]+button[3]/2 and button[2]-button[4]/2< y < button[2]+button[4]/2):
                print (button[0])
                ILog.log.put("@{time} s: {var} = {value}".format( time=gears.getTime(), 
                                                                 var=self.question, value=button[0]))
                gears.setResponded()
           
            
    def onKey(self, event):
        print(event.text())
        for button in self.buttons:
            if event.text() == button[5] : 
                print("@{time} s: {var} = {value}".format( time=gears.getTime(), 
                                                                 var=self.question, value=button[0]))
                ILog.log.put("@{time} s: {var} = {value}".format( time=gears.getTime(), 
                                                                 var=self.question, value=button[0]))
                gears.setResponded()
                return True
        return False
       


