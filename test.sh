#!/bin/bash

#touch text.txt
for i in $(seq 10000); do curl -0 -X GET http://127.0.0.1:12345/index.html 2>/dev/null  & done

#ab -c 64 -n 600000 http://127.0.0.1:12345/
