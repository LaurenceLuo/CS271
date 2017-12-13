#include<iostream>
using namespace std;

struct Node
{
    Node* left, *right, *up, *down;
    int col, row;
    Node(){
        left = NULL; right = NULL;
        up = NULL; down = NULL;
        col = 0; row = 0;
    }
    Node( int r, int c )
    {
        left = NULL; right = NULL;
        up = NULL; down  = NULL;
        col = c; row = r;
    }
};

struct ColunmHeader : public Node
{
    int size;
    ColunmHeader(){
        size = 0;
    }
};

class ExtractCoverMatrix
{
public:
    int ROWS, COLS;
    //这是存储结果的数组
    int* disjointSubet;
    //接收矩阵其及维度
    ExtractCoverMatrix( int rows, int cols, int** matrix );
    //释放内存
    ~ExtractCoverMatrix();
    //在行r列c的位置上插入一个元素
    void insert( int r, int c );
    //即我们的cover和uncover操作了
    void cover( int c );
    void uncover( int c );
    //即我们的algorithm X的实现了
    int search( int k=0 );
private:
    ColunmHeader* root;
    ColunmHeader* ColIndex;
    Node* RowIndex;
};

