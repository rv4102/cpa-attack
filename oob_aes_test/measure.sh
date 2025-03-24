#!/bin/bash

for i in {1..10000}
do
    peci_cmds RdPkgConfig 0x6 0x0 >> output.txt
done