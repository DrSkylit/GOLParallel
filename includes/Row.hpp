class Row{
    public:
        Row(int columns);
        ~Row(); // destructor
        void createGhostRow(int **grid, int rowNum);
        int* getGhostRow();
        void printRow();
    private:
        int* row;
        int columns;
};