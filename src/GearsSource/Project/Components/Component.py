import traceback
import warnings
import re
from SequenceError import SequenceError
import ILog
import Gears as gears
import math

class Component() : 
    def __init__(self, **args):
        self.args = args
        self.tb = traceback.extract_stack()
        try :
            while not self.tb[-1][0].endswith('pyx') :
                self.tb.pop()
        except IndexError:
            self.tb = None
            #raise SequenceError('Component not created in a .pyx file!', traceback.extract_stack())

    def __repr__(self):
        return type(self).__name__ + str(self.args)

    def apply(self, spass) :
        #if not self.tb[-2][0].endswith('pyx') :
        if self.tb == None :
            self.tb = spass.tb
        self.applyWithArgs(spass, **self.args)

    def warn(self, message):
        warnings.warn('Warning: <BR>' 
                + self.tb[-1][0] + '(' + str(self.tb[-1][1]) + '):<BR> in function "' 
                + self.tb[-1][2] + '":<BR> ' + self.tb[-1][3] + '<BR><BR>' + message )

    def multiple_replace(self, string, rep_dict):
        pattern = re.compile("|".join([re.escape(k) for k in rep_dict.keys()]), re.M)
        return pattern.sub(lambda x: rep_dict[x.group(0)], string)

    def glslEsc(self, string):
        return self.multiple_replace(string, {'{' : '{{', '}' : '}}', '@<' : '{', '>@' : '}', '`' : '{X}_' })

    def registerInteractiveControls(self, spass, stimulus, shaderVariableNamePrefix, **kwargs):
        self.interactiveControls = kwargs
        self.shaderVariableNamePrefix = shaderVariableNamePrefix
        #todo allow the same component in more passes? here we would need a list of passes and iterate over them in update
        self.spass = spass
        for varname, control in kwargs.items():
            try:
                control.register(stimulus, self)
            except:
                pass
            try:
                control[0].register(stimulus, self)
            except:
                pass
            try:
                control[1].register(stimulus, self)
            except:
                pass
            try:
                control[2].register(stimulus, self)
            except:
                pass
            try:
                stimulus.getSpatialFilter().makeUnique();
            except:
                pass
            try:
                control[0].register(stimulus, self)
            except:
                pass
            try:
                control[1].register(stimulus, self)
            except:
                pass
            try:
                control[2].register(stimulus, self)
            except:
                pass
            try:
                stimulus.getSpatialFilter().makeUnique();
            except:
                pass
        self.refresh()

    def refresh(self):
        d = {}
        for key, control in self.interactiveControls.items() :
            if isinstance(control, tuple):
                list=()
                for i in control:
                    try:
                        list+=(i.value,)
                        ILog.log.put("@{time} s: {var} = {value}".format( time=gears.getTime(), var=i.label, value=i.value))
                    except:
                        list+=(i,)
                control = list
            try:
                d[key] = control.value
                ILog.log.put("@{time} s: {var} = {value}".format( time=gears.getTime(), var=control.label, value=control.value))
            except:
                d[key] = control
        self.update(**d)

    def update(self, **kwargs):
        #for key, control in self.interactiveControls.items() :
        for key, control in kwargs.items() :
            try:
            #    control.applyAsShaderParameter(self.spass, self.shaderVariableNamePrefix + key)
                self.spass.setShaderColor(name=self.shaderVariableNamePrefix + key, red = control[0].value, green = control[1].value, blue = control[3].value)
            except:
                try:
                    self.spass.setShaderVector(name=self.shaderVariableNamePrefix + key, x = control[0].value, y = control[1].value)
                except:
                    try:
                        self.spass.setShaderColor(name=self.shaderVariableNamePrefix + key, red = control[0], green = control[1], blue = control[2])
                    except:
                        try:
                            self.spass.setShaderVector(name=self.shaderVariableNamePrefix + key, x = control[0], y = control[1])
                        except:
                            self.spass.setShaderVariable(name=self.shaderVariableNamePrefix + key, value = control)
