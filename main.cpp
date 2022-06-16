
#include <stdlib.h>
#include <ostream>
#include <iostream>
#include <mpi.h>
#include <Row.hpp>
#include <Column.hpp>

#define MCW MPI_COMM_WORLD

int ROWCOUNT = 5;
int COLUMNCOUNT = 5;
int TIMESTEPS = 1;

void initializeMpi(int argc, char *argv[],int &rank, int &size){

	MPI_Init(&argc,&argv);
	MPI_Comm_rank(MCW,&rank);
	MPI_Comm_size(MCW,&size);
}

int** initGrid(int **arr,int rows, int columns){
	arr = new int*[rows];
    for(int i = 0; i < rows; i++){
        arr[i] = new int[columns];
    }
	return arr;
}

int** fillGrid(int **arr,int rows, int columns){
	for(int i = 0; i < rows; i++){
		for(int j = 0; j < columns; j++){
			arr[i][j] = rand()%2;
    	}
    }
	return arr;
}

// print the processes grid with ghost rows and columns
void printProcessGrid(int **board,int rows, int columns){
	std::cout << "" << std::endl;
	for(int i = 0; i < rows; i++){
        for(int j = 0; j < columns; j++){
           std::cout << board[i][j] << " ";
        }
		std::cout << "" << std::endl;
    }
}

// print the processes grid without ghost rows and columns
void printActualProcessGrid(int **board,int rows, int columns){
	std::cout << "" << std::endl;
	for(int i = 1; i < rows-1; i++){
        for(int j = 1; j < columns-1; j++){
           std::cout << board[i][j] << " ";
        }
		std::cout << "" << std::endl;
    }
}
// prints the combined grid of all proccesses without ghost rows and columns
void printWholeGrid(int **board,int rank, int rows, int columns){
	std::cout << "" << std::endl;
	for(int i = 1; i < rows-1; i++){
        for(int j = 1; j < columns-1; j++){
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

int** ghostRowToGrid(int** grid, int* topGhostRow, int* bottomGhostRow, int rows, int columns){
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

int** ghostColumnToGrid(int** grid, int* leftGhostColumn, int* rightGhostColumn, int rows, int columns){
	for(int i = 0; i < rows; i++){
        for(int j = 0; j < columns; j++){
           if(j == 0){
           		grid[i][j] = rightGhostColumn[i];
           }else if( j == columns-1){
           		grid[i][j] = leftGhostColumn[i];
           }
        }
    }
    return grid;
}

int** fillRows(int** currentGrid, int rank, int size, int rows, int columns){
	Row bottomGhostRow(columns);
	Row topGhostRow(columns);

	bottomGhostRow.createGhostRow(currentGrid, 1);
	topGhostRow.createGhostRow(currentGrid, rows-2);
	// distributed memory is defined send and recive ghost rows
	// distributed memory is not defined no extra tasks is needed
	if(size > 1){
		int nextLowestRank = getNextLowestRank(rank,size);
		MPI_Send(bottomGhostRow.getGhostRow(),columns,MPI_INT,nextLowestRank,0,MCW);
		
		int nextHighestRank = getNextHighestRank(rank,size);
		MPI_Send(topGhostRow.getGhostRow(),columns,MPI_INT,nextHighestRank,1,MCW);

		int topSource = getTopSource(rank,size);
		int bottomSource = getBottomSource(rank,size);

		MPI_Recv(bottomGhostRow.getGhostRow(),columns,MPI_INT,topSource,0,MCW,MPI_STATUS_IGNORE);
		MPI_Recv(topGhostRow.getGhostRow(),columns,MPI_INT,bottomSource,1,MCW,MPI_STATUS_IGNORE);
	}

	currentGrid = ghostRowToGrid(currentGrid,topGhostRow.getGhostRow(),bottomGhostRow.getGhostRow(),rows,columns);

	return currentGrid;
}

int** fillColumn(int** currentGrid, int rank, int size, int rows, int columns){
	Column leftGhostRow(rows);
	Column rightGhostRow(rows);

	leftGhostRow.createGhostColumn(currentGrid,1);
	rightGhostRow.createGhostColumn(currentGrid,columns-2);

	currentGrid = ghostColumnToGrid(currentGrid,leftGhostRow.getGhostColumn(),rightGhostRow.getGhostColumn(),rows,columns);

	return currentGrid;
}

int main(int argc, char *argv[]){
	int rank;
	int size;
	int data;
	int work;
	MPI_Status status;

	initializeMpi(argc, argv,rank,size);
	srand(time(NULL) + rank);

	int rows = (ROWCOUNT/size);
	int columns = 0;

	// give the remaider of the work to the last proccess
	if(rank == size-1){
		int rowRemainder = ROWCOUNT%size;
		rows = rows+rowRemainder;
	}
	// add two for the ghost rows and columns
	rows = rows +2;
	columns = COLUMNCOUNT+2;
	
	int **currentGrid;
	currentGrid = initGrid(currentGrid,rows,columns);
	currentGrid = fillGrid(currentGrid,rows,columns);

	int **nextGrid;
	nextGrid = initGrid(nextGrid,rows,columns);
	nextGrid = copyGrid(nextGrid,currentGrid,rows,columns);

	for(int i = 0; i < TIMESTEPS; i++){
		currentGrid = fillRows(currentGrid, rank, size, rows, columns);
		currentGrid = fillColumn(currentGrid, rank, size, rows, columns);

		MPI_Barrier(MCW);

		if(rank !=0){
			// convert 2d pointer to 2d array
			int bk[(rows-2)][COLUMNCOUNT] = {};
			for(int i = 1; i < rows-1; i++){
				for(int j = 1; j < columns-1; j++){
					bk[i-1][j-1] = currentGrid[i][j];
				}
			}

			MPI_Send(bk,(rows-2)*COLUMNCOUNT,MPI_INT,0,3,MCW);
		}else{
			// rank 0 recives grids from other proccesses and organizes them
			// add a count varible to keep track of the amount of ranks that have been proccesed 
			// starts at 1 because 0 will proccess itself 
			int count = 1;
			// create a new 3d array so we can order the grids by rank 
			int arr[size][ROWCOUNT][COLUMNCOUNT] = {};
			// creates a new array that keeps track of ranks row sizes
			int rankRowSizes[size] = {};
			// sets 0 to its row size
			rankRowSizes[0] = (rows-2); 
			
			// copys rank zero's actual grid (no ghost rows or columns) to the array
			for(int i = 1; i < rows-1; i++){
				for(int j = 1; j < columns-1; j++){
					arr[0][i-1][j-1] = currentGrid[i][j];
				}
			}
			// infinitly loop to make sure we recive all the ranks data
			if(size > 1){
				while(1){ 
					count ++;
					int sendSize = 0;
					MPI_Probe(MPI_ANY_SOURCE, 3, MPI_COMM_WORLD, &status);
					// gets the size of the data getting recived
					MPI_Get_count(&status, MPI_INT, &sendSize);
					// gets the senders rank;
					int sendRank = status.MPI_SOURCE;
					// gets the rows and columns of the data getting recived
					int y = COLUMNCOUNT;
					int x = sendSize/(COLUMNCOUNT);
					// creates a new array so we can recive the data
					int actualGrid[x][y] = {};
					// fills the recived data row size
					rankRowSizes[sendRank] = x;
					// Finally we recive the data
					MPI_Recv(actualGrid,sendSize,MPI_INT,sendRank,3,MCW,&status);
					// copys the data in the correct order 
					for(int i = 0; i < x; i++){
						for(int j = 0; j < y; j++){
							arr[sendRank][i][j] = actualGrid[i][j];
						}
					}
					// once all the ranks have been proccesed we break out of our loop
					if(count == size){
						break;
					}
				}
			}
			std::cout << "Current Grid" << std::endl;
			// print the current grid
			for (int ranks = 0; ranks < size; ranks++){
				for(int i = 0; i < rankRowSizes[ranks]; i++){
					for(int j = 0; j < COLUMNCOUNT; j++){
						std::cout << arr[ranks][i][j] << " ";
					}
					std::cout << "" << std::endl;
				}
			}

			// // loop though the columns
			// for(int i = 0; i < ROWCOUNT+1; i++){
			// 	for(int j = 0; j < COLUMNCOUNT+1; i++){
			// 		// add threads here 
			// 		#pragma omp	parallel	for

			// 	}
			// }
		}
	}
	MPI_Finalize();
}

