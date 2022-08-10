# Quasi-1D de Laval nozzle numerical simulator

## Basic description
This is a tool made for my bachelor's thesis at the Faculty of Mechanical Engineering, CTU in Prague. 

It is used for simulating gas flow in a nozzle, accelerating the flow beyond the speed of sound and then letting the gas expand further until a shock wave
occurs.

The main file should compile in plain C (and also C++) without any dependencies (aside -lm). The 'aux' folder can mostly be ignored, as it contains other things used for the thesis.

It's good to use it with gnuplot installed - it can call gnuplot to make plots while running!

## Features
There are 2 main features: 
- a numerical solver (iter.c) utilizing an FTCS scheme with artificial viscosity, 
- and an analytical solver (refsol.c) used to evaluate the numerical results. 

The analytical solver usually executes automatically, while the numerical solver is mostly controlled from the command line - it resets to initial condition automatically when
some parameters are changed.

## Nozzle shape
The nozzle is symmetric around its axis. Here's a sketch of the geometry taken from the thesis:

![Screenshot from 2022-08-10 22-40-37](https://user-images.githubusercontent.com/104327272/184016021-7fc9d608-7939-4259-bbcc-fc87837df5ee.png)

In the program, the nozzle geometry is by default generated by defining the smallest and largest cross section and the angle of the divergent part
(α in the sketch).

## Gas physics
The model uses Euler equations and ideal gas - viscosity of the gas is ignored and specific heats are assumed to be constant.
This is a good approximation of the behavior of air in most cases, but the real properties of the gas should always be checked 
before making any conclusions. 
