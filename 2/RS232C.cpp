#include <iostream>
#include <vector>
#include <string>

using namespace std;

int rs232c_encode(double *volts, int volts_size, const char *msg, int size) {
    int idx = 0;
    for (int i = 0; i < size; ++i) {
        if (idx < volts_size) volts[idx++] = -12.0;
        if (idx < volts_size) volts[idx++] = 12.0;
        for (int j = 0; j < 7; ++j) {
            if (idx >= volts_size) break;
            volts[idx++] = (msg[i] & (1 << j)) ? -12.0 : 12.0;
        }
        if (idx < volts_size) volts[idx++] = -12.0;
    }
    return idx;
}

int rs232c_decode(char *msg, int size, const double *volts, int volts_size) {
    int char_count = 0;
    int i = 0;
    while (i < volts_size && char_count < size) {
        if (volts[i] == 12.0) {
            i++;
            char c = 0;
            for (int j = 0; j < 7 && i < volts_size; ++j) {
                if (volts[i++] == -12.0) c |= (1 << j);
            }
            msg[char_count++] = c;
            if (i < volts_size && volts[i] == -12.0) i++;
        } else {
            i++;
        }
    }
    return char_count;
}

int main() {
    string input;
    getline(cin, input);

    int total_bits = input.length() * 10;
    vector<double> volts(total_bits);

    int count = rs232c_encode(volts.data(), total_bits, input.c_str(), input.length());

    for (int i = 0; i < count; ++i) {
        cout << (volts[i] == -12.0 ? "1" : "0") << (i == count - 1 ? "" : " ");
    }
    cout << endl;

    for (int i = 0; i < count; ++i) {
        printf("%+.1f%s", volts[i], (i == count - 1 ? "" : " "));
    }
    cout << endl;

    return 0;
}