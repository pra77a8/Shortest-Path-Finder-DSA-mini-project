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
