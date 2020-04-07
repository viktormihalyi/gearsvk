import sys
import unittest
from unittest.mock import patch
sys.path.append("../")
import GearsUtils as utils
from numpy import array
from numpy import matmul

###########################################
#                Utilities                #
###########################################

def _assertEqualMatrix(self, m1, m2):
    i = 0
    for r in m1:
        j = 0
        for c in r:
            self.assertEqual(round(c, 5), round(m2[i][j], 5))
            j = j + 1
        i = i + 1

testIdentity3D = [[1,0,0],
                  [0,1,0],
                  [0,0,1]]

###########################################
#          Matrix inverse tests           #
###########################################

class TestMatrixInverse(unittest.TestCase):
    def test_Inverse_Square_Matrix(self):
        mtx = [[2,0,0],
               [0,2,0],
               [5,5,2]]
        imtx = [[0.5,0,0],
                [0,0.5,0],
                [-1.25,-1.25,0.5]]
        _assertEqualMatrix(self, utils.calculateInverseMatrix(mtx), imtx)
    def test_PseudoInverse_Square_Matrix(self):
        mtx = [[1,0,0,1],
               [2,1,0,1],
               [0,0,1,1]]
        imtx = [[0.25,0.25,-0.25],
                [-1.25,0.75,0.25],
                [-0.75,0.25,0.75],
                [0.75, -0.25, 0.25]]
        inv = utils.calculateInverseMatrix(mtx)
        _assertEqualMatrix(self, inv, imtx)
        pid = list(matmul(mtx, inv))
        pidT = []
        for item in list(matmul(mtx, inv).transpose()):
            pidT.append(list(item))
        _assertEqualMatrix(self, pid, pidT)

###########################################
#        Register converter tests         #
###########################################

class TestRegisterConverter(unittest.TestCase):
    def test_toRGB_Registration(self):
        utils.registerToRGBConverter("testId", testIdentity3D)
        _assertEqualMatrix(self, utils.getConverterByName("testId"), testIdentity3D)
        utils.resetRegisteredConverters()

    def test_fromRGB_Registration(self):
        utils.registerFromRGBConverter("testId", testIdentity3D)
        _assertEqualMatrix(self, utils.getConverterByName("testId", False), testIdentity3D)
        utils.resetRegisteredConverters()

    @patch('GearsUtils.print', create=True)
    def test_toRGB_Multiple_Registration(self, print_):
        utils.registerToRGBConverter("testId", testIdentity3D)
        utils.registerToRGBConverter("testId", testIdentity3D)
        print_.assert_called_with("Error: Color converter with name: testId already exists!")
        utils.resetRegisteredConverters()

    def test_toRGB_Multiple_Registration_Update(self):
        otherMtx = [[1,0,0,1],
                    [2,1,0,1],
                    [0,0,1,1]]
        utils.registerToRGBConverter("testId", testIdentity3D)
        utils.registerToRGBConverter("testId", otherMtx, True)
        _assertEqualMatrix(self, utils.getConverterByName("testId"), otherMtx)
        utils.resetRegisteredConverters()

    @patch('GearsUtils.print', create=True)
    def test_toRGB_With_No_str(self, print_):
        utils.registerFromRGBConverter([1,2,3], testIdentity3D)
        print_.assert_called_with("Error: Type of name argument for converter is not str!")
        utils.resetRegisteredConverters()

    @patch('GearsUtils.print', create=True)
    def test_toRGB_With_Tuple(self, print_):
        utils.registerToRGBConverter("testId", (1,0,0))
        print_.assert_called_with("Error: Type of converter or elements of it is not list!")
        utils.resetRegisteredConverters()

    @patch('GearsUtils.print', create=True)
    def test_toRGB_With_List(self, print_):
        utils.registerToRGBConverter("testId", [1,0,0])
        print_.assert_called_with("Error: Type of converter or elements of it is not list!")
        utils.resetRegisteredConverters()

    @patch('GearsUtils.print', create=True)
    def test_toRGB_With_Wrong_Matrix(self, print_):
        utils.registerToRGBConverter("testId", [[1,2,3]])
        print_.assert_called_with("Error: The dimension of color converter is invalid!")
        utils.resetRegisteredConverters()

###########################################
#            Conversion tests             #
###########################################

class TestConvertColorToRGB(unittest.TestCase):
    def test_identity_with_tuple(self):
        color = (1,0,0)
        self.assertEqual(utils.convertColorToRGB(color, testIdentity3D), color)
    
    def test_identity_with_array(self):
        color = [1,0,0]
        self.assertEqual(utils.convertColorToRGB(color, testIdentity3D), color)

    def test_red_abs(self):
        conv = [[0,0,0],
                [0,1,0],
                [0,0,1]]
        color = (1,1,1)
        self.assertEqual(utils.convertColorToRGB(color, conv), (0,1,1))

    def test_five_element_color(self):
        conv = [[0.5,0,0.5,0,0],
                [0.5,0,0,0.5,0],
                [0,0.5,0,0,0.5]]
        color = (1,0,1,0,1)
        self.assertEqual(utils.convertColorToRGB(color, conv), (1,0.5,0.5))

    @patch('GearsUtils.print', create=True)
    def test_five_element_color_wrong_color(self, print_):
        conv = [[0.4,0.1,0.5],
                [0,0.5,0.5],
                [0.5,0,0.2],
                [0,1,0],
                [1,0,0]]
        color = "1, 0, 1"
        self.assertEqual(utils.convertColorToRGB(color, conv), color)
        print_.assert_called_with("Error: Wrong color type! Type has to by tuple or list and not <class 'str'>")

    @patch('GearsUtils.print', create=True)
    def test_five_element_color_wrong_matrix(self, print_):
        conv = [[0.4,0.1,0.5],
                [0,0.5,0.5],
                [0.5,0,0.2],
                [0,1,0],
                [1,0,0]]
        color = (1,0,1)
        self.assertEqual(utils.convertColorToRGB(color, conv), color)
        print_.assert_called_with("Error: Wrong converter dimension! Number of rows has to be 3 and number of columns has to be 3!")

    @patch('GearsUtils.print', create=True)
    def test_five_element_color_wrong_type(self, print_):
        conv = (1,0,1)
        color = (1,0,1)
        self.assertEqual(utils.convertColorToRGB(color, conv), color)
        print_.assert_called_with("Error: Wrong converter type! Type has to be list and not <class 'tuple'>")

class TestConvertColorFromRGB(unittest.TestCase):

    def test_five_element_color_wrong_matrix(self, print_):
        conv = [[0.4,0.1,0.5],
                [0,0.5,0.5],
                [0.5,0,0.2],
                [0,1,0],
                [1,0,0]]
        color = (1,0,1)
        self.assertEqual(utils.convertFromRGBColor(color, conv), (0.9,0.5,0.7,0,1))

    @patch('GearsUtils.print', create=True)
    def test_five_element_color_wrong_matrix(self, print_):
        conv = [[0.5,0,0.5,0,0],
                [0.5,0,0,0.5,0],
                [0,0.5,0,0,0.5]]
        color = (1,0,1,0,1)
        self.assertEqual(utils.convertFromRGBColor(color, conv), color)
        print_.assert_called_with("Error: Wrong converter dimension! Number of rows has to be 5 and number of columns has to be 3!")

###########################################
#                  Main                   #
###########################################

if __name__ == '__main__':
    unittest.main()