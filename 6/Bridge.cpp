#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include <string>

using namespace std;

// 定义帧结构
struct Frame {
    int srcAddr;  // 源地址 (4 bit)
    int srcPort;  // 源端口 (1 或 2)，其实会更多
    int destAddr; // 目的地址 (4 bit, 0xF 为广播)
    // 其他不重要的代码
    int type;
    string data;
    long long crc;
};

class Bridge {
private:
    // MAC地址表: <MAC地址, 端口>
    map<int, int> macTable;

public:
    void processFrame(const Frame& f) {
        cout << "收到帧: [源:0x" << hex << f.srcAddr 
             << ", 端口:" << dec << f.srcPort 
             << ", 目的:0x" << hex << f.destAddr << "] -> ";

        // 1. 自学习：记录或更新源地址对应的端口
        macTable[f.srcAddr] = f.srcPort;

        // 2. 转发逻辑
        if (f.destAddr == 0xF) {
            // 广播帧
            cout << "目的端口: 广播 (Flood 到所有其他端口)" << endl;
        } 
        else if (macTable.find(f.destAddr) == macTable.end()) {
            // 查不到目的地址，泛洪
            cout << "目的端口: 未知地址 (Flood 到所有其他端口)" << endl;
        } 
        else {
            int destPort = macTable[f.destAddr];
            if (destPort == f.srcPort) {
                // 目的地址在同一个网段，丢弃（过滤）
                cout << "目的端口: 过滤 (目的地址在同一网段，不转发)" << endl;
            } else {
                // 找到明确的转发端口
                cout << "目的端口: " << dec << destPort << " (转发)" << endl;
            }
        }
    }

    void printMACTable() {
        cout << "\n--- 当前 MAC 地址表 ---" << endl;
        cout << "地址 (MAC) | 端口 (Port)" << endl;
        cout << "-----------|-----------" << endl;
        for (auto const& [addr, port] : macTable) {
            cout << "  0x" << hex << uppercase << addr 
                 << "      |    " << dec << port << endl;
        }
        cout << "-----------------------\n" << endl;
    }
};

int main() {
    Bridge myBridge;

    // 模拟一组输入帧
    vector<Frame> testFrames = {
        {0xA, 1, 0xB}, // A 在端口1 发给 B (此时表空，Flood)
        {0xB, 2, 0xA}, // B 在端口2 发给 A (此时表有A，转发到端口1)
        {0xC, 1, 0xA}, // C 在端口1 发给 A (此时表有A，但在同一网段，过滤)
        {0xD, 2, 0xF}, // D 在端口2 发送广播帧
        {0xE, 1, 0xB}  // E 在端口1 发给 B (转发到端口2)
    };

    for (const auto& f : testFrames) {
        myBridge.processFrame(f);
    }

    myBridge.printMACTable();

    return 0;
}