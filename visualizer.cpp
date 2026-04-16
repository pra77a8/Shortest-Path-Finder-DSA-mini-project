#include "visualizer.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <string>
#include <vector>

#include <windows.h>

#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif

namespace {
struct Point {
    int row;
    int col;
};

const int kRows = 34;
const int kCols = 112;

bool gAnsiEnabled = false;

const char* kReset = "\x1b[0m";
const char* kBold = "\x1b[1m";
const char* kDim = "\x1b[2m";
const char* kCyan = "\x1b[36m";
const char* kGreen = "\x1b[32m";
const char* kYellow = "\x1b[33m";
const char* kRed = "\x1b[31m";
const char* kMagenta = "\x1b[35m";

const Point kLayout[] = {
    {6, 12},    // Mumbai
    {12, 26},   // Pune
    {5, 40},    // Nashik
    {11, 62},   // Surat
    {20, 44},   // Indore
    {24, 70},   // Nagpur
    {24, 10},   // Goa
    {4, 84},    // Ahmedabad
    {18, 22},   // Kolhapur
    {12, 40},   // Aurangabad
    {16, 56},   // Bhopal
    {29, 56}    // Hyderabad
};

const std::pair<int, int> kRoads[] = {
    {0, 1}, {0, 2}, {0, 3}, {1, 6}, {1, 2}, {2, 4}, {2, 7},
    {3, 7}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {0, 6}, {1, 3}, {4, 7},
    {1, 8}, {8, 6}, {1, 9}, {2, 9}, {9, 10}, {10, 4}, {10, 3},
    {4, 11}, {5, 11}, {9, 11}
};

bool enableAnsiConsole() {
    if (gAnsiEnabled) {
        return true;
    }

    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    if (handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD mode = 0;
    if (!GetConsoleMode(handle, &mode)) {
        return false;
    }

    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(handle, mode)) {
        return false;
    }

    gAnsiEnabled = true;
    return true;
}

std::string colorize(const std::string& text, const char* color, bool bold = false) {
    if (!gAnsiEnabled) {
        return text;
    }

    std::string styled;
    if (bold) {
        styled += kBold;
    }
    styled += color;
    styled += text;
    styled += kReset;
    return styled;
}

std::string markerForNode(int index, int start, int target, int activeNode, const std::vector<int>& routePath) {
    std::string marker = std::to_string(index);
    if (index < 10) {
        marker = " " + marker;
    }

    const bool onRoute = std::find(routePath.begin(), routePath.end(), index) != routePath.end();
    if (index == activeNode) {
        return colorize("[" + marker + "]", kCyan, true);
    }
    if (index == start) {
        return colorize("[" + marker + "]", kGreen, true);
    }
    if (index == target) {
        return colorize("[" + marker + "]", kRed, true);
    }
    if (onRoute) {
        return colorize("[" + marker + "]", kYellow, true);
    }

    return colorize("[" + marker + "]", kDim, false);
}

void sleepFor(int delayMs) {
    if (delayMs > 0) {
        Sleep(static_cast<DWORD>(delayMs));
    }
}

void clearScreen() {
    if (gAnsiEnabled) {
        std::cout << "\x1b[2J\x1b[H";
    } else {
        std::cout << std::string(40, '\n');
    }
}

void putText(std::vector<std::string>& canvas, int row, int col, const std::string& text) {
    if (row < 0 || row >= static_cast<int>(canvas.size())) {
        return;
    }

    for (size_t index = 0; index < text.size(); ++index) {
        const int position = col + static_cast<int>(index);
        if (position >= 0 && position < static_cast<int>(canvas[row].size())) {
            canvas[row][position] = text[index];
        }
    }
}

void drawLine(std::vector<std::string>& canvas, Point start, Point end, char symbol) {
    int x0 = start.col;
    int y0 = start.row;
    const int x1 = end.col;
    const int y1 = end.row;

    const int dx = std::abs(x1 - x0);
    const int sx = (x0 < x1) ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = (y0 < y1) ? 1 : -1;
    int error = dx + dy;

    while (true) {
        if (y0 >= 0 && y0 < static_cast<int>(canvas.size()) && x0 >= 0 && x0 < static_cast<int>(canvas[y0].size())) {
            if (canvas[y0][x0] == ' ') {
                canvas[y0][x0] = symbol;
            }
        }

        if (x0 == x1 && y0 == y1) {
            break;
        }

        const int twiceError = 2 * error;
        if (twiceError >= dy) {
            error += dy;
            x0 += sx;
        }
        if (twiceError <= dx) {
            error += dx;
            y0 += sy;
        }
    }
}

void drawRoads(std::vector<std::string>& canvas, const std::vector<int>& routePath, bool highlightOnlyRoute) {
    for (size_t roadIndex = 0; roadIndex < sizeof(kRoads) / sizeof(kRoads[0]); ++roadIndex) {
        const int start = kRoads[roadIndex].first;
        const int end = kRoads[roadIndex].second;

        char roadSymbol = '.';
        if (highlightOnlyRoute) {
            roadSymbol = '-';
            for (size_t pathIndex = 0; pathIndex + 1 < routePath.size(); ++pathIndex) {
                const int a = routePath[pathIndex];
                const int b = routePath[pathIndex + 1];
                if ((a == start && b == end) || (a == end && b == start)) {
                    roadSymbol = '=';
                    break;
                }
            }
        }

        drawLine(canvas, kLayout[start], kLayout[end], roadSymbol);
    }
}

void drawNodes(std::vector<std::string>& canvas, const Graph& graph, int start, int target, int activeNode, const std::vector<int>& routePath) {
    for (int index = 0; index < graph.getNodeCount(); ++index) {
        putText(canvas, kLayout[index].row, kLayout[index].col, markerForNode(index, start, target, activeNode, routePath));
    }
}

void printCityLegend(const Graph& graph) {
    std::cout << "City legend: ";
    for (int index = 0; index < graph.getNodeCount(); ++index) {
        std::cout << index << "=" << graph.getNodeName(index);
        if (index + 1 < graph.getNodeCount()) {
            std::cout << " | ";
        }
    }
    std::cout << "\n";
}

void printHeader(const Graph& graph, int start, int target, int activeNode, size_t stepNumber, size_t totalSteps, const std::string& statusLine) {
    std::cout << colorize("Shortest Path Visualizer", kMagenta, true) << "\n";
    std::cout << "From " << colorize(graph.getNodeName(start), kGreen, true)
              << " to " << colorize(graph.getNodeName(target), kRed, true)
              << " | current: " << colorize(graph.getNodeName(activeNode), kCyan, true)
              << " | step " << stepNumber << "/" << totalSteps << "\n";
    std::cout << statusLine << "\n\n";
    printCityLegend(graph);
    std::cout << "\n";
}

void drawFrame(const Graph& graph, int start, int target, int activeNode, const std::vector<int>& routePath, size_t stepNumber, size_t totalSteps, const std::string& statusLine) {
    std::vector<std::string> canvas(kRows, std::string(kCols, ' '));

    drawRoads(canvas, routePath, !routePath.empty());
    drawNodes(canvas, graph, start, target, activeNode, routePath);

    clearScreen();
    printHeader(graph, start, target, activeNode, stepNumber, totalSteps, statusLine);

    for (const auto& line : canvas) {
        std::cout << line << '\n';
    }

    std::cout << "\nLegend: green=source, red=destination, cyan=current location, yellow=final route\n";
}
} // namespace

void animateShortestRoute(const Graph& graph, const PathResult& result, int start, int target, int delayMs) {
    enableAnsiConsole();

    if (!result.reachable || result.path.empty()) {
        std::cout << "No route available to animate.\n";
        return;
    }

    std::vector<int> animatedRoute;
    for (size_t step = 0; step < result.path.size(); ++step) {
        animatedRoute.push_back(result.path[step]);

        std::string statusLine = "Route step " + std::to_string(step + 1) + " of " + std::to_string(result.path.size()) +
            " | moving from " + graph.getNodeName(start) + " to " + graph.getNodeName(target);

        if (step + 1 < result.path.size()) {
            statusLine += " | next hop: " + graph.getNodeName(result.path[step + 1]);
        } else {
            statusLine += " | arrived at destination";
        }

        drawFrame(graph, start, target, result.path[step], animatedRoute, step + 1, result.path.size(), statusLine);
        sleepFor(delayMs);
    }

    std::cout << "\n" << colorize("Final route:", kBold, true) << " " << formatPath(graph, result.path) << "\n";
    std::cout << colorize("Total distance:", kBold, true) << " " << result.weightedCost << " km\n";
    std::cout << colorize("Hop count:", kBold, true) << " " << result.hopCount << "\n";
}