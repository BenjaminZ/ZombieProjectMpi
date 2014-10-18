#include "zombiesim.hpp"

#if defined(_OPENMP)
void lock(int i, bool *locks) 
{
	for (bool locked = false; locked == false; /*NOP*/) 
	{
		#pragma omp critical (LockRegion)
		{
			locked = !locks[i-1] && !locks[i] && !locks[i+1];
			if (locked) 
			{
				locks[i-1] = true; locks[i] = true; locks[i+1] = true;
			}
		}
	}
	return;
}

void unlock(int i, bool *locks) 
{
	#pragma omp critical (LockRegion)
	{
		locks[i-1] = false; 
		locks[i] = false; 
		locks[i+1] = false;
	}
}
#endif

int main(int argc, char **argv)
{
	time_t begin, end;

	char aux_str[100];
	int i, j, n, n_zombie;
	bool *locks = new bool[SIZE_I+2];
	MTRand mt_thread[64];
	FILE *output;

	Cell MeshA[SIZE_I+2][SIZE_J+2], MeshB[SIZE_I+2][SIZE_J+2];


	/*
	Initializes the MersenneTwister PRNG and the locks.
	*/
	for(i = 0; i < 64; i++) mt_thread[i].seed(i);
	for(i = 0; i < SIZE_I+2; i++) locks[i] = false;
	/*
	Initializes MeshA and MeshB.
	*/
	initializeMesh(MeshA);
	initializeMesh(MeshB);
	n_zombie = fillMesh(MeshA, &mt_thread[0]);

	/*
	Output definition
	*/
	sprintf(aux_str, "%soutput.txt", OUTPUT_PATH);
	output = fopen(str, "w+");
	fprintf(output, "Time\tMale\tFemale\tZombie\n");
	sprintf(str, "%sIP_%.3lf_ST_%05d.bmp", BITMAP_PATH, INFECTION_PROBABILITY, 0);
	outputAsBitmap(MeshA, aux_str, SIZE_I+2, SIZE_J+2);

	/*
	Main loop
	*/
	for(n = 1; n <= STEPS; n++)
	{
		double prob_birth = 0.0, death_prob[3] = {0.0, 0.0, 0.0};
		int babycounter = 0, non_empty = 0;

		#if defined(_OPENMP)
		#pragma omp parallel for default(none) firstprivate(babycounter, non_empty) shared(mt_thread, MeshA, MeshB, locks, n, prob_birth, death_prob);
		#endif

		for(i = 1; i <= SIZE_I; i++)
		{
			#if defined(_OPENMP)
			lock(i, locks);
			#endif

			int n_thread = omp_get_thread_num();

			for(int j = 1; j <= SIZE; j++)
			{
				if(babycounter > 0)
				{
					if(MeshA[i][j].celltype == EMPTY)
					{
						non_empty += 1;
						if(non_empty == 4)
						{
							non_empty = 0;
							babycounter -= 1;
						}

					}
				}
				else
				{
					if(mt_thread[n_thread].randExc() < (NT_MALE_PERCENTAGE/100))
						setCellToHuman(MALE, n, YOUNG);
					else
						setCellToHuman(FEMALE, n, YOUNG);
					babycounter -= 1;
					non_empty = 0;
					continue;
				}

				if(MeshA[i][j].celltype == HUMAN)
				{
					if(executeBirthControl(MeshA, i, j, prob_birth, &mt_thread[n_thread]) == TRUE)
						babycounter++;
				}
				if(MeshA[i][j].celltype == ZOMBIE)
					executeInfection(MeshA, i, j, n, &mt_thread[n_thread]);

				executeDeathControl(MeshA, i, j, &mt_thread[n_thread]);

				executeMovement(MeshA, MeshB, i, j, &mt_thread[num_thread]);
			}
			#if defined(_OPENMP)
			unlock(i, locks);
			#endif
		}
		manageBoundaries(MeshB);
		swap(MeshA, MeshB);

		if(! (n % BITMAP_STEP))
		{
			sprintf(str, "%sIP_%.3lf_ST_%05d.bmp", BITMAP_PATH, INFECTION_PROBABILITY, n);
			outputAsBitmap(MeshA, aux_str, SIZE_I+2, SIZE_J+2);
		}
	}

	sprintf(str, "%sIP_%.3lf_ST_%05d.bmp", BITMAP_PATH, INFECTION_PROBABILITY, n);
	outputAsBitmap(MeshA, aux_str, SIZE_I+2, SIZE_J+2);

	time(&end);
	fprintf(output, "\nTime spent: %.5f\n", (float)(end-begin));
	fclose(output);

	return 0;
}