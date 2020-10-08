#pragma once

#include "graph.h"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <iterator>
#include <optional>
#include <set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Graph {

  template <typename Weight>
  class Router {
  private:
    using Graph = DirectedWeightedGraph<Weight>;

  public:
    Router(const Graph& graph);

    using RouteId = uint64_t;

    struct RouteInfo {
      RouteId id;
      Weight weight;
      size_t edge_count;
    };

    std::optional<RouteInfo> BuildRoute(VertexId from, VertexId to) const;
    EdgeId GetRouteEdge(RouteId route_id, size_t edge_idx) const;
    void ReleaseRoute(RouteId route_id);

  private:
    const Graph& graph_;

    struct RouteInternalData {
      Weight weight;
      std::optional<EdgeId> prev_edge;
    };
    using RoutesInternalData = std::vector<std::optional<RouteInternalData>>;

    using ExpandedRoute = std::vector<EdgeId>;
    mutable RouteId next_route_id_ = 0;
    mutable std::unordered_map<RouteId, ExpandedRoute> expanded_routes_cache_;

    mutable RoutesInternalData routes_internal_data_;
  };


  template <typename Weight>
  Router<Weight>::Router(const Graph& graph)
      : graph_(graph)
  {
  }

  template <typename Weight>
  std::optional<typename Router<Weight>::RouteInfo> Router<Weight>::BuildRoute(VertexId from, VertexId to) const {
    const size_t vertex_count = graph_.GetVertexCount();
    routes_internal_data_.assign(vertex_count, std::nullopt);
    routes_internal_data_[from] = RouteInternalData{.weight = 0};
    std::set<std::pair<Weight, VertexId>> vertices_by_weight{{0, from}};
    std::unordered_set<VertexId> done_vertices;

    while (!vertices_by_weight.empty()) {
      const auto min_vertex_it = vertices_by_weight.begin();
      vertices_by_weight.erase(min_vertex_it);
      done_vertices.insert(min_vertex_it->second);

      for (const EdgeId edge_id : graph_.GetIncidentEdges(min_vertex_it->second)) {
        const auto& edge = graph_.GetEdge(edge_id);
        if (done_vertices.count(edge.to)) {
          continue;
        }
        if (!routes_internal_data_[edge.to] || routes_internal_data_[edge.to]->weight > min_vertex_it->first + edge.weight) {
          if (routes_internal_data_[edge.to]) {
            vertices_by_weight.erase({routes_internal_data_[edge.to]->weight, edge.to});
          }
          routes_internal_data_[edge.to] = RouteInternalData{.weight = min_vertex_it->first + edge.weight,
                                                             .prev_edge = edge_id};
          vertices_by_weight.emplace(routes_internal_data_[edge.to]->weight, edge.to);
        }
      }
    }

    const auto& route_internal_data = routes_internal_data_[to];
    if (!route_internal_data) {
      return std::nullopt;
    }
    const Weight weight = route_internal_data->weight;
    std::vector<EdgeId> edges;
    for (std::optional<EdgeId> edge_id = route_internal_data->prev_edge;
         edge_id;
         edge_id = routes_internal_data_[graph_.GetEdge(*edge_id).from]->prev_edge) {
      edges.push_back(*edge_id);
    }
    std::reverse(std::begin(edges), std::end(edges));

    const RouteId route_id = next_route_id_++;
    const size_t route_edge_count = edges.size();
    expanded_routes_cache_[route_id] = std::move(edges);
    return RouteInfo{route_id, weight, route_edge_count};
  }

  template <typename Weight>
  EdgeId Router<Weight>::GetRouteEdge(RouteId route_id, size_t edge_idx) const {
    return expanded_routes_cache_.at(route_id)[edge_idx];
  }

  template <typename Weight>
  void Router<Weight>::ReleaseRoute(RouteId route_id) {
    expanded_routes_cache_.erase(route_id);
  }

}
