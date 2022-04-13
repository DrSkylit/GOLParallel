#include <stdlib.h>
#include <iostream>
#include <Row.hpp>

Row::Row(void) {
   std::cout << "constructor" << std::endl;
}

void Row::getGhostRow(int **grid, int rows, int columns){

}

int* Row::getRow(){
  return this->row;
}

void Row::printRow(){

}