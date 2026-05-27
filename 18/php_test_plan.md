# PHP Endpoint Test Plan (LNMP)

Base URL (example): http://localhost/18888
Adjust paths if your server maps a different document root.

## 1) set_cookie.php
Steps:
1. Open Base URL + /set_cookie.php in browser.
2. Open DevTools -> Application/Storage -> Cookies.
3. Confirm session_token=abc123 with no expires/max-age.
4. Refresh page and inspect Network request headers for Cookie.
Expected:
- Set-Cookie response header on first load.
- Cookie present in subsequent request headers.

## 2) session_counter.php
Steps:
1. Open /session_counter.php.
2. Refresh multiple times.
3. Observe Set-Cookie: PHPSESSID in response headers.
4. Close browser and reopen, then revisit page.
Expected:
- First visit: "欢迎首次来访！"
- Later: "这是您第 N 次访问，上次访问时间为 ..."
- New browser session resets count.

## 3) method_test.php
Browser:
1. Visit /method_test.php?x=1&y=2
Expected:
- METHOD: GET
- GET shows x/y

curl:
1. POST: curl -X POST -d "a=1&b=2" http://localhost/18888/method_test.php
2. PUT: curl -X PUT -d "raw body" http://localhost/18888/method_test.php
Expected:
- METHOD matches request
- POST shows a/b for POST test
- PUT_BODY shows request body for PUT test

## 4) redirect.php + target.php
Steps:
1. Visit /redirect.php
2. Check Network: redirect.php returns 302 with Location: target.php
3. Check target.php request headers for Referer
4. Visit /redirect.php?noref=1 and repeat
Expected:
- 302 on redirect.php
- target.php shows Referer
- With noref, Referer is empty or absent

## 5) cache_demo.php
Steps:
1. Visit /cache_demo.php and note timestamp.
2. Refresh within 30 seconds; check Network Size as memory/disk cache.
3. Wait 30 seconds and refresh.
Expected:
- Timestamp unchanged within 30 seconds
- Timestamp updates after 30 seconds

## 6) ua.php
Steps:
1. Visit /ua.php
2. Note User-Agent, Accept-Language, Accept-Encoding.
3. In DevTools, emulate mobile UA and reload.
4. curl http://localhost/18888/ua.php
Expected:
- Values update based on UA.
- If UA contains curl, prints robot message.

## 7) statuscode.php
Steps:
1. Visit /statuscode.php?code=200, 204, 302, 400, 403, 404, 500, 502, 503.
2. Check status line and Location for 302.
3. Try /statuscode.php?code=999
Expected:
- Response status equals requested code (allowlist).
- 302 includes Location: target.php.
- 999 returns 400 with allowed list.

## 8) down.php
Steps:
1. Visit /down.php?type=inline
2. Visit /down.php?type=attachment
Expected:
- inline displays content in browser.
- attachment triggers download prompt.
- Content-Disposition header differs between requests.
