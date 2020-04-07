from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import QLabel, QDialog, QWidget, QGridLayout, QErrorMessage
from PyQt5.Qsci import QsciScintilla, QsciScintillaBase, QsciLexerPython, QsciAPIs
from PolymaskGenerator.PolymaskGeneratorWindow import *
from numpy import array as ar

class Calltip(QLabel):

    def __init__(self, editor, lpos, parent=None):
        self.lpos = lpos
        self.editor = editor
        super(Calltip, self).__init__(parent)
        font = QFont()
        font.setFamily('Courier')
        font.setFixedPitch(True)
        font.setPointSize(10)
        self.setFont(font)

        self.setTextFormat( Qt.RichText )
        self.setOpenExternalLinks( False )
        self.linkActivated.connect(self.onLink)

        #self.setFocusPolicy(Qt.NoFocus)
        self.window().installEventFilter(self)

    def skip_whitespace(self, pos, text):
        while text[pos] == ' ' or text[pos] == '\t' or text[pos] == '\n':
            pos += 1
        return pos

    def get_controls(self, pos, text):
        pos = self.skip_whitespace(pos, text)
        controls = []
        current = []
        start_pos = pos
        while pos < len(text):
            if text[pos] == "," or text[pos] == "]":
                value = text[start_pos:pos]
                current.append(float(value))
                if len(current) == 2:
                    controls.append(ar(current))
                    current.clear()
                
                if text[pos] == "]":
                    return pos, controls
                pos += 1
                pos = self.skip_whitespace(pos, text)
                start_pos = pos
            pos += 1
        
        box = QErrorMessage(self)
        box.showMessage("Control Points doesn't have the right syntax!")

        return pos, []

    def get_control_points(self, text):
        cps = []
        pos = 0
        while text[pos] != "[":
            pos += 1
        pos += 1
        while pos < len(text) and text[pos] != "]":
            if text[pos] == "[":
                pos, cp = self.get_controls(pos+1, text)
                cps.append(cp)
            pos += 1

        return cps

    def onLink(self, link):
        if link in self.pdict:
            self.editor.jumpTo(self.pdict[link][1])
        elif link == "generate_polymask":
            pos = self.pdict["polygonMask"][1] if "polygonMask" in self.pdict else self.lpos
            controlPoints = []
            try:
                text = self.pdict["polygonMask"][2] if "polygonMask" in self.pdict else ""
                idx = text.index("controlPoints")
                text = text[idx:]
                controlPoints = self.get_control_points(text)
            except:
                pass
            self.editor.polyMaskGenWnd.set(lambda triangles, controlPoints: self.editor.savePolymask(triangles, controlPoints, pos, self.lpos), controlPoints)
            self.editor.polyMaskGenWnd.show()
        else:
            citem = self.cdict[link]
            valrep = repr(citem[0])
            try :
                if citem[0].startswith("Project.Components.") :
                    valrep = citem[0][len("Project.Components."):] + "()"
            except AttributeError:
                pass
            self.editor.addKeyword( '''{keyword} = {defval} ,'''.format( keyword = link, defval = valrep ) , self.lpos )
            #TODO indent as in code

    #def mousePressEvent(self, e):
    #    super().mousePressEvent(e)
    #    self.close()
        
    def highlight(self, cdict, pdict):
        text = '<TABLE cellpadding=3>'
        for ckw, (cdefault, canno) in sorted(cdict.items()) :
            if ckw in pdict.keys() :
                text += '''
                <TR>
                <TD> <A HREF="{varname}" style="color:blue;">{varname}</A></TD>
                '''.format(
                            varname = ckw)
                if ckw == "polygonMask":
                    text+='''<TD align=right><A HREF="generate_polymask" style="color:black;">Generate Polymask</A></TD>'''
                else:
                    text+='''<TD align=right style="color:black">{val}</TD>'''.format(val=pdict[ckw][0])
                text += '''
                <TD>{doc}</TD>
                </TR>
                '''.format(doc=canno)

            #    text = text.replace('>' + key + '</A>', 'style="color:red;">' + key + '</A>')
            else:
                valrep = repr(cdefault)
                try :
                    if cdefault.startswith("Project.Components.") :
                        valrep = cdefault[len("Project.Components."):] + "()"
                except AttributeError:
                    pass
                text += '''
                <TR>
                <TD> <A HREF="{varname}" style="color:red;">{varname}</A></TD> 
                '''.format(
                            varname = ckw)

                if ckw == "polygonMask":
                    text+='''<TD align=right><A HREF="generate_polymask" style="color:green;">Generate Polymask</A></TD>'''
                else:
                    text+='''<TD align=right style="color:green">{val}</TD>'''.format(val = valrep)
                text += '''
                <TD>{doc}</TD>
                </TR>
                '''.format(doc=canno)
        text += '</TABLE>'
        first = True
        for kw in pdict.keys() :
            if not kw in cdict.keys():
                if first:
                    text += 'unknown keywords: '
                    first = False
                text += kw + ' '
        self.setText(text)
        self.pdict = pdict
        self.cdict = cdict
    
    #def focusOutEvent(self, e):
    #    print('callti[ lsot foxus\n')
    #    self.hide()

    def eventFilter(self, obj, event):
        #print(type(event))
        if event.type() == QEvent.WindowDeactivate:
            #print('dx')
            self.hide()
        return super().eventFilter(obj, event)