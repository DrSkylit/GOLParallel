class Column{
    public:
        Column(int rows);
        ~Column(); // destructor
        void createGhostColumn(int **grid, int columnNum);
        int* getGhostColumn();
        void printColumn();
    private:
        int* column;
        int rows;
};