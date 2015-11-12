#include <cstddef>
#include <cstdint>

#include <vector>
#include <string>
#include <iterator>
#include <iostream>
#include <unordered_map>

#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/lookup_edge.hpp>
#include <boost/graph/edmonds_karp_max_flow.hpp>

#include <boost/range/algorithm/for_each.hpp>

#include <z3++.h>


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


void makeMaxFlow(const Graph& graph, const Vertex s, const Vertex t) {
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

  const auto flow = edmonds_karp_max_flow(graph, s, t, capacity_map(capacityMap) //
                                                           .residual_capacity_map(residualMap)
                                                           .reverse_edge_map(reverseMap));

  std::cout << flow << std::endl;
}


void makeMaxFlowSolver(const Graph& graph, const Vertex s, const Vertex t) {
  z3::context context;
  z3::optimize solver{context};

  std::unordered_map<std::string, z3::expr> symbols;
  std::vector<z3::expr> constraints;

  const z3::expr zero{context.int_val(0)};

  const auto symbolize = [&](const auto edge) {
    const auto from = source(edge, graph);
    const auto to = target(edge, graph);
    return "x_" + std::to_string(from) + "_" + std::to_string(to);
  };

  boost::for_each(edges(graph), [&](const auto edge) {
    const auto edgeSym = symbolize(edge);
    symbols.emplace(edgeSym, context.int_const(edgeSym.c_str()));
  });

  boost::for_each(edges(graph), [&](const auto edge) {
    const auto edgeSym = symbolize(edge);
    constraints.emplace_back(symbols.at(edgeSym) >= 0);
    constraints.emplace_back(symbols.at(edgeSym) <= 1);
  });

  boost::for_each(vertices(graph), [&](const auto vertex) {
    if (vertex == s or vertex == t)
      return;

    z3::expr outgoing{zero}, incoming{zero};
    boost::for_each(out_edges(vertex, graph), [&](const auto edge) { outgoing = outgoing + symbols.at(symbolize(edge)); });

    std::vector<Edge> inEdges;
    boost::for_each(edges(graph), [&](const auto edge) {
      const auto to = target(edge, graph);
      if (to == vertex)
        inEdges.push_back(edge);
    });

    boost::for_each(inEdges, [&](const auto edge) { incoming = incoming + symbols.at(symbolize(edge)); });

    constraints.emplace_back(outgoing - incoming == 0);
  });


  z3::expr outgoing{zero}, incoming{zero};
  boost::for_each(out_edges(s, graph), [&](const auto edge) { outgoing = outgoing + symbols.at(symbolize(edge)); });

  std::vector<Edge> inEdges;
  boost::for_each(edges(graph), [&](const auto edge) {
    const auto to = target(edge, graph);
    if (to == s)
      inEdges.push_back(edge);
  });

  boost::for_each(inEdges, [&](const auto edge) { incoming = incoming + symbols.at(symbolize(edge)); });

  boost::for_each(constraints, [&](const auto& constraint) { std::cout << constraint << std::endl; });

  solver.maximize(outgoing - incoming);

  if (solver.check() not_eq z3::sat)
    return;

  const auto model = solver.get_model();
  std::cout << model << std::endl;
}


int main() {
  const auto graph = makeGraph();
  const Vertex source{0}, target{3};

  //makeMaxFlow(graph, source, target);
  makeMaxFlowSolver(graph, source, target);
}
