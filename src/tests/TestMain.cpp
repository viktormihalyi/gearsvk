#include <vulkan/vulkan.h>

#include "Instance.hpp"

#include <iostream>
#include <string>


int main ()
{
    try {
        Instance instance ({}, {});

        std::cerr << "OK" << std::endl;
        return EXIT_SUCCESS;
    } catch (...) {
        std::cerr << "FAILED" << std::endl;
        return EXIT_FAILURE;
    }
}