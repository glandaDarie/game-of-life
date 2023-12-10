# Game of Life with OpenMP Parallelization
This project implements Conway's Game of Life, a cellular automaton devised by mathematician John Conway. The project focuses on applying parallelization techniques using OpenMP to optimize the performance of the Game of Life simulation through multithreading.

## Features
**Parallelized Computation:** Utilizes OpenMP to distribute the computation across multiple threads for faster generation calculations.

**Customizable Parameters:** Allows users to adjust the number of generations, board size, chunk sizes, and more using command-line options.

**Logging and Metrics:** Logs important simulation parameters and metrics to monitor the behavior and performance of the simulation.

## Contents
**src/ Directory:** Contains the source code for the Game of Life simulation.

**constants.cpp and constants.h:** Define constants used in the simulation.

**game_helpers.cpp and game_helpers.h:** Implement functions to handle game logic and parallelized generation computations.

**logger.cpp and logger.h:** Implement a logging system for metrics and information output.

**openMP.cpp:** Main file orchestrating the Game of Life simulation with OpenMP parallelization.

**utils/ Directory:** Contains utility files such as logging functionalities and constant definitions.

## Usage
To run the simulation:

Clone the repository.
Compile the project using an appropriate C++ compiler.

Execute the compiled binary, providing necessary command-line arguments to customize the simulation parameters.

