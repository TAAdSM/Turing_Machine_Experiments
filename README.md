# Turing Machine Experiments
My experiments implementing simple Turing Machines, UTM 
(still very much in-progress), and others, to solidify my understanding of 
Charles Petzold's book ("The Annotated Turing").
Coding done by Teo Asinari
## How to run:
The main source file is currently src/main.cpp.
The following instructions assume you have cloned the repo and have installed 
the ccmake ncurses-based GUI for cmake.
- `cd <path_to_cloned_repo>/src`  
- `ccmake .`
- `Release` for the CMAKE_BUILD_TYPE in the ccmake gui then type `c`, then `g` on your keyboard.
- `make turingTest`
- `./turingTest`
- This will print (to `stdout`) the final tape states for various simple TMs 
  run (each of which has been run for a 
  certain hard-coded number of cycles -- see main.cpp for implementation 
  details). 
 
FWIW I ran this on an x86-64 machine running an ubuntu-based distro, hope 
that helps.
