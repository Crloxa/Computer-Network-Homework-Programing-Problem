# PHP Endpoint Documentation

This document explains the purpose and behavior of each PHP script in 18/.

## set_cookie.php
- Purpose: Set a session cookie named session_token with value abc123.
- Behavior: Sends Set-Cookie without expires/max-age, then prints a short hint.
- Headers:
  - Set-Cookie: session_token=abc123
  - Content-Type: text/plain; charset=UTF-8

## session_counter.php
- Purpose: Track visits using PHP session data.
- Behavior:
  - First visit: prints "欢迎首次来访！"
  - Subsequent visits: prints "这是您第 N 次访问，上次访问时间为 YYYY-MM-DD HH:MM:SS"
- State:
  - $_SESSION['count'] stores visit count
  - $_SESSION['last_visit'] stores last timestamp
- Headers:
  - Set-Cookie: PHPSESSID=...
  - Content-Type: text/plain; charset=UTF-8

## method_test.php
- Purpose: Demonstrate request method handling for GET, POST, and PUT.
- Behavior:
  - Prints request method
  - Prints GET params (JSON)
  - Prints POST params (JSON)
  - Prints raw body (used for PUT tests)
- Input:
  - GET: query string
  - POST: x-www-form-urlencoded or body params
  - PUT: raw body read from php://input
- Headers:
  - Content-Type: text/plain; charset=UTF-8

## redirect.php
- Purpose: Return 302 redirect to target.php.
- Behavior:
  - Optional no-referrer by adding ?noref=1
  - Always sends Location: target.php
- Headers:
  - Location: target.php
  - Status: 302
  - Optional: Referrer-Policy: no-referrer
  - Content-Type: text/plain; charset=UTF-8

## target.php
- Purpose: Display Referer seen after redirect.
- Behavior:
  - Prints Referer value or "无" if empty
- Headers:
  - Content-Type: text/plain; charset=UTF-8

## cache_demo.php
- Purpose: Demonstrate browser caching.
- Behavior:
  - Returns current UNIX timestamp
- Headers:
  - Cache-Control: max-age=30
  - Content-Type: text/plain; charset=UTF-8

## ua.php
- Purpose: Show User-Agent, Accept-Language, and Accept-Encoding.
- Behavior:
  - If User-Agent contains curl (case-insensitive), prints "机器人侦探，欢迎使用命令行"
  - Otherwise prints UA, Accept-Language, Accept-Encoding
- Headers:
  - Content-Type: text/plain; charset=UTF-8

## statuscode.php
- Purpose: Return different HTTP status codes based on ?code=xxx.
- Behavior:
  - Allowlist: 200, 204, 302, 400, 403, 404, 500, 502, 503
  - Invalid code returns 400 with list of allowed codes
  - For 302, sets Location: target.php
  - For 204, returns empty body
- Headers:
  - Status: selected code
  - Optional: Location: target.php (for 302)
  - Content-Type: text/plain; charset=UTF-8 (except 204)

## down.php
- Purpose: Demonstrate Content-Disposition: inline vs attachment.
- Behavior:
  - ?type=inline or ?type=attachment
  - Default is inline
  - Uses filename report.txt for inline, confidential.txt for attachment
  - Returns a short text payload
- Headers:
  - Content-Type: text/plain; charset=UTF-8
  - Content-Disposition: inline|attachment; filename="..."
