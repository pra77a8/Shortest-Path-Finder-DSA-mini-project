#include "algorithms.h"
#include "visualizer.h"

#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace {
void printTitle() {
    std::cout << "========================================\n";
    std::cout << "   Shortest Path Finder & Visualizer\n";
    std::cout << "========================================\n";
}

void printCities(const Graph& graph) {
    std::cout << "\nCities:\n";
    for (int index = 0; index < graph.getNodeCount(); ++index) {
        std::cout << "  " << index << ". " << graph.getNodeName(index) << '\n';
    }
}

int readCityChoice(const Graph& graph, const std::string& prompt) {
    int choice = -1;
    while (true) {
        std::cout << prompt;
        if (std::cin >> choice && graph.isValidIndex(choice)) {
            return choice;
        }

        std::cout << "Invalid choice. Try again.\n";
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }
}

void printVisitOrder(const Graph& graph, const std::vector<int>& order) {
    for (size_t index = 0; index < order.size(); ++index) {
        std::cout << graph.getNodeName(order[index]);
        if (index + 1 < order.size()) {
            std::cout << " -> ";
        }
    }
    std::cout << '\n';
}

void showComparison(const Graph& graph, int start, int target) {
    const PathResult dijkstraResult = runDijkstra(graph, start, target);
    const PathResult bfsResult = runBFS(graph, start, target);

    std::cout << "\nFrom " << graph.getNodeName(start) << " to " << graph.getNodeName(target) << "\n";

    std::cout << "\nDijkstra (weighted shortest path)\n";
    if (dijkstraResult.reachable) {
        std::cout << "Path: " << formatPath(graph, dijkstraResult.path) << '\n';
        std::cout << "Weight: " << dijkstraResult.weightedCost << " km\n";
        std::cout << "Total distance: " << dijkstraResult.weightedCost << " km\n";
        std::cout << "Hops: " << dijkstraResult.hopCount << '\n';
        std::cout << "Visited: ";
        printVisitOrder(graph, dijkstraResult.visitOrder);
    } else {
        std::cout << "No path found.\n";
    }

    std::cout << "\nBFS (fewest edges, ignores weights)\n";
    if (bfsResult.reachable) {
        std::cout << "Path: " << formatPath(graph, bfsResult.path) << '\n';
        std::cout << "Weight: " << bfsResult.weightedCost << " km\n";
        std::cout << "Hop count: " << bfsResult.hopCount << '\n';
        std::cout << "Weighted distance along BFS path: " << bfsResult.weightedCost << " km\n";
        std::cout << "Visited: ";
        printVisitOrder(graph, bfsResult.visitOrder);
    } else {
        std::cout << "No path found.\n";
    }
}

PathResult chooseRouteByAlgorithm(const Graph& graph, int start, int target, int algorithmChoice) {
    if (algorithmChoice == 2) {
        return runBFS(graph, start, target);
    }
    return runDijkstra(graph, start, target);
}

void showAnimatedRoute(const Graph& graph, int start, int target) {
    int algorithmChoice = 1;
    std::cout << "Animate using algorithm (1=Dijkstra, 2=BFS): ";
    if (!(std::cin >> algorithmChoice) || (algorithmChoice != 1 && algorithmChoice != 2)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        algorithmChoice = 1;
    }

    const PathResult selectedResult = chooseRouteByAlgorithm(graph, start, target, algorithmChoice);

    if (!selectedResult.reachable) {
        std::cout << "No route found for animation.\n";
        return;
    }

    int delayMs = 650;
    std::cout << "Animation speed in milliseconds per step [650]: ";
    if (!(std::cin >> delayMs)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        delayMs = 650;
    }

    std::cout << "Animating " << (algorithmChoice == 1 ? "Dijkstra" : "BFS") << " route...\n";
    animateShortestRoute(graph, selectedResult, start, target, delayMs);
}

Graph buildCityGraph() {
    Graph graph;

    graph.addNode("Mumbai");
    graph.addNode("Pune");
    graph.addNode("Nashik");
    graph.addNode("Surat");
    graph.addNode("Indore");
    graph.addNode("Nagpur");
    graph.addNode("Goa");
    graph.addNode("Ahmedabad");
    graph.addNode("Kolhapur");
    graph.addNode("Aurangabad");
    graph.addNode("Bhopal");
    graph.addNode("Hyderabad");

    graph.addEdge(0, 1, 150);
    graph.addEdge(0, 2, 170);
    graph.addEdge(0, 3, 280);
    graph.addEdge(1, 6, 450);
    graph.addEdge(1, 2, 210);
    graph.addEdge(2, 4, 420);
    graph.addEdge(2, 7, 520);
    graph.addEdge(3, 7, 240);
    graph.addEdge(3, 4, 330);
    graph.addEdge(4, 5, 360);
    graph.addEdge(5, 6, 730);
    graph.addEdge(6, 7, 850);
    graph.addEdge(0, 6, 970);
    graph.addEdge(1, 3, 280);
    graph.addEdge(4, 7, 590);

    graph.addEdge(1, 8, 230);
    graph.addEdge(8, 6, 140);
    graph.addEdge(1, 9, 235);
    graph.addEdge(2, 9, 190);
    graph.addEdge(9, 10, 340);
    graph.addEdge(10, 4, 190);
    graph.addEdge(10, 3, 380);
    graph.addEdge(4, 11, 500);
    graph.addEdge(5, 11, 500);
    graph.addEdge(9, 11, 560);

    return graph;
}
} // namespace

int main() {
    Graph graph = buildCityGraph();

    printTitle();
    graph.display();

    while (true) {
        std::cout << "\nMenu\n";
        std::cout << "  1. Compare Dijkstra vs BFS\n";
        std::cout << "  2. Animate shortest route\n";
        std::cout << "  3. Show graph again\n";
        std::cout << "  4. Exit\n";

        int option = 0;
        std::cout << "Choose an option: ";
        if (!(std::cin >> option)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input.\n";
            continue;
        }

        if (option == 1) {
            printCities(graph);
            const int start = readCityChoice(graph, "Source city index: ");
            const int target = readCityChoice(graph, "Destination city index: ");
            showComparison(graph, start, target);
        } else if (option == 2) {
            printCities(graph);
            const int start = readCityChoice(graph, "Source city index: ");
            const int target = readCityChoice(graph, "Destination city index: ");
            showAnimatedRoute(graph, start, target);
        } else if (option == 3) {
            graph.display();
        } else if (option == 4) {
            std::cout << "Goodbye.\n";
            break;
        } else {
            std::cout << "Please choose a valid menu option.\n";
        }
    }

    return 0;
}