# PHP Endpoint Test Commands (curl)

Base URL: http://localhost/18888

## method_test.php
- GET:
  curl "http://localhost/18888/method_test.php?x=1&y=2"
- POST:
  curl -X POST -d "a=1&b=2" "http://localhost/18888/method_test.php"
- PUT:
  curl -X PUT -d "raw body" "http://localhost/18888/method_test.php"

## redirect.php
- Normal:
  curl -I "http://localhost/18888/redirect.php"
- No referrer:
  curl -I "http://localhost/18888/redirect.php?noref=1"

## cache_demo.php
- First:
  curl -I "http://localhost/18888/cache_demo.php"

## ua.php
- Default UA:
  curl "http://localhost/18888/ua.php"
- Custom UA:
  curl -A "Mozilla/5.0" "http://localhost/18888/ua.php"

## statuscode.php
- Example:
  curl -I "http://localhost/18888/statuscode.php?code=404"

## down.php
- inline:
  curl -I "http://localhost/18888/down.php?type=inline"
- attachment:
  curl -I "http://localhost/18888/down.php?type=attachment"
