#pragma once

#include <string>

namespace datagui {

unsigned int create_program(
    const std::string& vs_code,
    const std::string& fs_code);

unsigned int create_program_gs(
    const std::string& vs_code,
    const std::string& fs_code,
    const std::string& gs_code);

} // namespace datagui
