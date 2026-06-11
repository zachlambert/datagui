#pragma once

#include <string>

namespace dgui {

unsigned int create_program(
    const std::string& vs_code,
    const std::string& fs_code);

unsigned int create_program_gs(
    const std::string& vs_code,
    const std::string& fs_code,
    const std::string& gs_code);

} // namespace dgui
