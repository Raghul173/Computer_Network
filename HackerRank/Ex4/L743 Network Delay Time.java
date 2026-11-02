import java.util.*;

// LeetCode 743: Network Delay Time
// Approach: Dijkstra's Shortest Path Algorithm using Min-Heap
// Direct application of routing algorithms (Ex4 Dijkstra)
// Time Complexity: O(E log V)
// Space Complexity: O(V + E)

class Solution {
    public int networkDelayTime(int[][] times, int n, int k) {
        // times[i] = [u, v, w]
        List<List<int[]>> graph = new ArrayList<>();
        for (int i = 0; i <= n; i++) {
            graph.add(new ArrayList<>());
        }

        for (int[] time : times) {
            graph.get(time[0]).add(new int[]{time[1], time[2]});
        }

        // Min-heap storing [node, distance_from_k]
        PriorityQueue<int[]> pq = new PriorityQueue<>(Comparator.comparingInt(a -> a[1]));
        int[] dist = new int[n + 1];
        Arrays.fill(dist, Integer.MAX_VALUE);

        dist[k] = 0;
        pq.offer(new int[]{k, 0});

        while (!pq.isEmpty()) {
            int[] curr = pq.poll();
            int u = curr[0];
            int d = curr[1];

            if (d > dist[u]) continue;

            for (int[] neighbor : graph.get(u)) {
                int v = neighbor[0];
                int weight = neighbor[1];

                if (dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    pq.offer(new int[]{v, dist[v]});
                }
            }
        }

        int maxDelay = 0;
        for (int i = 1; i <= n; i++) {
            if (dist[i] == Integer.MAX_VALUE) return -1;
            maxDelay = Math.max(maxDelay, dist[i]);
        }

        return maxDelay;
    }
}
