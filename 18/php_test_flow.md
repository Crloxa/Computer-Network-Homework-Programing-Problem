# PHP Endpoint Test Flow (Demonstration)

This document mirrors the test plan flow in a step-by-step narrative format. It is ready for actual execution once the server is running.

Base URL: http://localhost/18888

## Step 1: Cookie behavior
1. Open /set_cookie.php
2. Check Application/Storage -> Cookies for session_token
3. Refresh and inspect Network request headers for Cookie

## Step 2: Session counter
1. Open /session_counter.php
2. Refresh twice
3. Observe PHPSESSID in response headers
4. Close and reopen browser, revisit to confirm reset

## Step 3: HTTP method handling
1. GET: /method_test.php?x=1&y=2
2. POST: curl -X POST -d "a=1&b=2" http://localhost/18888/method_test.php
3. PUT: curl -X PUT -d "raw body" http://localhost/18888/method_test.php

## Step 4: Redirect and Referer
1. Open /redirect.php
2. Confirm 302 and Location in Network
3. Confirm Referer on /target.php
4. Open /redirect.php?noref=1 and confirm Referer absent

## Step 5: Cache demo
1. Open /cache_demo.php, note timestamp
2. Refresh within 30 seconds, confirm cache hit and same timestamp
3. Wait 30 seconds, refresh and confirm new timestamp

## Step 6: User-Agent
1. Open /ua.php and record UA
2. Emulate mobile UA and reload, record change
3. curl http://localhost/18888/ua.php and confirm robot message

## Step 7: Status code
1. Open /statuscode.php?code=200, 204, 302, 400, 403, 404, 500, 502, 503
2. Confirm HTTP status code matches
3. Check 302 includes Location: target.php
4. Open /statuscode.php?code=999, confirm 400 error

## Step 8: Download disposition
1. Open /down.php?type=inline, confirm inline display
2. Open /down.php?type=attachment, confirm download prompt
3. Compare Content-Disposition headers
