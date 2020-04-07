import Gears as gears
from .. import * 

try:
  from OpenGL.GL import *
  from OpenGL.GLU import *
except:
  print ('ERROR: PyOpenGL not installed properly.')

import random

def box() :
    glBegin(GL_QUADS)
    glColor3f(0.0,1.0,0.0)
    glVertex3f(1.0, 1.0,-1.0)
    glVertex3f(-1.0, 1.0,-1.0)
    glVertex3f(-1.0, 1.0, 1.0)
    glVertex3f(1.0, 1.0, 1.0)
    glColor3f(1.0,0.5,0.0)
    glVertex3f(1.0,-1.0, 1.0)
    glVertex3f(-1.0,-1.0, 1.0)
    glVertex3f(-1.0,-1.0,-1.0)
    glVertex3f(1.0,-1.0,-1.0)
    glColor3f(1.0,0.0,0.0)
    glVertex3f(1.0, 1.0, 1.0)
    glVertex3f(-1.0, 1.0, 1.0)
    glVertex3f(-1.0,-1.0, 1.0)
    glVertex3f(1.0,-1.0, 1.0)
    glColor3f(1.0,1.0,0.0)
    glVertex3f(1.0,-1.0,-1.0)
    glVertex3f(-1.0,-1.0,-1.0)
    glVertex3f(-1.0, 1.0,-1.0)
    glVertex3f(1.0, 1.0,-1.0)
    glColor3f(0.0,0.0,1.0)
    glVertex3f(-1.0, 1.0, 1.0)
    glVertex3f(-1.0, 1.0,-1.0)
    glVertex3f(-1.0,-1.0,-1.0)
    glVertex3f(-1.0,-1.0, 1.0)
    glColor3f(1.0,0.0,1.0)
    glVertex3f(1.0, 1.0,-1.0)
    glVertex3f(1.0, 1.0, 1.0)
    glVertex3f(1.0,-1.0, 1.0)
    glVertex3f(1.0,-1.0,-1.0)
    glEnd()

class Cube() : 
    args = None

    def __init__(self, **args):
        self.args = args

    def apply(self, stimulus) :
        self.applyWithArgs(stimulus, **self.args)

    def applyWithArgs(
            self,
            stimulus,
            ) :
        stimulus.setForwardRenderingCallback(self.render)
        stimulus.onStart( self.start )
        stimulus.onFinish( self.finish )

    def start( self ):
        self.glist = glGenLists(1)
        glNewList(self.glist, GL_COMPILE)
        glPushMatrix()
        glRotated(30, 1, 0, 0,)
        glRotated(45, 0, 1, 0,)
        box()
        glPopMatrix()
        glEndList()

    def finish( self ):
        glDeleteLists(self.glist, 1)

    def render(self, iFrame):
        glEnable(GL_DEPTH_TEST)
        glDepthMask(GL_TRUE);
        glClearColor(0.0, 0.0, 0.0, 1.0 )
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(45, 1, 0.1, 1000)
        glMatrixMode(GL_MODELVIEW)
        glLoadIdentity()
        gluLookAt(0, 0, 20, 0, 0, 0, 0, 1, 0)
        glRotated(iFrame, 0, 1, 0)
        glCallList(self.glist)
        glDisable(GL_DEPTH_TEST)
        glDepthMask(GL_FALSE);

