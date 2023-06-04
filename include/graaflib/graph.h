#pragma once

#include <graaflib/types.h>

#include <memory>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>

namespace graaf {

namespace detail {

// Type trait to check if the type is a primitive numeric type
template <typename EDGE_T>
struct is_primitive_numeric;

template <typename EDGE_T>
inline constexpr bool is_primitive_numeric_v =
    is_primitive_numeric<EDGE_T>::value;

}  // namespace detail

enum class graph_spec { DIRECTED, UNDIRECTED };

template <typename VERTEX_T, typename EDGE_T, graph_spec GRAPH_SPEC_V>
class graph {
 public:
  using vertex_t = VERTEX_T;

  /**
   * To provide a common interface for weighted edges, user provided edge
   * classes can publically derive from weighted edge and optionally override
   * the get_weight method. If a primitive numeric type is passes as the edge
   * type, it is internally wrapped in a primitive_numeric_adapter, which
   * derives from weighted_edge.
   */
  using edge_t =
      std::conditional_t<detail::is_primitive_numeric_v<EDGE_T>,
                         std::shared_ptr<primitive_numeric_adapter<EDGE_T>>,
                         std::shared_ptr<EDGE_T>>;

  using vertices_t = std::unordered_set<vertex_id_t>;

  using vertex_id_to_vertex_t = std::unordered_map<vertex_id_t, VERTEX_T>;
  using edge_id_to_edge_t = std::unordered_map<edge_id_t, edge_t, edge_id_hash>;

  /**
   * Checks whether graph is directed.
   *
   * @return bool - Return true for directed graphs otherwise false
   */
  [[nodiscard]] constexpr bool is_directed() const {
    return GRAPH_SPEC_V == graph_spec::DIRECTED;
  }

  /**
   * Checks whether graph is undirected.
   *
   * @return bool - Returns true for undirected graphs otherwise false
   */
  [[nodiscard]] constexpr bool is_undirected() const {
    return GRAPH_SPEC_V == graph_spec::UNDIRECTED;
  }

  /**
   * Query the number of vertices
   *
   * @return size_t - Number of vertices
   */
  [[nodiscard]] std::size_t vertex_count() const noexcept;

  /**
   * Query the number of edges
   *
   * @return size_t - Number of edges
   */
  [[nodiscard]] std::size_t edge_count() const noexcept;

  /**
   * @brief Get the intrnal vertices
   *
   * @return const vertex_id_to_vertex_t& Map from vertex id to the user
   * provided vertex.
   */
  [[nodiscard]] const vertex_id_to_vertex_t& get_vertices() const noexcept {
    return vertices_;
  }

  /**
   * @brief Get the internal edges
   *
   * One thing to not here is that edges are internally stored as shared
   * pointers to either the user provided edge type or to
   * primitive_numeric_adapter.
   *
   * @return const edge_id_to_edge_t& Map from edge id to edge_t.
   */
  [[nodiscard]] const edge_id_to_edge_t& get_edges() const noexcept {
    return edges_;
  }

  /**
   * Checks whether a vertex with a given ID is contained in the graph.
   *
   * @param  vertex_id The ID of the vertex we want to check
   * @return boolean - This return Ture to indicate the vertex is contained in
   * the graph otherwise False
   */
  [[nodiscard]] bool has_vertex(vertex_id_t vertex_id) const noexcept;

  /**
   * Checks whether two vertices are connected
   *
   * @param  vertex_id_lhs The ID of the first vertex
   * @param  vertex_id_rhs The ID of the second vertex
   * @return boolean - This return Ture to indicate there is an edge between the
   * two vertices otherwise False
   */

  [[nodiscard]] bool has_edge(vertex_id_t vertex_id_lhs,
                              vertex_id_t vertex_id_rhs) const noexcept;

  /**
   * Get a reference to the vertex using its vertex_id
   *
   * @param vertex_id The ID of the vertex
   * @return vertex_t - A reference to the vertex
   * @throws out_of_range exception - If vertex_id does not existing within the
   * graph
   */
  [[nodiscard]] vertex_t& get_vertex(vertex_id_t vertex_id);

  /**
   * Get a const reference to the vertex using its vertex_id
   *
   * @see graph#get_vertex()
   * @param  vertex_id The ID of the vertex
   * @return vertex_t - A const reference to the vertex
   */

  [[nodiscard]] const vertex_t& get_vertex(vertex_id_t vertex_id) const;

  /**
   * Get edge between two vertices with their ID
   *
   * @param  vertex_id_lhs The ID of the first vertex
   * @param  vertex_id_rhs The ID of the second vertex
   * @return edge_t - Shared pointer to either the provided edge or to
   * primitive_numeric_adapter.
   * @throws out_of_range exception - If No edge exit between the two vertices
   */
  [[nodiscard]] edge_t& get_edge(vertex_id_t vertex_id_lhs,
                                 vertex_id_t vertex_id_rhs);

  /**
   * Get const version of edge between two vertices with their ID by calling
   * get_edge()
   *
   * @see    graph#get_edge()
   * @param  vertex_id_lhs The ID of the first vertex
   * @param  vertex_id_rhs The ID of the second vertex
   * @return edge_t - Shared pointer to either the provided edge or to
   * primitive_numeric_adapter.
   */
  [[nodiscard]] const edge_t& get_edge(vertex_id_t vertex_id_lhs,
                                       vertex_id_t vertex_id_rhs) const;

  /**
   * Get a list of neighbour vertices
   *
   * @param  vertex_id The ID of the vertex
   * @return vertices_t - A list of neigbounthood vertices
   */
  [[nodiscard]] vertices_t get_neighbors(vertex_id_t vertex_id) const;

  /**
   * Add a vertex to the graph
   *
   * @param  vertex The vertex to be added
   * @return vertices_id_t - The ID of the new vertex
   */
  vertex_id_t add_vertex(VERTEX_T vertex);

  /**
   * Remove a vertex from the graph and update all its neighbors
   *
   * @param  vertex_id - The ID of the vertex
   */
  void remove_vertex(vertex_id_t vertex_id);

  /**
   * Add a new edge between two existing vertices
   *
   * @param  vertex_id The ID of the vertex
   * @throws out_of_range - If either of the vertex does not exist in graph
   */
  void add_edge(vertex_id_t vertex_id_lhs, vertex_id_t vertex_id_rhs,
                EDGE_T edge);

  /**
   * Remove the edge between two vertices
   *
   * @param  vertex_id_lhs The ID of the first vertex
   * @param  vertex_id_rhs The ID of the second vertex
   */
  void remove_edge(vertex_id_t vertex_id_lhs, vertex_id_t vertex_id_rhs);

 protected:
  std::unordered_map<vertex_id_t, vertices_t> adjacency_list_{};

  vertex_id_to_vertex_t vertices_{};
  edge_id_to_edge_t edges_{};

 private:
  [[nodiscard]] virtual bool do_has_edge(
      vertex_id_t vertex_id_lhs, vertex_id_t vertex_id_rhs) const noexcept = 0;
  [[nodiscard]] virtual edge_t& do_get_edge(vertex_id_t vertex_id_lhs,
                                            vertex_id_t vertex_id_rhs) = 0;
  virtual void do_add_edge(vertex_id_t vertex_id_lhs, vertex_id_t vertex_id_rhs,
                           edge_t edge) = 0;
  virtual void do_remove_edge(vertex_id_t vertex_id_lhs,
                              vertex_id_t vertex_id_rhs) = 0;

  size_t vertex_id_supplier_{0};
};

}  // namespace graaf

#include "graph.tpp"