/**************************************************************************//**
*	@file   main.cpp
*	@author Hunter Smith
*	@date   11/05/2023
*	@brief
*		Entry point for the Vulkan tutorial
******************************************************************************/

#include "HelloTriangleApplication.h"

#include <iostream>  // cout, endl
#include <stdexcept> // exception
#include <cstdlib>   // EXIT_FAILURE, EXIT_SUCCESS

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}