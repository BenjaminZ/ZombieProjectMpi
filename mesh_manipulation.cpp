#include "mesh_manipulatio.cpp"

int getPopulation(Cell** Mesh)
{
	int total = 0;
	for(int i = 1; i <= SIZE_I; i++)
		for(int j; j <= SIZE_J; j++)
			if(Mesh[i][j].celltype == HUMAN) total += 1;
	return total;
}

void getDeathProb(Cell** Mesh, double* death_prob)
{

}

void printPopulation(FILE* output, Cell** Mesh, int t)
{
	int male = 0, female = 0, zombies = 0;
	for(int i = 1; i <= SIZE_I; i++)
	{
		for(int j = 1; j <= SIZE_J; j++)
		{
			if(Mesh[i][j].celltype == HUMAN)
			{
				if(Mesh[i][j].gender == MALE)
			}
		}
	}
}