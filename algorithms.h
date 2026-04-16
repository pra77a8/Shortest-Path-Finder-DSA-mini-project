#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "graph.h"

#include <limits>
#include <string>
#include <vector>

struct PathResult {
    bool reachable = false;
    int weightedCost = std::numeric_limits<int>::max();
    int hopCount = -1;
    std::vector<int> path;
    std::vector<int> visitOrder;
};

PathResult runDijkstra(const Graph& graph, int start, int target);
PathResult runBFS(const Graph& graph, int start, int target);
int calculatePathWeight(const Graph& graph, const std::vector<int>& path);
std::string formatPath(const Graph& graph, const std::vector<int>& path);

#endif