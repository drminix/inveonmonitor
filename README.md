# inveonmonitor
[C++/QT] simulation setup and post processing tools for Inveon GATE simulation model

By default, GATE is a single-threaded Monte Carlo simulation package so it runs the entire simulation on a single core. Since Monte Carlo requires a huge amount of computing power, it is not feasible run any meaningful simulation on a single core. Therefore, the simulation needs to be parallelized in order to reduce the simulation time. 

We developed a Graphical User Interface program for parallelizing and monitoring a GATE simulation. This program is written in C/C++ with QT GUI library package; therefore, it can be compiled to run on Windows, Linux, and Mac. Source code of this program and executables are located in UTILITY/gatemonitor/ directory. 

Moreover, this program can convert text listmode data to projection data in Inveon Reconstruction Software compatible format.

![](https://github.com/drminix/inveonmonitor/blob/master/image/screenshot.png)

(c) Sanghyeb(Sam) Lee MIT License
