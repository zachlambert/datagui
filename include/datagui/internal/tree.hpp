#pragma once

#include "datagui/geometry.hpp"
#include "datagui/internal/element.hpp"
#include "datagui/internal/renderers.hpp"
#include "datagui/internal/vector_map.hpp"
#include <functional>
#include <string>

/* High-level overview
 *
 *  A given GUI is structured as a tree of nodes.
 *  Each node stores the following bits of data:
 *  - Definition: Node key, element type and element data
 *  - Connectivity: Neighbour, parent, child nodes
 *  - Dimensions: Consists of "independent" and "dependent" values
 *    - fixed_size and dynamic_size are defined by the element type and user
 * configuration
 *    - origin and size are calculated by the tree after definition, since these
 * may depend on parent and/or children properties
 *  - State: Other state associated with the node
 *
 * All element-specific data and logic is handled by an ElementSystem
 * - Most importantly, each node contains an Element type (enum) and element
 *   index, where each element must have an associated ElementHandler, and
 *   internally stores a list of all the elements.
 *
 * The tree provides a "declarative" interface for building a tree, as follows:
 * tree.begin()
 *   tree.next("root");
 *   tree.down();
 *   tree.next("key1", ...);
 *   tree.up();
 * tree.end()
 *
 * The series of statements above define what route you travel through the tree.
 * The important point is that:
 * - The tree persists it's structure and element state
 * - New nodes are created if a new key is visited that wasn't present
 *   previously
 * - If keys aren't revisited on iterating over neighbours, then they are
 * removed
 * - Overall, the result is that the tree can be dynamically updated, while not
 *   re-creating itself each time
 */

namespace datagui {

// Defines all element types possible in the library
enum class Element {
  Undefined,
  Button,
  Checkbox,
  LinearLayout,
  Selection,
  Option,
  Text,
  TextInput
};

// Used as a return value when the user queries the state
// of a given node
struct NodeState {
  bool held;
  bool focused;

  NodeState() : held(false), focused(false) {}
};

// Stores all (common) data relevant to a node
struct Node {
  // Definition
  std::string key;
  Element element;
  int element_index;

  // Connectivity
  int parent;
  int prev;
  int next;
  int first_child;
  int last_child;

  // Layout calculation
  Vecf fixed_size;
  Vecf dynamic_size;
  bool floating;
  Vecf origin;
  Vecf size;

  // State
  bool changed;
  bool hidden;

  Node(const std::string& key, Element element, int parent) :
      key(key),
      element(element),
      element_index(-1),
      parent(parent),
      prev(-1),
      next(-1),
      first_child(-1),
      last_child(-1),
      fixed_size(Vecf::Zero()),
      dynamic_size(Vecf::Zero()),
      floating(false),
      origin(Vecf::Zero()),
      size(Vecf::Zero()),
      changed(true),
      hidden(false) {}
};

class Tree {
public:
  using construct_element_t = std::function<int()>;

  Tree();
  void register_element(Element element, ElementSystem& system);

  // Define the tree
  void begin();
  int next(const std::string& key, Element element, const construct_element_t& construct_element);
  void down();
  void up();
  void end(const Vecf& root_size);

  void render(Renderers& renderers, const Vecf& window_size);

  const Node& operator[](std::size_t i) const { return nodes[i]; }
  Node& operator[](std::size_t i) { return nodes[i]; }

  int root_node() const { return root_node_; }

  void mouse_press(const Vecf& mouse_pos);
  void mouse_release(const Vecf& mouse_pos);
  void focus_next(bool reverse = false);
  void focus_leave(bool success);

  int node_held() const { return node_held_; }
  int node_focused() const { return node_focused_; }

  void node_changed(Node& node);

  ElementSystem& get_elements(const Node& node);

private:
  NodeState node_state(int node) const;
  int create_node(const std::string& key, Element element, int parent, int prev);
  void remove_node(int root_node);

  std::vector<ElementSystem*> element_systems;

  VectorMap<Node> nodes;
  int root_node_;
  int parent;
  int current;
  std::vector<int> floating_nodes;

  int node_held_;
  int node_focused_;
};

} // namespace datagui
