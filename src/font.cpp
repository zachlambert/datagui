#include "datagui/font.hpp"
#include <unordered_map>
#include <vector>
#include <filesystem>
#include <algorithm>


namespace datagui {

std::optional<std::string> find_font_path(Font font)
{
    static const std::unordered_map<Font, std::string> names = {
        { Font::DejaVuSans, "DejaVuSans" },
        { Font::DejaVuSerif, "DejaVuSerif" },
        { Font::DejaVuSansMono, "DejaVuSansMono" }
    };
    // Crash if the above list is missing an entry for a given font
    std::string font_name = names.at(font);

    std::vector<std::filesystem::path> candidates;

    std::vector<std::string> paths = {
        "/usr/share/fonts"
    };

    for (const auto& path: paths) {
        for (
            auto iter = std::filesystem::recursive_directory_iterator(path);
            iter != std::filesystem::recursive_directory_iterator();
            ++iter)
        {
            if (iter->is_directory()) continue;
            if (iter->path().extension() != ".ttf") continue;

            std::string name = iter->path().stem();
            if (name.find(font_name) == std::string::npos) continue;

            candidates.push_back(iter->path());
        }
    }

    if (candidates.empty()) {
        return std::nullopt;
    }

    std::sort(candidates.begin(), candidates.end(),
        // Return true if a.stem < b.stem
        [](const std::filesystem::path& a, const std::filesystem::path& b) -> bool {
            return a.stem().string().size() < b.stem().string().size();
        }
    );
    return candidates.front();
}

} // namespace datagui
