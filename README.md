# Shortest Path Finder & Visualizer

A 100% C++ shortest-path project with both console and GUI versions. It simulates city-to-city route planning, local-area navigation, traffic-aware rerouting, and time-based route changes.

## What This Project Does

- Finds shortest paths using Dijkstra's algorithm.
- Compares Dijkstra vs BFS.
- Simulates morning vs night traffic conditions.
- Shows alternative routes when traffic is detected.
- Supports both intercity routes and local area maps.
- Provides a Windows GUI visualizer with animated routes.

## Files

- [graph.h](graph.h) and [graph.cpp](graph.cpp) - graph data model and utilities.
- [algorithms.h](algorithms.h) and [algorithms.cpp](algorithms.cpp) - Dijkstra, BFS, path formatting, and weight calculations.
- [main.cpp](main.cpp) - console version.
- [gui_main.cpp](gui_main.cpp) - WinAPI GUI version.
- [visualizer.h](visualizer.h) and [visualizer.cpp](visualizer.cpp) - console ASCII map visualizer.

## Data Structures Used

### 1. Graph using adjacency list
The core structure is an adjacency-list graph.

- Each city/node is stored in a `vector<Node>`.
- Each `Node` stores a `vector<Edge>` of connected roads.
- Each `Edge` stores:
  - destination node index
  - road weight

This is efficient for sparse and medium-sized graphs because it stores only actual roads, not all possible city pairs.

### 2. `vector`
Used throughout the project for:

- storing nodes
- storing edges
- storing routes
- storing visit orders
- storing local-area layouts
- storing traffic alternatives

### 3. `queue`
Used in BFS to explore nodes level by level.

### 4. `priority_queue`
Used in Dijkstra to always process the node with the current smallest distance first.

### 5. `pair<int, int>`
Used for weighted edge-like data in priority queues and route handling.

### 6. `struct`
Used to group related data cleanly:

- `Node`
- `Edge`
- `PathResult`
- `Point`
- `RoadDef`
- `LocalMap`

## Data Structure Concepts Used

### 1. Weighted Graph
The road network is a weighted graph where each edge has a cost, representing distance or traffic-adjusted travel time.

### 2. Adjacency List Representation
Instead of storing a full matrix, only connected roads are stored. This keeps memory usage lower and traversal efficient.

### 3. Traversal
- BFS explores by number of edges.
- Dijkstra explores by minimum accumulated weight.

### 4. Greedy Choice
Dijkstra uses a greedy strategy with a min-priority queue to always expand the current best candidate first.

### 5. Path Reconstruction
Both algorithms store parent nodes so the final path can be rebuilt after search completes.

### 6. Traffic Simulation
Road weights are adjusted by time-of-day traffic profiles.

### 7. Dynamic Rerouting
When traffic is detected, the system computes alternative routes before entering the congested segment.

### 8. Spatial Mapping
The GUI and ASCII console visualizers place nodes on a 2D map layout, which makes the graph easier to understand visually.

## Performance

### Dijkstra
- Time complexity: `O(E log V)`
- Space complexity: `O(V + E)`

### BFS
- Time complexity: `O(V + E)`
- Space complexity: `O(V)`

### Visualizer
- The GUI redraws the scene on each update.
- Node positions are scaled to fit the available map rectangle.
- Footer text is wrapped and compacted so important details stay visible.

### Practical Behavior
- Fast enough for the current city network and demo-sized graphs.
- Real-time rerouting is responsive because the graph is small and the search space is limited.

## Best Demo Examples

### Example 1: Mumbai to Hyderabad
This is the best full demo path for judges.

Why it is good:
- shows a long multi-city route
- shows traffic-aware rerouting
- shows alternative routes
- shows time-based route changes
- looks visually impressive in the GUI

Suggested demo flow:
1. Choose `Intercity` scope.
2. Set source to `Mumbai`.
3. Set destination to `Hyderabad`.
4. Set algorithm to `Dijkstra`.
5. Use `Auto (Current Time)` or manually try `Morning` and `Night`.
6. Click `Compare`.
7. Click `Animate`.
8. When traffic appears, choose an alternative route in the popup.

What to say:
- "The app detects traffic before entering a congested zone."
- "It computes alternative routes dynamically."
- "The selected reroute is drawn immediately on the map."

### Example 2: Nashik Local Area Demo
This is the best local-area demo.

Suggested flow:
1. Choose `Local Areas` scope.
2. Select city `Nashik`.
3. Choose source area `Panchavati`.
4. Choose destination area `Adgaon`.
5. Click `Compare`.
6. Click `Animate`.

Why it is good:
- clearly demonstrates city-local routing
- easier to explain than intercity at first
- good for showing the city-specific map logic

### Example 3: Morning vs Night Comparison
This is ideal for demonstrating time-based route changes.

Suggested flow:
1. Keep the same source/destination.
2. Run once in `Morning`.
3. Run again in `Night`.
4. Compare the weights and the selected route.

What to highlight:
- traffic changes route cost
- same trip can produce different best routes
- route selection adapts to time profile

### Example 4: Traffic Alert and Alternative Choice
This is best for proving the smart-routing part.

Suggested flow:
1. Pick a route that crosses a busy city or congested segment.
2. Start animation.
3. Wait for the traffic popup.
4. Choose Alternative 1 or Alternative 2.

What to highlight:
- popup appears before entering traffic
- the app avoids the congested city/segment when possible
- the orange route on the graph updates to the selected alternative

## How To Run

### Console version
Build and run the console app using your compiler setup.

### GUI version
Build the WinAPI version from `gui_main.cpp` and run the generated executable.

## Notes

- The project is written in pure C++.
- The GUI uses WinAPI, so it is Windows-only.
- The current implementation supports both intercity routing and city-local routing.
- Traffic logic is time-aware and can auto-select the current profile.

## Presentation Tip

A strong 30-second line for demo:

"This project is a traffic-aware shortest path visualizer in pure C++. It compares Dijkstra and BFS, simulates morning and night traffic, supports local city maps, and dynamically reroutes before entering a congested area."
