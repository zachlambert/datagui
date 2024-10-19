#pragma once

#include <string>
#include "datagui/internal/element.hpp"


namespace datagui {

class TextInput: public ElementInterface {
public:
    TextInput(
        const std::string& default_text,
        float max_width
    ):
        max_width(max_width),
        initial_text(default_text),
        text_(default_text),
        changed_(false)
    {}

    void calculate_size_components(
        const Style& style,
        const FontStructure& font,
        Node& node,
        const Tree& tree) const override;

    void render(
        const Style& style,
        const FontStructure& font,
        const Node& node,
        const NodeState& state,
        const TextSelection& selection,
        Renderers& renderers) const override;

    void revert() {
        text_ = initial_text;
    }

    void confirm() {
        if (initial_text != text_) {
            initial_text = text_;
            changed_ = true;
        }
    }

    bool changed() const {
        return changed_;
    }

    const std::string& text() const {
        return text_;
    }

    bool check_changed() {
        if (changed_) {
            changed_ = false;
            return true;
        }
        return false;
    }

    // TODO: Move text handling logic to class
public:
    float max_width;
    std::string initial_text;
    std::string text_;
    bool changed_;
};

} // namespace datagui
