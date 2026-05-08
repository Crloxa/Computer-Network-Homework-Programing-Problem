#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
using namespace std;

const int INF = 1e9;

int main() {
    int n, m, start;
    cout << "输入节点数 边数 起点:\n";
    cin >> n >> m >> start;

    vector<vector<pair<int,int>>> graph(n + 1);

    cout << "输入边 (u v weight):\n";
    for (int i = 0; i < m; i++) {
        int u, v, w;
        cin >> u >> v >> w;
        graph[u].push_back({v, w});
        graph[v].push_back({u, w}); // 无向图
    }

    vector<int> dist(n + 1, INF);
    vector<bool> vis(n + 1, false);

    dist[start] = 0;

    for (int i = 1; i <= n; i++) {
        int u = -1, minDist = INF;

        for (int j = 1; j <= n; j++) {
            if (!vis[j] && dist[j] < minDist) {
                u = j;
                minDist = dist[j];
            }
        }

        if (u == -1) break;

        vis[u] = true;

        for (auto &edge : graph[u]) {
            int v = edge.first;
            int w = edge.second;

            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
            }
        }
    }
    cout<< endl;
    cout << "最短路径结果:";
    for (int i = 1; i <= n; i++) {
        cout << "到节点 " << i << " 的距离: " << dist[i] << endl;
    }

    return 0;
}