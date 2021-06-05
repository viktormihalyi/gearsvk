#include "Response.h"
#include "Sequence.h"

#include <algorithm>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <sstream>


Response::Response ()
    : question ("Pizza?"), loop (false), duration (1), startingFrame (0)
{
}


void Response::addButton (std::string label, float x, float y, float w, float h, uint32_t key, bool visible)
{
    Button b = {label, x, y, w, h, key, visible};
    buttons.push_back (b);
}