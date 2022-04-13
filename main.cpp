
#include <stdlib.h>
#include <ostream>
#include <iostream>
#include <mpi.h>

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

int* getRow(int **grid, int rows, int columns){
	int *arr;
	arr = new int[columns];
	for(int i = 0; i < columns; i++){
		arr[i] = grid[rows][i];
	}
	return arr;
}

void printRow(int *row,int columns){
	for(int i = 0; i < columns;i++){
		std::cout << row[i] << " ";
	}
	std::cout << "" << std::endl;
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
		int nextLowestRank = 0;
		// distributed memory is defined
		if(size > 1){
			nextLowestRank = getNextLowestRank(rank,size);
			// get second from top
			int *bottomGhostRow;
			bottomGhostRow = getRow(currentGrid, 1,columns);
			//send to next lowest rank
			MPI_Send(bottomGhostRow,columns,MPI_INT,nextLowestRank,0,MCW);
			
			int nextHighestRank = getNextHighestRank(rank,size);
			int *topGhostRow;
			topGhostRow = getRow(currentGrid, rows-2,columns);
			MPI_Send(topGhostRow,columns,MPI_INT,nextHighestRank,1,MCW);

			int topSource = rank+1;
			if(topSource == size){
				topSource = 0;
			}

			int bottomSource = rank-1;
			if(bottomSource == -1){
				bottomSource = size-1;
			}

			MPI_Recv(bottomGhostRow,columns,MPI_INT,topSource,0,MCW,MPI_STATUS_IGNORE);
			MPI_Recv(topGhostRow,columns,MPI_INT,bottomSource,1,MCW,MPI_STATUS_IGNORE);

			std::cout << "rank: " << rank << std::endl;
			std::cout << "Grid:";
			printGrid(currentGrid,rows,columns);
			std::cout << "Top Ghost Row:";
			printRow(topGhostRow,columns);
			std::cout << "Bottom Ghost Row:";
			printRow(bottomGhostRow,columns);
			currentGrid = ghostToGrid(currentGrid,topGhostRow,bottomGhostRow,rows,columns);
			std::cout << "Completed Grid:";
			printGrid(currentGrid,rows,columns);

		}else{
			// distributed memory is not defined

		}
	}

	MPI_Finalize();
}
