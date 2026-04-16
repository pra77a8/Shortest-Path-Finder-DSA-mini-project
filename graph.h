#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <vector>
#include <string>

using namespace std;

struct Edge {
    int destination;
    int weight;
};

struct Node {
    string name;           // City name e.g. "Mumbai"
    vector<Edge> edges;    // All edges from this node
};

class Graph {
private:
    vector<Node> nodes;

public:
    Graph();
    void addNode(const string& name);
    void addEdge(int src, int dest, int weight);
    void display() const;
    int getNodeCount() const;
    const string& getNodeName(int index) const;
    const vector<Edge>& getEdges(int index) const;
    int findNode(const string& name) const;
    bool isValidIndex(int index) const;
};

#endif