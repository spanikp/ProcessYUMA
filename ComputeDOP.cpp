#include <iostream>
#include <fstream>
#include <array>
#include <string>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <cstdio>
#include <exception>

#include <Triple.hpp>
#include <PRSolution.hpp>
// #include "YumaData.hpp"
// #include "YumaStream.hpp"
// #include "YumaBase.hpp"
// #include "GPSWeekZcount.hpp"
// #include "TimeCorrection.hpp"
// #include "SystemTime.hpp"
// #include "GPSWeekSecond.hpp"

std::vector<std::string> string_split(const std::string s, const char delimiter) {
    std::vector<std::string> s_split;
    std::stringstream ss(s);
    while (ss.good()) {
        std::string substr;
        getline(ss,substr,delimiter);
        s_split.push_back(substr);
    }

    return s_split;
}

gpstk::Triple str_to_recPos(const std::string& s) {
    auto v = string_split(s,',');
    if (v.size() == 3) {
        double x, z, y; 
        x = std::stod(v[0]);
        y = std::stod(v[1]);
        z = std::stod(v[2]);
        return gpstk::Triple(x,y,z);
    } else {
        throw std::invalid_argument("Invalid format of receiver position, 3-element vector delimitted by commas is required!");
    }
}


int main( int argc, char* argv[] ) {

    // Parsing input arguments
    if ( argc != 3 ) {
        std::cout << "Incorrect number of arguments!" << std::endl;
        return EXIT_FAILURE;
    }

    std::filesystem::path input_file;
    gpstk::Triple recPos;
    try {
        recPos = str_to_recPos(std::string(argv[1]));
        input_file = std::filesystem::canonical(argv[2]);
    } catch (const std::exception& e) {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;

}