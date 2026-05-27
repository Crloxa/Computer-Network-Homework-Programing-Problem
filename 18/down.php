<?php
$type = $_GET['type'] ?? 'inline';
$disposition = $type === 'attachment' ? 'attachment' : 'inline';
$filename = $disposition === 'attachment' ? 'confidential.txt' : 'report.txt';

header('Content-Type: text/plain; charset=UTF-8');
header('Content-Disposition: ' . $disposition . '; filename="' . $filename . '"');

echo "这是绝密情报，请在浏览器内直接阅读。\n";
