CC=mpic++

default:
	$(CC) -openmp -lm -g mesh_manipulation.cpp simulation_exec.cpp main.cpp -o zombiesim