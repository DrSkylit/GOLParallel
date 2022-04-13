class Row{
    public:
        Row();
        void getGhostRow(int **grid, int rows, int columns);
        int* getRow();
        void printRow();
    private:
        int* row;
        int size;
};