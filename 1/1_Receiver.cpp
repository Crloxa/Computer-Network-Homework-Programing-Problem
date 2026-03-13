#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <cmath> // 用于计算平方根和幂运算

using namespace cv;
using namespace std;

/**
 * 预定义的标准颜色库（必须与发送端完全一致）
 * OpenCV 默认颜色空间是 BGR (Blue, Green, Red)
 */
vector<Scalar> targetColors = {
    Scalar(0, 0, 0),       // 0: 黑色 (所有通道关闭)
    Scalar(255, 0, 0),     // 1: 蓝色 (仅 B 通道开启)
    Scalar(0, 255, 0),     // 2: 绿色 (仅 G 通道开启)
    Scalar(255, 255, 0),   // 3: 青色 (B+G 混合)
    Scalar(0, 0, 255),     // 4: 红色 (仅 R 通道开启)
    Scalar(255, 0, 255),   // 5: 品红 (R+B 混合)
    Scalar(0, 255, 255),   // 6: 黄色 (R+G 混合)
    Scalar(255, 255, 255)  // 7: 白色 (RGB 全部开启)
};

/**
 * 颜色距离算法：计算两个颜色在 3D 颜色空间中的直线距离
 * 目的：找到摄像头捕获到的“杂质色”最接近哪一个“标准色”
 */
double getColorDistance(Scalar c1, Scalar c2) {
    // 欧几里得距离公式: sqrt((b2-b1)^2 + (g2-g1)^2 + (r2-r1)^2)
    return sqrt(pow(c1[0] - c2[0], 2) + 
                pow(c1[1] - c2[1], 2) + 
                pow(c1[2] - c2[2], 2));
}

/**
 * 逻辑核心：识别比特位
 */
int identifyBit(Scalar avgColor) {
    int bestMatch = 0;
    double minDistance = 1000000; // 初始化为一个很大的数

    // 遍历所有标准色，寻找距离最短的那一个
    for (int i = 0; i < (int)targetColors.size(); i++) {
        double dist = getColorDistance(avgColor, targetColors[i]);
        if (dist < minDistance) {
            minDistance = dist; // 更新最小值
            bestMatch = i;      // 记录下标，这个下标就是我们要的 0-7
        }
    }
    return bestMatch;
}

int main() {
    // 1. 初始化摄像头
    VideoCapture cap(0); // 0 通常是内置摄像头
    if (!cap.isOpened()) {
        cout << "错误：无法打开摄像头！" << endl;
        return -1;
    }

    // 设置采集分辨率为 720p (需摄像头硬件支持)
    cap.set(CAP_PROP_FRAME_WIDTH, 1280);
    cap.set(CAP_PROP_FRAME_HEIGHT, 720);

    Mat frame;
    string winName = "Receiver_Detection";
    namedWindow(winName);

    cout << "--- 接收端已启动 ---" << endl;
    cout << "提示：请确保发送端的色块位于画面中央的绿框内。" << endl;

    while (true) {
        cap >> frame; // 读取当前帧图像
        if (frame.empty()) break;

        // 2. 确定采样区域 (Region of Interest, ROI)
        // 我们在画面中心定义一个 100x100 的正方形区域
        int roiSize = 100;
        Rect roiRect((frame.cols - roiSize) / 2, (frame.rows - roiSize) / 2, roiSize, roiSize);
        
        // 提取该区域的子矩阵
        Mat roi = frame(roiRect);

        // 3. 核心步骤：计算区域内的平均颜色
        // 因为摄像头会有噪点，取平均值可以极大地提高识别的稳定性
        Scalar avgColor = mean(roi);

        // 4. 进行分类识别
        int detectedBit = identifyBit(avgColor);

        // 5. 绘图反馈 (视觉辅助)
        // 绘制绿色的采样框，告诉用户摄像头在看哪里
        rectangle(frame, roiRect, Scalar(0, 255, 0), 2);
        
        // 在屏幕上实时打印识别到的数字
        string resultText = "Detected Bit: " + to_string(detectedBit);
        putText(frame, resultText, Point(30, 60), FONT_HERSHEY_SIMPLEX, 1.2, Scalar(0, 255, 0), 3);
        
        // 在屏幕上显示原始 BGR 数据，方便调试颜色偏差
        string bgrText = "B:" + to_string((int)avgColor[0]) + 
                         " G:" + to_string((int)avgColor[1]) + 
                         " R:" + to_string((int)avgColor[2]);
        putText(frame, bgrText, Point(30, 110), FONT_HERSHEY_SIMPLEX, 0.6, Scalar(255, 255, 255), 1);

        // 显示结果画面
        imshow(winName, frame);

        // 6. 退出逻辑
        // waitKey(1) 允许窗口渲染，并检测按键
        char key = (char)waitKey(1);
        if (key == 'q' || key == 'Q') {
            cout << "用户请求退出..." << endl;
            break;
        }
    }

    // 释放资源
    cap.release();
    destroyAllWindows();
    return 0;
}