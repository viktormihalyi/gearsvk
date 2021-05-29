#include "gtest/gtest.h"

#include "CommandLineFlag.hpp"


int main (int argc, char** argv)
{
    Utils::CommandLineFlag::MatchAll (argc, argv);

    ::testing::InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
