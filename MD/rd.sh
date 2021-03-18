#!/bin/sh

HOST="127.0.0.1"
PORT=6380
TIMEOUT_LEVEL3=3
TIMEOUT_LEVEL5=$((${TIMEOUT_LEVEL3}*2-${TIMEOUT_LEVEL3}))
KEY_HEADER=HOTSPOT_LEVEL
echo $TIMEOUT_LEVEL5
#del redis 6380 KEY_HEADER 1-3
sleep $TIMEOUT_LEVEL3
for nums in 1 2 3
do
        redis-cli -h $HOST -p $PORT DEL ${KEY_HEADER}${nums}
done
sleep $TIMEOUT_LEVEL5
#del hotspot level4-5
for nums in 4 5
do
        redis-cli -h $HOST -p $PORT DEL ${KEY_HEADER}${nums}
done
