#include<iostream>
using namespace std;

int parity_check(const unsigned char *msg, const int msg_length) {
    if(msg == NULL) return 0;
    if(msg_length <= 0) return 0;
    int checker = 0;
    for(int i = 0; i < msg_length; i++){
        checker^=msg[i];
    }
    return !checker;
}

void random_data(unsigned char *msg, int length) {
    srand((unsigned int)time(NULL)); 
    for (int i = 0; i < length; i++) {
        msg[i] = rand() % 2;
    }
}

int main() {
    srand((unsigned int)time(NULL));

    unsigned char a[11], b[45], c[14];

    random_data(a, 11);
    cout << "A: ";
    for (int i = 0; i < 11; i++) cout << (int)a[i] << " ";
    cout << endl;
    cout << (parity_check(a, 11) ? "A正确" : "A错误") << endl;

    random_data(b, 45);
    cout << "B: ";
    for (int i = 0; i < 45; i++) cout << (int)b[i] << " ";
    cout << endl;
    cout << (parity_check(b, 45) ? "B正确" : "B错误") << endl;

    random_data(c, 14);
    cout << "C: ";
    for (int i = 0; i < 14; i++) cout << (int)c[i] << " ";
    cout << endl;
    cout << (parity_check(c, 14) ? "C正确" : "C错误") << endl;

    return 0;
}