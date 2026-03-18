#include <iostream>
#include <vector>
#include <cstring>
using namespace std;

enum MuxMode { TDM, STDM, FDM, CDM };


int multiplex(unsigned char *c, const int c_size, 
              const unsigned char *a, const int a_len, 
              const unsigned char *b, const int b_len, 
              MuxMode mode) {
    int c_index = 0;
    switch (mode) {
        case TDM:{
            for (int i = 0; i < a_len || i < b_len; ++i) {
                // A B 段轮流占用，即 TDM 每个都分配固定的时长
                if (i < a_len && c_index < c_size)
                    c[c_index++] = a[i]; 

                if (i < b_len && c_index < c_size)
                    c[c_index++] = b[i];
            }
        }
        break;
        case STDM:{
        // 真的要按照任务有无，切分起来太麻烦了，这里还是TDM的逻辑
        // 反正 STDM 就是谁有请求谁来发，这点知道就行了，需要保存 来源 索引 数据内容
            int max_len = std::max(a_len, b_len);
            for (int t = 0; t < max_len; ++t) {
                // A 在 t 时刻有数据
                if (t < a_len && a[t]) {
                    if (c_index + 3 > c_size) break;

                    c[c_index++] = 1;   // 来源 A
                    c[c_index++] = t;
                    c[c_index++] = a[t];
                }
                // B 在 t 时刻有数据
                if (t < b_len && b[t]) {
                    if (c_index + 3 > c_size) break;

                    c[c_index++] = 2;   // 来源 B
                    c[c_index++] = t;
                    c[c_index++] = b[t];
                }
            }
        }
        break;
        // 用bits模拟并行传输（AI的），说实话我真没想到怎么模拟FDM，我又没有频率
        case FDM:{
            int max_len = max(a_len, b_len);
            for (int i = 0; i < max_len; ++i) {
                if (c_index >= c_size) break;
                unsigned char val = 0;
                // 只有在 i 范围内才取值，否则该“频段”保持静默（0）
                if (i < a_len) val |= (a[i] & 0x0F);
                if (i < b_len) val |= ((b[i] & 0x0F) << 4);

                c[c_index++] = val;
            }
        }
        break;

        // CDM  这里上课没很搞懂正交以后怎么处理，AI问了问代码逻辑，有点理解了
        //A: [1, 1]
        //B: [1, -1]
        case CDM:{
            for (int i = 0; i < a_len && i < b_len; ++i) {
                if (c_index + 2 > c_size) break;

                // 信号转换为逻辑电平
                int sa = a[i] ? 1 : -1;
                int sb = b[i] ? 1 : -1;

                int d1 = sa + sb;
                int d2 = sa - sb;

                c[c_index++] = (unsigned char)(d1 + 128);
                c[c_index++] = (unsigned char)(d2 + 128);
            }
        }
        break;
    }

    return c_index;
}


int demultiplex(unsigned char *a, const int a_size, 
                unsigned char *b, const int b_size, 
                const unsigned char *c, const int c_len, 
                MuxMode mode) {

    memset(a, 0, a_size);
    memset(b, 0, b_size);
    switch (mode) {

        case TDM:{
            for (int i = 0; i < c_len; i += 2) {
                if (i/2 < a_size)
                    a[i/2] = c[i];

                if (i+1 < c_len && i/2 < b_size)
                    b[i/2] = c[i+1];
            }
        }
        break;

        case STDM:{
            int i = 0;
            while (i + 2 < c_len) {
                int tag   = c[i];     // 来源
                int index = c[i+1];   // 时间索引
                int val   = c[i+2];   // 数据
                if (tag == 1) { // A
                    if (index < a_size)
                        a[index] = val;
                } else if (tag == 2) { // B
                    if (index < b_size)
                        b[index] = val;
                }
                i += 3;
            }
        }
        break;

        case FDM:{
            for (int i = 0; i < c_len; ++i) {
                // 低4位 -> A
                if (i < a_size) {
                    unsigned char low = c[i] & 0x0F;
                    a[i] = low;
                }
                // 高4位 -> B
                if (i < b_size) {
                    unsigned char high = (c[i] >> 4) & 0x0F;
                    b[i] = high;
                }
            }
        }
        break;

        case CDM:{
            for (int i = 0; i < c_len / 2; ++i) {
                int d1 = (int)c[2*i] - 128;
                int d2 = (int)c[2*i+1] - 128;

                // 正交解码（内积）
                int resA = d1 + d2;
                int resB = d1 - d2;

                if (i < a_size)
                    a[i] = (resA > 0) ? 1 : 0;

                if (i < b_size)
                    b[i] = (resB > 0) ? 1 : 0;
            }
        }
        break;
    }

    return 1;
}

void run_test(MuxMode mode, const char* label) {
    unsigned char a_in[8] = {1, 0, 0, 1, 0, 1, 1, 0};
    unsigned char b_in[8] = {0, 1, 0, 1, 1, 0, 0, 1};
    unsigned char buffer[64];
    unsigned char a_out[8], b_out[8];

    cout << "当前测试：" << label;
    cout << endl;

    int len = multiplex(buffer, 256, a_in, 8, b_in, 8, mode);
    cout << "编码结果：" << endl;
    for(int i = 0; i < len; i++)
        cout << (int)buffer[i] << " " ;
    cout << endl;

    demultiplex(a_out, 8, b_out, 8, buffer, len, mode);

    bool success = (memcmp(a_in, a_out, 8) == 0 && memcmp(b_in, b_out, 8) == 0);
    cout << (success ? "[成功]": "[失败]") << endl;
    
    cout << "A 结果: " ; 
    for(int i = 0; i < 8; i++) 
        cout << (int)a_out[i] << " " ;
    cout << endl;
    
    cout << "B 结果: ";
    for(int i=0; i<8; i++)
        cout << (int)b_out[i] << " " ;
    cout << endl;
}

int main() {
    run_test(TDM, "TDM");
    run_test(STDM, "STDM");
    run_test(FDM, "FTDM");
    run_test(CDM, "CDM");
    return 0;
}