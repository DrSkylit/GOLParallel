
#include <stdlib.h>
#include <ostream>
#include <iostream>
#include <mpi.h>
#include <Row.hpp>

#define MCW MPI_COMM_WORLD

int ROWCOUNT = 5;
int COLUMNCOUNT = 5;
int TIMESTEPS = 1;
void initializeMpi(int argc, char *argv[],int &rank, int &size){

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MCW,&rank);
	MPI_Comm_size(MCW,&size);
}

int** initGrid(int **arr,int row, int column){
	arr = new int*[row];
    for(int i = 0; i < row; i++){
        arr[i] = new int[column];
    }
	return arr;
}

int** fillGrid(int **arr,int row, int column){
	for(int i = 0; i < row; i++){
		for(int j = 0; j < column; j++){
			arr[i][j] = rand()%2;
    	}
    }
	return arr;
}

void printGrid(int **board,int row, int column){
	std::cout << "" << std::endl;
	for(int i = 0; i < row; i++){
        for(int j = 0; j < column; j++){
           std::cout << board[i][j] << " ";
        }
		std::cout << "" << std::endl;
    }
}

int** copyGrid(int **copyTo, int **copyFrom,int row, int column){
	for(int i = 0; i < row; i++){
		for(int j = 0; j < column; j++){
			copyTo[i][j] = copyFrom[i][j];
		}
	}
	return copyTo;
}

int getNextLowestRank(int rank, int size){
	int nextLowestRank = rank-1;	
	if(nextLowestRank == -1){
		nextLowestRank = size-1;
	}
	return nextLowestRank;
}

int getNextHighestRank(int rank, int size){
	int nextHighestRank = rank+1;	
	if(nextHighestRank == size){
		nextHighestRank = 0;
	}
	return nextHighestRank;
}

int getTopSource(int rank, int size){
	int source = rank+1;
	if(source == size){
		source = 0;
	}
	return source;
}

int getBottomSource(int rank, int size){
	int source = rank-1;
	if(source == -1){
		source = size-1;
	}
	return source;
}

int** ghostToGrid(int** grid, int* topGhostRow, int* bottomGhostRow, int rows, int columns){
	for(int i = 0; i < rows; i++){
        for(int j = 0; j < columns; j++){
           if(i == 0){
           		grid[i][j] = topGhostRow[j];
           }else if( i == rows-1){
           		grid[i][j] = bottomGhostRow[j];
           }
        }
    }
    return grid;
}

int main(int argc, char *argv[]){
	int rank;
	int size;
	int data;
	int work;

	initializeMpi(argc, argv,rank,size);
	srand(time(NULL) + rank);

	int rows = (ROWCOUNT/size);
	int columns = 0;

	// give the remaider of the work to the last proccess
	if(rank == size-1){
		int rowRemainder = ROWCOUNT%size;
		rows = rows+rowRemainder;
	}

	// add two for the ghost rows
	rows = rows +2;
	columns = COLUMNCOUNT+2;
	
	int **currentGrid;
	currentGrid = initGrid(currentGrid,rows,columns);
	currentGrid = fillGrid(currentGrid,rows,columns);

	int **nextGrid;
	nextGrid = initGrid(nextGrid,rows,columns);
	nextGrid = copyGrid(nextGrid,currentGrid,rows,columns);

	for(int i = 0; i < TIMESTEPS; i++){
		Row bottomGhostRow(columns);
		Row topGhostRow(columns);
		int nextLowestRank = 0;
		// distributed memory is defined
		if(size > 1){
			nextLowestRank = getNextLowestRank(rank,size);
			bottomGhostRow.createGhostRow(currentGrid, 1);
			MPI_Send(bottomGhostRow.getGhostRow(),columns,MPI_INT,nextLowestRank,0,MCW);
			
			int nextHighestRank = getNextHighestRank(rank,size);
			topGhostRow.createGhostRow(currentGrid, rows-2);
			MPI_Send(topGhostRow.getGhostRow(),columns,MPI_INT,nextHighestRank,1,MCW);

			int topSource = getTopSource(rank,size);
			int bottomSource = getBottomSource(rank,size);

			MPI_Recv(bottomGhostRow.getGhostRow(),columns,MPI_INT,topSource,0,MCW,MPI_STATUS_IGNORE);
			MPI_Recv(topGhostRow.getGhostRow(),columns,MPI_INT,bottomSource,1,MCW,MPI_STATUS_IGNORE);

			currentGrid = ghostToGrid(currentGrid,topGhostRow.getGhostRow(),bottomGhostRow.getGhostRow(),rows,columns);

		}else{
			// distributed memory is not defined
			bottomGhostRow.createGhostRow(currentGrid, 1);
			topGhostRow.createGhostRow(currentGrid, rows-2);
			currentGrid = ghostToGrid(currentGrid,topGhostRow.getGhostRow(),bottomGhostRow.getGhostRow(),rows,columns);
		}
	}

	MPI_Finalize();
}