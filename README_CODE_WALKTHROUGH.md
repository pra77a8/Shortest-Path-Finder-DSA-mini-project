# Code Walkthrough with Line References

This document explains the logic of each project file and points to the important lines in code.

## 1. Graph Layer

### graph.h
- `Edge` structure (destination + weight): line 10
- `Node` structure (city name + outgoing edges): line 15
- `Graph` class declaration: line 20
- Public API:
  - `addNode`: line 26
  - `addEdge`: line 27
  - `display`: line 28
  - `getNodeCount`: line 29
  - `getNodeName`: line 30
  - `getEdges`: line 31
  - `findNode`: line 32
  - `isValidIndex`: line 33

### graph.cpp
- Constructor: line 5
- `addNode` implementation: line 7
- `addEdge` implementation with index validation: line 13
- `display` graph printer: line 22
- Accessor implementations:
  - `getNodeCount`: line 33
  - `getNodeName`: line 35
  - `getEdges`: line 37
- Lookup and validation:
  - `findNode`: line 39
  - `isValidIndex`: line 48

## 2. Algorithm Layer

### algorithms.h
- `PathResult` data structure: line 10
- Algorithms and helpers:
  - `runDijkstra`: line 18
  - `runBFS`: line 19
  - `calculatePathWeight`: line 20
  - `formatPath`: line 21

### algorithms.cpp
- Infinity constant (`kInf`): line 8
- Path reconstruction helper (`rebuildPath`): line 10
- Dijkstra implementation: line 33
  - Distance/parent setup: line 40
  - Build output path: line 82
- BFS implementation: line 92
  - Build output path: line 126
- Path weight calculator: line 136
- Path formatting for display: line 163

## 3. Console App Layer

### main.cpp
- UI helpers:
  - `printTitle`: line 10
  - `printCities`: line 16
  - `readCityChoice`: line 23
  - `printVisitOrder`: line 37
- Compare output for Dijkstra vs BFS: line 47
- Algorithm selection for animation: `chooseRouteByAlgorithm` line 78
- Animation entry for console: `showAnimatedRoute` line 85
- Demo graph construction: `buildCityGraph` line 113
- Program entry + menu loop: `main` line 160

## 4. Console Visualizer Layer

### visualizer.h
- Public animation API: `animateShortestRoute` line 6

### visualizer.cpp
- ANSI setup for color/clear behavior: line 57
- Node marker logic: line 96
- Core raster line drawing on ASCII canvas: line 146
- Road drawing: line 181
- Node drawing: line 203
- Legend/header:
  - `printCityLegend`: line 209
  - `printHeader`: line 220
- Frame render function: `drawFrame` line 231
- Animation loop: `animateShortestRoute` line 248

## 5. GUI App Layer (WinAPI)

### gui_main.cpp
This file contains the full GUI workflow: data setup, traffic simulation, reroute logic, rendering, and event loop.

#### Global state and traffic variables
- Alternative route storage: line 93
- Alerted traffic edges: line 94
- Last traffic message: line 95
- Active blocked traffic node: line 96

#### Traffic model and time profile
- Weight transform rule: `applyTrafficRule` line 119
- Auto morning/night profile: `effectiveTrafficTimeIndex` line 142

#### Data initialization
- Local map data initialization: `initializeLocalMaps` line 156
- Intercity graph build: `buildIntercityGraph` line 256
- Local graph build: `buildLocalGraph` line 272

#### Scenario graph builders
- Base builder by mode: `buildScenarioGraph` line 422
- Edge-blocked builder: `buildScenarioGraphAvoidEdge` line 429
- Node-blocked builder: `buildScenarioGraphAvoidNode` line 469

#### Traffic-aware rerouting
- Penalty for routes near traffic zone: `pathTrafficPenalty` line 575
- Alternative generation: `buildAlternativeRoutes` line 595
- User pick dialog (Alt 1 / Alt 2 / keep): `askUserToSelectAlternative` line 630

#### Route computation and animation
- Main compute pass: `computeRoutes` line 657
- Start animation: `startAnimation` line 686
- Reset animation and state: `resetView` line 707

#### Rendering
- Coordinate scaling to map rectangle: `scalePointsToRect` line 776
- Complete render pass: `paintScene` line 821

#### WinAPI event flow
- Window procedure: `WindowProc` line 1033
- Timer-based animation tick: `WM_TIMER` block line 1151
  - Traffic detection + popup: around line 1164 onwards
  - Alternative selection dialog call: line 1184
- App entrypoint: `WinMain` line 1237

## 6. Documentation Files

### README.md
- Project overview: line 1
- Data structures section: line 22
- Performance section: line 91
- Demo examples section: line 110

### README_CODE_WALKTHROUGH.md
- This file documents logic and line numbers for quick reviewer navigation.

## Suggested Reviewer Path

For fastest understanding, read in this order:
1. `graph.h` and `graph.cpp`
2. `algorithms.h` and `algorithms.cpp`
3. `main.cpp`
4. `visualizer.cpp`
5. `gui_main.cpp` (especially lines 575, 595, 630, 657, 821, 1151)

## Notes

- Line numbers are based on the current workspace state.
- If code changes, line references may shift and should be refreshed.

## Detailed Explanation (How the System Works)

This section explains the runtime logic in simple flow order.

### A. Graph and Data Setup

1. The app creates a weighted graph where each node is a city (or local area) and each edge is a road.
2. The graph is represented by an adjacency list (`vector<Node>` with `vector<Edge>` inside each node).
3. For GUI mode, road weights are adjusted using traffic multipliers (`morningPct`, `nightPct`) before graph construction.
4. In local mode, city-specific area maps are loaded first, then converted into a graph for search.

Why this design:
- Adjacency list is memory-efficient and fast for sparse real-world road networks.
- The same graph interface is reused for intercity and local routing.

### B. Route Computation

1. User selects source, destination, mode, time profile, and algorithm.
2. `computeRoutes` builds the scenario graph with the correct traffic profile.
3. Both Dijkstra and BFS are computed for comparison.
4. The currently selected algorithm route becomes the active display route.

Why this design:
- Dijkstra gives weighted shortest route.
- BFS gives fewest-hop route.
- Calculating both allows immediate side-by-side comparison in the footer.

### C. Time-Based Behavior

1. If user chooses Auto time mode, system clock decides profile (Morning/Night).
2. Effective traffic profile changes edge weights.
3. That can change shortest path, route cost, and recommended best time.

Why this design:
- Same map can have different optimal routes at different times.
- Demonstrates dynamic route planning instead of static shortest path.

### D. Animation Lifecycle

1. On Animate, the current selected path is stored.
2. Timer tick (`WM_TIMER`) advances one step at a time.
3. Current node is highlighted and map redraws every tick.
4. Footer updates current location and route context.

Why this design:
- Timer-driven stepping keeps GUI responsive.
- Visual state and algorithm state remain synchronized.

### E. Traffic Alert and Alternative Logic

1. Before entering next edge, system checks that segment traffic factor.
2. If threshold is high (heavy traffic), it triggers a warning popup before entry.
3. Alternative routes are generated by:
  - trying to avoid congested node/zone,
  - falling back to avoiding critical edge when needed.
4. Alternatives are scored using cost + traffic penalty.
5. User selects Alternative 1, Alternative 2, or keeps current route.
6. Selected alternative becomes displayed route and animation route.

Why this design:
- Prevents rerouting too late.
- Keeps user in control of decision.
- Makes orange route reflect chosen alternative immediately.

### F. Rendering Logic

1. Raw layout coordinates are scaled into the current map rectangle.
2. Footer area is reserved so map nodes do not overlap footer.
3. Routes, node states, traffic block, and alternatives are drawn with separate visual cues.

Why this design:
- Avoids clipping/overlap across different window sizes.
- Keeps route readability stable during demos.

### G. What to Explain During Viva/Demo

Use this quick explanation:

1. "I model roads as a weighted graph using adjacency list."
2. "I run Dijkstra for weighted shortest path and BFS for hop-minimal comparison."
3. "Road weights are time-sensitive, so morning/night can produce different best routes."
4. "Before entering congestion, the app warns and computes alternatives."
5. "User picks the reroute, and the selected route is shown immediately on the graph."

This connects data structures, algorithms, system behavior, and GUI output in one coherent story.
