#include <stdlib.h>
#include <iostream>
#include <Column.hpp>

Column::Column(int rows) {
    this->rows = rows;
}

Column::~Column(){
    delete this->column;
}

void Column::createGhostColumn(int **grid, int columnNum){
    int *arr;
    arr = new int[this->rows];
    for(int i = 0; i < this->rows; i++){
        arr[i] = grid[i][columnNum];
    }
    this->column = arr;
}

void Column::printColumn(){
    for(int i = 0; i < this->rows;i++){
        std::cout << column[i] << " ";
    }
    std::cout << "" << std::endl;
}

int* Column::getGhostColumn(){
    return this->column;
}