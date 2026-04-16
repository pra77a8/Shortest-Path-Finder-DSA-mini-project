#include "graph.h"

#include <iomanip>

Graph::Graph() = default;

void Graph::addNode(const string& name) {
    Node node;
    node.name = name;
    nodes.push_back(node);
}

void Graph::addEdge(int src, int dest, int weight) {
    if (!isValidIndex(src) || !isValidIndex(dest) || src == dest) {
        return;
    }

    nodes[src].edges.push_back({dest, weight});
    nodes[dest].edges.push_back({src, weight});
}

void Graph::display() const {
    cout << "\n=== Graph Structure ===\n";
    for (int i = 0; i < getNodeCount(); i++) {
        cout << "[" << i << "] " << nodes[i].name << " --> ";
        for (const auto& edge : nodes[i].edges) {
            cout << nodes[edge.destination].name << "(" << edge.weight << "km) ";
        }
        cout << "\n";
    }
}

int Graph::getNodeCount() const { return static_cast<int>(nodes.size()); }

const string& Graph::getNodeName(int index) const { return nodes[index].name; }

const vector<Edge>& Graph::getEdges(int index) const { return nodes[index].edges; }

int Graph::findNode(const string& name) const {
    for (int index = 0; index < getNodeCount(); ++index) {
        if (nodes[index].name == name) {
            return index;
        }
    }
    return -1;
}

bool Graph::isValidIndex(int index) const {
    return index >= 0 && index < getNodeCount();
}