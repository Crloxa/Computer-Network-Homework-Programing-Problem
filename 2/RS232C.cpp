#include <iostream>
#include <vector>
#include <bitset>

using namespace std;

int rs232c_encode(double *volts, int volts_size, const char *msg, int size) {
    int index = 0;
    for (int i = 0; i < size; ++i) {
        if (index + 10 > volts_size) break; // 检查剩余空间
        volts[index++] = -12.0; //Idle
        volts[index++] = 12.0;  //Start
        char c = msg[i];
        for (int j = 0; j < 7; ++j) {
            if ((c >> j) & 1) volts[index++] = -12.0;
            else volts[index++] = 12.0;
        }
        volts[index++] = -12.0; //Stop
    }
    return index;
}

int rs232c_decode(char *msg, int size, const double *volts, int volts_size) {
    int msg_ptr = 0;
    for (int i = 0; i < volts_size; i += 10) {
        if (msg_ptr >= size) break; // 大小超了
        char c = 0;
        for (int j = 0; j < 7; ++j) {
            if (volts[i + 2 + j] < 0) c |= (1 << j);
        }
        msg[msg_ptr++] = c;
    }
    return msg_ptr;
}

int main() {
    char input;
    cout << "请输入一个字符: ";
    cin >> input;

    double volts[10];
    const char* labels[] = {"Idle", "Start", "b1", "b2", "b3", "b4", "b5", "b6", "b7", "Stop"};

    rs232c_encode(volts, size(volts), &input, 1);

    cout << "Timing Slot:\t";
    for (int i = 0; i < 10; ++i) cout << labels[i] << "\t";
    cout << endl;

    cout << "Logic Level:\t";
    for (int i = 0; i < 10; ++i) {
        cout << (volts[i] < 0 ? "1" : "0") << "\t";
    }
    cout << endl;

    cout << "Voltage (V):\t";
    for (int i = 0; i < 10; ++i) {
        cout << (int)volts[i] << "V\t";
    }
    cout << endl;

    cout << "\n[Decoding Test]\n";
    char decoded_msg[1] = {0};
    int decoded_count = rs232c_decode(decoded_msg, 1, volts, 10);
    
    if (decoded_count > 0) {
        cout << "Decoded Character: " << decoded_msg[0] << endl;
        cout << "Verification: " << (decoded_msg[0] == input ? "PASS" : "FAIL") << endl;
    }

    return 0;
}