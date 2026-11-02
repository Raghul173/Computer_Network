import java.util.*;

// LeetCode 787: Cheapest Flights Within K Stops
// Approach: Bellman-Ford Algorithm with at most K stops
// Time Complexity: O(K * E) where E is number of flights
// Space Complexity: O(V) where V is number of cities

class Solution {
    public int findCheapestPrice(int n, int[][] flights, int src, int dst, int k) {
        int[] cost = new int[n];
        Arrays.fill(cost, Integer.MAX_VALUE);
        cost[src] = 0;

        // Perform at most k + 1 edge relaxations
        for (int i = 0; i <= k; i++) {
            int[] temp = Arrays.copyOf(cost, n);

            for (int[] flight : flights) {
                int u = flight[0];
                int v = flight[1];
                int price = flight[2];

                if (cost[u] != Integer.MAX_VALUE && cost[u] + price < temp[v]) {
                    temp[v] = cost[u] + price;
                }
            }

            cost = temp;
        }

        return cost[dst] == Integer.MAX_VALUE ? -1 : cost[dst];
    }
}
