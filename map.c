#define _GNU_SOURCE 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "mpi.h"

#define error(source) (perror(source),\
	     fprintf(stderr, "%s: %d\n", __PRETTY_FUNCTION__, __LINE__),\
	     exit(EXIT_FAILURE))

typedef struct 
{
	int x, y;
}Location;

int main(int argc, char* argv[])
{
	int rank, size, foundTreasures = 0;
	int initResult, *myFoundTreasures;
	struct Location* map;
	struct Location* pointsToCheck;
	int i;
	int N, K;
	
	MPI_Datatype structType, oldType;
	MPI_Aint offset;
	MPI_File file;
	int blockCount; 

	MPI_Status status; 

	// Initialize the environment
	if((initResult = MPI_Init( &argc, &argv )) != MPI_SUCCESS)
	{
		MPI_Abort(MPI_COMM_WORLD, initResult);
		error("Starting MPI");
	}

	// Read the rank and the size
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	MPI_Comm_size( MPI_COMM_WORLD, &size );
	N = size;
	K = rank;
	
	map = malloc(N * K * sizeof(struct Location));
	pointsToCheck = malloc(K * sizeof(struct Location));
	myFoundTreasures = malloc(N * sizeof(int));
	
	// Setup description of structure
	offset = 0;
	oldType = MPI_INT;
	blockCount = 2;
	
	// Commit the data type
	MPI_Type_struct(2, blockCount, offset, oldType, &structType); 
	MPI_Type_commit(&structType); 

	// Counters to determine how many treasures were found
	for(i = 0; i < N; i++)
		myFoundTreasures[i] = K;
	foundTreasures = 0;
	
    MPI_File_open(MPI_COMM_WORLD, "mapa", MPI_MODE_RDONLY, MPI_INFO_NULL, &file);
    MPI_File_read(file, map, 0, MPI_INT, &status );
    MPI_File_close(&file);

	// Divide the read map between the processes
	// Divide the map, send K parts of type structType
	// Receive them to pointsToCheck, read K parts of type structType
	// Process 0 will be the one to read using MPI_COMM_WORLD communicator
	MPI_Scatter(map, K, structType, pointsToCheck, K, structType, 0, MPI_COMM_WORLD);

	// Let the user know which process is working on which points
	printf("\nMy name is %d and my pointsToCheck are\n", rank);
	for(i = 0; i < K; i++)
		printf("\t[%d, %d]", pointsToCheck[i].x, pointsToCheck[i].y);

	// Check for the treasure!
	for (i = 0; i < K; i++)
	{
		usleep(rand());
		
		if(rand % 5 != 0)
		{
			// If there was no treasure, set the location to (0, 0) and substract from the found treasures counter
			pointsToCheck[i].x = 0;
			pointsToCheck[i].y = 0;
			myFoundTreasures[rank]--;
			
			printf("\nNo treasure at: [%d, %d]", pointsToCheck[i].x, pointsToCheck[i].y);
		}
		else
			printf("\nTreasure found at: [%d, %d]", pointsToCheck[i].x, pointsToCheck[i].y);
	}
	
	// Send the information about found treasures to the process 0
    MPI_Send(myFoundTreasures, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	MPI_Recv(myFoundTreasures, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,&status);
			
	if (rank == 0)
		usleep(2000000);

	for(i = 0; i < N; i++)
		foundTreasures+=myFoundTreasures[i];
		
	printf("We searched the map and found %d treasures\n", foundTreasures);

	// Send the results from pointsToCheck, which are K parts of type structType
	// Receive it to map by the process 0 through MPI_COMM_WORLD communicator
	MPI_Gather(&pointsToCheck, K, structType, &map, K, structType, 0, MPI_COMM_WORLD);

	if(rank == 0)
	{
		printf("We searched the map and found %d treasures in following positions:\n", foundTreasures);
		
		for(i = 0; i < N * K; i++)
			if(map[i].x != 0 && map[i].y != 0)
				printf("[%d, %d]\n", map[i].x, map[i].y);
	}
	
	free(&map);
	free(&pointsToCheck);
	free(&myFoundTreasures);

	MPI_Finalize();
	return 0;
}
