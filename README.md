# COL216-Assignment-2

## This is a COL216 Assignment 2. 
This Team consists of:

1. Dhruv Gupta 2021CS50125
2. Niranjan Sarode 2021CS50612

## Files Submitted:

1. 5stage.cpp
2. 5stage.hpp
3. 5stage_bypass.cpp
4. 5stage_bypass.hpp
5. 79stage.cpp
6. 79stage.hpp
7. 79stage_bypass.cpp
8. 79stage_bypass.hpp
9. Makefile
10. input.asm
11. README.md
12. report.pdf
13. input.txt


## Instructions to compile and run the programs:

These actions are to be performed in working directory of the project.

1. To compile the programs, run the following command in the terminal:
    ```bash
    make compile
    ```
2. To run the simulator for the 5-stage pipeline:
    ```bash
    make run_5stage
    ```
3. To run the simulator for the 5-stage pipeline with forwarding and bypassing:
    ```bash
    make run_5stage_bypass
    ```
4. To run the simulator for the 7 and 9-stage stage pipeline:
    ```bash
    make run_79stage
    ```
5. To run the simulator for the 7 and 9-stage stage pipeline with forwarding and bypassing:
    ```bash
    make run_79stage_bypass
    ```
6. To give the result and accuracy of the Branch Predictor in the required 3 cases (Part 5):
   ```bash
    make run_branch_predictor
    ```
7. To clear the executables for running them again:
    ```bash
    make clean
    ```