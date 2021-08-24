#include <iostream>
#include <array>
#include <vector>
#include <sstream>
#include <fstream>
#include <optional>
#include "parsed_io.h"
#include "wrapper.h"

// Constants
/// Template pathfile
static const std::string template_filepath = "../template/template.sv";

// Functions
/// Load template
std::optional<std::vector<std::string>> load_template(std::string filepath){
    std::ifstream template_file(filepath);
    if (template_file.fail()) { std::cout << "ERROR: load_template: Failed to open template file\n"; return {}; }
    std::vector<std::string> ret_vec;
    std::string temp_string;
    while (std::getline(template_file, temp_string)){
        ret_vec.push_back(temp_string);
    }

    return ret_vec;
}