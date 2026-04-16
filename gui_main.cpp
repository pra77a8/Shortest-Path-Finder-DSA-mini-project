#include "algorithms.h"

#include <windows.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

namespace {
struct Point {
    int x;
    int y;
};

const int kStartComboId = 1001;
const int kTargetComboId = 1002;
const int kAlgorithmComboId = 1006;
const int kCompareButtonId = 1003;
const int kAnimateButtonId = 1004;
const int kResetButtonId = 1005;

const int kUiTop = 10;
const int kMapTop = 80;

const Point kLayout[] = {
    {90, 170},    // Mumbai
    {210, 250},   // Pune
    {340, 150},   // Nashik
    {520, 230},   // Surat
    {390, 380},   // Indore
    {620, 420},   // Nagpur
    {120, 420},   // Goa
    {700, 150},   // Ahmedabad
    {260, 360},   // Kolhapur
    {360, 250},   // Aurangabad
    {520, 320},   // Bhopal
    {560, 520}    // Hyderabad
};

const std::pair<int, int> kRoads[] = {
    {0, 1}, {0, 2}, {0, 3}, {1, 6}, {1, 2}, {2, 4}, {2, 7},
    {3, 7}, {3, 4}, {4, 5}, {5, 6}, {6, 7}, {0, 6}, {1, 3}, {4, 7},
    {1, 8}, {8, 6}, {1, 9}, {2, 9}, {9, 10}, {10, 4}, {10, 3},
    {4, 11}, {5, 11}, {9, 11}
};

Graph gGraph;
HWND gStartCombo = NULL;
HWND gTargetCombo = NULL;
HWND gAlgorithmCombo = NULL;
HWND gMainWindow = NULL;

PathResult gDijkstraResult;
PathResult gBfsResult;
PathResult gSelectedAnimationResult;
std::vector<int> gAnimatedRoute;

int gStartIndex = 0;
int gTargetIndex = 4;
int gAlgorithmIndex = 0;
int gAnimationStep = -1;
bool gAnimating = false;

std::wstring toWide(const std::string& text) {
    return std::wstring(text.begin(), text.end());
}

std::wstring intToWide(int value) {
    std::wstringstream stream;
    stream << value;
    return stream.str();
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

void drawLine(HDC hdc, Point a, Point b, COLORREF color, int width) {
    HPEN pen = CreatePen(PS_SOLID, width, color);
    HPEN oldPen = static_cast<HPEN>(SelectObject(hdc, pen));
    MoveToEx(hdc, a.x, a.y, NULL);
    LineTo(hdc, b.x, b.y);
    SelectObject(hdc, oldPen);
    DeleteObject(pen);
}

void drawNode(HDC hdc, int index, COLORREF fillColor, COLORREF textColor, bool bold) {
    const Point p = kLayout[index];
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
        18, 0, 0, 0, bold ? FW_BOLD : FW_NORMAL, FALSE, FALSE, FALSE,
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

    std::wstring name = toWide(gGraph.getNodeName(index));
    RECT labelRect = {p.x - 60, p.y + 20, p.x + 60, p.y + 44};
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

void computeRoutes() {
    gDijkstraResult = runDijkstra(gGraph, gStartIndex, gTargetIndex);
    gBfsResult = runBFS(gGraph, gStartIndex, gTargetIndex);
}

void updateSelectionFromControls() {
    gStartIndex = static_cast<int>(SendMessageW(gStartCombo, CB_GETCURSEL, 0, 0));
    gTargetIndex = static_cast<int>(SendMessageW(gTargetCombo, CB_GETCURSEL, 0, 0));
    gAlgorithmIndex = static_cast<int>(SendMessageW(gAlgorithmCombo, CB_GETCURSEL, 0, 0));

    if (gStartIndex < 0) {
        gStartIndex = 0;
    }
    if (gTargetIndex < 0) {
        gTargetIndex = 0;
    }

    if (gStartIndex == gTargetIndex) {
        gTargetIndex = (gTargetIndex + 1) % gGraph.getNodeCount();
        SendMessageW(gTargetCombo, CB_SETCURSEL, gTargetIndex, 0);
    }

    if (gAlgorithmIndex < 0) {
        gAlgorithmIndex = 0;
        SendMessageW(gAlgorithmCombo, CB_SETCURSEL, gAlgorithmIndex, 0);
    }
}

void startAnimation(HWND hwnd) {
    updateSelectionFromControls();
    computeRoutes();

    gSelectedAnimationResult = (gAlgorithmIndex == 1) ? gBfsResult : gDijkstraResult;

    gAnimatedRoute.clear();
    gAnimationStep = 0;
    gAnimating = gSelectedAnimationResult.reachable;
    if (gAnimating) {
        SetTimer(hwnd, 1, 700, NULL);
    }
    InvalidateRect(hwnd, NULL, TRUE);
}

void stopAnimation(HWND hwnd) {
    gAnimating = false;
    KillTimer(hwnd, 1);
}

void resetView(HWND hwnd) {
    stopAnimation(hwnd);
    gAnimatedRoute.clear();
    gAnimationStep = -1;
    computeRoutes();
    InvalidateRect(hwnd, NULL, TRUE);
}

void paintScene(HWND hwnd, HDC hdc) {
    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    HBRUSH backgroundBrush = CreateSolidBrush(RGB(245, 248, 252));
    FillRect(hdc, &clientRect, backgroundBrush);
    DeleteObject(backgroundBrush);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, RGB(25, 25, 25));

    HFONT titleFont = CreateFontW(
        26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
    );
    HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, titleFont));

    std::wstring title = L"Shortest Path Finder - WinAPI GUI";
    TextOutW(hdc, 20, 18, title.c_str(), static_cast<int>(title.size()));

    SelectObject(hdc, oldFont);
    DeleteObject(titleFont);

    RECT mapRect = {20, kMapTop, clientRect.right - 20, clientRect.bottom - 20};
    HBRUSH mapBrush = CreateSolidBrush(RGB(255, 255, 255));
    FrameRect(hdc, &mapRect, static_cast<HBRUSH>(GetStockObject(GRAY_BRUSH)));
    FillRect(hdc, &mapRect, mapBrush);
    DeleteObject(mapBrush);

    std::vector<int> routeToDraw;
    if (gSelectedAnimationResult.reachable) {
        routeToDraw = gSelectedAnimationResult.path;
    }
    if (gAnimating) {
        routeToDraw = gAnimatedRoute;
    }

    for (size_t i = 0; i < sizeof(kRoads) / sizeof(kRoads[0]); ++i) {
        const int a = kRoads[i].first;
        const int b = kRoads[i].second;
        COLORREF color = RGB(180, 180, 190);
        int width = 2;

        if (isEdgeInPath(routeToDraw, a, b)) {
            color = RGB(255, 140, 0);
            width = 4;
        }

        drawLine(hdc, kLayout[a], kLayout[b], color, width);
    }

    for (int i = 0; i < gGraph.getNodeCount(); ++i) {
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

        drawNode(hdc, i, fill, textColor, bold);
    }

    HFONT textFont = CreateFontW(
        18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
    );
    oldFont = static_cast<HFONT>(SelectObject(hdc, textFont));

    std::wstring dijkstraLine = L"Dijkstra: ";
    if (gDijkstraResult.reachable) {
        dijkstraLine += L"distance=" + intToWide(gDijkstraResult.weightedCost) + L" km, hops=" + intToWide(gDijkstraResult.hopCount);
    } else {
        dijkstraLine += L"No path";
    }

    std::wstring bfsLine = L"BFS: ";
    if (gBfsResult.reachable) {
        bfsLine += L"distance=" + intToWide(gBfsResult.weightedCost) + L" km, hops=" + intToWide(gBfsResult.hopCount);
    } else {
        bfsLine += L"No path";
    }

    std::wstring dijkstraWeight = L"Weight (Dijkstra): ";
    dijkstraWeight += gDijkstraResult.reachable ? (intToWide(gDijkstraResult.weightedCost) + L" km") : L"N/A";

    std::wstring bfsWeight = L"Weight (BFS): ";
    bfsWeight += gBfsResult.reachable ? (intToWide(gBfsResult.weightedCost) + L" km") : L"N/A";

    TextOutW(hdc, 20, 595, dijkstraLine.c_str(), static_cast<int>(dijkstraLine.size()));
    TextOutW(hdc, 20, 620, dijkstraWeight.c_str(), static_cast<int>(dijkstraWeight.size()));
    TextOutW(hdc, 20, 650, bfsLine.c_str(), static_cast<int>(bfsLine.size()));
    TextOutW(hdc, 20, 675, bfsWeight.c_str(), static_cast<int>(bfsWeight.size()));

    std::wstring selectedAlgorithm = L"Selected animation algorithm: ";
    selectedAlgorithm += (gAlgorithmIndex == 1) ? L"BFS" : L"Dijkstra";
    TextOutW(hdc, 420, 620, selectedAlgorithm.c_str(), static_cast<int>(selectedAlgorithm.size()));

    if (gAnimating && gAnimationStep >= 0 && gAnimationStep < static_cast<int>(gSelectedAnimationResult.path.size())) {
        std::wstring current = L"Current location: " + toWide(gGraph.getNodeName(gSelectedAnimationResult.path[gAnimationStep]));
        TextOutW(hdc, 420, 650, current.c_str(), static_cast<int>(current.size()));
    }

    SelectObject(hdc, oldFont);
    DeleteObject(textFont);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE: {
        gMainWindow = hwnd;
        gGraph = buildCityGraph();

        CreateWindowW(L"STATIC", L"Source:", WS_VISIBLE | WS_CHILD,
                      20, kUiTop + 42, 70, 24, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Destination:", WS_VISIBLE | WS_CHILD,
                      280, kUiTop + 42, 100, 24, hwnd, NULL, NULL, NULL);
        CreateWindowW(L"STATIC", L"Algorithm:", WS_VISIBLE | WS_CHILD,
                  545, kUiTop + 42, 90, 24, hwnd, NULL, NULL, NULL);

        gStartCombo = CreateWindowW(
            L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            90, kUiTop + 38, 160, 300, hwnd, reinterpret_cast<HMENU>(kStartComboId), NULL, NULL
        );

        gTargetCombo = CreateWindowW(
            L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            380, kUiTop + 38, 160, 300, hwnd, reinterpret_cast<HMENU>(kTargetComboId), NULL, NULL
        );

        gAlgorithmCombo = CreateWindowW(
            L"COMBOBOX", L"", WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST,
            625, kUiTop + 38, 120, 100, hwnd, reinterpret_cast<HMENU>(kAlgorithmComboId), NULL, NULL
        );

        CreateWindowW(L"BUTTON", L"Compare", WS_VISIBLE | WS_CHILD,
                      760, kUiTop + 36, 90, 28, hwnd, reinterpret_cast<HMENU>(kCompareButtonId), NULL, NULL);
        CreateWindowW(L"BUTTON", L"Animate", WS_VISIBLE | WS_CHILD,
                      860, kUiTop + 36, 90, 28, hwnd, reinterpret_cast<HMENU>(kAnimateButtonId), NULL, NULL);
        CreateWindowW(L"BUTTON", L"Reset", WS_VISIBLE | WS_CHILD,
                      760, kUiTop + 68, 90, 28, hwnd, reinterpret_cast<HMENU>(kResetButtonId), NULL, NULL);

        for (int i = 0; i < gGraph.getNodeCount(); ++i) {
            std::wstring city = intToWide(i) + L" - " + toWide(gGraph.getNodeName(i));
            SendMessageW(gStartCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(city.c_str()));
            SendMessageW(gTargetCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(city.c_str()));
        }

        SendMessageW(gAlgorithmCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Dijkstra"));
        SendMessageW(gAlgorithmCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"BFS"));

        SendMessageW(gStartCombo, CB_SETCURSEL, gStartIndex, 0);
        SendMessageW(gTargetCombo, CB_SETCURSEL, gTargetIndex, 0);
        SendMessageW(gAlgorithmCombo, CB_SETCURSEL, gAlgorithmIndex, 0);

        computeRoutes();
        gSelectedAnimationResult = gDijkstraResult;
        return 0;
    }

    case WM_COMMAND: {
        const int controlId = LOWORD(wParam);
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
            if (gAnimationStep < static_cast<int>(gSelectedAnimationResult.path.size())) {
                gAnimatedRoute.assign(gSelectedAnimationResult.path.begin(), gSelectedAnimationResult.path.begin() + gAnimationStep + 1);
                ++gAnimationStep;
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
        980,
        760,
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