#!/bin/bash
n=$1
bash run-sim.sh RAFULL$n ../../../stamp-ps $1
bash run-sim.sh SCFULL$n ../../../stamp-ps-sc $1