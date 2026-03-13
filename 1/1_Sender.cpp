#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <string>

using namespace cv;
using namespace std;

// BGR
vector<Scalar> colors = {
    Scalar(0, 0, 0),       // 0: 黑
    Scalar(255, 0, 0),     // 1: 蓝
    Scalar(0, 255, 0),     // 2: 绿
    Scalar(255, 255, 0),   // 3: 青
    Scalar(0, 0, 255),     // 4: 红
    Scalar(255, 0, 255),   // 5: 品红
    Scalar(0, 255, 255),   // 6: 黄
    Scalar(255, 255, 255)  // 7: 白
};

int main() {
    string winName = "Sender";
    namedWindow(winName, WINDOW_AUTOSIZE); 
    int width = 300;
    int height = 300;
    Mat canvas(height, width, CV_8UC3, Scalar(0, 0, 0));

    // 初始显示黑色
    imshow(winName, canvas);
    waitKey(1);

    cout << "Sender" << endl;
    cout << "- 输入 0-7 切换颜色" << endl;
    cout << "- 输入 q 退出" << endl;
    string userInput;
    while (true) {
        cout << "请输入指令: ";
        cin >> userInput;
        // 1. 检查是否输入 q 退出
        if (userInput == "q" || userInput == "Q") {
            cout << "退出程序..." << endl;
            break;
        }
        try {
            int val = stoi(userInput);
            if (val >= 0 && val <= 7) {
                canvas.setTo(colors[val]);
                imshow(winName, canvas);
                
                // 更新渲染
                waitKey(1); 
            } else {
                cout << "错误：请输入 0-7 之间的数字。" << endl;
            }
        } catch (...) {
            cout << "无效输入！请输入数字或 q。" << endl;
        }
    }

    destroyAllWindows();
    return 0;
}