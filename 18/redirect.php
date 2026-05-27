<?php
if (isset($_GET['noref'])) {
    header('Referrer-Policy: no-referrer');
}

header('Location: target.php', true, 302);
header('Content-Type: text/plain; charset=UTF-8');

echo "Redirecting to target.php...\n";
