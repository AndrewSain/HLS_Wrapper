#include "template.h"
#include "parsed_io.h"
#include "wrapper.h"
#include <iterator>
#include <sstream>
#include <fstream>
#include <optional>

//#define __use_stdin__

// Function Declarations
template<typename GEN>
std::vector<GEN> move_to_new_vec(
        std::vector<GEN> &vec1, std::vector<GEN> &vec2
);


int main() {
    // Usefull Lambdas
    auto lmbd_adv_all = [](int x, auto& ... iter){ (std::advance(iter, x), ...); };
    auto lmbd_make_variadic = [&](auto& func, auto& ... args){ (func(args), ...); };


    // Get parsed IO
#ifndef __use_stdin__
    auto data = parsed_IO::parsed_data::from_file("../inputs/parser1.out");
#else
    auto ret = parsed_IO::parsed_data::from_stdin();
#endif
    auto parsed_IO_in = parsed_IO::parsed_data::to_parsed_IO(data[0], parsed_IO::INPUT_REG);
    auto parsed_IO_out = parsed_IO::parsed_data::to_parsed_IO(data[1], parsed_IO::OUTPUT_REG);
    auto parsed_IO = move_to_new_vec(parsed_IO_in, parsed_IO_out);

    
    // Create necessary strings for output generation
    std::vector<std::string> upper_clk_rst;
    std::vector<std::string> conn_clk_rst;

    /// Handle the clock and reset
    //// Check that ap_clk and ap_rst exist
    {
        bool has_clk = false;
        bool has_rst = false;
        for (const auto& io : parsed_IO) {
            if ( io.io_name == "ap_clk" ) { has_clk = true; }
            if ( io.io_name == "ap_rst" ) { has_rst = true; }
        }
        if ( ( has_clk != true ) || ( has_rst != true ) ) {
            std::cout << "ERROR: Main: Missing \"ap_clk\" and/or \"ap_rst\" as io from parsed io\n";
            return -1;
        }
    }

    //// Create wrapper module IO instantiaions for clock and reset
    upper_clk_rst.push_back("input wire clock,\n");
    upper_clk_rst.push_back("input wire reset,\n");

    //// Create connections for wrapper to tl for clock and reset
    conn_clk_rst.push_back(".ap_clk(clock),\n");
    conn_clk_rst.push_back(".ap_rst(reset),\n");

    /// Create wrapper module io for bus wires/registers
    std::vector<std::string> upper_bus_wires;
    for (const auto& io : parsed_IO) {
        if ((io.io_name != "ap_clk") && (io.io_name != "ap_rst")) {
            upper_bus_wires.push_back( write_upper_bus_wires(io.io_name, io.io_type, io.io_bus) );
        }
    }
    
    /// Create wrapper module io for control wires
    std::vector<std::string> upper_ctrl_wires;
    for (const auto& io : parsed_IO) {
        if ((io.io_name != "ap_clk") && (io.io_name != "ap_rst")) {
            upper_ctrl_wires.push_back( write_upper_ctrl_wires(io.io_name, io.io_type) );
        }
    }
    upper_ctrl_wires.back().pop_back();
    upper_ctrl_wires.back().pop_back();
    upper_ctrl_wires.back().push_back('\n');

    /// STUFFFFFF
    std::vector<std::string> io_reg_decl; /// Create register instantiations for the wrapper
    std::vector<std::string> wire_connection_decl; /// Create Connection Wires declarations
    std::vector<std::vector<std::string>> io_names_vec; /// Creating names of inputs and outputs of top level file (i.e. "phi_0_0_0_V"..."reg_phi_5_8_1_V")
    std::vector<std::vector<std::string>> expl_reg_name_vec; /// Create explicit register array names (i.e. "reg_phi[0][0][0]"..."reg_phi[5][8][1]")
    std::vector<std::vector<std::string>> conn_wires_vec; /// Create connection wires
    std::vector<std::string> shift_in_logic; /// Create shift in logic
    std::vector<std::string> shift_out_logic; /// Create shift out logic
    std::vector<std::string> reset_logic; /// Creates the reset logic for IO registers + output busses
    for (const auto& io : parsed_IO) {
        if ((io.io_name != "ap_clk") && (io.io_name != "ap_rst")) {
            io_reg_decl.push_back(rolled_io_reg_decl(io.io_name, io.io_bus, io.io_dim));
            io_names_vec.push_back(write_name(io.io_name, io.io_dim));
            expl_reg_name_vec.push_back(write_expl_reg(io.io_name, io.io_dim));
            conn_wires_vec.push_back(write_connection_wires(io.io_name, io.io_dim));
            reset_logic.push_back(reg_reset_maker(io.io_name, io.io_dim, io.io_type));

            if (io.io_type == parsed_IO::INPUT_REG) {
                shift_in_logic.push_back(shift_in_maker(io.io_name, io.io_dim));
            } else if (io.io_type == parsed_IO::OUTPUT_REG) {
                wire_connection_decl.push_back(rolled_io_wire_decl(io.io_name, io.io_bus, io.io_dim));
                shift_out_logic.push_back(shift_out_maker(io.io_name, io.io_dim));
            }   
        }
    }

    /// connect top-level io names to expliciter register/wire array names (i.e. ".pho_0_0_0_1_V(wire_pho[0][0][0][1]),")
    std::vector<std::vector<std::string>> tl_io_connected;
    {
        auto itr0 = parsed_IO.begin();
        auto itr1 = io_names_vec.begin();
        auto itr2 = expl_reg_name_vec.begin();
        auto itr3 = conn_wires_vec.begin();
        for (; itr0 != parsed_IO.end(); std::advance(itr0, 1)){
            if (((*itr0).io_name != "ap_clk") && ((*itr0).io_name != "ap_rst")) {
                int checker;
                tl_io_connected.push_back(std::vector<std::string>());
                // Inputs (direct connection to output of registers)
                if ((*itr0).io_type == parsed_IO::reg_io_type::INPUT_REG) {
                    checker = write_connect_tl_io(*itr1, *itr2, (*itr0).io_dim, tl_io_connected.back());
                }
                // Outputs (connect to a intermediary wire before connecting to register (prevents multiple drivers))
                else {
                    checker = write_connect_tl_io(*itr1, *itr3, (*itr0).io_dim, tl_io_connected.back());
                }

                if (checker != 0) { 
                    std::cout << "\nERROR: Mismatch between number of top level io and wrapper connections!\n\n";
                    return -1;
                }
                lmbd_adv_all(1, itr1, itr2, itr3);
            }
        }
    }

    /// Create all the _ap_vld, and leave them unconnected
    std::vector<std::vector<std::string>> ap_vld_unconnected;
    {
        auto itr0 = parsed_IO.begin();
        auto itr1 = io_names_vec.begin();
        for (; (((itr0 != parsed_IO.end()) && (itr1 != io_names_vec.end()))); 
                std::advance(itr0, 1)
        ){
            if (((*itr0).io_name != "ap_clk") && ((*itr0).io_name != "ap_rst")) {
                // Output & has dimensions, create "_ap_vld" and connect it to nothing
                if (((*itr0).io_type == parsed_IO::OUTPUT_REG) && !((*itr0).io_dim.empty())){
                    ap_vld_unconnected.push_back(std::vector<std::string>());
                    for (auto itr1_sub = (*itr1).begin(); itr1_sub != (*itr1).end(); std::advance(itr1_sub, 1)) {
                        std::stringstream ss_entry;
                        ss_entry << "." << *itr1_sub << "_ap_vld(),\n";
                        ap_vld_unconnected.back().push_back(ss_entry.str());
                    }
                }
                std::advance(itr1, 1);
            }
        }
        // remove the comma at the end of the last connection entry
        ap_vld_unconnected.back().back().pop_back();
        ap_vld_unconnected.back().back().pop_back();
        ap_vld_unconnected.back().back().push_back('\n');
    }


    // load template
    std::optional<std::vector<std::string>> opt_template = load_template(template_filepath);
    std::vector<std::string> template_vec;
    try {
        template_vec = opt_template.value();
    } catch (const std::bad_optional_access& e) {
        std::cout << "ERROR: Main: No Template Loaded\n";
        return -1;
    }


    // output to file
    /// Construct output
    std::stringstream ss_output;
    auto lmdb_vec1_to_ss_output = [&ss_output](std::vector<auto> &dump_vec1){
        for (auto itr = dump_vec1.begin(); itr < dump_vec1.end(); std::advance(itr, 1)){
            ss_output << *itr;
        }
    };
    auto lmbd_vec2_to_ss_output = [&ss_output](std::vector<auto> &dump_vec2){
        for (auto itr = dump_vec2.begin(); itr != dump_vec2.end(); std::advance(itr, 1)) {
            for (auto itr_sub = (*itr).begin(); itr_sub != (*itr).end(); std::advance(itr_sub, 1)) {
                ss_output << *itr_sub;
            }
        }
    };
    auto lmbd_shift_to_ss_output = [&ss_output](
        std::vector<auto> &check_vec,
        std::vector<auto> &shift_i_vec,
        std::vector<auto> &shift_o_vec
    ){
        auto itr0 = check_vec.begin();
        auto itr1 = shift_i_vec.begin();
        auto itr2 = shift_o_vec.begin();
        for (; itr0 != check_vec.end(); std::advance(itr0, 1)) {
            if (((*itr0).io_name != "ap_clk") && ((*itr0).io_name != "ap_rst")) {
                if ((*itr0).io_type == parsed_IO::INPUT_REG) {
                    ss_output << "\n// SHIFT LOGIC - INPUT: [" << (*itr0).io_name << "]\n\n";
                    ss_output << *itr1;
                    std::advance(itr1, 1);
                } else if ((*itr0).io_type == parsed_IO::OUTPUT_REG) {
                    ss_output << "\n// SHIFT LOGIC - OUTPUT: [" << (*itr0).io_name << "]\n\n";
                    ss_output << *itr2;
                    std::advance(itr2, 1);
                }
            }
        }
    };

    for (const auto& io : template_vec) {
        if        (io == "//<<module_io>>") {
            ss_output << "\n// Module IO - Clock/Reset\n\n";
            lmdb_vec1_to_ss_output(upper_clk_rst);

            ss_output << "\n// Module IO - Bus Signals\n\n";
            lmdb_vec1_to_ss_output(upper_bus_wires);

            ss_output << "\n// Module IO - Control Signals\n\n";
            lmdb_vec1_to_ss_output(upper_ctrl_wires);

        } else if (io == "//<<reg_wire_dec>>") {
            ss_output << "\n// Register/Wire Declarations\n\n";
            lmbd_make_variadic(lmdb_vec1_to_ss_output, io_reg_decl, wire_connection_decl);

        } else if (io == "//<<tl_conn>>") {
            ss_output << "\n// IO Connections\n\n";
            lmbd_make_variadic(lmbd_vec2_to_ss_output, conn_clk_rst, tl_io_connected, ap_vld_unconnected);
            
        } else if (io == "//<<rst_logic>>") {
            ss_output << "\n// RESET LOGIC\n\n";
            lmdb_vec1_to_ss_output(reset_logic);
            
        } else if (io == "//<<shift_logic>>") {
            ss_output << "\n// SHIFT LOGIC\n\n";
            lmbd_shift_to_ss_output(parsed_IO, shift_in_logic, shift_out_logic);
            
        } else {
            ss_output << io << "\n";
        }
    }

    /*
    ss_output << "\n// Module IO - Clock/Reset\n\n";
    lmdb_vec1_to_ss_output(upper_clk_rst);

    ss_output << "\n// Module IO - Bus Signals\n\n";
    lmdb_vec1_to_ss_output(upper_bus_wires);

    ss_output << "\n// Module IO - Control Signals\n\n";
    lmdb_vec1_to_ss_output(upper_ctrl_wires);

    ss_output << "\n// Register/Wire Declarations\n\n";
    lmbd_make_variadic(lmdb_vec1_to_ss_output, io_reg_decl, wire_connection_decl);

    ss_output << "\n// IO Connections\n\n";
    lmbd_make_variadic(lmbd_vec2_to_ss_output, conn_clk_rst, tl_io_connected, ap_vld_unconnected);
    
    ss_output << "\n// SHIFT LOGIC\n\n";
    lmbd_shift_to_ss_output(parsed_IO, shift_in_logic, shift_out_logic);

    ss_output << "\n// RESET LOGIC\n\n";
    lmdb_vec1_to_ss_output(reset_logic);
    */

    /// Output the ss_output to the output file
    std::ofstream output_file(glob_filepath, std::ios::trunc);
    if (output_file.fail()) { std::cout << "Failed to open file\n"; return -1; }
    output_file << ss_output.rdbuf();

    return 0;
}


// Function Definitions
/// Moves the elements of two vectors to a new vector
///     <<side-effect>>: erases the input vectors
template<typename GEN>
std::vector<GEN> move_to_new_vec(
    std::vector<GEN> &vec1, std::vector<GEN> &vec2
){
    // Instantiate new vector
    std::vector<GEN> ret_vec = std::vector<GEN>();

    // Move vec1 to beginning of ret_vec, and move vec2 to after where vec1 has been moved
    ret_vec.insert(ret_vec.begin(), std::make_move_iterator(vec1.begin()), std::make_move_iterator(vec1.end()));
    ret_vec.insert(ret_vec.end(), std::make_move_iterator(vec2.begin()), std::make_move_iterator(vec2.end()));

    // Erase vec1 & vec2 (their elements have been moved!)
    vec1.erase(vec1.begin(), vec1.end());
    vec2.erase(vec2.begin(), vec2.end());

    return ret_vec;
}