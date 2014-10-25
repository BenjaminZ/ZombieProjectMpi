#include "simulation_exec.hpp"

int executeBirthControl(Cell** Mesh, int i, int j, double prob_birth, MTRand* mt)
{
	int flags[4] = {FALSE, FALSE, FALSE, FALSE};
	double prob;

	prob = mt->randExc();

	if(Mesh[i][j].gender == MALE)
	{
		if(Mesh[i-1][j].celltype == HUMAN)
			if(Mesh[i-1][j].gender == FEMALE)
				flags[0] = TRUE;
		if(Mesh[i][j+1].celltype == HUMAN)
			if(Mesh[i][j+1].gender == FEMALE)
				flags[1] = TRUE;
		if(Mesh[i+1][j].celltype == HUMAN)
			if(Mesh[i+1][j].gender == FEMALE)
				flags[2] = TRUE;
		if(Mesh[i][j-1].celltype == HUMAN)
			if(Mesh[i][j-1].gender == FEMALE)
				flags[3] = TRUE;
	}
	else
	{
		if(Mesh[i-1][j].celltype == HUMAN)
			if(Mesh[i-1][j].gender == MALE)
				flags[0] = TRUE;
		if(Mesh[i][j+1].celltype == HUMAN)
			if(Mesh[i][j+1].gender == MALE)
				flags[1] = TRUE;
		if(Mesh[i+1][j].celltype == HUMAN)
			if(Mesh[i+1][j].gender == MALE)
				flags[2] = TRUE;
		if(Mesh[i][j-1].celltype == HUMAN)
			if(Mesh[i][j-1].gender == MALE)
				flags[3] = TRUE;
	}
	if(prob < prob_birth) 
		return flags[0] || flags[1] || flags[2] || flags[3];
	else return FALSE;
}

void executeMovement(Cell** MeshA, Cell** MeshB, int i, int j, MTRand* mt)
{
	int flags[4] = {FALSE, FALSE, FALSE, FALSE};
	double move, range, offset, step;


	if(!(MeshA[i][j].celltype == EMPTY))
	{
		if(MeshA[i][j].celltype == HUMAN)
		{
			switch(MeshA[i][j].age_group)
			{
				case YOUNG:
					step = YOUNG_MOVE;
					break;
				case ADULT:
					step = ADULT_MOVE;
					break;
				case ELDER:
					step = ELDER_MOVE;
					break;
			}
		}
		else step = ZOMBIE_MOVE;

		move = mt->randExc();
		range = 4.0*step;
		offset = (1.0 - range)/2.0;

		if(move >= offset && move < offset + 1.0*step) flags[0] = TRUE;
		else if(move >= offset + 1.0*step && move < offset + 2.0*step) flags[1] = TRUE;
		else if(move >= offset + 2.0*step && move < offset + 3.0*step) flags[2] = TRUE;
		else if(move >= offset + 3.0*step && move < offset + 4.0*step) flags[3] = TRUE;

		/*
		Move up
		*/
		if(flags[0] && MeshA[i-1][j].celltype == EMPTY && MeshB[i-1][j].celltype == EMPTY) 
		{
			MeshB[i-1][j] = MeshA[i][j];
			MeshA[i][j].celltype = EMPTY;
		} 
		/*
		Move down
		*/
		else if(flags[1] && MeshA[i+1][j].celltype == EMPTY && MeshB[i+1][j].celltype == EMPTY) 
		{
			MeshB[i+1][j] = MeshA[i][j];
			MeshA[i][j].celltype = EMPTY;
		} 
		/*
		Move left
		*/
		else if(flags[2] && MeshA[i][j-1].celltype == EMPTY && MeshB[i][j-1].celltype == EMPTY)
		{
			MeshB[i][j-1] = MeshA[i][j];
			MeshA[i][j].celltype = EMPTY;
		} 
		/*
		Move right
		*/
		else if(flags[3] && MeshA[i][j+1].celltype == EMPTY && MeshB[i][j+1].celltype == EMPTY) 
		{
			MeshB[i][j+1] = MeshA[i][j];
			MeshA[i][j].celltype = EMPTY;
		}
		/*
		Does not move
		*/
		else 
		{
			MeshB[i][j] = MeshA[i][j];
			MeshA[i][j].celltype = EMPTY;
		}
	}
}

void executeInfection(Cell** Mesh, int i, int j, int n, MTRand* mt)
{
	double random_num, random_inf, vector_aux, aux;
	
	vector<int> pos_i, pos_j;

	random_inf = mt->randExc();
	
	if(random_inf > INFECTION_PROB && random_inf < 1 - KILL_ZOMBIE) return;
	
	
	if(Mesh[i-1][j].celltype == HUMAN) 
	{
		pos_i.push_back(i-1);
		pos_j.push_back(j);
	}
	if(Mesh[i][j+1].celltype == HUMAN)
	{
		pos_i.push_back(i);
		pos_j.push_back(j+1);
	}
	if(Mesh[i+1][j].celltype == HUMAN)
	{
		pos_i.push_back(i+1);
		pos_j.push_back(j);
	}
	if(Mesh[i][j-1].celltype == HUMAN)
	{
		pos_i.push_back(i);
		pos_j.push_back(j-1);
	}

	if(pos_i.size() == 0) return;
	
	else
	{
		if(random_inf >= (1.0 - KILL_ZOMBIE))
		{
			Mesh[i][j].celltype = EMPTY;
			return;
		}

		random_num = mt->randExc();
		aux = (1.0)/((double)pos_i.size());

		for(int k = 0, vector_aux = aux; k < pos_i.size(); k++, vector_aux += aux)
		{
			if(random_num < vector_aux)
			{
				Mesh[(pos_i[k])][(pos_j[k])].celltype = ZOMBIE;
				Mesh[(pos_i[k])][(pos_j[k])].date = n;
				break;
			}
		}
	}
	pos_i.clear();
	pos_j.clear();
	return;
}

void executeDeathControl(Cell** Mesh, int i, int j, double* prob_death, int n, MTRand* mt)
{
	double prob_kill;

	if(Mesh[i][j].celltype == HUMAN)
	{
		prob_kill = mt->randExc();

		switch(Mesh[i][j].age_group)
		{
			case YOUNG:
				if(prob_kill < prob_death[0]) Mesh[i][j].celltype = EMPTY;
				break;
			case ADULT:
				if(prob_kill < prob_death[1]) Mesh[i][j].celltype = EMPTY;
				break;
			case ELDER:
				if(prob_kill < prob_death[2]) Mesh[i][j].celltype = EMPTY;
				break;
		}
	}
	else if(Mesh[i][j].celltype == ZOMBIE)
	{
		if((n - Mesh[i][j].date) > ZOMBIE_LIFESPAN) 
			Mesh[i][j].celltype = EMPTY;
	}
	return;
}