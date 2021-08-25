#ifndef _IO_H_
#define _IO_H_

#include <iostream>
#include <array>
#include <vector>
#include <sstream>
#include <fstream>
#include <optional>
#include "parsed_io.h"
#include "wrapper.h"

// Defines
//#define __use_stdin__ // for using stdin to retrieve parsed data instead of loading from a file

// Constants
/// Filepaths
static const std::string parsed_filepath =   "../inputs/parser1.out";   // parsed data pathfile
static const std::string template_filepath = "../template/template.sv"; // Template pathfile
static const std::string wrapper_filepath =  "../outputs/wrapper1.sv";  // generated wrapper pathfile

// IO Functions
/// IO Functions for template loading
namespace w_template {
    // Loads a template file into a vector of strings (by line)
    std::optional<std::vector<std::string>> from_file(std::string filepath){
        std::ifstream template_file(filepath);
        if (template_file.fail()) { std::cout << "ERROR: w_template::frome_file: Failed to open template file\n"; return {}; }
        std::vector<std::string> ret_vec;
        std::string temp_string;
        while (std::getline(template_file, temp_string)){
            ret_vec.push_back(temp_string);
        }

        return ret_vec;
    }
}

/// IO Functions for parsed data loading
namespace parsed_IO::parsed_data {
    // Loads parsed_data from a file into a string, then uses from_string to convert to two parsed_data_t,
    //      one for inputs and one for outputs
    std::array<parsed_data_t, 2> from_file(
        const std::string &filepath
    ){
        // Open file
        std::ifstream in_file;
        in_file.open(filepath.c_str());
        if (in_file.fail()) {
            std::cout << "ERROR: parsed_IO::parsed_data: could not open input file - {" << filepath << "}\n";
        }

        // fstream to string
        std::string pass_string( (std::istreambuf_iterator<char>(in_file)), (std::istreambuf_iterator<char>()) );
        in_file.close();

        return from_string(pass_string);
    }

    // Loads parsed_data from stdin into a string, then uses from_string to convert to two parsed_data_t,
    //      one for inputs and one for outputs
    std::array<parsed_data_t, 2> from_stdin(){
        // Take in stdin into string
        std::string temp_string;
        std::string pass_string = "";
        while (std::cin >> temp_string){
            pass_string += temp_string;
            pass_string += " ";
        }
        pass_string.pop_back();

        return from_string(pass_string);
    }
}

#endif // _IO_H_