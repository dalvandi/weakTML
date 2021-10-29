#!/bin/bash
stamp=$1
echo "Remake all the benchmarks"
cd "$stamp"/genome/
make -f Makefile.stm clean
make -f Makefile.stm
cd "$stamp"/intruder/
make -f Makefile.stm clean
make -f Makefile.stm
cd "$stamp"/labyrinth/
make -f Makefile.stm clean
make -f Makefile.stm
cd "$stamp"/ssca2/
make -f Makefile.stm clean
make -f Makefile.stm
cd "$stamp"/vacation/
make -f Makefile.stm clean
make -f Makefile.stm
cd "$stamp"/yada/
make -f Makefile.stm clean
make -f Makefile.stm
cd "$stamp"/bayes/
make -f Makefile.stm clean
make -f Makefile.stm
cd "$stamp"/kmeans/
make -f Makefile.stm clean
make -f Makefile.stm