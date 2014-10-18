#include "mesh_manipulation.hpp"

int getPopulation(Cell** Mesh)
{
	int total = 0;
	for(int i = 1; i <= SIZE_I; i++)
		for(int j; j <= SIZE_J; j++)
			if(Mesh[i][j].celltype == HUMAN) total += 1;
	return total;
}

double getPairingNumber(Cell** Mesh)
{
	int i, j;
	double male, female;
	double no_female, paired_female, allcells;
	double no_male, paired_male;
	double pob_male, pob_female;

	for(i = 1; i <= SIZE_I; i++)
	{
		for(j = 1; j <= SIZE_J; j++)
		{
			if(Mesh[i][j].celltype == HUMAN)
			{
				if(Mesh[i][j].gender == MALE) male += 1.0;
				else female += 1.0;
			}
		}
	}
	allcells = (double)(SIZE_I*SIZE_J);
	
	no_female = (allcells - female)/allcells;
	paired_female = 1 - pow(no_female, 4);
	pob_female = female*paired_female;

	no_male = (allcells - male)/allcells;
	paired_male = 1 - pow(no_male, 4);
	pob_male = male*paired_male;

	if(pob_male > pob_female) return pob_female;
	else return pob_male;
}

double getBirthRate(Cell** Mesh)
{
	return ((double)getPopulation(Mesh))*(NT_BIRTHS_PER_DAY)/(NT_POP);
}

void getDeathProb(Cell** Mesh, double* death_prob)
{
	double deaths;
	int groups[3] = {0,0,0}, population;

	population = getPopulation(Mesh);
	deaths = NT_DEATHS_PER_DAY*((double)population)/NT_POP;
	getAgeGroups(Mesh, groups);

	death_prob[0] = deaths*NT_YOUNG_DEATH/((double)groups[0]);
	death_prob[1] = deaths*NT_ADULT_DEATH/((double)groups[1]);
	death_prob[2] = deaths*NT_ELDER_DEATH/((double)groups[2]);

	return;
}

void getAgeGroups(Cell** Mesh, int* groups)
{
	for (int i = 1; i <= SIZE_I; i++) 
	{	
		for (int j = 1; j <= SIZE_J; j++)
		{
			if (Mesh[i][j].celltype == HUMAN)
			{
				switch(Mesh[i][j].age_group)
				{
					case YOUNG:
						groups[0] += 1;
						break;
					case ADULT:
						groups[1] += 1;
						break;
					case ELDER:
						groups[2] += 1;
						break;
				}
			}
		}
	}
	return;
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
				if(Mesh[i][j].gender == MALE) male += 1;
				else female += 1;
			}
			else if(Mesh[i][j].celltype == ZOMBIE) zombies += 1;
		}
	}
	fprintf(output, "%d\t%d\t%d\t%d\n", t, male, female, zombies);
	return;
}

void initializeMesh(Cell** Mesh)
{
	int i, j;

	for(i = 0; i < SIZE_I+2; i++)
	{	
		for(j = 0; j < SIZE_J+2; j++)
		{
			Mesh[i][j].celltype = EMPTY;
			Mesh[i][j].date = 0;
			Mesh[i][j].gender = 0;
			Mesh[i][j].age_group = 0;
		}
	}
	return;
}

int fillMesh(Cell** Mesh, MTRand* mt)
{
	int n_zombies, i, j, counter;
	int gender, age_group, birthdate;
	double aux_rand, acc, step;

	for(n_zombies = 0, i = 1; i <= SIZE_I; i++)
	{
		for(j = 1; j <= SIZE_J; j++)
		{
			aux_rand = mt->randExc();

			if(aux_rand < NT_POP_DENSITY)
			{
				Mesh[i][j].celltype = HUMAN;

				if(mt->randExc() < (NT_MALE_PERCENTAGE/100)) Mesh[i][j].gender = MALE;
				else Mesh[i][j].gender = FEMALE;

				aux_rand = mt->randExc();
				if(aux_rand < (NT_YOUNG/100)) Mesh[i][j].age_group = YOUNG;
				else if(aux_rand < ((NT_YOUNG+NT_ADULT)/100)) Mesh[i][j].age_group = ADULT;
				else Mesh[i][j].age_group = ELDER;

				switch(Mesh[i][j].age_group)
				{
					case YOUNG:
						step = 1/(double)(NT_YOUNG_FINAL_AGE);
						aux_rand = mt->randExc();
						for(counter = 1, acc = step; acc < 1; acc += step, counter++)
						{
							if(aux_rand < acc)
							{
								birthdate = -365*counter;
								break;
							}
						}
						if(acc >= 1) birthdate = -365*NT_YOUNG_FINAL_AGE;
						break;
					case ADULT:
						step = 1/(double)(NT_ADULT_FINAL_AGE-NT_YOUNG_FINAL_AGE);
						aux_rand = mt->randExc();
						for(counter = NT_YOUNG_FINAL_AGE+1, acc = step; acc < 1; acc += step, counter++)
						{
							if(aux_rand < acc)
							{
								birthdate = -365*counter;
								break;
							}
						}
						if(acc >= 1) birthdate = -365*NT_ADULT_FINAL_AGE;
						break;
					case ELDER:
						birthdate = -(NT_ADULT_FINAL_AGE+1)*365;
						break;
				}
				Mesh[i][j].date = birthdate;
			}
			else if(aux_rand < NT_POP_DENSITY + (NUM_OF_ZOMBIES/(double)(SIZE_I*SIZE_J)))
			{
				Mesh[i][j].celltype = ZOMBIE;
				Mesh[i][j].date = 0; 	
			}
		}	
	}
	return n_zombies;
}

void outputAsBitmap(Cell** Mesh, char* str, int w, int h)
{
	FILE *f;
	unsigned char *img = NULL;
	int r, g, b, red[w][h], green[w][h], blue[w][h];
	int x, y;
	int filesize = 54 + 3*w*h;
	
	img = (unsigned char *)malloc(3*w*h);
	memset(img,0,sizeof(img));

	for(int i = 0; i < w; i++)
	{
		for(int j = 0; j < h; j++)
		{
				switch(Mesh[i][j].celltype)
				{
					case HUMAN:
						switch(Mesh[i][j].gender)
						{
							case MALE:
								red[i][j] = 0;
								green[i][j] = 0;
								blue[i][j] = 255;
								break;
							case FEMALE:
								red[i][j] = 0;
								green[i][j] = 255;
								blue[i][j] = 0;
								break;
						}
						break;

					case ZOMBIE:
						red[i][j] = 255;
						green[i][j] = 0;
						blue[i][j] = 0;
						break;
					default:
						red[i][j] = 255;
						green[i][j] = 255;
						blue[i][j] = 255;
						break;
				}
		}
	}

	for(int i = 0; i < w; i++)
	{
	    for(int j = 0; j < h; j++)
		{
		    x = i; 
		    y = (h-1)-j;
		    r = red[i][j]*255;
		    g = green[i][j]*255;
		    b = blue[i][j]*255;
		    if (r > 255) r = 255;
		    if (g > 255) g = 255;
		    if (b > 255) b = 255;
		    img[(x+y*w)*3+2] = (unsigned char)(r);
		    img[(x+y*w)*3+1] = (unsigned char)(g);
		    img[(x+y*w)*3+0] = (unsigned char)(b);
		}
	}

	unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
	unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
	unsigned char bmppad[3] 		= {0,0,0};

	bmpfileheader[ 2] = (unsigned char)(filesize    );
	bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
	bmpfileheader[ 4] = (unsigned char)(filesize>>16);
	bmpfileheader[ 5] = (unsigned char)(filesize>>24);

	bmpinfoheader[ 4] = (unsigned char)(       w    );
	bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
	bmpinfoheader[ 6] = (unsigned char)(       w>>16);
	bmpinfoheader[ 7] = (unsigned char)(       w>>24);
	bmpinfoheader[ 8] = (unsigned char)(       h    );
	bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
	bmpinfoheader[10] = (unsigned char)(       h>>16);
	bmpinfoheader[11] = (unsigned char)(       h>>24);

	f = fopen(str,"wb");
	fwrite(bmpfileheader, 1, 14, f);
	fwrite(bmpinfoheader, 1, 40, f);
	
	for(int i = 0; i < h; i++)
	{
	    fwrite(img+(w*(h-i-1)*3),3,w,f);
	    fwrite(bmppad,1,(4-(w*3)%4)%4,f);
	}
	
	fclose(f);

	return;
}

void swapMesh(Cell** MeshA, Cell** MeshB)
{
	int i, j;

	for(i = 0; i < SIZE_I+2; i++)
		for(j = 0; j < SIZE_J+2; j++)
			MeshA[i][j] = MeshB[i][j];

	initializeMesh(MeshB);
	return;
}

void manageBoundaries(Cell** Mesh)
{
	MTRand mtwister;
	int i = 0, j;
	double odds;
	Cell aux, emptycell;

	emptycell.celltype = EMPTY;
	emptycell.date = 0;
	emptycell.gender = 0;
	emptycell.age_group = 0;

	mtwister.seed(i);
	/*
	Top boundary
	*/
	for(j = 1; j < SIZE_J+1; j++)
	{
		if(!(Mesh[0][j].celltype == EMPTY))
		{
			aux = Mesh[0][j];
			Mesh[0][j] = emptycell;
			
			if(Mesh[1][j].celltype == EMPTY)
			{
				Mesh[1][j] = aux;
			}
			else if(j > 1 && j < SIZE_J)
			{
				odds = mtwister.randExc();

				if(odds < 0.5)
				{
					if(Mesh[1][j+1].celltype == EMPTY)
					{
						Mesh[1][j+1] = aux;
					}
					else if(Mesh[1][j-1].celltype == EMPTY)
					{
						Mesh[1][j-1] = aux;
					}
				}
				else
				{
					if(Mesh[1][j-1].celltype == EMPTY)
					{
						Mesh[1][j-1] = aux;
					}
					else if(Mesh[1][j+1].celltype == EMPTY)
					{
						Mesh[1][j+1] = aux;
					}
				}
				
			}
			else if(j == 1)
			{
				/*Checks the right of original place for empty cell*/
				if(Mesh[1][j+1].celltype == EMPTY)
				{
					Mesh[1][j+1] = aux;
				}
			}
			else if(j == SIZE_J)
			{
				/*Checks the left of original place for empty cell*/
				if(Mesh[1][j-1].celltype == EMPTY)
				{
					Mesh[1][j-1] = aux;
				}
			}
		}
	}
	
	/*
	Bottom boundary
	*/
	for(j = 1; j < SIZE_J+1; j++)
	{
		if(!(Mesh[SIZE_I+1][j].celltype == EMPTY))
		{
			aux = Mesh[SIZE_I+1][j];
			Mesh[SIZE_I+1][j] = emptycell;
			
			if(Mesh[SIZE_I][j].celltype == EMPTY)
			{
				Mesh[SIZE_I][j] = aux;
			}
			else if(j > 1 && j < SIZE_J)
			{
				odds = mtwister.randExc();

				if(odds < 0.5)
				{
					if(Mesh[SIZE_I][j-1].celltype == EMPTY)
					{
						Mesh[SIZE_I][j-1] = aux;
					}
					else if(Mesh[SIZE_I][j+1].celltype == EMPTY)
					{
						Mesh[SIZE_I][j+1] = aux;
					}
				}
				else
				{
					if(Mesh[SIZE_I][j+1].celltype == EMPTY)
					{
						Mesh[SIZE_I][j+1] = aux;
					}
					else if(Mesh[SIZE_I][j-1].celltype == EMPTY)
					{
						Mesh[SIZE_I][j-1] = aux;
					}
				}
				
			}
			else if(j == 1)
			{
				if(Mesh[SIZE_I][j+1].celltype == EMPTY)
				{
					Mesh[SIZE_I][j+1] = aux;
				}
			}
			else if(j == SIZE_J)
			{
				if(Mesh[SIZE_I][j-1].celltype == EMPTY)
				{
					Mesh[SIZE_I][j-1] = aux;
				}
			}
		}
	}
	
	/*
	Left boundary
	*/
	for(i = 1; i < SIZE_I+1; i++)
	{
		if(!(Mesh[i][0].celltype == EMPTY))
		{
			aux = Mesh[i][0];
			Mesh[i][0] = emptycell;
			
			if(Mesh[i][1].celltype == EMPTY)
			{
				Mesh[i][1] = aux; 
			}
			else if(i > 1 && i < SIZE_I)
			{
				odds = mtwister.randExc();

				if(odds < 0.5)
				{
					if(Mesh[i-1][1].celltype == EMPTY)
					{
						Mesh[i-1][1] = aux;
					}
					else if(Mesh[i+1][1].celltype == EMPTY)
					{
						Mesh[i+1][1] = aux;
					}
				}
				else
				{
					if(Mesh[i+1][1].celltype == EMPTY)
					{
						Mesh[i+1][1] = aux;
					}
					else if(Mesh[i-1][1].celltype == EMPTY)
					{
						Mesh[i-1][1] = aux;
					}
				}
				
			}
			else if(i == 1)
			{
				if(Mesh[i-1][1].celltype == EMPTY)
				{
					Mesh[i-1][1] = aux;
				}
			}
			else if(i == SIZE_I)
			{
				if(Mesh[i+1][1].celltype == EMPTY)
				{
					Mesh[i+1][1] = aux;
				}
			}
		}
	}
	
	/*
	Right boundary
	*/
	for(i = 1; i < SIZE_I+1; i++)
	{
		if(!(Mesh[i][SIZE_J+1].celltype == EMPTY))
		{
			aux = Mesh[i][SIZE_J+1];
			Mesh[i][SIZE_J+1] = emptycell;
			
			if(Mesh[i][SIZE_J].celltype == EMPTY)
			{
				Mesh[i][SIZE_J] = aux; 
			}
			else if(i > 1 && i < SIZE_I)
			{
				odds = mtwister.randExc();

				if(odds < 0.5)
				{
					if(Mesh[i-1][SIZE_J].celltype == EMPTY)
					{
						Mesh[i-1][SIZE_J] = aux;	
					}
					else if(Mesh[i+1][SIZE_J].celltype == EMPTY)
					{
						Mesh[i+1][SIZE_J] = aux;
					}
				}
				else
				{
					if(Mesh[i+1][SIZE_J].celltype == EMPTY)
					{
						Mesh[i+1][SIZE_J] = aux;
					}
					else if(Mesh[i-1][SIZE_J].celltype == EMPTY)
					{
						Mesh[i-1][SIZE_J] = aux;	
					}
				}
				
			}
			else if(i == 1)
			{
				if(Mesh[i+1][SIZE_J].celltype == EMPTY)
				{
					Mesh[i+1][SIZE_J] = aux;
				}
			}
			else if(i == SIZE_I)
			{
				if(Mesh[i-1][SIZE_J].celltype == EMPTY)
				{
					Mesh[i-1][SIZE_J] = aux;	
				}
			}
		}
	}
	return;
}