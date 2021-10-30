#!/bin/bash
stamp=$1
echo "Remake all the benchmarks"
cd "$stamp"/genome/
make -f Makefile.stm clean
make -f Makefile.stm
cd ".."/intruder/
make -f Makefile.stm clean
make -f Makefile.stm
cd ".."/labyrinth/
make -f Makefile.stm clean
make -f Makefile.stm
cd ".."/ssca2/
make -f Makefile.stm clean
make -f Makefile.stm
cd ".."/vacation/
make -f Makefile.stm clean
make -f Makefile.stm
cd ".."/yada/
make -f Makefile.stm clean
make -f Makefile.stm
cd ".."/bayes/
make -f Makefile.stm clean
make -f Makefile.stm
cd ".."/kmeans/
make -f Makefile.stm clean
make -f Makefile.stm