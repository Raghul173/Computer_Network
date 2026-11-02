import java.util.*;

// LeetCode 815: Bus Routes
// Approach: Breadth-First Search (BFS) on Stops and Bus Routes
// Time Complexity: O(N * S) where N is number of routes and S is max stops in a route
// Space Complexity: O(N * S)

class Solution {
    public int numBusesToDestination(int[][] routes, int source, int target) {
        if (source == target) return 0;

        // Map each stop to the list of bus routes passing through it
        Map<Integer, List<Integer>> stopToRoutes = new HashMap<>();
        for (int i = 0; i < routes.length; i++) {
            for (int stop : routes[i]) {
                stopToRoutes.computeIfAbsent(stop, k -> new ArrayList<>()).add(i);
            }
        }

        if (!stopToRoutes.containsKey(source) || !stopToRoutes.containsKey(target)) {
            return -1;
        }

        Queue<Integer> queue = new LinkedList<>();
        Set<Integer> visitedStops = new HashSet<>();
        boolean[] visitedBuses = new boolean[routes.length];

        queue.offer(source);
        visitedStops.add(source);
        int busesTaken = 0;

        while (!queue.isEmpty()) {
            int size = queue.size();
            busesTaken++;

            for (int i = 0; i < size; i++) {
                int currStop = queue.poll();

                for (int busId : stopToRoutes.getOrDefault(currStop, Collections.emptyList())) {
                    if (visitedBuses[busId]) continue;
                    visitedBuses[busId] = true;

                    for (int nextStop : routes[busId]) {
                        if (nextStop == target) return busesTaken;
                        if (!visitedStops.contains(nextStop)) {
                            visitedStops.add(nextStop);
                            queue.offer(nextStop);
                        }
                    }
                }
            }
        }

        return -1;
    }
}
