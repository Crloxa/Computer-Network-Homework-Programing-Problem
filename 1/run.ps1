# 运行 OpenCV C++ 程序的简单脚本
param([string]$file = "1_Sender.cpp")

# 设置 OpenCV 环境
$env:PATH = "E:/Opencv/opencv/build/x64/vc16/bin;$env:PATH"

# 获取文件名（不含扩展名）
$basename = [System.IO.Path]::GetFileNameWithoutExtension($file)

# 编译
Write-Host "正在编译 $file ..." -ForegroundColor Cyan
E:/LLVM/bin/clang++.exe -g -I E:/Opencv/opencv/build/include $file -L E:/Opencv/opencv/build/x64/vc16/lib -lopencv_world4120 -o "$basename.exe"

if ($LASTEXITCODE -eq 0) {
    Write-Host "编译成功！正在运行..." -ForegroundColor Green
    & ".\$basename.exe"
} else {
    Write-Host "编译失败！" -ForegroundColor Red
}
