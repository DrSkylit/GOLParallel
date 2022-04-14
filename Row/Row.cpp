#include <stdlib.h>
#include <iostream>
#include <Row.hpp>

Row::Row(int columns) {
    this->columns = columns;
}

Row::~Row(){
    delete this->row;
}

void Row::createGhostRow(int **grid, int rowNum){
    int *arr;
    arr = new int[this->columns];
    for(int i = 0; i < this->columns; i++){
        arr[i] = grid[rowNum][i];
    }
    this->row = arr;
}

void Row::printRow(){
    for(int i = 0; i < this->columns;i++){
        std::cout << row[i] << " ";
    }
    std::cout << "" << std::endl;
}

int* Row::getGhostRow(){
    return this->row;
}