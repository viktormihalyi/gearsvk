import math
from SequenceError import SequenceError

def processDirection(direction, tb) :
    if(direction == 'east') :
        return 0.0
    elif(direction == 'northeast') :
        return math.pi * 0.25
    elif(direction == 'north') :
        return math.pi * 0.5
    elif(direction == 'northwest') :
        return math.pi * 0.75
    elif(direction == 'west') :
        return math.pi
    elif(direction == 'southwest') :
        return math.pi * 1.25
    elif(direction == 'south') :
        return math.pi * 1.5
    elif(direction == 'southeast') :
        return math.pi * 1.75
    elif isinstance(direction, str) :
        raise SequenceError('Unknown direction name ' + direction, tb)
    return direction

def processColor(color, tb) :
    if(color == 'red') :
        return (1, 0, 0)
    elif(color == 'green') :
        return (0, 1, 0)
    elif(color == 'blue') :
        return (0, 0, 1)
    elif(color == 'yellow') :
        return (1, 1, 0)
    elif(color == 'magenta') :
        return (1, 0, 1)
    elif(color == 'cyan') :
        return (0, 1, 1)
    elif(color == 'white') :
        return (1, 1, 1)
    elif(color == 'black') :
        return (0, 0, 0)
    elif(color == 'grey') :
        return (0.5, 0.5, 0.5)
    elif isinstance(color, str) :
        raise SequenceError('Unknown color name ' + color, tb)
    if isinstance(color, (int, float)) :
        return (color, color, color)
    return color

def isGrey(color):
    try:
        if max(color) - min(color) < 0.03:
            return True
    except:
        pass
    return False


