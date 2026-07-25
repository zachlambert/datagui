#pragma once

#include "datagui/asset/image.hpp"
#include "datagui/render/glyph_2d_vertex.hpp"
#include "datagui/render/image_2d_vertex.hpp"
#include "datagui/render/shape_2d_instance.hpp"
#include <map>

namespace dgui {

/*
 * Draw calls can be categorized as:
 * - Geometry: queue_box, queue_rect, etc...
 * - Image: queue_image
 * - Text: queue_text
 * - Scene: queue_scene (2d or 3d scene)
 *
 * A "group" is defined as a set of geometry/text/image calls where it is
 * guaranteed that all calls can be ordered via geometry < image < text.
 *
 * This occurs whenever traversing a GUI tree with the following conditions:
 * - All child content is constrained to a box that doesn't overlap any
 * text/images created by the current node.
 *     - This means that all child geometry can be rendered before the parent
 * text/images without affecting the output
 * - There is no overflow in the child content, such that the child content
 * won't overflow the box and invalidate the above constraint
 * - The parent always renders text above images, if this matters at all
 *
 * So essentially a new group needs to be created when:
 * - A container has overflow in the child content and needs to add a scroll bar
 * - A container has floating children which are expected to overlap the
 * surroundings
 *
 * Each new group constructs a new pair of geometry/text/image batches and
 * optionally has a new mask. Due to the overlap condition above, all draw calls
 * within a group should share a mask.
 *
 * Each group must also be given a z_index which determines the ordering of
 * groups. Lower z indices are rendered first, all groups with the same z_index
 * are rendered in the order they were added. Z indices should only be used
 * where the call order doesn't match the render order, which only occurs for
 * floating elements.
 *
 * 2d and 3d scenes are always treated as their own groups.
 * - For 3d scenes, this starts with a single box render for the background,
 * then it switches to 3D rendering which uses depth filtering and no longer
 * cares about render order
 * - For 2d scenes, this uses the same geometry/image/text rendering, but can no
 * longer define general ordering rules. Instead a new batch/group will be
 * created when the render call type changes, to guarantee that the ordering is
 * preserved.
 */

class Scene3d;

struct DrawList {
  std::vector<Shape2dInstance> shape_2d_instances;
  std::vector<Glyph2dVertex> glyph_2d_vertices;
  std::vector<Image2dVertex> image_2d_vertices;

  struct GlyphGroup {
    size_t font_texture;
    Color color;
    size_t offset = 0;
    size_t count = 0;
    bool matches(int font_texture, const Color& color) {
      return font_texture == this->font_texture && color.equals(this->color);
    }
  };

  struct ImageGroup {
    // The image is retained so its texture stays alive until the draw list is
    // executed; texture() also serves as the grouping key.
    Image image;
    size_t offset = 0;
    size_t count = 0;
    bool matches(const Image& other) {
      return image.texture() == other.texture();
    }
  };

  struct Batch {
    Box2 mask;
    size_t shape_offset = 0;
    size_t shape_count = 0;
    std::vector<GlyphGroup> glyph_groups;
    std::vector<ImageGroup> image_groups;
    const Scene3d* scene_3d = nullptr;
  };
  std::map<int, std::vector<Batch>> batches;
};

} // namespace dgui
