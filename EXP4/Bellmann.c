// This C program constructs a directed graph from user-defined edges and weights, performs a Depth-First Search (DFS) to discover all possible simple paths from a designated source vertex to every other vertex, calculates their total costs, and identifies the shortest path for each destination.



#include <stdio.h>
#include <stdlib.h>

#define INF 99999
#define MAX 20

int adj[MAX][MAX];
int V;

// Array to store paths during DFS exploration
int pathBuffer[MAX];
int pathLen = 0;

// Structure to store path details
struct PathInfo {
    int nodes[MAX];
    int length;
    int cost;
};

// Global storage for paths to destinations
struct PathInfo allPaths[MAX][100];
int pathCount[MAX];

// Recursive Function to find ALL paths from current node to all destinations
void findAllPaths(int u, int visited[]) {
    visited[u] = 1;
    pathBuffer[pathLen++] = u;

    // Calculate current path cost from buffer
    int currentCost = 0;
    for (int i = 0; i < pathLen - 1; i++) {
        currentCost += adj[pathBuffer[i]][pathBuffer[i + 1]];
    }

    // Save this path for the current destination node `u`
    int dest = u;
    int count = pathCount[dest];
    allPaths[dest][count].length = pathLen;
    allPaths[dest][count].cost = currentCost;
    for (int i = 0; i < pathLen; i++) {
        allPaths[dest][count].nodes[i] = pathBuffer[i];
    }
    pathCount[dest]++;

    // Explore adjacent vertices
    for (int v = 0; v < V; v++) {
        if (adj[u][v] != INF && !visited[v]) {
            findAllPaths(v, visited);
        }
    }

    // Backtrack
    pathLen--;
    visited[u] = 0;
}

void printAllAndShortestPaths(int source) {
    int visited[MAX] = {0};

    // Initialize counts
    for (int i = 0; i < V; i++) {
        pathCount[i] = 0;
    }

    // Find all possible paths starting from source
    findAllPaths(source, visited);

    printf("\n=======================================================\n");
    printf("         ALL POSSIBLE PATHS & SHORTEST SELECTION       \n");
    printf("=======================================================\n");

    for (int dest = 0; dest < V; dest++) {
        printf("\n DESTINATION VERTEX: %d\n", dest);

        if (pathCount[dest] == 0) {
            printf("No path exists from %d to %d.\n", source, dest);
            continue;
        }

        printf("\nAll Possible Paths:\n");
        int minCost = INF;
        int minIndex = -1;

        for (int i = 0; i < pathCount[dest]; i++) {
            printf("  Path %d: ", i + 1);
            for (int j = 0; j < allPaths[dest][i].length; j++) {
                printf("%d", allPaths[dest][i].nodes[j]);
                if (j < allPaths[dest][i].length - 1) printf(" -> ");
            }
            printf(" | Cost = %d\n", allPaths[dest][i].cost);

            // Track minimum cost path
            if (allPaths[dest][i].cost < minCost) {
                minCost = allPaths[dest][i].cost;
                minIndex = i;
            }
        }

        // Print Selected Shortest Path
        printf("\nSELECTED SHORTEST PATH: ");
        for (int j = 0; j < allPaths[dest][minIndex].length; j++) {
            printf("%d", allPaths[dest][minIndex].nodes[j]);
            if (j < allPaths[dest][minIndex].length - 1) printf(" -> ");
        }
        printf(" (Min Cost = %d)\n", minCost);
        printf("-------------------------------------------------------\n");
    }
}

int main(void) {
    int E, source;

    printf("Enter number of vertices (|V|): ");
    if (scanf("%d", &V) != 1 || V <= 0 || V > MAX) return 1;

    // Initialize adjacency matrix with INF
    for (int i = 0; i < V; i++) {
        for (int j = 0; j < V; j++) {
            adj[i][j] = INF;
        }
    }

    printf("Enter number of edges (|E|): ");
    if (scanf("%d", &E) != 1 || E <= 0) return 1;

    printf("\nEnter edges in the format (u v w):\n");
    for (int i = 0; i < E; i++) {
        int u, v, w;
        printf("Edge %d (u v w): ", i + 1);
        if (scanf("%d %d %d", &u, &v, &w) == 3) {
            adj[u][v] = w;
        }
    }

    printf("\nEnter source vertex 's': ");
    if (scanf("%d", &source) != 1 || source < 0 || source >= V) return 1;

    printAllAndShortestPaths(source);

    return 0;
}
