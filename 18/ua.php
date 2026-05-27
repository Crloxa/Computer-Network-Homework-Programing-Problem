<?php
$userAgent = $_SERVER['HTTP_USER_AGENT'] ?? '';
$acceptLanguage = $_SERVER['HTTP_ACCEPT_LANGUAGE'] ?? '';
$acceptEncoding = $_SERVER['HTTP_ACCEPT_ENCODING'] ?? '';

header('Content-Type: text/plain; charset=UTF-8');

if (stripos($userAgent, 'curl') !== false) {
    echo "机器人侦探，欢迎使用命令行\n";
    exit;
}

echo "你的浏览器是： {$userAgent}\n";
echo "Accept-Language: {$acceptLanguage}\n";
echo "Accept-Encoding: {$acceptEncoding}\n";
