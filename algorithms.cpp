#include "algorithms.h"

#include <algorithm>
#include <queue>
#include <utility>

namespace {
constexpr int kInf = std::numeric_limits<int>::max();

std::vector<int> rebuildPath(const std::vector<int>& parent, int start, int target) {
    std::vector<int> path;
    if (target < 0 || target >= static_cast<int>(parent.size())) {
        return path;
    }

    for (int current = target; current != -1; current = parent[current]) {
        path.push_back(current);
        if (current == start) {
            break;
        }
    }

    if (path.empty() || path.back() != start) {
        path.clear();
        return path;
    }

    std::reverse(path.begin(), path.end());
    return path;
}
} // namespace

PathResult runDijkstra(const Graph& graph, int start, int target) {
    PathResult result;
    const int nodeCount = graph.getNodeCount();
    if (!graph.isValidIndex(start) || !graph.isValidIndex(target)) {
        return result;
    }

    std::vector<int> distance(nodeCount, kInf);
    std::vector<int> parent(nodeCount, -1);
    std::vector<bool> settled(nodeCount, false);

    using QueueEntry = std::pair<int, int>;
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, std::greater<QueueEntry>> queue;

    distance[start] = 0;
    queue.push({0, start});

    while (!queue.empty()) {
        const std::pair<int, int> entry = queue.top();
        queue.pop();

        const int currentDistance = entry.first;
        const int currentNode = entry.second;

        if (settled[currentNode]) {
            continue;
        }

        settled[currentNode] = true;
        result.visitOrder.push_back(currentNode);

        if (currentNode == target) {
            break;
        }

        for (const auto& edge : graph.getEdges(currentNode)) {
            if (settled[edge.destination]) {
                continue;
            }

            const int candidateDistance = currentDistance + edge.weight;
            if (candidateDistance < distance[edge.destination]) {
                distance[edge.destination] = candidateDistance;
                parent[edge.destination] = currentNode;
                queue.push(std::make_pair(candidateDistance, edge.destination));
            }
        }
    }

    result.path = rebuildPath(parent, start, target);
    result.reachable = !result.path.empty();
    if (result.reachable) {
        result.weightedCost = distance[target];
        result.hopCount = static_cast<int>(result.path.size()) - 1;
    }

    return result;
}

PathResult runBFS(const Graph& graph, int start, int target) {
    PathResult result;
    const int nodeCount = graph.getNodeCount();
    if (!graph.isValidIndex(start) || !graph.isValidIndex(target)) {
        return result;
    }

    std::vector<int> parent(nodeCount, -1);
    std::vector<bool> visited(nodeCount, false);
    std::queue<int> queue;

    visited[start] = true;
    queue.push(start);

    while (!queue.empty()) {
        const int currentNode = queue.front();
        queue.pop();
        result.visitOrder.push_back(currentNode);

        if (currentNode == target) {
            break;
        }

        for (const auto& edge : graph.getEdges(currentNode)) {
            if (visited[edge.destination]) {
                continue;
            }

            visited[edge.destination] = true;
            parent[edge.destination] = currentNode;
            queue.push(edge.destination);
        }
    }

    result.path = rebuildPath(parent, start, target);
    result.reachable = !result.path.empty();
    if (result.reachable) {
        result.hopCount = static_cast<int>(result.path.size()) - 1;
        result.weightedCost = calculatePathWeight(graph, result.path);
    }

    return result;
}

int calculatePathWeight(const Graph& graph, const std::vector<int>& path) {
    if (path.size() < 2) {
        return 0;
    }

    int totalWeight = 0;
    for (size_t index = 0; index + 1 < path.size(); ++index) {
        const int currentNode = path[index];
        const int nextNode = path[index + 1];
        bool edgeFound = false;

        for (const auto& edge : graph.getEdges(currentNode)) {
            if (edge.destination == nextNode) {
                totalWeight += edge.weight;
                edgeFound = true;
                break;
            }
        }

        if (!edgeFound) {
            return kInf;
        }
    }

    return totalWeight;
}

std::string formatPath(const Graph& graph, const std::vector<int>& path) {
    if (path.empty()) {
        return "(no path)";
    }

    std::string formattedPath;
    for (size_t index = 0; index < path.size(); ++index) {
        formattedPath += graph.getNodeName(path[index]);
        if (index + 1 < path.size()) {
            formattedPath += " -> ";
        }
    }

    return formattedPath;
}