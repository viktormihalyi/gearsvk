from PyQt5.QtCore import *
from PyQt5.QtGui import *
from PyQt5.QtWidgets import QLabel, QDialog, QWidget, QGridLayout
from PyQt5.Qsci import QsciScintilla, QsciScintillaBase, QsciLexerPython, QsciAPIs

class QsciStrictAPIs(QsciAPIs):

    def __init__(self, lexer):
        super(QsciAPIs, self).__init__(lexer)

    def updateAutoCompletionList(self, context: [str], options: [str]):
        options = super().updateAutoCompletionList(context, options)
        return  [k for k in options if not '(' in k]
        
    