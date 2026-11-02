import java.util.*;

// LeetCode 1466: Reorder Routes to Make All Paths Lead to the City Zero
// Approach: Depth-First Search (DFS) / BFS treating directed tree as undirected graph with edge flags
// Time Complexity: O(N)
// Space Complexity: O(N)

class Solution {
    public int minReorder(int n, int[][] connections) {
        // Build adjacency list: node -> (neighbor, originalDirection)
        // originalDirection = 1 if the directed edge is u -> v (needs reversal so v can reach 0)
        // originalDirection = 0 if the directed edge is v -> u (already pointing towards 0)
        List<List<int[]>> adj = new ArrayList<>();
        for (int i = 0; i < n; i++) {
            adj.add(new ArrayList<>());
        }

        for (int[] conn : connections) {
            int u = conn[0];
            int v = conn[1];
            adj.get(u).add(new int[]{v, 1});
            adj.get(v).add(new int[]{u, 0});
        }

        boolean[] visited = new boolean[n];
        return dfs(0, adj, visited);
    }

    private int dfs(int node, List<List<int[]>> adj, boolean[] visited) {
        visited[node] = true;
        int changes = 0;

        for (int[] edge : adj.get(node)) {
            int next = edge[0];
            int direction = edge[1];

            if (!visited[next]) {
                changes += direction + dfs(next, adj, visited);
            }
        }

        return changes;
    }
}
