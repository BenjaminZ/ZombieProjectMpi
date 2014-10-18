#ifndef __MESH_MANIPULATION_HPP_INCLUDED__
#define __MESH_MANIPULATION_HPP_INCLUDED__

#include "MersenneTwister.h"
#include "zombiesim.hpp"
#include "Zombie.hpp"
#include "Human.hpp"
#include "GridCell.hpp"

int getPopulation(GridCell***);

double getPairingNumber(GridCell***);

double getBirthRate(GridCell***);

void getAgeGroupNumbers(GridCell***, int*);

void getDeathProb(GridCell***, double*);

void printPopulation(FILE*, GridCell***, int);

void proccessBoundaries(GridCell***);

void initializeMesh(GridCell***, GridCell***);

int fillMesh(GridCell***, MTRand*);

void printToBitmap(GridCell***, char*, int, int);

#endif // __MESH_MANIPULATION_HPP_INCLUDED__