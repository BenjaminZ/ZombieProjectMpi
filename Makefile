CC=mpic++

default:
	$(CC) -fopenmp -lm -g mesh_manipulation.cpp simulation_exec.cpp main.cpp -o zombiesim