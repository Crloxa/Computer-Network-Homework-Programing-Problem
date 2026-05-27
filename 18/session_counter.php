<?php
session_start();

$previousCount = $_SESSION['count'] ?? 0;
$previousVisit = $_SESSION['last_visit'] ?? null;

$currentCount = $previousCount + 1;
$_SESSION['count'] = $currentCount;
$_SESSION['last_visit'] = date('Y-m-d H:i:s');

header('Content-Type: text/plain; charset=UTF-8');

if ($currentCount === 1) {
    echo "欢迎首次来访！\n";
} else {
    $lastText = $previousVisit ?? '未知';
    echo "这是您第 {$currentCount} 次访问，上次访问时间为 {$lastText}\n";
}
