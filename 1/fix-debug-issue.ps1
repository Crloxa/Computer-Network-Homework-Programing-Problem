# 快速修复 lldb 卡住问题的脚本
Write-Host "=== C++ 调试器问题修复工具 ===" -ForegroundColor Cyan
Write-Host ""

# 1. 停止所有卡住的进程
Write-Host "1. 正在停止卡住的进程..." -ForegroundColor Yellow
$processes = Get-Process | Where-Object {$_.ProcessName -match "lldb|Sender|WindowsDebugLauncher|homwork"}
if ($processes) {
    $processes | Stop-Process -Force -ErrorAction SilentlyContinue
    Write-Host "   ✓ 已停止 $($processes.Count) 个进程" -ForegroundColor Green
} else {
    Write-Host "   ✓ 没有需要停止的进程" -ForegroundColor Green
}

# 2. 检查配置文件
Write-Host ""
Write-Host "2. 正在检查配置文件..." -ForegroundColor Yellow

$settingsFile = ".vscode\settings.json"
$launchFile = ".vscode\launch.json"

if (Test-Path $settingsFile) {
    Write-Host "   ✓ settings.json 存在" -ForegroundColor Green
} else {
    Write-Host "   ✗ settings.json 不存在" -ForegroundColor Red
}

if (Test-Path $launchFile) {
    Write-Host "   ✓ launch.json 存在" -ForegroundColor Green
} else {
    Write-Host "   ✗ launch.json 不存在" -ForegroundColor Red
}

# 3. 检查是否有运行脚本
Write-Host ""
Write-Host "3. 正在检查运行脚本..." -ForegroundColor Yellow

if (Test-Path "run.bat") {
    Write-Host "   ✓ run.bat 存在" -ForegroundColor Green
} else {
    Write-Host "   ✗ run.bat 不存在" -ForegroundColor Red
}

if (Test-Path "run.ps1") {
    Write-Host "   ✓ run.ps1 存在" -ForegroundColor Green
} else {
    Write-Host "   ✗ run.ps1 不存在" -ForegroundColor Red
}

# 4. 检查 OpenCV 环境
Write-Host ""
Write-Host "4. 正在检查 OpenCV 环境..." -ForegroundColor Yellow

if (Test-Path "E:\Opencv\opencv\build\x64\vc16\bin\opencv_world4120.dll") {
    Write-Host "   ✓ OpenCV DLL 存在" -ForegroundColor Green
} else {
    Write-Host "   ✗ OpenCV DLL 不存在" -ForegroundColor Red
}

# 5. 检查编译器
Write-Host ""
Write-Host "5. 正在检查编译器..." -ForegroundColor Yellow

if (Test-Path "E:\LLVM\bin\clang++.exe") {
    Write-Host "   ✓ clang++ 编译器存在" -ForegroundColor Green
} else {
    Write-Host "   ✗ clang++ 编译器不存在" -ForegroundColor Red
}

# 完成
Write-Host ""
Write-Host "=== 检查完成 ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "推荐运行方式：" -ForegroundColor Yellow
Write-Host "  1. 点击右上角 ▶️ Run Code 按钮" -ForegroundColor White
Write-Host "  2. 运行：.\run.bat 1_Sender.cpp" -ForegroundColor White
Write-Host "  3. 运行：.\run.ps1 1_Sender.cpp" -ForegroundColor White
Write-Host ""
Write-Host "避免的操作：" -ForegroundColor Yellow
Write-Host "  ✗ 不要按 F5（会触发调试器）" -ForegroundColor Red
Write-Host "  ✗ 不要点击左侧"运行和调试"按钮" -ForegroundColor Red
Write-Host ""
