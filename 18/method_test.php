<?php
$method = $_SERVER['REQUEST_METHOD'] ?? 'UNKNOWN';
$getParams = $_GET;
$postParams = $_POST;
$rawBody = file_get_contents('php://input');

header('Content-Type: text/plain; charset=UTF-8');

echo "METHOD: {$method}\n";
echo 'GET: ' . json_encode($getParams, JSON_UNESCAPED_UNICODE) . "\n";
echo 'POST: ' . json_encode($postParams, JSON_UNESCAPED_UNICODE) . "\n";

echo "PUT_BODY: {$rawBody}\n";
