from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import QLabel, QDialog, QApplication
from PyQt5.Qsci import QsciScintilla, QsciScintillaBase, QsciLexerPython, QsciAPIs
from PolymaskGenerator.PolymaskGeneratorWindow import *

import AppData

import types
import inspect
import ast
import pkgutil
import pydoc
import os

import colorsys
import traceback
import random
#import Project.Components
from importlib.machinery import SourceFileLoader

Components = SourceFileLoader("Components", AppData.appDataDir + "/Project/Components/__init__.py").load_module()

from Calltip import Calltip
from QsciStrictAPIs import QsciStrictAPIs

class Editor(QsciScintilla):
    ARROW_MARKER_NUM = 8

    def makeTip(self, bootfunc):
        calltip = {}
        for kw, kwval in bootfunc.__kwdefaults__.items() :
            #calltip += '<TR> <TD> <A HREF="{varname}">{varname}</A></TD> <TD align=right> {defval} <TD>'.format(varname=kw, defval=repr(kwval))
            try:
                anno = bootfunc.__annotations__[kw]
                if anno and isinstance(anno, str) :
                    if kwval.__class__.__module__ != 'builtins':#inspect.isclass(kwval) : #hasattr(kwval, '__class__') :
                        calltip[kw] = (kwval.__class__.__module__, anno) #("@@#{0}.{1}".format(kwval.__class__.__module__,kwval.__class__.__name__), anno)
                    else:
                        calltip[kw] = (kwval, anno)
                if anno and inspect.isclass(anno) and hasattr(anno, 'applyWithArgs') :
                    calltip.update( self.makeTip(anno.applyWithArgs) )
            except KeyError:
                pass
        for argn, anno in bootfunc.__annotations__.items() :
            if anno and inspect.isclass(anno) and hasattr(anno, 'applyWithArgs') :
                calltip.update( self.makeTip(anno.applyWithArgs) )
        return calltip

    def addApiClass(self, api, obj, prefix):
        if(hasattr(obj, 'boot')):
            self.modules.append(obj)
            calltip = obj.__module__[len(prefix)+1:]
            if obj.boot.__kwdefaults__ :
                calltip += '(' + str(self.makeTip(obj.boot)) + ')'
            api.add( calltip )
        elif(hasattr(obj, 'applyWithArgs')):
            self.modules.append(obj)
            calltip = obj.__module__[len(prefix)+1:]
            if obj.applyWithArgs.__kwdefaults__ :
                calltip += '(' + str(self.makeTip(obj.applyWithArgs)) + ')'
            api.add( calltip )
        elif (hasattr(obj, '__gears_api_helper_class__')):
            self.modules.append(obj)
            calltip = (obj.__module__+'.'+obj.__name__)[len(prefix)+1:]
            if obj.__init__.__kwdefaults__ :
                calltip += '(' + str(self.makeTip(obj.__init__)) + ')'
            api.add( calltip )
        for name, var in inspect.getmembers(obj) :
            if not str(name).startswith('_'):
                if inspect.isclass(var):
                    if not var in self.modules:
                        self.addApiClass(api, var, prefix)

    def addModuleFuncs(self, api, module, prefix):
        self.counter += 1
        #if self.counter > 20:
        #    return
        for name, obj in inspect.getmembers(module) :
            if not obj in self.modules:
                if inspect.isclass(obj):
                    self.addApiClass(api, obj, prefix)
                if inspect.ismodule(obj):
                    try:
                        #print('module: ' + name)
                        if obj.__gears_api__:
                            self.addModuleFuncs(api, obj, prefix)
                    except AttributeError:
                        pass
                #ela = getattr(module, elem)
                #if ela != module:
                #    if elem.istitle():
                #        if type(ela) == types.ModuleType:
                #            api.add(elem)
                #            self.addModuleFuncs(api, ela)
                #    if type(ela) == types.FunctionType or inspect.isclass(ela):
                #        api.add(elem)

    def addPackageFuncs(self, api, package, prefix):
        for module_loader, modname, ispkg in pkgutil.walk_packages(
                            path= package.__path__,
                            #path = [AppData.appDataDir + '\\Project\\Components'],
                            prefix=prefix+'.'):
            for name, obj in inspect.getmembers(pydoc.locate(modname)) :
                if not obj in self.modules:
                    if inspect.isclass(obj):
                        self.addApiClass(api, obj, prefix)

    def __init__(self, sequencePath, errline, parent=None):
        super(Editor, self).__init__(parent)
        #Set the default font
        self.sequencePath = sequencePath
        self.polyMaskGenWnd = PolymaskGeneratorWindow()
        font = QFont()
        font.setFamily('Courier')
        font.setFixedPitch(True)
        font.setPointSize(10)
        self.setFont(font)
        self.setMarginsFont(font)
        self.file = open(self.sequencePath)
        self.setTabWidth(4)
        self.setText(self.file.read())
        self.file.close()
        self.sequenceErrorIndicator = self.indicatorDefine(QsciScintilla.SquiggleIndicator)
        self.setIndicatorForegroundColor(Qt.magenta, self.sequenceErrorIndicator)
        if errline > 0:
            self.fillIndicatorRange(errline-1, 0, errline, 0, self.sequenceErrorIndicator)
        self.previewTrackIndicator = self.indicatorDefine(QsciScintilla.DotBoxIndicator)
        self.setIndicatorForegroundColor(Qt.blue, self.previewTrackIndicator)
        
        # Margin 0 is used for line numbers
        fontmetrics = QFontMetrics(font)
        self.setMarginsFont(font)
        self.setMarginWidth(0, fontmetrics.width("00000") + 6)
        self.setMarginLineNumbers(0, True)
        self.setMarginsBackgroundColor(QColor("#cccccc"))
        
        # Clickable margin 1 for showing markers
        self.setMarginSensitivity(1, True)
        self.marginClicked.connect(self.on_margin_clicked)
        self.markerDefine(QsciScintilla.RightArrow,
            self.ARROW_MARKER_NUM)
        self.setMarkerBackgroundColor(QColor("#ee1111"),
            self.ARROW_MARKER_NUM)
        
        # Brace matching: enable for a brace immediately before or after
        # the current position
        #
        self.setBraceMatching(QsciScintilla.SloppyBraceMatch)
        
        self.setAutoIndent(True)

        # Current line visible with special background color
        self.setCaretLineVisible(True)
        self.setCaretLineBackgroundColor(QColor("#ffe4e4"))
        
        # Set Python lexer
        # Set style for Python comments (style number 1) to a fixed-width
        # courier.
        #
        lexer = QsciLexerPython(self)

        api = QsciStrictAPIs(lexer)
        self.counter = 0
        self.modules = []
        #print('Processing components for calltips at path:')
        #print(Components.__path__)
        #print('1-')
        #print(Components.__file__)
        #print('2-')
        #print(Components.__spec__)
        #print('4-')
        #print(inspect.getfile(Components))
        #print('--')
        #print(AppData.appDataDir)
        #print(AppData.appDataDir + '\\Project\\Components')
        #print('@@')
        self.addPackageFuncs(api, Components, 'Components')
        #self.addModuleFuncs(api, Project.Components, 'Project.Components')

        api.prepare()

        lexer.setDefaultFont(font)
        self.setLexer(lexer)
        self.setAutoCompletionThreshold(1)
        self.setAutoCompletionCaseSensitivity( False )
        self.setAutoCompletionSource(QsciScintilla.AcsAPIs)
        self.setCallTipsStyle(QsciScintilla.CallTipsContext)
        #self.SendScintilla(QsciScintilla.SCI_STYLESETFONT, 1, 'Courier')
        
        # Don't want to see the horizontal scrollbar at all
        # Use raw message to Scintilla here (all messages are documented
        # here: http://www.scintilla.org/ScintillaDoc.html)
        self.SendScintilla(QsciScintilla.SCI_SETHSCROLLBAR, 0)
        self.SendScintilla(QsciScintilla.SCI_SETEOLMODE, QsciScintilla.SC_EOL_LF )
        self.SendScintilla(QsciScintilla.SCI_SETPASTECONVERTENDINGS, True)

        #self.SCN_CALLTIPCLICK.connect(self.calltipClicked)
        # not too small
        self.setMinimumSize(1024, 768)

        self.calltip = None

    #def calltipClicked(self, dir):
    #    pass

    def on_margin_clicked(self, nmargin, nline, modifiers):
        # Toggle marker for the line the margin was clicked on
        if self.markersAtLine(nline) != 0:
            self.markerDelete(nline, self.ARROW_MARKER_NUM)
        else:
            self.markerAdd(nline, self.ARROW_MARKER_NUM)

    def closeEvent(self, e):
        self.calltip = None
        return super().closeEvent(e)

    def mousePressEvent(self, e):
        QsciScintilla.mousePressEvent(self, e)
        self.callTip()

    def getCharacter(self, pos):
        if (pos <= 0):
            return '\0', pos

        pos = pos - 1
        ch = self.SendScintilla(QsciScintilla.SCI_GETCHARAT, pos);

        # Don't go past the end of the previous line.
        if (ch == '\n' or ch == '\r'):
            pos = pos + 1
            return '\0', pos
        
        return chr(ch), pos

    def getCharacterRight(self, pos):
        if (pos <= 0):
            return '\0', pos

        pos = pos + 1
        ch = self.SendScintilla(QsciScintilla.SCI_GETCHARAT, pos);

        # Don't go past the end of file
        if (ch == -1):
            pos = pos - 1
            return '\0', pos
        
        return chr(ch), pos

    # Shift the position of the call tip (to take any context into account) but
    # don't go before the start of the line.
    def adjustedCallTipPosition(self, ctPos, ctshift) :
        ct = ctPos
        if (ctshift) :
            ctmin = self.SendScintilla(QsciScintilla.SCI_POSITIONFROMLINE, self.SendScintilla(QsciScintilla.SCI_LINEFROMPOSITION, ct))

            if (ct - ctshift < ctmin) :
                ct = ctmin
        return ct

    def read_param(self, pos, text, end_pos):
        start_pos = pos
        while pos != end_pos and text[pos] != ',':
            p = text[pos]
            if p == '(' or p == '[' or p == '{':
                pos = self.skip_parens(pos, text, end_pos)
            else:
                pos+=1

        pos += 1 # skip ,
        return pos, text[start_pos:pos-1] # , not included

    def skip_parens(self, pos, text, end_pos):
        skipRound = 0
        skipCurly = 0
        skipSquare = 0
        while pos < end_pos:
            p  = text[pos]
            if p == '[':
                skipSquare += 1
            elif p == '(':
                skipRound += 1
            elif p == '{':
                skipCurly += 1
            elif p == ']' and skipSquare > 0:
                skipSquare -= 1
            elif p == ')'and skipRound > 0:
                skipRound -= 1
            elif p == '}' and skipCurly > 0:
                skipCurly -= 1
            elif skipRound == 0 and skipCurly == 0 and skipSquare == 0:
                return pos

            pos += 1

        return pos

    def stripParens(self, test_str):
        ret = ''
        skip1c = 0
        skip2c = 0
        skipped = 0
        for i in test_str:
            if i == '[':
                skip1c += 1
            elif i == '(':
                skip2c += 1
            elif i == ']' and skip1c > 0:
                skip1c -= 1
            elif i == ')'and skip2c > 0:
                skip2c -= 1
            elif skip1c == 0 and skip2c == 0:
                ret += i
            else:
                skipped += 1
        return ret, skipped

    def callTip(self):
        pos = self.SendScintilla(QsciScintilla.SCI_GETCURRENTPOS)
        rpos = pos

        # Move backwards through the line looking for the start of the current
        # call tip and working out which argument it is.
        ch, pos = self.getCharacter(pos)
        inKeyword = False
        semanticsKnown = False
        found = False
        multiline = False
        while (ch != '\0'):
            if (ch == ',' and not semanticsKnown):
                semanticsKnown = True
                inKeyword = True
            elif (ch == '=' and not semanticsKnown):
                semanticsKnown = True
                inKeyword = False
            elif (ch == ')'):
                depth = 1
                # Ignore everything back to the start of the corresponding
                # parenthesis.
                ch, pos = self.getCharacter(pos)
                while (ch != '\0') :
                    if (ch == ')'):
                        depth += 1
                    elif ch == '(' :
                        depth -= 1
                        if depth == 0 :
                            break
                    ch, pos = self.getCharacter(pos)
            elif (ch == '('):
                found = True;
                break
            elif (ch == '\n' or ch == '\r'):
                multiline = True
            ch, pos = self.getCharacter(pos)

        lpos = pos + 1

        #move over spaces to get context
        if ch != '\0' :
            ch, pos = self.getCharacter(pos)
            while ch in ' \t\n\r':
                ch, pos = self.getCharacter(pos)
            pos = pos+1

        # Done if there is no new call tip to set.
        if (not found):
            self.calltip = None
            return

        if not semanticsKnown:
            inKeyword = True;

        ch, rpos = self.getCharacterRight(rpos)
        while (ch != '\0'):
            if ch == ',':
                if inKeyword:
                    break
                inKeyword = True
            elif ch == '=':
                if not inKeyword:
                    break
                inKeyword = False
            elif (ch == '('):
                depth = 1
                # Ignore everything back to the start of the corresponding
                # parenthesis.
                ch, rpos = self.getCharacterRight(rpos)
                while (ch != '\0') :
                    if (ch == '('):
                        depth += 1
                    elif ch == ')' :
                        depth -= 1
                        if depth == 0 :
                            break
                    ch, rpos = self.getCharacterRight(rpos)
            elif (ch == ')'):
                break
            elif (ch == '\n' or ch == '\r'):
                multiline = True
            ch, rpos = self.getCharacterRight(rpos)
   

        # Cancel any existing call tip.
        #self.SendScintilla(QsciScintilla.SCI_CALLTIPCANCEL);

        context, pos, ctPos = self.apiContext(pos)

        fileText = self.text()
        
        if (not context):
            self.calltip = None
            return

        contextPrefix = fileText[pos:ctPos+len(context[-1])]

        # // The last word is complete, not partial.
        context = context + ['']

        ct_shifts = []
        ct_entries = self.lexer().apis().callTips(context, 0, self.callTipsStyle(), ct_shifts);

        if (not ct_entries):
            self.calltip = None
            return

        ct = None
        for ent in ct_entries:
            if ent.startswith(contextPrefix) :
                ct = ent
                break

        if not ct :
            self.calltip = None
            return

        #ct_ba = ct.toLatin1()
        #cts = ct_ba.data()
        cts = (ct + '\0').encode('latin-1')

        #if multiline:
        #    #self.SendScintilla(QsciScintilla.SCI_CALLTIPSHOW, rpos, cts);
        #    widgetPos = rpos
        #else:
            #self.SendScintilla(QsciScintilla.SCI_CALLTIPSHOW, self.adjustedCallTipPosition(ctPos, 0), cts);
        widgetPos = self.adjustedCallTipPosition(ctPos, 0)

        #parse context
        fileText = self.text()
        #parlist_text = fileText[lpos:rpos]
        # parlist = self.stripParens(parlist_text)
        # params = parlist.split(',')
        # parpos = lpos
        # for p in params:
        pos = lpos
        pdict = {}
        while(pos <= rpos):
            pos, p = self.read_param(pos, fileText, len(fileText))
            #p = self.stripParens(p)
            p = p.partition('=')
            p0 = p[0].strip(' \n\t\r')
            if p0 and not ' ' in p0:
                pdict[p0] = (p[2].strip(' \n\t\r'), pos-len(p[2])-1, p[2])

        self.calltip = Calltip(self, lpos)
        #self.calltip.setStyleSheet("""
        #    QWidget{
        #        background-color: black;
        #        border-color: red;
        #        border-style: solid;
        #        border-width: 1px;
        #        border-radius: 0px;
        #        font: bold 14px;
        #        color: red;
        #        padding: 0px;
        #        font-family: "Candara";
        #        font-size: 14px;
        #    }
        #    QLabel{
        #        font-size: 24px;
        #         border-width: 0px;
        #    }
        #    """
        #    )
        
        self.calltip.setWindowFlags(Qt.FramelessWindowHint);
        #self.setWindowFlags(Qt.WindowStaysOnTopHint)
        #self.setWindowFlags(Qt.WindowTransparentForInput)
        #self.setWindowFlags(Qt.WindowDoesNotAcceptFocus)
        self.calltip.setAttribute(Qt.WA_ShowWithoutActivating)
        self.setFocusPolicy(Qt.NoFocus)

        x = self.SendScintilla(QsciScintilla.SCI_POINTXFROMPOSITION, 0, widgetPos);
        y = self.SendScintilla(QsciScintilla.SCI_POINTYFROMPOSITION, 0, widgetPos);
        #wing = self.geometry()
        screenPos = self.mapToGlobal(QPoint(x,y))

        sct = ct[ct.index('(')+1 : -1]
        try:
            self.calltip.highlight( ast.literal_eval( sct ),  pdict)
        except SyntaxError:
            print('Syntax error in calltip:\n')
            print(sct)

        self.calltip.adjustSize()
        g = self.calltip.geometry()
        #self.calltip.setGeometry( topLeft.x() + x + self.marginWidth(1), topLeft.y() + y - g.height(), g.width(), g.height())
        self.calltip.setGeometry( screenPos.x() + self.marginWidth(0), screenPos.y() - g.height(), g.width(), g.height())
        #self.calltip.setFocus()
        self.calltip.show()

    def jumpTo(self, pos) :
        line, index = self.lineIndexFromPosition(pos)
        self.setCursorPosition( line, index )
        self.calltip = None

    def addKeyword(self, code, pos):
        line, index = self.lineIndexFromPosition(pos)
        headerline = self.text(line)
        indent = headerline[:headerline.find(headerline.strip())]
        self.insertAt('\n' + indent + '\t\t' + code, line, index)
        self.calltip = None

    def saveTriangles(self, trianglesPerSpline, code):
        first = True
        for key in trianglesPerSpline:
            triangles = trianglesPerSpline[key]
            i = 0
            while i < len(triangles):
                if i > 0 or not first:
                    code += ", "
                code += "{"
                code += "'x': {xcoord}, 'y': {ycoord}".format(xcoord=triangles[i], ycoord=triangles[i+1])
                code += "}"
                i+=2
            if first:
                first = False
        return code

    def saveControlpoints(self, controlPoints, code):
        first_s = True
        for s in controlPoints:
            if first_s:
                first_s = False
            else:
                code += ", "
            code += "["
            first = True
            for p in s:
                if first:
                    first = False
                else:
                    code += ", "
                code += "{xcoord}, {ycoord}".format(xcoord=p[0], ycoord=p[1])
            code += "]"
        
        return code

    def savePolymask(self, trianglesPerSpline, controlPoints, curPos, basePos):
        if trianglesPerSpline == None:
            return

        indent = ""
        if curPos != basePos:
            start_line, _ = self.lineIndexFromPosition(curPos)
            headerline = self.text(start_line)
            indent = headerline[:headerline.find(headerline.strip())]
        newline = '\n' + indent + '\t'
        code = "polygonMask = {" + newline + "'triangles': ["
        code = self.saveTriangles(trianglesPerSpline, code)
        code += "]," + newline + "'controlPoints': ["
        code = self.saveControlpoints(controlPoints, code)
        code += "]" + newline + "},"
        if curPos != basePos:
            file_text = self.text()
            pos, _ = self.read_param(curPos, file_text, len(file_text))
            end_line, _ = self.lineIndexFromPosition(pos)
            self.setSelection(start_line, 0, end_line+1, 0)
            self.replaceSelectedText(indent + code + '\n')
            self.calltip = None
        else:
            self.addKeyword(code, curPos)

    def save(self):
        print('Saving sequence:')
        print(self.sequencePath)
        self.file = open(self.sequencePath, 'w')
        self.file.write(self.text())
        self.file.close()

    def focusOutEvent(self, e):
        if self.calltip and not self.calltip.isActiveWindow():
        #if self.calltip:
            self.calltip = None
        super().focusOutEvent(e)

    def focusInEvent(self, e):
        #self.callTip()
        super().focusInEvent(e)

    def indicatePreviewProgress(self, progline):
        self.clearIndicatorRange(0, 0, self.lines(), -1, self.previewTrackIndicator)
        self.fillIndicatorRange(progline-1, 0, progline, 0, self.previewTrackIndicator)

    def minimumSizeHint(self):
        return QSize(512, 512)