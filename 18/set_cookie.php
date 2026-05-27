<?php
setcookie('session_token', 'abc123');
header('Content-Type: text/plain; charset=UTF-8');

echo "Cookie session_token set to abc123.\n";
echo "Refresh the page and inspect the Cookie header.\n";
