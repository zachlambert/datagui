#pragma once

#include <vector>
#include "datagui/geometry.hpp"
#include "datagui/color.hpp"


namespace datagui {

class Renderer {
public:
    Renderer();
    void init();
    void queue_box(int depth, const Boxi& box, const Color& color);
    void render(const Vecf& viewport_size);

private:
    struct BoxCommand {
        int depth;
        Boxi box;
        Color color;
    };

    struct {
        // Shader
        unsigned int program_id;
        // Uniforms
        unsigned int uniform_viewport_size;
        // Array/buffer objects
        unsigned int VAO, static_VBO, instance_VBO;
    } gl_data;

    struct Element {
        Vecf offset;
        Vecf size;
        float depth;
        float radius;
        float border_width;
        Color bg_color;
        Color border_color;
    };
    std::vector<Element> elements;
};

} // namespace datagui
