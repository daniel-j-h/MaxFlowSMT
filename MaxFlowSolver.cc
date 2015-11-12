#include <cstddef>
#include <cstdint>

#include <vector>
#include <iterator>
#include <iostream>

#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/lookup_edge.hpp>
#include <boost/graph/edmonds_karp_max_flow.hpp>

#include <boost/range/algorithm/for_each.hpp>


using Graph = boost::compressed_sparse_row_graph<>;
using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
using Edge = typename boost::graph_traits<Graph>::edge_descriptor;


Graph makeGraph() {
  const auto tag = boost::edges_are_unsorted_multi_pass;
  using VertexPair = std::pair<std::size_t, std::size_t>;

  /*
     1
    / \
   0 - 3
    \ /
     2
  */
  const std::size_t numVertices{4};
  std::vector<VertexPair> edges{{0, 1}, {1, 0}, {0, 2}, {2, 0}, {1, 3}, {3, 1}, {2, 3}, {3, 2}, {0, 3}, {3, 0}};

  return {tag, begin(edges), end(edges), numVertices};
}


std::size_t makeMaxFlow(const Graph& graph, const Vertex s, const Vertex t) {
  std::vector<std::uint8_t> capacities(num_edges(graph), 0);
  std::vector<std::uint8_t> residuals(num_edges(graph), 0);
  std::vector<Edge> reverse(num_edges(graph));

  auto capacityMap = boost::make_iterator_property_map(begin(capacities), get(boost::edge_index, graph));
  auto residualMap = boost::make_iterator_property_map(begin(residuals), get(boost::edge_index, graph));
  auto reverseMap = boost::make_iterator_property_map(begin(reverse), get(boost::edge_index, graph));

  for_each(edges(graph), [&](const auto edge) {
    const auto reverseEdge = lookup_edge(target(edge, graph), source(edge, graph), graph).first;
    put(reverseMap, edge, reverseEdge);

    put(capacityMap, edge, 1);
    put(residualMap, edge, 1);
  });

  return edmonds_karp_max_flow(graph, s, t, capacity_map(capacityMap) //
                                                .residual_capacity_map(residualMap)
                                                .reverse_edge_map(reverseMap));
}


int main() {
  const auto graph = makeGraph();

  const Vertex source{0}, target{3};
  const auto flow = makeMaxFlow(graph, source, target);

  std::cout << flow << std::endl;
}
