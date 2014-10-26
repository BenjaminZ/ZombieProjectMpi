#ifndef __MESH_MANIPULATION_HPP_INCLUDED__
#define __MESH_MANIPULATION_HPP_INCLUDED__

#include "MersenneTwister.h"
#include "zombiesim.hpp"

int getPopulation(Cell** Mesh);

double getPairingNumber(int *genders);

double getBirthRate(Cell** Mesh, int population);

void getGenderNumber(Cell** Mesh, int *genders);

void getDeathProb(double* death_prob, int population, int *groups);

void getAgeGroups(Cell** Mesh, int* groups);

void printPopulation(FILE* output, Cell** Mesh, int t);

void initializeMesh(Cell** Mesh);

int fillMesh(Cell** Mesh, MTRand* mt);

void outputAsBitmap(Cell** Mesh, char* str, int w, int h);

void manageBoundaries(Cell** Mesh);

void swapMesh(Cell** MeshA, Cell** MeshB);

#endif // __MESH_MANIPULATION_HPP_INCLUDED__