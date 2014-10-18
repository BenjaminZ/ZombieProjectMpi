#ifndef __MESH_MANIPULATION_HPP_INCLUDED__
#define __MESH_MANIPULATION_HPP_INCLUDED__

#include "MersenneTwister.h"
#include "zombiesim.hpp"

int getPopulation(Cell** Mesh);

double getPairingNumber(Cell** Mesh);

double getBirthRate(Cell** Mesh);

void getDeathProb(Cell** Mesh, double* death_prob);

void getAgeGroups(Cell** Mesh, int* groups);

void printPopulation(FILE* output, Cell** Mesh, int t);

void initializeMesh(Cell** Mesh);

int fillMesh(Cell** Mesh, MTRand* mt);

void outputAsBitmap(Cell** Mesh, char* str, int w, int h);

void manageBoundaries(Cell** Mesh);

void swapMesh(Cell** MeshA, Cell** MeshB);

#endif // __MESH_MANIPULATION_HPP_INCLUDED__