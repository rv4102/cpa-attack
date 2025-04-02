#!/bin/bash

id=$1

for i in {1..10000}
do
    echo "Running $i"
    peci_cmds RdPkgConfig 0x6 0x0 >> output${id}.txt
    sleep 0.1
done