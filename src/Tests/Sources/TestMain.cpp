﻿#include "gtest/gtest.h"

#include "Utils/CommandLineFlag.hpp"


int main (int argc, char** argv)
{
    Utils::CommandLineFlag::MatchAll (argc, argv, false);

    ::testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
