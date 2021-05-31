from PyQt5.QtWidgets import (QWidget, QSplitter, QGridLayout, QPushButton, QVBoxLayout, QListWidget, QFileDialog, QColorDialog)
from PyQt5.QtCore import (Qt)
from numpy import array as ar
from PolymaskGenerator.PolymaskGenerator import PolymaskGenerator, PolymaskChangeEvent

class PolymaskGeneratorWindow(QWidget):
    def __init__(self, splines = []):
        super().__init__()
        self.setMinimumSize(1628, 742) # 1600 plus margins
        self.setMaximumSize(1628, 742)
        self.setGeometry(100, 100, 1628, 742)

        layout = QGridLayout(self)
        self.setLayout(layout)
        self.setWindowTitle("Polymask Generator")

        panelStyleSheet = """
        .QWidget {
            border: 2px solid black;
            }
        """

        right_panel = QSplitter(self)
        right_panel.setChildrenCollapsible(False)

        generate_panel = QWidget(right_panel)
        generate_panel.setStyleSheet(panelStyleSheet)
        generate_layout = QVBoxLayout(generate_panel)
        generate_panel.setLayout(generate_layout)

        control_polygon_panel = QWidget(right_panel)
        control_polygon_panel.setStyleSheet(panelStyleSheet)
        control_polygon_layout = QVBoxLayout(control_polygon_panel)
        control_polygon_panel.setLayout(control_polygon_layout)

        control_splines_panel = QWidget(right_panel)
        control_splines_panel.setStyleSheet(panelStyleSheet)
        control_splines_layout = QVBoxLayout(control_splines_panel)
        control_splines_panel.setLayout(control_splines_layout)

        saveButton = QPushButton("Save", right_panel)
        saveButton.clicked.connect(self.save)
        generate_layout.addWidget(saveButton)

        generateButton = QPushButton("Show filled polygons", right_panel)
        generateButton.clicked.connect(self.generateTriangles)
        generate_layout.addWidget(generateButton)

        backgroundButton = QPushButton("Select background image", right_panel)
        backgroundButton.clicked.connect(self.setBackground)
        generate_layout.addWidget(backgroundButton)

        colorGrid = QWidget(right_panel)
        colorGrid.setStyleSheet("""
        .QWidget {
            border: 0px solid black;
            }
        """)
        colorLayout = QGridLayout(colorGrid)
        colorLayout.setColumnStretch(1,1)
        colorLayout.setSpacing(1)

        wnd = self
        splineColorButton = QPushButton("Set spline color", colorGrid)
        splineColorButton.clicked.connect(lambda: wnd.setSplineColor("splineColor"))
        colorLayout.addWidget(splineColorButton)
        splineColorButton = QPushButton("Set selected spline color", colorGrid)
        splineColorButton.clicked.connect(lambda: wnd.setSplineColor("selectedSplineColor"))
        colorLayout.addWidget(splineColorButton, 0, 1)
        splineColorButton = QPushButton("Set control point color", colorGrid)
        splineColorButton.clicked.connect(lambda: wnd.setSplineColor("CPColor"))
        colorLayout.addWidget(splineColorButton, 1, 0)
        splineColorButton = QPushButton("Set selected control point color", colorGrid)
        splineColorButton.clicked.connect(lambda: wnd.setSplineColor("selectedCPColor"))
        colorLayout.addWidget(splineColorButton, 1, 1)

        colorGrid.setLayout(colorLayout)
        generate_layout.addWidget(colorGrid)
        
        self.cp_list = QListWidget()
        self.cp_list.setFixedHeight(100)
        self.cp_list.currentItemChanged.connect(self.currentPointChanged)
        control_polygon_layout.addWidget(self.cp_list)

        addBeforeButton = QPushButton("Add Controlpoint before selected", right_panel)
        addBeforeButton.clicked.connect(self.addBefore)
        control_polygon_layout.addWidget(addBeforeButton)

        addAfterButton = QPushButton("Add Controlpoint after selected", right_panel)
        addAfterButton.clicked.connect(self.addAfter)
        control_polygon_layout.addWidget(addAfterButton)

        removeButton = QPushButton("Remove selected Controlpoint", right_panel)
        removeButton.clicked.connect(self.removeCurrent)
        control_polygon_layout.addWidget(removeButton)

        self.curve_list = QListWidget()
        self.curve_list.setFixedHeight(100)

        splines = self.set_control_points(splines)
        
        self.curve_list.currentItemChanged.connect(self.currentSplineChanged)
        control_splines_layout.addWidget(self.curve_list)

        addSplineButton = QPushButton("Add New Spline", right_panel)
        addSplineButton.clicked.connect(self.addSpline)
        control_splines_layout.addWidget(addSplineButton)

        removeSplineButton = QPushButton("Remove selected Spline", right_panel)
        removeSplineButton.clicked.connect(self.removeSpline)
        control_splines_layout.addWidget(removeSplineButton)

        right_panel.setOrientation(Qt.Vertical)
        right_panel.addWidget(generate_panel)
        right_panel.addWidget(control_polygon_panel)
        right_panel.addWidget(control_splines_panel)
        
        self.polymaskGenerator = PolymaskGenerator(self, self.winId(), splines, self.dataChanged)
        layout.addWidget(self.polymaskGenerator)
        layout.setColumnStretch(0, 4)

        self.cp_list.setCurrentRow(0)
        self.curve_list.setCurrentRow(0)

        layout.addWidget(right_panel, 0, 1)
        layout.setColumnStretch(1, 1)

    def set_control_points(self, splines):
        self.cp_list.clear()
        self.curve_list.clear()
        if len(splines) == 0:
            controlPoints = [
                ar([-0.25, -0.25]),
                ar([-0.25, 0.25]),
                ar([0.25, 0.25]),
                ar([0.25, -0.25])
            ]

            splines = [controlPoints]

        idx = 0
        for npcp in splines[0]:
            cp = npcp.tolist()
            if len(cp) < 2:
                print("Error: controlPoint size less then 2")
                continue
            self.cp_list.addItem(self.pointToString(cp))
            idx += 1

        for idx in range(len(splines)):
            self.curve_list.addItem("Spline " + str(idx))

        return splines

    def pointToString(self, point):
        return "( " + "{:.4f}".format(point[0]) + ", " + "{:.4f}".format(point[1]) + " )"

    def save(self):
        if self.generateTriangles():
            self.saveFunction(self.polymaskGenerator.fill, self.polymaskGenerator.splines)
            self.close()

    def addBefore(self):
        self.polymaskGenerator.addBefore(self.cp_list.currentRow())

    def addAfter(self):
        self.polymaskGenerator.addAfter(self.cp_list.currentRow())

    def removeCurrent(self):
        self.polymaskGenerator.removeCurrent(self.cp_list.currentRow())

    def generateTriangles(self):
        return self.polymaskGenerator.generateTriangles()

    def set(self, saveFunction, splines):
        self.saveFunction = saveFunction
        if len(splines) > 0:
            self.set_control_points(splines)
            self.polymaskGenerator.splines = splines

            self.cp_list.setCurrentRow(0)
            self.curve_list.setCurrentRow(0)

    def dataChanged(self, type, data):
        if type == PolymaskChangeEvent.SelectionChanged:
            if self.curve_list.currentRow() != data["s_idx"]:
                self.changeSpline(data["s_idx"])
            self.cp_list.setCurrentRow(data["idx"])
        elif type == PolymaskChangeEvent.ItemChanged:
            self.cp_list.item(data["idx"]).setText(self.pointToString(data["value"]))
        elif type == PolymaskChangeEvent.ItemAdded:
            self.cp_list.insertItem(data["idx"], self.pointToString(data["value"]))
        elif type == PolymaskChangeEvent.ItemRemoved:
            self.cp_list.takeItem(data["idx"])

    def changeSpline(self, idx):
        self.curve_list.setCurrentRow(idx)
        self.cp_list.clear()
        for s in self.polymaskGenerator.splines[idx]:
            self.cp_list.addItem(self.pointToString(s.tolist()))

    def currentPointChanged(self):
        self.polymaskGenerator.currentPointChanged(self.cp_list.currentRow())

    def closeEvent(self, event):
        self.saveFunction = None
        event.accept()

    def addSpline(self):
        points = [[-0.25, -0.25],
                  [-0.25, 0.25],
                  [0.25, 0.25],
                  [0.25, -0.25]]
        self.curve_list.addItem("Spline " + str(len(self.polymaskGenerator.splines)))
        self.polymaskGenerator.addSpline(points)

    def removeSpline(self):
        current = self.curve_list.currentRow()
        if current >= 0:
            self.curve_list.takeItem(current)
            self.polymaskGenerator.removeSpline(current, self.curve_list.currentRow())

    def currentSplineChanged(self):
        self.polymaskGenerator.currentSplineChanged(self.curve_list.currentRow())

    def setBackground(self):
        fname = QFileDialog.getOpenFileName(self, 'Select Background Image', "","Image files(*.jpg *.png *.gif *.bmp);;All Files (*)")
        self.polymaskGenerator.setBackground(fname[0])

    def setSplineColor(self, obj):
        color = QColorDialog().getColor()
        self.polymaskGenerator.setSplineColor(obj, color.red(), color.green(), color.blue())