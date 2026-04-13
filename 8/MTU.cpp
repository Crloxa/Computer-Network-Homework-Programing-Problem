#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

struct Fragment {
    int length;   // 总长度（包含20字节头）
    int offset;   // 偏移（单位：8字节）
};

vector<Fragment> fragmentPacket(int packetLength,
                               const vector<int>& pathMTUs) {
    const int HEADER = 20;

    // 初始只有一个分片
    vector<pair<int,int>> frags; 
    // pair<data_length, offset_in_bytes>
    frags.push_back({packetLength - HEADER, 0});

    for (int mtu : pathMTUs) {
        vector<pair<int,int>> newFrags;

        int maxData = mtu - HEADER;
        int alignedSize = (maxData / 8) * 8;

        for (auto &frag : frags) {
            int dataLen = frag.first;
            int offset = frag.second;

            // 不需要分片
            if (dataLen + HEADER <= mtu) {
                newFrags.push_back(frag);
                continue;
            }

            // 需要分片
            int remaining = dataLen;
            int currentOffset = offset;

            while (remaining > maxData) {
                newFrags.push_back({alignedSize, currentOffset});
                currentOffset += alignedSize;
                remaining -= alignedSize;
            }

            // 最后一片
            newFrags.push_back({remaining, currentOffset});
        }

        frags = newFrags;
    }

    // 转换为结果格式
    vector<Fragment> result;
    for (auto &f : frags) {
        Fragment frag;
        frag.length = f.first + HEADER;
        frag.offset = f.second / 8;  // 转换为8字节单位
        result.push_back(frag);
    }

    return result;
}

int main() {

    int packetLength = 4000;  // 总长度（含头）
    vector<int> pathMTUs = {1500, 1000, 600};

    cout << "Packet Length: " << packetLength << endl;
    cout << "Path MTUs: ";
    for (int mtu : pathMTUs) cout << mtu << " ";
    cout << endl << endl;

    vector<Fragment> result = fragmentPacket(packetLength, pathMTUs);

    // ===== 输出结果 =====
    cout << "Fragments:" << endl;
    cout << "---------------------------------\n";
    cout << "Index\tLength\tOffset(8B units)\n";
    cout << "---------------------------------\n";

    for (int i = 0; i < result.size(); i++) {
        cout << i << "\t"
             << result[i].length << "\t"
             << result[i].offset << endl;
    }

    cout << "---------------------------------\n";
    cout << "Total Fragments: " << result.size() << endl;

    return 0;
}