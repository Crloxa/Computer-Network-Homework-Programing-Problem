<?php
$referer = $_SERVER['HTTP_REFERER'] ?? '';
$refererText = $referer === '' ? '无' : $referer;

header('Content-Type: text/plain; charset=UTF-8');

echo "你被重定向到了这里！原始请求头中的 Referer 是： {$refererText}\n";
