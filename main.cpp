#include "zombiesim.hpp"
#include "mesh_manipulation.hpp"
#include "simulation_exec.hpp"
#include <mpi.h>

void checkMPI_mesh_END(Cell** MeshB, Cell* buffer, int rank);
void checkMPI_mesh_MIDDLE(Cell** Mesh, Cell* buffer, int rank);
void eraseRow_mesh(Cell** Mesh, int row);

/*
OpenMP locks.
*/
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

/*
	MPI tags.
*/
#define MPI_POP 		0
#define MPI_START 		1
#define MPI_MIDDLE 		2
#define MPI_MIDDLE_2 	3
#define MPI_END 		4
#define MPI_GROUPS		5


int main(int argc, char* argv[])
{
	char aux_str[100];
	int i, j, n, n_zombie;
	
	MTRand mt_thread[64];
	FILE *output;

	Cell *MeshA[SIZE_I+2];
	Cell *MeshB[SIZE_I+2];
	
	bool *locks = new bool[SIZE_I+2];

	/*
	Initialize MPI.
	*/
	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	
	/*
	This code is meant to work with 2 nodes only.
	*/
	if(size != 2) return -1;

	/*
	MPI declarations.
	Define a MPI_Datatype to the Cell struct.
	Reference:
		http://static.msi.umn.edu/tutorial/scicomp/general/MPI/deriveddata/struct_c.html
	*/
	MPI_Datatype cell_type, old_types[1];
	int blockcounts[1];
	MPI_Aint offsets[1];
	MPI_Status stat;

	offsets[0] = 0;			//There's no need for offsets
	blockcounts[0] = 4;		//There are 4 ints inside the struct
	old_types[0] = MPI_INT;	//Type INT

	MPI_Type_struct(1, blockcounts, offsets, old_types, &cell_type);
	MPI_Type_commit(&cell_type);

	/*
	Initializes the MersenneTwister PRNG and the locks.
	*/
	for(i = 0; i < SIZE_I+2; i++) MeshA[i] = (Cell*)malloc((SIZE_J+2)*sizeof(Cell));
	for(i = 0; i < SIZE_I+2; i++) MeshB[i] = (Cell*)malloc((SIZE_J+2)*sizeof(Cell));
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
	sprintf(aux_str, "%d_%soutput.txt", rank, OUTPUT_PATH);
	output = fopen(aux_str, "w+");
	fprintf(output, "Time\tMale\tFemale\tZombie\n");
	sprintf(aux_str, "%d_%sIP_%.3lf_ST_%05d.bmp", rank, BITMAP_PATH, INFECTION_PROB, 0);
	outputAsBitmap(MeshA, aux_str, SIZE_I+2, SIZE_J+2);
	printPopulation(output, MeshA, 0);
	
	/*
	Main loop
	*/
	for(n = 1; n <= STEPS; n++)
	{
		int other_population, this_population;
		
		/*
		The grid statistics, such as population, have to be synchronized.
		Also, the last line of Rank0 has to be copied to the first (ghost) line of Rank1,
		as well as, the first of Rank1 has to be copied to the last (ghost) line of Rank0.
		*/
		this_population = getPopulation(MeshA);
		
		MPI_Send(&this_population, 1, MPI_INT, (rank+1)%2, MPI_POP, MPI_COMM_WORLD);
		MPI_Recv(&other_population, 1, MPI_INT, (rank+1)%2, MPI_POP, MPI_COMM_WORLD, &stat);

		this_population += other_population;

		if(!rank)
			MPI_Send(MeshA[SIZE_I], SIZE_J+2, cell_type, (rank+1)%2, MPI_START, MPI_COMM_WORLD);
		else
			MPI_Send(MeshA[1], SIZE_J+2, cell_type, (rank+1)%2, MPI_START, MPI_COMM_WORLD);

		if(!rank)
			MPI_Recv(MeshA[SIZE_I+1], SIZE_J+2, cell_type, (rank+1)%2, MPI_START, MPI_COMM_WORLD, &stat); 
		else
			MPI_Recv(MeshA[0], SIZE_J+2, cell_type, (rank+1)%2, MPI_START, MPI_COMM_WORLD, &stat);

		
		double prob_birth = 0.0;
		int babycounter = 0, non_empty = 0;
		int this_genders[2], other_genders[2];
		double death_prob[3] = {0.0, 0.0, 0.0};
		int this_groups[3], other_groups[3];

		/*
		Exchange gender groups size to calculate birth probability.
		*/
		getGenderNumber(MeshA, this_genders);
		/*
		Exchange age groups size to calculate death probability.
		*/
		getAgeGroups(MeshA, this_groups);

		int this_vector[5], other_vector[5];

		this_vector[0] = this_genders[0];
		this_vector[1] = this_genders[1];
		this_vector[2] = this_groups[0];
		this_vector[3] = this_groups[1];
		this_vector[4] = this_groups[2];

		MPI_Send(this_vector, 5, MPI_INT, (rank+1)%2, MPI_GROUPS, MPI_COMM_WORLD);
		MPI_Recv(other_vector, 5, MPI_INT, (rank+1)%2, MPI_GROUPS, MPI_COMM_WORLD, &stat);

		this_genders[0] += other_vector[0];
		this_genders[1] += other_vector[1];
		this_groups[0]  += other_vector[2];
		this_groups[1]  += other_vector[3];
		this_groups[2]  += other_vector[4];

		prob_birth = getBirthRate(MeshA, this_population)/getPairingNumber(this_genders);
		getDeathProb(death_prob, this_population, this_groups);
		
		/*
		Executes birth control for humans or infection for zombies
		*/
		#if defined(_OPENMP)
		#pragma omp parallel for default(none) firstprivate(babycounter, non_empty) shared(mt_thread, MeshA, MeshB, locks, n, prob_birth, death_prob)
		#endif

		for(int i = 1; i <= SIZE_I; i++)
		{
			#if defined(_OPENMP)
			lock(i, locks);
			#endif

			int n_thread = omp_get_thread_num();

			for(int j = 1; j <= SIZE_J; j++)
			{
				if(MeshA[i][j].celltype == HUMAN)
					executeBirthControl(MeshA, i, j, n, prob_birth, &mt_thread[n_thread]);

				if(MeshA[i][j].celltype == ZOMBIE)
					executeInfection(MeshA, i, j, n, &mt_thread[n_thread]);

				executeDeathControl(MeshA, i, j, death_prob, n, &mt_thread[n_thread]);
			}
			#if defined(_OPENMP)
			unlock(i, locks);
			#endif
		}
		/*
		Corrects the Mesh.
		*/
		Cell empty_cell;

		empty_cell.celltype = EMPTY;
		empty_cell.date = 0;
		empty_cell.gender = 0;
		empty_cell.age_group = 0;

		MeshA[0][0] = empty_cell;
		MeshA[SIZE_I+1][0] = empty_cell;
		MeshA[0][SIZE_J+1] = empty_cell;
		MeshA[SIZE_I+1][SIZE_J+1] = empty_cell;

		/*
		Synchronizes the meshes.
		*/
		Cell buffer[SIZE_J];
		
		if(!rank)
			MPI_Send(MeshA[SIZE_I+1], SIZE_J+2, cell_type, (rank+1)%2, MPI_MIDDLE, MPI_COMM_WORLD);
		else
			MPI_Send(MeshA[0], SIZE_J+2, cell_type, (rank+1)%2, MPI_MIDDLE, MPI_COMM_WORLD);
		
		MPI_Recv(buffer, SIZE_J+2, cell_type, (rank+1)%2, MPI_MIDDLE, MPI_COMM_WORLD, &stat); 

		checkMPI_mesh_MIDDLE(MeshA, buffer, rank);

		if(!rank)
			MPI_Send(MeshA[SIZE_I], SIZE_J+2, cell_type, (rank+1)%2, MPI_MIDDLE, MPI_COMM_WORLD);
		else
			MPI_Send(MeshA[1], SIZE_J+2, cell_type, (rank+1)%2, MPI_MIDDLE_2, MPI_COMM_WORLD);

		if(!rank)
			MPI_Recv(MeshA[SIZE_I+1], SIZE_J+2, cell_type, (rank+1)%2, MPI_MIDDLE, MPI_COMM_WORLD, &stat); 
		else
			MPI_Recv(MeshA[0], SIZE_J+2, cell_type, (rank+1)%2, MPI_MIDDLE_2, MPI_COMM_WORLD, &stat); 

		/*
		Executes movement.
		*/
		#if defined(_OPENMP)
		#pragma omp parallel for default(none) firstprivate(babycounter, non_empty) shared(mt_thread, MeshA, MeshB, locks, n, prob_birth, death_prob)
		#endif

		for(int i = 1; i <= SIZE_I; i++)
		{
			#if defined(_OPENMP)
			lock(i, locks);
			#endif

			int n_thread = omp_get_thread_num();

			for(int j = 1; j <= SIZE_J; j++)
				executeMovement(MeshA, MeshB, i, j, &mt_thread[n_thread]);

			#if defined(_OPENMP)
			unlock(i, locks);
			#endif
		}

		if(!rank)
			MPI_Send(MeshB[SIZE_I+1], SIZE_J+2, cell_type, (rank+1)%2, MPI_MIDDLE, MPI_COMM_WORLD);
		else
			MPI_Send(MeshB[0], SIZE_J+2, cell_type, (rank+1)%2, MPI_MIDDLE_2, MPI_COMM_WORLD);

		MPI_Recv(buffer, SIZE_J, cell_type, (rank+1)%2, MPI_MIDDLE_2, MPI_COMM_WORLD, &stat);

		checkMPI_mesh_END(MeshB, buffer, rank);

		if(!rank) 
			eraseRow_mesh(MeshB, SIZE_I+1);
		else 
			eraseRow_mesh(MeshB, 0);

		manageBoundaries(MeshB);

		swapMesh(MeshA, MeshB);

		if(! (n % BITMAP_STEP))
		{
			sprintf(aux_str, "%d_%sIP_%.3lf_ST_%05d.bmp", rank, BITMAP_PATH, INFECTION_PROB, n);
			outputAsBitmap(MeshA, aux_str, SIZE_I+2, SIZE_J+2);
			printPopulation(output, MeshA, n);
		}
	}

	/*
	Finishes the output
	*/
	sprintf(aux_str, "%d_%sIP_%.3lf_ST_%05d.bmp",rank, BITMAP_PATH, INFECTION_PROB, n);
	outputAsBitmap(MeshA, aux_str, SIZE_I+2, SIZE_J+2);
	printPopulation(output, MeshA, n);
	fclose(output);

	/*
	Releases memory resources
	*/
	for(i = 0; i < SIZE_I+2; i++) free(MeshA[i]);
	for(i = 0; i < SIZE_I+2; i++) free(MeshB[i]);
	
	return 0;
}

/*
New functions to deal with MPI constraints.
*/
void eraseRow_mesh(Cell** Mesh, int row)
{
	Cell empty_cell;

	empty_cell.celltype = EMPTY;
	empty_cell.date = 0;
	empty_cell.gender = 0;
	empty_cell.age_group = 0;

	for(int j = 0; j < SIZE_J+2; j++)
		Mesh[row][j] = empty_cell;

	return;
}

/*
After executing birth and death control and infection,
this functions synchronizes the meshes from different nodes.
*/
void checkMPI_mesh_MIDDLE(Cell** Mesh, Cell* buffer, int rank)
{
	if(!rank)
	{
		for(int j = 1; j <= SIZE_J; j++)
		{
			if(Mesh[SIZE_I][j].celltype == EMPTY && buffer[j].celltype == EMPTY) continue;
			else
			{
				/*
				If a cell from the mesh is empty now, there are 2 cases:
					- If there was any human/zombie there, he died;
					- The cell was empty, but a baby was created. Ignore the baby. (Check if it is possible)
				*/
				if(Mesh[SIZE_I][j].celltype == EMPTY) continue;
				else Mesh[SIZE_I][j] = buffer[j];
			}
		}
	}
	else
	{
		for(int j = 1; j <= SIZE_J; j++)
		{
			if(Mesh[1][j].celltype == EMPTY && buffer[j].celltype) continue;
			else
			{
				/*
				If a cell from the mesh is empty now, there are 2 cases:
					- If there was any human/zombie there, he died;
					- The cell was empty, but a baby was created. Ignore the baby. (Check if it is possible)
				*/
				if(Mesh[1][j].celltype == EMPTY) continue;
				else Mesh[1][j] = buffer[j];
			}
		}
	}
	return;
}

void checkMPI_mesh_END(Cell** MeshB, Cell* buffer, int rank)
{
	/*
	Strategy adopted:
		buffer contains all humans/zombies that changed from one node to another.
		If the receiving node has an empty cell in the same place where this human/zombie
		moved, then he will be placed. Otherwise, he will be removed.
	*/
	if(!rank)
	{
		for(int j = 1; j <= SIZE_J; j++)
			if(buffer[j].celltype != EMPTY)
				if(MeshB[SIZE_I][j].celltype == EMPTY)
					MeshB[SIZE_I][j] = buffer[j];
	}
	else
	{
		for(int j = 1; j <= SIZE_J; j++)
			if(buffer[j].celltype != EMPTY)
				if(MeshB[1][j].celltype == EMPTY)
					MeshB[1][j] = buffer[j];
	}

	return;	
}