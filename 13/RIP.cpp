#include <bits/stdc++.h>
using namespace std;

const int INF = 16; // RIP 最大跳数

struct Route {
    string dest;   // 目的网络
    int cost;      // 距离
    string next;   // 下一跳
};

void printTable(vector<Route>& table) {
    cout << "Dest\tCost\tNext\n";
    for (auto &r : table) {
        cout << r.dest << "\t" << r.cost << "\t" << r.next << endl;
    }
}

// 在R1中查找目的网络
int findRoute(vector<Route>& table, string dest) {
    for (int i = 0; i < table.size(); i++) {
        if (table[i].dest == dest) return i;
    }
    return -1;
}

int main() {
    vector<Route> R1, R2;

    int n, m;
    cout << "输入R1路由表条目数:\n";
    cin >> n;

    cout << "输入R1路由表 (dest cost next):\n";
    for (int i = 0; i < n; i++) {
        Route r;
        cin >> r.dest >> r.cost >> r.next;
        R1.push_back(r);
    }

    cout << "输入R2路由表条目数:\n";
    cin >> m;

    cout << "输入R2路由表 (dest cost):\n";
    for (int i = 0; i < m; i++) {
        Route r;
        cin >> r.dest >> r.cost;
        r.next = "R2";
        R2.push_back(r);
    }

    // RIP 更新
    for (auto &r2 : R2) {
        int newCost = min(r2.cost + 1, INF);

        int idx = findRoute(R1, r2.dest);

        if (idx == -1) {
            // 新增
            R1.push_back({r2.dest, newCost, "R2"});
        } else {
            // 已存在
            if (R1[idx].next == "R2" || newCost < R1[idx].cost) {
                R1[idx].cost = newCost;
                R1[idx].next = "R2";
            }
        }
    }

    cout << "\n更新后的R1路由表:\n";
    printTable(R1);

    return 0;
}