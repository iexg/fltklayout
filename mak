#!/bin/bash -x

PATH=./fltk-1.4:$PATH

which fltk-config

FLAGS="-g3 -O0 -fno-inline -std=gnu++11"
#FLAGS="-g -O2 -std=gnu++11"

g++ $FLAGS -I. -I$(fltk-config --includedir) -c fltklayout.cpp 
ar rvs libfltklayout.a fltklayout.o

g++ $FLAGS -I. -I$(fltk-config --includedir) designer.cpp -o fltklayout_designer -L. -lfltklayout $(fltk-config --ldflags) -pthread

#g++ -g3 -O0 -fno-inline -std=c++11 -I. -I$(fltk-config --includedir) gui.cpp x.cpp log.cpp time_utils.cpp $(fltk-config --ldflags) -pthread
#g++ -g3 -O2 -std=c++11 -I. -I$(fltk-config --includedir) gui.cpp x.cpp log.cpp time_utils.cpp $(fltk-config --ldflags) -pthread
#g++ -g3 -O0 -fno-inline -std=c++11 -I. x.cpp log.cpp time_utils.cpp -pthread
#g++ -g -O2 -std=c++11 -I. x.cpp log.cpp time_utils.cpp -pthread
#g++ -g -O2 -std=c++11 -I. x.cpp

g++ -g -O2 -std=c++11 -I. test.cpp -L. -lfltklayout $(fltk-config --ldflags) -pthread

