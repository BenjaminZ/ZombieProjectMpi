#ifndef __SIMULATION_EXEC_HPP_INCLUDED__
#define __SIMULATION_EXEC_HPP_INCLUDED__

#include "MersenneTwister.h"
#include "zombiesim.hpp"

void executeBirthControl(Cell** Mesh, int i, int j, int n, double prob_birth, MTRand* mt);

void executeMovement(Cell** MeshA, Cell** MeshB, int i, int j, MTRand* mt);

void executeInfection(Cell** MeshA, int i, int j, int n, MTRand* mt);

void executeDeathControl(Cell** Mesh, int i, int j, double* prob_death, int n, MTRand* mt);

#endif // __SIMULATION_EXEC_HPP_INCLUDED__