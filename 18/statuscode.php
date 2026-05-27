<?php
$allowed = [200, 204, 302, 400, 403, 404, 500, 502, 503];
$code = isset($_GET['code']) ? (int) $_GET['code'] : 200;

if (!in_array($code, $allowed, true)) {
    http_response_code(400);
    header('Content-Type: text/plain; charset=UTF-8');
    echo "Unsupported code. Allowed: " . implode(', ', $allowed) . "\n";
    exit;
}

if ($code === 302) {
    header('Location: target.php');
}

http_response_code($code);

if ($code === 204) {
    exit;
}

header('Content-Type: text/plain; charset=UTF-8');

echo "status: {$code}\n";
