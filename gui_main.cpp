#include "algorithms.h"

#include <windows.h>

#include <algorithm>
#include <ctime>
#include <sstream>
#include <string>
#include <vector>

namespace {
struct Point {
    int x;
    int y;
};

struct RoadDef {
    int from;
    int to;
    int baseWeight;
    int morningPct;
    int nightPct;
};

struct LocalMap {
    std::string cityName;
    std::vector<std::string> areaNames;
    std::vector<Point> areaPoints;
    std::vector<RoadDef> roads;
};

const int kStartComboId = 1001;
const int kTargetComboId = 1002;
const int kAlgorithmComboId = 1006;
const int kCompareButtonId = 1003;
const int kAnimateButtonId = 1004;
const int kResetButtonId = 1005;
const int kScopeComboId = 1007;
const int kTimeComboId = 1008;
const int kLocalCityComboId = 1009;

const int kUiTop = 10;
const int kMapTop = 115;

const Point kIntercityLayout[] = {
    {90, 190},    // Mumbai
    {210, 270},   // Pune
    {340, 170},   // Nashik
    {520, 250},   // Surat
    {390, 400},   // Indore
    {620, 440},   // Nagpur
    {120, 440},   // Goa
    {700, 170},   // Ahmedabad
    {260, 380},   // Kolhapur
    {360, 270},   // Aurangabad
    {520, 340},   // Bhopal
    {560, 540}    // Hyderabad
};

const char* kIntercityCities[] = {
    "Mumbai", "Pune", "Nashik", "Surat", "Indore", "Nagpur",
    "Goa", "Ahmedabad", "Kolhapur", "Aurangabad", "Bhopal", "Hyderabad"
};

const RoadDef kIntercityRoads[] = {
    {0, 1, 150, 150, 85}, {0, 2, 170, 140, 90}, {0, 3, 280, 120, 95},
    {1, 6, 450, 105, 105}, {1, 2, 210, 130, 90}, {2, 4, 420, 115, 100},
    {2, 7, 520, 110, 95}, {3, 7, 240, 125, 90}, {3, 4, 330, 130, 90},
    {4, 5, 360, 115, 100}, {5, 6, 730, 95, 110}, {6, 7, 850, 100, 110},
    {0, 6, 970, 95, 110}, {1, 3, 280, 135, 88}, {4, 7, 590, 115, 98},
    {1, 8, 230, 125, 90}, {8, 6, 140, 130, 95}, {1, 9, 235, 140, 88},
    {2, 9, 190, 150, 85}, {9, 10, 340, 120, 100}, {10, 4, 190, 125, 92},
    {10, 3, 380, 115, 100}, {4, 11, 500, 110, 108}, {5, 11, 500, 105, 115},
    {9, 11, 560, 108, 112}
};

std::vector<LocalMap> gLocalMaps;
Graph gScenarioGraph;
HWND gStartCombo = NULL;
HWND gTargetCombo = NULL;
HWND gAlgorithmCombo = NULL;
HWND gScopeCombo = NULL;
HWND gTimeCombo = NULL;
HWND gLocalCityCombo = NULL;

PathResult gDijkstraResult;
PathResult gBfsResult;
PathResult gSelectedAnimationResult;
PathResult gMorningInsightResult;
PathResult gNightInsightResult;
std::vector<int> gDisplayedRoute;
std::vector<int> gAnimatedRoute;
std::vector<PathResult> gTrafficAlternatives;
std::vector<std::pair<int, int>> gAlertedTrafficEdges;
std::wstring gLastTrafficAlert;
int gTrafficBlockedNode = -1;

int gStartIndex = 0;
int gTargetIndex = 1;
int gAlgorithmIndex = 0;
int gScopeIndex = 0;
int gTimeIndex = 0;
int gLocalCityIndex = 0;
int gAnimationStep = -1;
bool gAnimating = false;

std::wstring routePathText(const Graph& graph, const std::vector<int>& path);

std::wstring toWide(const std::string& text) {
    return std::wstring(text.begin(), text.end());
}

std::wstring intToWide(int value) {
    std::wstringstream stream;
    stream << value;
    return stream.str();
}

int applyTrafficRule(const RoadDef& road, int timeIndex) {
    const int factor = (timeIndex == 1) ? road.nightPct : road.morningPct;
    return (road.baseWeight * factor) / 100;
}

std::pair<int, int> normalizeEdge(int a, int b) {
    if (a <= b) {
        return std::make_pair(a, b);
    }
    return std::make_pair(b, a);
}

int detectCurrentTimeProfile() {
    std::time_t now = std::time(NULL);
    std::tm* localNow = std::localtime(&now);
    if (localNow == NULL) {
        return 0;
    }

    const int hour = localNow->tm_hour;
    return (hour >= 6 && hour < 18) ? 0 : 1;
}

int effectiveTrafficTimeIndex() {
    if (gTimeIndex == 0) {
        return detectCurrentTimeProfile();
    }
    return (gTimeIndex == 1) ? 0 : 1;
}

PathResult runSelectedAlgorithm(const Graph& graph, int start, int target, int algorithmIndex) {
    if (algorithmIndex == 1) {
        return runBFS(graph, start, target);
    }
    return runDijkstra(graph, start, target);
}

void initializeLocalMaps() {
    if (!gLocalMaps.empty()) {
        return;
    }

    LocalMap mumbai;
    mumbai.cityName = "Mumbai";
    mumbai.areaNames = {"Andheri", "Bandra", "Dadar", "Colaba", "Powai", "Borivali"};
    mumbai.areaPoints = {{230, 200}, {360, 190}, {330, 285}, {265, 360}, {470, 240}, {170, 120}};
    mumbai.roads.push_back({0, 1, 25, 160, 90});
    mumbai.roads.push_back({1, 2, 18, 150, 88});
    mumbai.roads.push_back({2, 3, 16, 140, 95});
    mumbai.roads.push_back({0, 4, 20, 170, 92});
    mumbai.roads.push_back({4, 1, 15, 130, 85});
    mumbai.roads.push_back({0, 5, 22, 135, 95});
    mumbai.roads.push_back({5, 1, 21, 145, 90});
    mumbai.roads.push_back({5, 2, 26, 130, 95});

    LocalMap pune;
    pune.cityName = "Pune";
    pune.areaNames = {"Shivaji Nagar", "Kothrud", "Hinjewadi", "Hadapsar", "Viman Nagar", "Wakad"};
    pune.areaPoints = {{260, 210}, {180, 280}, {110, 170}, {380, 320}, {430, 200}, {200, 120}};
    pune.roads.push_back({0, 1, 12, 140, 92});
    pune.roads.push_back({0, 2, 22, 165, 95});
    pune.roads.push_back({0, 3, 20, 150, 90});
    pune.roads.push_back({3, 4, 11, 145, 85});
    pune.roads.push_back({4, 5, 18, 135, 90});
    pune.roads.push_back({5, 2, 15, 125, 88});
    pune.roads.push_back({1, 5, 14, 130, 90});

    LocalMap hyderabad;
    hyderabad.cityName = "Hyderabad";
    hyderabad.areaNames = {"Banjara Hills", "Madhapur", "Gachibowli", "Secunderabad", "Charminar", "Kukatpally"};
    hyderabad.areaPoints = {{300, 210}, {380, 200}, {460, 260}, {250, 130}, {220, 320}, {430, 140}};
    hyderabad.roads.push_back({0, 1, 10, 150, 90});
    hyderabad.roads.push_back({1, 2, 14, 140, 95});
    hyderabad.roads.push_back({0, 3, 16, 135, 90});
    hyderabad.roads.push_back({3, 4, 22, 125, 98});
    hyderabad.roads.push_back({4, 0, 18, 145, 90});
    hyderabad.roads.push_back({1, 5, 12, 155, 92});
    hyderabad.roads.push_back({5, 2, 13, 145, 88});

    LocalMap nashik;
    nashik.cityName = "Nashik";
    nashik.areaNames = {"Panchavati", "Gangapur Road", "CIDCO", "Nashik Road", "Satpur", "Adgaon"};
    nashik.areaPoints = {{300, 160}, {390, 210}, {240, 260}, {360, 340}, {180, 170}, {450, 130}};
    nashik.roads.push_back({0, 1, 12, 155, 92});
    nashik.roads.push_back({1, 3, 18, 140, 95});
    nashik.roads.push_back({0, 2, 14, 150, 90});
    nashik.roads.push_back({2, 3, 10, 145, 88});
    nashik.roads.push_back({4, 0, 11, 130, 96});
    nashik.roads.push_back({4, 2, 13, 135, 92});
    nashik.roads.push_back({5, 1, 16, 145, 90});
    nashik.roads.push_back({5, 3, 19, 140, 95});

    LocalMap surat;
    surat.cityName = "Surat";
    surat.areaNames = {"Adajan", "Vesu", "Katargam", "Udhna", "Athwa", "Varachha"};
    surat.areaPoints = {{250, 170}, {390, 180}, {190, 250}, {360, 340}, {470, 240}, {300, 120}};
    surat.roads.push_back({0, 1, 11, 150, 88});
    surat.roads.push_back({0, 2, 13, 145, 92});
    surat.roads.push_back({2, 3, 14, 140, 95});
    surat.roads.push_back({3, 4, 12, 155, 90});
    surat.roads.push_back({4, 1, 10, 150, 88});
    surat.roads.push_back({5, 0, 9, 160, 92});
    surat.roads.push_back({5, 1, 13, 145, 90});

    LocalMap nagpur;
    nagpur.cityName = "Nagpur";
    nagpur.areaNames = {"Sitabuldi", "Dharampeth", "Manish Nagar", "Sadar", "Mihan", "Trimurti Nagar"};
    nagpur.areaPoints = {{280, 190}, {390, 170}, {360, 320}, {200, 150}, {470, 360}, {250, 280}};
    nagpur.roads.push_back({0, 1, 8, 150, 90});
    nagpur.roads.push_back({0, 3, 10, 140, 92});
    nagpur.roads.push_back({0, 5, 9, 135, 95});
    nagpur.roads.push_back({5, 2, 11, 145, 90});
    nagpur.roads.push_back({2, 4, 14, 130, 98});
    nagpur.roads.push_back({1, 2, 12, 140, 92});
    nagpur.roads.push_back({3, 5, 10, 145, 90});

    LocalMap ahmedabad;
    ahmedabad.cityName = "Ahmedabad";
    ahmedabad.areaNames = {"Navrangpura", "Maninagar", "Bopal", "Chandkheda", "Gota", "Vastral"};
    ahmedabad.areaPoints = {{300, 170}, {330, 320}, {470, 270}, {210, 120}, {180, 250}, {430, 360}};
    ahmedabad.roads.push_back({0, 1, 15, 145, 90});
    ahmedabad.roads.push_back({0, 2, 16, 140, 92});
    ahmedabad.roads.push_back({0, 3, 12, 150, 90});
    ahmedabad.roads.push_back({3, 4, 11, 135, 95});
    ahmedabad.roads.push_back({4, 1, 17, 145, 90});
    ahmedabad.roads.push_back({2, 5, 13, 150, 88});
    ahmedabad.roads.push_back({1, 5, 10, 140, 95});

    gLocalMaps.push_back(mumbai);
    gLocalMaps.push_back(pune);
    gLocalMaps.push_back(nashik);
    gLocalMaps.push_back(surat);
    gLocalMaps.push_back(nagpur);
    gLocalMaps.push_back(ahmedabad);
    gLocalMaps.push_back(hyderabad);
}

Graph buildIntercityGraph(int timeIndex) {
    Graph graph;

    const int cityCount = static_cast<int>(sizeof(kIntercityCities) / sizeof(kIntercityCities[0]));
    for (int i = 0; i < cityCount; ++i) {
        graph.addNode(kIntercityCities[i]);
    }

    for (size_t i = 0; i < sizeof(kIntercityRoads) / sizeof(kIntercityRoads[0]); ++i) {
        const RoadDef& road = kIntercityRoads[i];
        graph.addEdge(road.from, road.to, applyTrafficRule(road, timeIndex));
    }

    return graph;
}

Graph buildLocalGraph(int localCityIndex, int timeIndex) {
    Graph graph;
    if (localCityIndex < 0 || localCityIndex >= static_cast<int>(gLocalMaps.size())) {
        return graph;
    }

    const LocalMap& localMap = gLocalMaps[localCityIndex];
    for (size_t i = 0; i < localMap.areaNames.size(); ++i) {
        graph.addNode(localMap.areaNames[i]);
    }

    for (size_t i = 0; i < localMap.roads.size(); ++i) {
        const RoadDef& road = localMap.roads[i];
        graph.addEdge(road.from, road.to, applyTrafficRule(road, timeIndex));
    }

    return graph;
}

std::vector<Point> getCurrentPoints() {
    std::vector<Point> points;
    if (gScopeIndex == 1 && gLocalCityIndex >= 0 && gLocalCityIndex < static_cast<int>(gLocalMaps.size())) {
        return gLocalMaps[gLocalCityIndex].areaPoints;
    }

    for (size_t i = 0; i < sizeof(kIntercityLayout) / sizeof(kIntercityLayout[0]); ++i) {
        points.push_back(kIntercityLayout[i]);
    }
    return points;
}

std::vector<RoadDef> getCurrentRoads() {
    std::vector<RoadDef> roads;
    if (gScopeIndex == 1 && gLocalCityIndex >= 0 && gLocalCityIndex < static_cast<int>(gLocalMaps.size())) {
        return gLocalMaps[gLocalCityIndex].roads;
    }

    for (size_t i = 0; i < sizeof(kIntercityRoads) / sizeof(kIntercityRoads[0]); ++i) {
        roads.push_back(kIntercityRoads[i]);
    }
    return roads;
}

void drawLine(HDC hdc, Point a, Point b, COLORREF color, int width) {
    HPEN pen = CreatePen(PS_SOLID, width, color);
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
    MoveToEx(hdc, a.x, a.y, NULL);
    LineTo(hdc, b.x, b.y);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

void drawNode(HDC hdc, const Graph& graph, const Point& p, int index, COLORREF fillColor, COLORREF textColor, bool bold) {
    const int radius = 16;

    HBRUSH brush = CreateSolidBrush(fillColor);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(hdc, brush));
    HPEN pen = CreatePen(PS_SOLID, 2, RGB(40, 40, 40));
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));

    Ellipse(hdc, p.x - radius, p.y - radius, p.x + radius, p.y + radius);

    SelectObject(hdc, oldPen);
    SelectObject(hdc, oldBrush);
    DeleteObject(pen);
    DeleteObject(brush);

    HFONT font = CreateFontW(
        17, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
    );
    HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, font));
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, textColor);

    std::wstring number = intToWide(index);
    RECT textRect = {p.x - 8, p.y - 12, p.x + 10, p.y + 12};
    DrawTextW(hdc, number.c_str(), -1, &textRect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(hdc, oldFont);
    DeleteObject(font);

    std::wstring name = toWide(graph.getNodeName(index));
    RECT labelRect = {p.x - 80, p.y + 20, p.x + 80, p.y + 44};
    DrawTextW(hdc, name.c_str(), -1, &labelRect, DT_CENTER | DT_TOP | DT_SINGLELINE);
}

bool isEdgeInPath(const std::vector<int>& path, int a, int b) {
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        const int u = path[i];
        const int v = path[i + 1];
        if ((u == a && v == b) || (u == b && v == a)) {
            return true;
        }
    }
    return false;
}

void updateSelectionFromControls() {
    gScopeIndex = static_cast<int>(SendMessageW(gScopeCombo, CB_GETCURSEL, 0, 0));
    gTimeIndex = static_cast<int>(SendMessageW(gTimeCombo, CB_GETCURSEL, 0, 0));
    gAlgorithmIndex = static_cast<int>(SendMessageW(gAlgorithmCombo, CB_GETCURSEL, 0, 0));
    gLocalCityIndex = static_cast<int>(SendMessageW(gLocalCityCombo, CB_GETCURSEL, 0, 0));
    gStartIndex = static_cast<int>(SendMessageW(gStartCombo, CB_GETCURSEL, 0, 0));
    gTargetIndex = static_cast<int>(SendMessageW(gTargetCombo, CB_GETCURSEL, 0, 0));

    if (gScopeIndex < 0) gScopeIndex = 0;
    if (gTimeIndex < 0) gTimeIndex = 0;
    if (gAlgorithmIndex < 0) gAlgorithmIndex = 0;
    if (gLocalCityIndex < 0) gLocalCityIndex = 0;

    if (gStartIndex < 0) gStartIndex = 0;
    if (gTargetIndex < 0) gTargetIndex = 1;
}

void populateStartTargetCombos() {
    SendMessageW(gStartCombo, CB_RESETCONTENT, 0, 0);
    SendMessageW(gTargetCombo, CB_RESETCONTENT, 0, 0);

    if (gScopeIndex == 1 && gLocalCityIndex >= 0 && gLocalCityIndex < static_cast<int>(gLocalMaps.size())) {
        const LocalMap& localMap = gLocalMaps[gLocalCityIndex];
        for (size_t i = 0; i < localMap.areaNames.size(); ++i) {
            std::wstring item = intToWide(static_cast<int>(i)) + L" - " + toWide(localMap.areaNames[i]);
            SendMessageW(gStartCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()));
            SendMessageW(gTargetCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()));
        }
    } else {
        const int cityCount = static_cast<int>(sizeof(kIntercityCities) / sizeof(kIntercityCities[0]));
        for (int i = 0; i < cityCount; ++i) {
            std::wstring item = intToWide(i) + L" - " + toWide(kIntercityCities[i]);
            SendMessageW(gStartCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()));
            SendMessageW(gTargetCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()));
        }
    }

    if (gStartIndex >= static_cast<int>(SendMessageW(gStartCombo, CB_GETCOUNT, 0, 0))) {
        gStartIndex = 0;
    }
    if (gTargetIndex >= static_cast<int>(SendMessageW(gTargetCombo, CB_GETCOUNT, 0, 0))) {
        gTargetIndex = 1;
    }
    if (gStartIndex == gTargetIndex) {
        gTargetIndex = (gTargetIndex + 1) % static_cast<int>(SendMessageW(gTargetCombo, CB_GETCOUNT, 0, 0));
    }

    SendMessageW(gStartCombo, CB_SETCURSEL, gStartIndex, 0);
    SendMessageW(gTargetCombo, CB_SETCURSEL, gTargetIndex, 0);
}

Graph buildScenarioGraph(int scopeIndex, int timeIndex, int localCityIndex) {
    if (scopeIndex == 1) {
        return buildLocalGraph(localCityIndex, timeIndex);
    }
    return buildIntercityGraph(timeIndex);
}

Graph buildScenarioGraphAvoidEdge(int scopeIndex, int timeIndex, int localCityIndex, int avoidA, int avoidB) {
    Graph graph;

    if (scopeIndex == 1) {
        if (localCityIndex < 0 || localCityIndex >= static_cast<int>(gLocalMaps.size())) {
            return graph;
        }

        const LocalMap& localMap = gLocalMaps[localCityIndex];
        for (size_t i = 0; i < localMap.areaNames.size(); ++i) {
            graph.addNode(localMap.areaNames[i]);
        }

        for (size_t i = 0; i < localMap.roads.size(); ++i) {
            const RoadDef& road = localMap.roads[i];
            if ((road.from == avoidA && road.to == avoidB) || (road.from == avoidB && road.to == avoidA)) {
                continue;
            }
            graph.addEdge(road.from, road.to, applyTrafficRule(road, timeIndex));
        }

        return graph;
    }

    const int cityCount = static_cast<int>(sizeof(kIntercityCities) / sizeof(kIntercityCities[0]));
    for (int i = 0; i < cityCount; ++i) {
        graph.addNode(kIntercityCities[i]);
    }

    for (size_t i = 0; i < sizeof(kIntercityRoads) / sizeof(kIntercityRoads[0]); ++i) {
        const RoadDef& road = kIntercityRoads[i];
        if ((road.from == avoidA && road.to == avoidB) || (road.from == avoidB && road.to == avoidA)) {
            continue;
        }
        graph.addEdge(road.from, road.to, applyTrafficRule(road, timeIndex));
    }

    return graph;
}

Graph buildScenarioGraphAvoidNode(int scopeIndex, int timeIndex, int localCityIndex, int avoidNode, int sourceNode, int destinationNode) {
    Graph graph;

    if (scopeIndex == 1) {
        if (localCityIndex < 0 || localCityIndex >= static_cast<int>(gLocalMaps.size())) {
            return graph;
        }

        const LocalMap& localMap = gLocalMaps[localCityIndex];
        for (size_t i = 0; i < localMap.areaNames.size(); ++i) {
            graph.addNode(localMap.areaNames[i]);
        }

        for (size_t i = 0; i < localMap.roads.size(); ++i) {
            const RoadDef& road = localMap.roads[i];
            if (avoidNode != sourceNode && avoidNode != destinationNode) {
                if (road.from == avoidNode || road.to == avoidNode) {
                    continue;
                }
            }
            graph.addEdge(road.from, road.to, applyTrafficRule(road, timeIndex));
        }

        return graph;
    }

    const int cityCount = static_cast<int>(sizeof(kIntercityCities) / sizeof(kIntercityCities[0]));
    for (int i = 0; i < cityCount; ++i) {
        graph.addNode(kIntercityCities[i]);
    }

    for (size_t i = 0; i < sizeof(kIntercityRoads) / sizeof(kIntercityRoads[0]); ++i) {
        const RoadDef& road = kIntercityRoads[i];
        if (avoidNode != sourceNode && avoidNode != destinationNode) {
            if (road.from == avoidNode || road.to == avoidNode) {
                continue;
            }
        }
        graph.addEdge(road.from, road.to, applyTrafficRule(road, timeIndex));
    }

    return graph;
}

bool tryGetCurrentRoad(int a, int b, RoadDef* outRoad) {
    const std::vector<RoadDef> roads = getCurrentRoads();
    for (size_t i = 0; i < roads.size(); ++i) {
        const RoadDef& road = roads[i];
        if ((road.from == a && road.to == b) || (road.from == b && road.to == a)) {
            if (outRoad != NULL) {
                *outRoad = road;
            }
            return true;
        }
    }
    return false;
}

bool isAlreadyAlerted(int a, int b) {
    const std::pair<int, int> edge = normalizeEdge(a, b);
    return std::find(gAlertedTrafficEdges.begin(), gAlertedTrafficEdges.end(), edge) != gAlertedTrafficEdges.end();
}

void markAlerted(int a, int b) {
    gAlertedTrafficEdges.push_back(normalizeEdge(a, b));
}

bool samePath(const std::vector<int>& a, const std::vector<int>& b) {
    if (a.size() != b.size()) {
        return false;
    }

    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            return false;
        }
    }
    return true;
}

bool pathContainsNode(const std::vector<int>& path, int node) {
    return std::find(path.begin(), path.end(), node) != path.end();
}

std::vector<int> getAdjacentNodes(const Graph& graph, int node) {
    std::vector<int> adjacent;
    if (!graph.isValidIndex(node)) {
        return adjacent;
    }

    const std::vector<Edge>& edges = graph.getEdges(node);
    for (size_t i = 0; i < edges.size(); ++i) {
        adjacent.push_back(edges[i].destination);
    }
    return adjacent;
}

bool pathTouchesAnyNode(const std::vector<int>& path, const std::vector<int>& nodes) {
    for (size_t i = 0; i < nodes.size(); ++i) {
        if (pathContainsNode(path, nodes[i])) {
            return true;
        }
    }
    return false;
}

int pathTrafficPenalty(const Graph& graph, const std::vector<int>& path, int blockedNode) {
    if (path.empty()) {
        return 1000000;
    }

    int penalty = 0;
    if (pathContainsNode(path, blockedNode)) {
        penalty += 500000;
    }

    const std::vector<int> adjacent = getAdjacentNodes(graph, blockedNode);
    for (size_t i = 0; i < adjacent.size(); ++i) {
        if (pathContainsNode(path, adjacent[i])) {
            penalty += 5000;
        }
    }

    return penalty;
}

std::vector<PathResult> buildAlternativeRoutes(int currentNode, int destinationNode, int resolvedTimeIndex, int blockedA, int blockedB) {
    std::vector<PathResult> alternatives;
    const Graph currentGraph = buildScenarioGraph(gScopeIndex, resolvedTimeIndex, gLocalCityIndex);
    const std::vector<int> trafficZone = getAdjacentNodes(currentGraph, blockedB);

    Graph avoidBusyNode = buildScenarioGraphAvoidNode(gScopeIndex, resolvedTimeIndex, gLocalCityIndex, blockedB, currentNode, destinationNode);
    PathResult alt1 = runSelectedAlgorithm(avoidBusyNode, currentNode, destinationNode, gAlgorithmIndex);
    if (!alt1.reachable) {
        Graph avoidMainEdge = buildScenarioGraphAvoidEdge(gScopeIndex, resolvedTimeIndex, gLocalCityIndex, blockedA, blockedB);
        alt1 = runSelectedAlgorithm(avoidMainEdge, currentNode, destinationNode, gAlgorithmIndex);
    }

    if (alt1.reachable && alt1.path.size() > 1 && !pathContainsNode(alt1.path, blockedB)) {
        alternatives.push_back(alt1);
    }

    if (!alternatives.empty() && alternatives[0].path.size() > 2) {
        const int altEdgeA = alternatives[0].path[0];
        const int altEdgeB = alternatives[0].path[1];
        Graph avoidSecondEdge = buildScenarioGraphAvoidEdge(gScopeIndex, resolvedTimeIndex, gLocalCityIndex, altEdgeA, altEdgeB);
        PathResult alt2 = runSelectedAlgorithm(avoidSecondEdge, currentNode, destinationNode, gAlgorithmIndex);
        if (alt2.reachable && alt2.path.size() > 1 && !samePath(alt2.path, alternatives[0].path) && !pathContainsNode(alt2.path, blockedB)) {
            alternatives.push_back(alt2);
        }
    }

    std::sort(alternatives.begin(), alternatives.end(), [&currentGraph, blockedB](const PathResult& left, const PathResult& right) {
        const int leftScore = left.weightedCost + pathTrafficPenalty(currentGraph, left.path, blockedB);
        const int rightScore = right.weightedCost + pathTrafficPenalty(currentGraph, right.path, blockedB);
        return leftScore < rightScore;
    });

    return alternatives;
}

int askUserToSelectAlternative(HWND hwnd, const Graph& graph, const std::vector<PathResult>& alternatives, int blockedNode) {
    if (alternatives.empty()) {
        return -1;
    }

    if (alternatives.size() == 1) {
        return 0;
    }

    std::wstring prompt = L"Choose alternative route:\n";
    prompt += L"Traffic zone: " + toWide(graph.getNodeName(blockedNode)) + L"\n\n";
    prompt += L"Yes = Alternative 1\n";
    prompt += L"No = Alternative 2\n";
    prompt += L"Cancel = Keep current route\n\n";
    prompt += L"Alternative 1: " + routePathText(graph, alternatives[0].path) + L"\n";
    prompt += L"Alternative 2: " + routePathText(graph, alternatives[1].path);

    const int response = MessageBoxW(hwnd, prompt.c_str(), L"Choose Alternative", MB_ICONQUESTION | MB_YESNOCANCEL);
    if (response == IDYES) {
        return 0;
    }
    if (response == IDNO) {
        return 1;
    }
    return -1;
}

void computeRoutes() {
    const int resolvedTimeIndex = effectiveTrafficTimeIndex();
    gScenarioGraph = buildScenarioGraph(gScopeIndex, resolvedTimeIndex, gLocalCityIndex);
    if (!gScenarioGraph.isValidIndex(gStartIndex) || !gScenarioGraph.isValidIndex(gTargetIndex)) {
        gDijkstraResult = PathResult();
        gBfsResult = PathResult();
        gSelectedAnimationResult = PathResult();
        gMorningInsightResult = PathResult();
        gNightInsightResult = PathResult();
        return;
    }

    gDijkstraResult = runDijkstra(gScenarioGraph, gStartIndex, gTargetIndex);
    gBfsResult = runBFS(gScenarioGraph, gStartIndex, gTargetIndex);
    gSelectedAnimationResult = (gAlgorithmIndex == 1) ? gBfsResult : gDijkstraResult;
    gDisplayedRoute = gSelectedAnimationResult.path;
    gTrafficAlternatives.clear();

    Graph morningGraph = buildScenarioGraph(gScopeIndex, 0, gLocalCityIndex);
    Graph nightGraph = buildScenarioGraph(gScopeIndex, 1, gLocalCityIndex);
    gMorningInsightResult = runSelectedAlgorithm(morningGraph, gStartIndex, gTargetIndex, gAlgorithmIndex);
    gNightInsightResult = runSelectedAlgorithm(nightGraph, gStartIndex, gTargetIndex, gAlgorithmIndex);
}

void stopAnimation(HWND hwnd) {
    gAnimating = false;
    KillTimer(hwnd, 1);
}

void startAnimation(HWND hwnd) {
    updateSelectionFromControls();
    computeRoutes();

    gAnimatedRoute.clear();
    gAnimationStep = 0;
    gAlertedTrafficEdges.clear();
    gTrafficAlternatives.clear();
    gLastTrafficAlert.clear();
    gTrafficBlockedNode = -1;
    gDisplayedRoute = gSelectedAnimationResult.path;
    gAnimating = gSelectedAnimationResult.reachable;
    if (gAnimating && !gSelectedAnimationResult.path.empty()) {
        gAnimatedRoute.push_back(gSelectedAnimationResult.path[0]);
    }
    if (gAnimating) {
        SetTimer(hwnd, 1, 700, NULL);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void resetView(HWND hwnd) {
    stopAnimation(hwnd);
    gAnimatedRoute.clear();
    gAlertedTrafficEdges.clear();
    gTrafficAlternatives.clear();
    gLastTrafficAlert.clear();
    gTrafficBlockedNode = -1;
    gDisplayedRoute.clear();
    gAnimationStep = -1;
    computeRoutes();
    InvalidateRect(hwnd, NULL, TRUE);
}

std::wstring timeLabel(int timeIndex) {
    if (timeIndex == 0) {
        return (detectCurrentTimeProfile() == 0) ? L"Auto (Morning)" : L"Auto (Night)";
    }
    return (timeIndex == 2) ? L"Night" : L"Morning";
}

std::wstring modeLabel() {
    return (gScopeIndex == 1) ? L"Local Area" : L"Intercity";
}

std::wstring routePathText(const Graph& graph, const std::vector<int>& path) {
    if (path.empty()) {
        return L"(no route)";
    }

    std::wstring text;
    for (size_t i = 0; i < path.size(); ++i) {
        text += toWide(graph.getNodeName(path[i]));
        if (i + 1 < path.size()) {
            text += L" -> ";
        }
    }
    return text;
}

std::wstring ellipsize(const std::wstring& text, size_t maxChars) {
    if (text.size() <= maxChars) {
        return text;
    }
    if (maxChars < 8) {
        return text.substr(0, maxChars);
    }

    const size_t head = (maxChars - 3) / 2;
    const size_t tail = maxChars - 3 - head;
    return text.substr(0, head) + L"..." + text.substr(text.size() - tail);
}

std::wstring compactRouteSummary(const Graph& graph, const std::vector<int>& path) {
    if (path.empty()) {
        return L"Selected route: (no route)";
    }

    if (path.size() <= 6) {
        return L"Selected route: " + routePathText(graph, path);
    }

    std::wstring compact = L"Selected route: ";
    compact += toWide(graph.getNodeName(path[0])) + L" -> ";
    compact += toWide(graph.getNodeName(path[1])) + L" -> ... -> ";
    compact += toWide(graph.getNodeName(path[path.size() - 2])) + L" -> ";
    compact += toWide(graph.getNodeName(path[path.size() - 1]));
    return compact;
}

std::vector<Point> scalePointsToRect(const std::vector<Point>& sourcePoints, const RECT& mapRect) {
    std::vector<Point> scaled = sourcePoints;
    if (sourcePoints.empty()) {
        return scaled;
    }

    int minX = sourcePoints[0].x;
    int maxX = sourcePoints[0].x;
    int minY = sourcePoints[0].y;
    int maxY = sourcePoints[0].y;

    for (size_t i = 1; i < sourcePoints.size(); ++i) {
        minX = std::min(minX, sourcePoints[i].x);
        maxX = std::max(maxX, sourcePoints[i].x);
        minY = std::min(minY, sourcePoints[i].y);
        maxY = std::max(maxY, sourcePoints[i].y);
    }

    const int srcWidth = std::max(1, maxX - minX);
    const int srcHeight = std::max(1, maxY - minY);

    const int padLeft = 70;
    const int padRight = 70;
    const int padTop = 60;
    const int padBottom = 80;

    const int usableLeft = mapRect.left + padLeft;
    const int usableRight = mapRect.right - padRight;
    const int usableTop = mapRect.top + padTop;
    const int usableBottom = mapRect.bottom - padBottom;

    const int usableWidth = std::max(1, usableRight - usableLeft);
    const int usableHeight = std::max(1, usableBottom - usableTop);

    for (size_t i = 0; i < sourcePoints.size(); ++i) {
        const int dx = sourcePoints[i].x - minX;
        const int dy = sourcePoints[i].y - minY;

        scaled[i].x = usableLeft + (dx * usableWidth) / srcWidth;
        scaled[i].y = usableTop + (dy * usableHeight) / srcHeight;
    }

    return scaled;
}

void paintScene(HWND hwnd, HDC hdc) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);

    const int panelPadding = 20;
    const int footerHeight = 280;
    int footerBottom = clientRect.bottom - 20;
    int footerTop = footerBottom - footerHeight;
    if (footerTop < kMapTop + 260) {
        footerTop = kMapTop + 260;
        footerBottom = std::max(footerTop + 180, static_cast<int>(clientRect.bottom) - 20);
    }

    int mapBottom = footerTop - 12;
    if (mapBottom < kMapTop + 240) {
        mapBottom = kMapTop + 240;
    }

    HBRUSH backgroundBrush = CreateSolidBrush(RGB(244, 247, 252));
    FillRect(hdc, &clientRect, backgroundBrush);
    DeleteObject(backgroundBrush);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(25, 25, 25));

    HFONT titleFont = CreateFontW(
        25, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
    );
    HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, titleFont));

    std::wstring title = L"Adaptive Path Visualizer (Time + Local Areas)";
    TextOutW(hdc, 20, 18, title.c_str(), static_cast<int>(title.size()));

    SelectObject(hdc, oldFont);
    DeleteObject(titleFont);

    RECT mapRect = {panelPadding, kMapTop, clientRect.right - panelPadding, mapBottom};
    HBRUSH mapBrush = CreateSolidBrush(RGB(255, 255, 255));
    FrameRect(hdc, &mapRect, static_cast<HBRUSH>(GetStockObject(GRAY_BRUSH)));
    FillRect(hdc, &mapRect, mapBrush);
    DeleteObject(mapBrush);

    std::vector<Point> points = scalePointsToRect(getCurrentPoints(), mapRect);
    std::vector<RoadDef> roads = getCurrentRoads();

    std::vector<int> routeToDraw = gDisplayedRoute;
    if (routeToDraw.empty() && gSelectedAnimationResult.reachable) {
        routeToDraw = gSelectedAnimationResult.path;
    }

    for (size_t i = 0; i < roads.size(); ++i) {
        COLORREF color = RGB(180, 182, 190);
        int width = 2;
        if (isEdgeInPath(routeToDraw, roads[i].from, roads[i].to)) {
            color = RGB(255, 140, 0);
            width = 4;
        }
        drawLine(hdc, points[roads[i].from], points[roads[i].to], color, width);
    }

    for (int i = 0; i < gScenarioGraph.getNodeCount(); ++i) {
        COLORREF fill = RGB(232, 236, 244);
        COLORREF textColor = RGB(40, 40, 40);
        bool bold = false;

        if (i == gStartIndex) {
            fill = RGB(58, 191, 115);
            textColor = RGB(255, 255, 255);
            bold = true;
        }
        if (i == gTargetIndex) {
            fill = RGB(225, 74, 74);
            textColor = RGB(255, 255, 255);
            bold = true;
        }
        if (gTrafficBlockedNode >= 0 && i == gTrafficBlockedNode) {
            fill = RGB(196, 66, 66);
            textColor = RGB(255, 255, 255);
            bold = true;
        }
        if (std::find(routeToDraw.begin(), routeToDraw.end(), i) != routeToDraw.end()) {
            fill = RGB(255, 193, 77);
            textColor = RGB(35, 35, 35);
            bold = true;
        }
        if (gAnimating && gAnimationStep >= 0 && gAnimationStep < static_cast<int>(gSelectedAnimationResult.path.size()) &&
            i == gSelectedAnimationResult.path[gAnimationStep]) {
            fill = RGB(67, 120, 255);
            textColor = RGB(255, 255, 255);
            bold = true;
        }

        drawNode(hdc, gScenarioGraph, points[i], i, fill, textColor, bold);
    }

    HFONT textFont = CreateFontW(
        17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
    );
    oldFont = static_cast<HFONT>(SelectObject(hdc, textFont));

    std::wstring modeLine = L"Mode: " + modeLabel() + L" | Time Profile: " + timeLabel(gTimeIndex) +
        L" | Algorithm: " + std::wstring((gAlgorithmIndex == 1) ? L"BFS" : L"Dijkstra");
    RECT modeRect = {panelPadding, 84, clientRect.right - panelPadding, 108};
    DrawTextW(hdc, modeLine.c_str(), -1, &modeRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    std::wstring dijkstraLine = L"Dijkstra => weight: ";
    dijkstraLine += gDijkstraResult.reachable ? (intToWide(gDijkstraResult.weightedCost) + L" km") : L"N/A";
    dijkstraLine += L", hops: ";
    dijkstraLine += gDijkstraResult.reachable ? intToWide(gDijkstraResult.hopCount) : L"N/A";

    std::wstring bfsLine = L"BFS => weight: ";
    bfsLine += gBfsResult.reachable ? (intToWide(gBfsResult.weightedCost) + L" km") : L"N/A";
    bfsLine += L", hops: ";
    bfsLine += gBfsResult.reachable ? intToWide(gBfsResult.hopCount) : L"N/A";

    std::wstring morningLine = L"Morning route weight: ";
    morningLine += gMorningInsightResult.reachable ? (intToWide(gMorningInsightResult.weightedCost) + L" km") : L"N/A";

    std::wstring nightLine = L"Night route weight: ";
    nightLine += gNightInsightResult.reachable ? (intToWide(gNightInsightResult.weightedCost) + L" km") : L"N/A";

    std::wstring bestLine = L"Best time now: ";
    if (gMorningInsightResult.reachable && gNightInsightResult.reachable) {
        if (gMorningInsightResult.weightedCost < gNightInsightResult.weightedCost) {
            bestLine += L"Morning";
        } else if (gMorningInsightResult.weightedCost > gNightInsightResult.weightedCost) {
            bestLine += L"Night";
        } else {
            bestLine += L"Same for both";
        }
    } else {
        bestLine += L"Unavailable";
    }

    RECT footerRect = {panelPadding, footerTop, clientRect.right - panelPadding, footerBottom};
    HBRUSH footerBrush = CreateSolidBrush(RGB(236, 242, 250));
    FillRect(hdc, &footerRect, footerBrush);
    DeleteObject(footerBrush);
    FrameRect(hdc, &footerRect, static_cast<HBRUSH>(GetStockObject(GRAY_BRUSH)));

    HFONT footerTitleFont = CreateFontW(
        18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
    );
    HFONT oldFooterTitleFont = static_cast<HFONT>(SelectObject(hdc, footerTitleFont));
    std::wstring footerTitle = L"Routing Summary";
    RECT footerTitleRect = {footerRect.left + 15, footerTop + 10, footerRect.right - 15, footerTop + 34};
    DrawTextW(hdc, footerTitle.c_str(), -1, &footerTitleRect, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, oldFooterTitleFont);
    DeleteObject(footerTitleFont);

    RECT leftInfoRect = {footerRect.left + 15, footerTop + 40, footerRect.left + (footerRect.right - footerRect.left) / 2 + 60, footerBottom - 12};
    std::wstring leftInfoBlock = dijkstraLine + L"\n" + bfsLine + L"\n" + morningLine + L"\n" + nightLine;
    DrawTextW(hdc, leftInfoBlock.c_str(), -1, &leftInfoRect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);

    HFONT bestFont = CreateFontW(
        18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
    );
    HFONT oldBestFont = static_cast<HFONT>(SelectObject(hdc, bestFont));
    RECT bestRect = {footerRect.left + (footerRect.right - footerRect.left) / 2 + 80, footerTop + 40, footerRect.right - 15, footerTop + 72};
    DrawTextW(hdc, bestLine.c_str(), -1, &bestRect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);
    SelectObject(hdc, oldBestFont);
    DeleteObject(bestFont);

    std::wstring legendLine = L"Legend: Green=Source | Red=Destination | Blue=Current Location | Orange=Route";
    RECT legendRect = {footerRect.left + (footerRect.right - footerRect.left) / 2 + 80, footerTop + 76, footerRect.right - 15, footerTop + 112};
    DrawTextW(hdc, legendLine.c_str(), -1, &legendRect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);

    std::wstring selectedRouteLine = ellipsize(compactRouteSummary(gScenarioGraph, gSelectedAnimationResult.path), 130);
    RECT selectedRouteRect = {footerRect.left + 15, footerTop + 196, footerRect.right - 15, footerTop + 236};
    DrawTextW(hdc, selectedRouteLine.c_str(), -1, &selectedRouteRect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);

    if (!gTrafficAlternatives.empty()) {
        std::wstring altHeader = L"Alternative routes:";
        RECT altHeaderRect = {footerRect.left + (footerRect.right - footerRect.left) / 2 + 80, footerTop + 116, footerRect.right - 15, footerTop + 136};
        DrawTextW(hdc, altHeader.c_str(), -1, &altHeaderRect, DT_LEFT | DT_TOP | DT_SINGLELINE);

        const int maxAltToShow = 2;
        for (int i = 0; i < static_cast<int>(gTrafficAlternatives.size()) && i < maxAltToShow; ++i) {
            std::wstring altLine = intToWide(i + 1) + L") from current: " + ellipsize(routePathText(gScenarioGraph, gTrafficAlternatives[i].path), 48);
            altLine += L" [" + intToWide(gTrafficAlternatives[i].weightedCost) + L" km]";
            RECT altLineRect = {footerRect.left + (footerRect.right - footerRect.left) / 2 + 80, footerTop + 138 + (i * 20), footerRect.right - 15, footerTop + 158 + (i * 20)};
            DrawTextW(hdc, altLine.c_str(), -1, &altLineRect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
        }
    }

    std::wstring exampleLine = L"Example: Local Areas -> Nashik, Source=Panchavati, Destination=Adgaon, Compare then Animate.";
    RECT exRect = {footerRect.left + 15, footerBottom - 30, footerRect.right - 15, footerBottom - 10};
    DrawTextW(hdc, exampleLine.c_str(), -1, &exRect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);

    if (gAnimating && gAnimationStep >= 0 && gAnimationStep < static_cast<int>(gSelectedAnimationResult.path.size())) {
        std::wstring current = L"Current location: " + toWide(gScenarioGraph.getNodeName(gSelectedAnimationResult.path[gAnimationStep]));
        RECT currentRect = {footerRect.left + (footerRect.right - footerRect.left) / 2 + 80, footerTop + 184, footerRect.right - 15, footerTop + 206};
        DrawTextW(hdc, current.c_str(), -1, &currentRect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
    }

    if (!gLastTrafficAlert.empty()) {
        RECT alertRect = {footerRect.left + 15, footerTop + 116, footerRect.left + (footerRect.right - footerRect.left) / 2 + 60, footerTop + 170};
        DrawTextW(hdc, gLastTrafficAlert.c_str(), -1, &alertRect, DT_LEFT | DT_TOP | DT_WORDBREAK | DT_END_ELLIPSIS);
    }

    SelectObject(hdc, oldFont);
    DeleteObject(textFont);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        initializeLocalMaps();

        CreateWindowW(L"STATIC", L"Scope:", WS_VISIBLE | WS_CHILD,
                      20, kUiTop + 42, 55, 24, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Time:", WS_VISIBLE | WS_CHILD,
                      215, kUiTop + 42, 50, 24, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"City(Local):", WS_VISIBLE | WS_CHILD,
                      390, kUiTop + 42, 85, 24, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Source:", WS_VISIBLE | WS_CHILD,
                      590, kUiTop + 42, 60, 24, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Destination:", WS_VISIBLE | WS_CHILD,
                      780, kUiTop + 42, 85, 24, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Algorithm:", WS_VISIBLE | WS_CHILD,
                      980, kUiTop + 42, 75, 24, hwnd, NULL, NULL, NULL);

        gScopeCombo = CreateWindowW(
            L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            70, kUiTop + 38, 130, 200, hwnd, reinterpret_cast<HMENU>(kScopeComboId), NULL, NULL
        );

        gTimeCombo = CreateWindowW(
            L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            255, kUiTop + 38, 120, 120, hwnd, reinterpret_cast<HMENU>(kTimeComboId), NULL, NULL
        );

        gLocalCityCombo = CreateWindowW(
            L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            475, kUiTop + 38, 110, 200, hwnd, reinterpret_cast<HMENU>(kLocalCityComboId), NULL, NULL
        );

        gStartCombo = CreateWindowW(
            L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            650, kUiTop + 38, 120, 300, hwnd, reinterpret_cast<HMENU>(kStartComboId), NULL, NULL
        );

        gTargetCombo = CreateWindowW(
            L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            865, kUiTop + 38, 110, 300, hwnd, reinterpret_cast<HMENU>(kTargetComboId), NULL, NULL
        );

        gAlgorithmCombo = CreateWindowW(
            L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            1055, kUiTop + 38, 120, 120, hwnd, reinterpret_cast<HMENU>(kAlgorithmComboId), NULL, NULL
        );

        CreateWindowW(L"BUTTON", L"Compare", WS_VISIBLE | WS_CHILD,
                      650, kUiTop + 72, 90, 28, hwnd, reinterpret_cast<HMENU>(kCompareButtonId), NULL, NULL);
        CreateWindowW(L"BUTTON", L"Animate", WS_VISIBLE | WS_CHILD,
                      750, kUiTop + 72, 90, 28, hwnd, reinterpret_cast<HMENU>(kAnimateButtonId), NULL, NULL);
        CreateWindowW(L"BUTTON", L"Reset", WS_VISIBLE | WS_CHILD,
                      850, kUiTop + 72, 90, 28, hwnd, reinterpret_cast<HMENU>(kResetButtonId), NULL, NULL);

        SendMessageW(gScopeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Intercity"));
        SendMessageW(gScopeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Local Areas"));
        SendMessageW(gTimeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Auto (Current Time)"));
        SendMessageW(gTimeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Morning"));
        SendMessageW(gTimeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Night"));
        SendMessageW(gAlgorithmCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Dijkstra"));
        SendMessageW(gAlgorithmCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"BFS"));

        for (size_t i = 0; i < gLocalMaps.size(); ++i) {
            SendMessageW(gLocalCityCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(toWide(gLocalMaps[i].cityName).c_str()));
        }

        SendMessageW(gScopeCombo, CB_SETCURSEL, gScopeIndex, 0);
        SendMessageW(gTimeCombo, CB_SETCURSEL, gTimeIndex, 0);
        SendMessageW(gLocalCityCombo, CB_SETCURSEL, gLocalCityIndex, 0);
        SendMessageW(gAlgorithmCombo, CB_SETCURSEL, gAlgorithmIndex, 0);

        EnableWindow(gLocalCityCombo, FALSE);

        populateStartTargetCombos();
        computeRoutes();
        return 0;
    }

    case WM_COMMAND: {
        const int controlId = LOWORD(wParam);
        const int eventCode = HIWORD(wParam);

        if (eventCode == CBN_SELCHANGE) {
            updateSelectionFromControls();

            if (controlId == kScopeComboId || controlId == kLocalCityComboId) {
                EnableWindow(gLocalCityCombo, gScopeIndex == 1 ? TRUE : FALSE);
                populateStartTargetCombos();
            }

            stopAnimation(hwnd);
            computeRoutes();
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }

        if (controlId == kCompareButtonId) {
            stopAnimation(hwnd);
            updateSelectionFromControls();
            computeRoutes();
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }

        if (controlId == kAnimateButtonId) {
            startAnimation(hwnd);
            return 0;
        }

        if (controlId == kResetButtonId) {
            resetView(hwnd);
            return 0;
        }

        return 0;
    }

    case WM_TIMER: {
        if (wParam == 1 && gAnimating) {
            if (gAnimationStep + 1 < static_cast<int>(gSelectedAnimationResult.path.size())) {
                const int currentNode = gSelectedAnimationResult.path[gAnimationStep];
                const int nextNode = gSelectedAnimationResult.path[gAnimationStep + 1];

                RoadDef nextRoad;
                const int resolvedTimeIndex = effectiveTrafficTimeIndex();
                if (tryGetCurrentRoad(currentNode, nextNode, &nextRoad) && !isAlreadyAlerted(currentNode, nextNode)) {
                    const int trafficFactor = (resolvedTimeIndex == 1) ? nextRoad.nightPct : nextRoad.morningPct;
                    if (trafficFactor >= 145) {
                        markAlerted(currentNode, nextNode);

                        gTrafficAlternatives = buildAlternativeRoutes(currentNode, gTargetIndex, resolvedTimeIndex, currentNode, nextNode);

                        std::wstring altMessage = L"No alternative route found.";
                        if (!gTrafficAlternatives.empty()) {
                            altMessage = L"Alternative 1: " + routePathText(gScenarioGraph, gTrafficAlternatives[0].path);
                            if (gTrafficAlternatives.size() > 1) {
                                altMessage += L"\nAlternative 2: " + routePathText(gScenarioGraph, gTrafficAlternatives[1].path);
                            }
                        }

                        std::wstring alertTitle = L"Traffic Alert";
                        std::wstring alertMessage = L"Heavy traffic detected ahead near " + toWide(gScenarioGraph.getNodeName(nextNode)) +
                            L". Rerouting now before entering this segment.\n\n" + altMessage;
                        MessageBoxW(hwnd, alertMessage.c_str(), alertTitle.c_str(), MB_OK | MB_ICONWARNING);
                        gTrafficBlockedNode = nextNode;
                        gLastTrafficAlert = L"Traffic before " + toWide(gScenarioGraph.getNodeName(nextNode)) + L". Choose an alternative route.";

                        PathResult rerouted;
                        int selectedAlternative = -1;
                        if (!gTrafficAlternatives.empty()) {
                            selectedAlternative = askUserToSelectAlternative(hwnd, gScenarioGraph, gTrafficAlternatives, nextNode);
                        }

                        if (selectedAlternative >= 0 && selectedAlternative < static_cast<int>(gTrafficAlternatives.size())) {
                            rerouted = gTrafficAlternatives[selectedAlternative];
                            gDisplayedRoute = rerouted.path;
                            gLastTrafficAlert = L"Using Alternative " + intToWide(selectedAlternative + 1) +
                                L" before entering traffic near " + toWide(gScenarioGraph.getNodeName(nextNode));
                        } else {
                            gDisplayedRoute = gSelectedAnimationResult.path;
                            gLastTrafficAlert = L"You kept current route; traffic zone ahead near " + toWide(gScenarioGraph.getNodeName(nextNode));
                        }

                        if (rerouted.reachable && rerouted.path.size() > 1) {
                            std::vector<int> mergedPath = gAnimatedRoute;
                            mergedPath.insert(mergedPath.end(), rerouted.path.begin() + 1, rerouted.path.end());
                            gSelectedAnimationResult.path = mergedPath;
                            gDisplayedRoute = mergedPath;
                            gSelectedAnimationResult.hopCount = static_cast<int>(mergedPath.size()) - 1;
                            gSelectedAnimationResult.weightedCost = calculatePathWeight(gScenarioGraph, mergedPath);
                        }
                    }
                }

                ++gAnimationStep;
                gAnimatedRoute.push_back(gSelectedAnimationResult.path[gAnimationStep]);
                InvalidateRect(hwnd, NULL, TRUE);
            } else {
                stopAnimation(hwnd);
                InvalidateRect(hwnd, NULL, TRUE);
            }
        }
        return 0;
    }

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        paintScene(hwnd, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        stopAnimation(hwnd);
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(hwnd, message, wParam, lParam);
}
} // namespace

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int cmdShow) {
    const wchar_t className[] = L"ShortestPathVisualizerWindow";

    WNDCLASSW windowClass = {};
    windowClass.lpfnWndProc = WindowProc;
    windowClass.hInstance = instance;
    windowClass.lpszClassName = className;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    RegisterClassW(&windowClass);

    HWND window = CreateWindowExW(
        0,
        className,
        L"Shortest Path Finder & Visualizer",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        1360,
        920,
        NULL,
        NULL,
        instance,
        NULL
    );

    if (window == NULL) {
        return 0;
    }

    ShowWindow(window, cmdShow);
    UpdateWindow(window);

    MSG message = {};
    while (GetMessageW(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return static_cast<int>(message.wParam);
}
