#include "ExtractCoverMatrix.h"
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

const int N_Cell = 4;
const int N = 16;
const int OFFSET = 16*16;
const int ROWS = N*N*N;
const int COLS = N*N*4;
#define NUM_DIGITS 16

int digitCharToInt(char c){
    if(c >= '0' && c <= '9')
        return (int)c - (int)'0';
    if(c >= 'A' && c <= 'F')
        return (int)c - (int)'A' + 10;
    return -1;
}
char intToDigitChar(int i){
    if(i >= 0 && i <= 9)
        return (char)(i + (int)'0');
    if(i >= 10 && i <= 15)
        return (char)(i - 10 + (int)'A');
    
    return '\0';
}
int** getInputArray(string s){
    int** array=new int*[NUM_DIGITS];
    for(int i=0;i<NUM_DIGITS;i++)
        array[i]=new int[NUM_DIGITS];
    int pos=0;
    string token;
    for(int i=0;i<NUM_DIGITS;i++){
        for(int j=0;j<NUM_DIGITS;j++){
            pos = s.find(" ");
            //cout<<"pos: "<<pos<<endl;
            if(pos==0){
                s.erase(0, pos + 1);
                pos = s.find(" ");
            }
            pos = s.find(" ");
            if(pos==0){
                s.erase(0, pos + 1);
                pos = s.find(" ");
            }
            token = s.substr(0, pos);
            //cout<<"token: "<<token<<endl;
            array[i][j]=digitCharToInt(token.at(0));
            //cout<<"array "i<<", "<<j<<": "<<array[i][j]<<endl;
            s.erase(0, pos + 1);
        }
    }
    return array;
}

ExtractCoverMatrix::ExtractCoverMatrix( int rows, int cols, int** matrix )
{
    ROWS = rows;
    COLS = cols;
    disjointSubet = new int[rows+1];
    ColIndex = new ColunmHeader[cols+1];
    RowIndex = new Node[rows];
    root = &ColIndex[0];
    ColIndex[0].left = &ColIndex[COLS];
    ColIndex[0].right = &ColIndex[1];
    ColIndex[COLS].right = &ColIndex[0];
    ColIndex[COLS].left = &ColIndex[COLS-1];
    for( int i=1; i<cols; i++ )
    {
        ColIndex[i].left = &ColIndex[i-1];
        ColIndex[i].right = &ColIndex[i+1];
    }
    
    for ( int i=0; i<=cols; i++ )
    {
        ColIndex[i].up = &ColIndex[i];
        ColIndex[i].down = &ColIndex[i];
        ColIndex[i].col = i;
    }
    ColIndex[0].down = &RowIndex[0];
    
    for( int i=0; i<rows; i++ )
        for( int j=0; j<cols&&matrix[i][j]>0; j++ )
        {
            insert(  i, matrix[i][j] );
        }
}

ExtractCoverMatrix::~ExtractCoverMatrix()
{
    delete[] disjointSubet;
    for( int i=1; i<=COLS; i++ )
    {
        Node* cur = ColIndex[i].down;
        Node* del = cur->down;
        while( cur != &ColIndex[i] )
        {
            delete cur;
            cur = del;
            del = cur->down;
        }
    }
    delete[] RowIndex;
    delete[] ColIndex;
}

void ExtractCoverMatrix::insert( int r, int c )
{
    Node* cur = &ColIndex[c];
    ColIndex[c].size++;
    Node* newNode = new Node( r, c );
    while( cur->down != &ColIndex[c] && cur->down->row < r )
        cur = cur->down;
    newNode->down = cur->down;
    newNode->up = cur;
    cur->down->up = newNode;
    cur->down = newNode;
    if( RowIndex[r].right == NULL )
    {
        RowIndex[r].right = newNode;
        newNode->left = newNode;
        newNode->right = newNode;
    }
    else
    {
        Node* rowHead = RowIndex[r].right;
        cur = rowHead;
        while( cur->right != rowHead && cur->right->col < c )
            cur = cur->right;
        newNode->right = cur->right;
        newNode->left = cur;
        cur->right->left = newNode;
        cur->right = newNode;
    }
}

void ExtractCoverMatrix::cover( int c )
{
    ColunmHeader* col = &ColIndex[c];
    col->right->left = col->left;
    col->left->right = col->right;
    Node* curR, *curC;
    curC = col->down;
    while( curC != col )
    {
        Node* noteR = curC;
        curR = noteR->right;
        while( curR != noteR )
        {
            curR->down->up = curR->up;
            curR->up->down = curR->down;
            ColIndex[curR->col].size--;
            curR = curR->right;
        }
        curC = curC->down;
    }
}

void ExtractCoverMatrix::uncover( int c )
{
    Node* curR, *curC;
    ColunmHeader* col = &ColIndex[c];
    curC = col->up;
    while( curC != col )
    {
        Node* noteR = curC;
        curR = curC->left;
        while( curR != noteR )
        {
            ColIndex[curR->col].size++;
            curR->down->up = curR;
            curR->up->down = curR;
            curR = curR->left;
        }
        curC = curC->up;
    }
    col->right->left = col;
    col->left->right = col;
}

int ExtractCoverMatrix::search( int k )
{
    if( root->right == root )
        return k;
    ColunmHeader* choose = (ColunmHeader*)root->right, *cur=choose;
    while( cur != root )
    {
        if( choose->size > cur->size )
            choose = cur;
        cur = (ColunmHeader*)cur->right;
    }
    if( choose->size <= 0 )
        return -1;
    
    cover( choose->col );
    Node* curC = choose->down;
    while( curC != choose )
    {
        disjointSubet[k] = curC->row;
        Node* noteR = curC;
        Node* curR = curC->right;
        while( curR != noteR )
        {
            cover( curR->col );
            curR = curR->right;
        }
        int res=-1;
        if( (res = search( k+1 ))>0 )
            return res;
        curR = noteR->left;
        while( curR != noteR )
        {
            uncover( curR->col );
            curR = curR->left;
        }
        curC = curC->down;
    }
    uncover( choose->col );
    return -1;
}

int** solve_sudoku( int (*grid)[N] )
{
    //tranform a sudoku to a exact cover problem
    int** sudoku_matrix = new int*[ROWS];
    for( int i=0; i<ROWS; i++ )
        sudoku_matrix[i] = new int[5];
    for( int i=0; i<N; i++ )
        for( int j=0; j<N; j++ )
        {
            int b = (i/N_Cell)*N_Cell + (j/N_Cell);
            int r = i*N*N + j*N;
            for( int k=1; k<=N; k++ )
            {
                sudoku_matrix[r][0]=i*N+j+1;
                sudoku_matrix[r][1]=OFFSET+(i*N+k);
                sudoku_matrix[r][2]=OFFSET*2+(j*N+k);
                sudoku_matrix[r][3]=OFFSET*3+(b*N+k);
                sudoku_matrix[r][4]=-1;
                r++;
            }
        }
    ExtractCoverMatrix sudoku( ROWS, COLS, sudoku_matrix );
    //cover those have been filled
    for( int i=0; i<N; i++ )
        for( int j=0; j<N; j++ )
        {
            int k;
            if( k = grid[i][j] )
            {
                /* corresponding to
                 insert( r, i*N+j+1 );
                 insert( r, OFFSET+(i*N+k) );
                 insert( r, OFFSET*2+(j*N+k) );
                 insert( r, OFFSET*3+(b*N+k) );
                 */
                int b = (i/N_Cell)*N_Cell + j/N_Cell;
                sudoku.cover( i*N+j+1 );
                sudoku.cover( OFFSET+( i*N+k ) );
                sudoku.cover( OFFSET*2+(j*N+k) );
                sudoku.cover( OFFSET*3+(b*N+k) );
            }
        }
    int solved=0;
    int** solution = new int*[16];
    for( int i=0; i<16; i++ )
        solution[i] = new int[16];
    if( (solved=sudoku.search())>0 )
    {
        int c, r, k;
        for( int i=0; i<solved; i++ )
        {
            r = sudoku.disjointSubet[i]/(N*N);
            c = (sudoku.disjointSubet[i]/N)%N;
            k = sudoku.disjointSubet[i]%N;
            grid[r][c]=k+1;
        }
        //cout<<"here is the result: "<<endl;
        for( int i=0; i<N; i++ )
        {
            for( int j=0; j<N; j++ ){
                solution[i][j]=grid[i][j];
                //cout<<solution[i][j]<<' ';
            }
            //cout<<endl;
        }
    }
    else
        cout<<"no solution for this sudoku"<<endl;
    return solution;
}

bool verify_puzzle( int (*grid)[N] )
{
    int line[N+1]={0};
    for( int i=0; i<N; i++ )
    {
        for( int j=0; j<=N; j++ )
            line[j] = 0;
        for( int j=0; j<N; j++ )
        {
            if( line[grid[i][j]] )
            {
                cout<<"row wrong: "<<i<<endl;
                return false;
            }
            else
                line[grid[i][j]] = 1;
        }
    }
    for( int i=0; i<N; i++ )
    {
        for( int j=0; j<N+1; j++ )
            line[j] = 0;
        for( int j=0; j<N; j++ )
        {
            if( line[grid[j][i]] )
            {
                cout<<"column wrong"<<endl;
                return false;
            }
            else
                line[grid[j][i]] = 1;
        }
    }
    for( int i=0; i<N_Cell; i++ )
        for( int j=0; j<N_Cell; j++ )
        {
            for( int x=0; x<=N; x++ )
                line[x] = 0;
            for( int k=0; k<N_Cell; k++ )
                for( int t=0; t<N_Cell; t++ )
                {
                    int x = k+i*N_Cell;
                    int y = t+j*N_Cell;
                    if( line[grid[x][y]] )
                    {
                        cout<<"block wrong"<<endl;
                        return false;
                    }
                    else
                        line[grid[x][y]] = 1;
                }
        }
    return true;
}

int main()
{
    string input="-1 -1 -1  E  -1 -1 -1 -1  -1 -1  9 -1   C -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1  6 -1  0  -1  1 -1 -1  -1 -1 -1 -1  -1  C -1 -1  -1 -1 -1 -1   F -1 -1 -1  -1 -1 -1 -1  -1  B -1  9  -1 -1 -1 -1  -1 -1 -1 -1  -1  8 -1 -1  -1 -1 -1 -1   A  B -1 -1  -1 -1  4 -1  -1 -1 -1  4   E -1 -1  5  -1  F -1  2  -1 -1  1 -1  9  A -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1  6 -1  -1  0  F -1  -1 -1 -1  1   3 -1  C -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1  6  -1  A -1 -1  -1 -1  C  4  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1  B  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1  0 -1  -1 -1  A -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1  5 -1 -1  -1 -1 -1 -1   1 -1 -1 -1  -1 -1 -1 -1  -1  4 -1 -1  -1  5 -1 -1  -1 -1 -1 -1  -1 -1 -1  0  -1  1 -1 -1  -1 -1  3  D  -1 -1 -1 -1";//hexa-101
    //"-1 -1 -1 -1  -1  8 -1 -1  -1  9 -1 -1  -1 -1  C -1  -1  B  1 -1  -1 -1  5 -1  -1 -1 -1  2  -1 -1  4 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1  8  7 -1  2 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1   E -1  5 -1  D -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1  C  -1 -1 -1 -1  -1  E -1 -1  -1 -1 -1  7  -1  F  9 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1  6 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1   C -1  B -1  -1 -1 -1 -1  -1 -1 -1  6  -1  2 -1 -1  -1 -1 -1  9   C -1 -1 -1  -1 -1  0 -1  -1 -1 -1  6  -1 -1 -1 -1   4 -1 -1 -1  -1 -1 -1  F  -1 -1 -1 -1  -1  D -1 -1  -1  2 -1 -1  -1  7  2 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1 -1  6  -1 -1  D  E  -1 -1 -1  A   9  4 -1 -1  -1  C -1 -1  -1 -1 -1  0  -1 -1 -1  E   8  7 -1 -1  -1 -1  3  F  -1  9 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1  7 -1 -1  -1 -1  7 -1  -1 -1 -1 -1  -1 -1 -1 -1   6 -1 -1 -1";//hexa-100
    //"-1 -1  6  2  -1 -1  C -1   E -1 -1 -1   F -1  B -1  1  7  4 -1   8 -1 -1  9   B -1  A -1  -1  5 -1 -1  -1 -1 -1 -1   1 -1 -1 -1  -1  D  8 -1  -1 -1  7  2  -1 -1 -1  9   6 -1 -1  F  -1 -1 -1 -1  -1 -1  4 -1  -1 -1 -1 -1  -1  6 -1 -1  -1  C  D -1   E -1 -1  1  D -1  E  4   C -1 -1  5  -1  0 -1  2  -1 -1  A -1  7  2 -1  A  -1 -1 -1 -1   F  9 -1 -1  -1 -1 -1 -1  C  8  3 -1  -1 -1  9 -1  -1 -1  5  E  -1  F -1  D  E -1 -1  6  -1  3  B  D  -1 -1 -1 -1  -1 -1  1  4  -1 -1 -1 -1   E -1 -1  4   8  3 -1  1   A -1 -1 -1  4 -1 -1 -1  -1  8 -1 -1   A -1 -1  D  -1  E  2 -1  -1 -1  B -1  -1 -1 -1  A  -1  5 -1 -1  -1 -1  8  0  -1 -1  C -1  -1 -1  7  B  -1 -1 -1 -1   4 -1 -1 -1  3 -1  F  E  -1  0 -1  8  -1 -1 -1  9  -1 -1 -1  A  -1 -1 -1 -1  -1 -1 -1 -1   6  F -1 -1  -1 -1  5 -1  -1 -1  1 -1   2 -1  F -1   5 -1 -1  4   D -1 -1  E";//hexa-82
    //"-1 -1 -1 -1  -1 -1  1 -1  -1 -1  6  B  -1  5  0 -1  6 -1 -1 -1   8  9 -1 -1  -1 -1 -1 -1   D -1  4 -1  D  2  1 -1   3 -1  C  5   E -1 -1 -1   7 -1 -1 -1  -1  4  8 -1  -1  0 -1  7  -1 -1  9 -1  -1  B  A  2  -1  B -1  E  -1 -1 -1  4  -1 -1 -1 -1  -1 -1 -1 -1  F -1  D -1   5 -1 -1 -1  -1 -1 -1 -1   3  8 -1 -1  -1 -1 -1 -1  -1 -1 -1  D  -1 -1 -1  E   9 -1 -1  B  7  3 -1  6   C -1 -1 -1   2  4 -1 -1  -1 -1 -1 -1  -1  7 -1 -1   A  F  0  E   D  8  3 -1  -1  C  9 -1  -1 -1 -1 -1  -1  8  D -1  -1 -1  F  4   1 -1 -1 -1  -1 -1 -1  1  -1  B -1 -1  -1  A -1  0  -1 -1  7 -1  9  6 -1  8   7  4 -1 -1  -1 -1  5 -1  -1  0 -1 -1  -1 -1 -1  3  -1  C -1 -1  -1 -1  2 -1  -1 -1 -1  A  1 -1 -1  4  -1 -1  E -1  -1 -1 -1  7   F  3 -1 -1  -1 -1  2 -1  -1 -1  A -1   F -1  0 -1   8 -1 -1 -1  5 -1 -1 -1   9 -1 -1  8  -1 -1  E -1   C  6  B  7";//hexa-81
    //"A -1 -1  0   5 -1  9  2  -1 -1  C -1   D -1 -1 -1  -1  9  D -1  -1  B  E  6   F -1  5 -1   A  1 -1 -1  -1 -1 -1  8  -1 -1 -1 -1  -1 -1 -1  E  -1 -1  3 -1  1 -1 -1 -1   A -1  F -1  -1 -1  4 -1  -1  E -1 -1  -1 -1  8  6  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1  4 -1  -1  F -1  E  -1  6 -1 -1  -1 -1 -1 -1  -1  7  2 -1  4 -1 -1  A  -1 -1 -1  D   5 -1 -1 -1   F  9  E  B  7 -1 -1 -1   8 -1  B -1  -1 -1 -1 -1   C  5 -1 -1  -1  B -1 -1  -1 -1  3 -1  -1 -1  7 -1   0  F  8  A  E -1  9 -1  -1  8 -1 -1   B  6  0 -1   5 -1  1 -1  -1 -1  0 -1  -1  7  A -1  -1  E -1  2  -1 -1 -1 -1  -1  D  6  7   4  F -1 -1  -1 -1  A -1   B -1  9 -1  C -1  E -1  -1 -1 -1 -1  -1  4 -1  5  -1  0 -1 -1  5 -1 -1 -1   F  C  4 -1  -1  A  D  3   1 -1 -1 -1  -1 -1 -1 -1   7 -1  2 -1  -1 -1  B -1  -1  3 -1 -1  -1 -1  B  2   0 -1 -1 -1  -1  C -1  1  -1 -1  6 -1";//hexa-80
    //"-1 -1  2  A  -1 -1  D -1   F  0  B -1  -1  8 -1 -1  -1  8  3 -1   5  F -1 -1  -1 -1 -1  9  -1 -1 -1 -1  -1  F  5 -1  -1 -1 -1 -1   C -1 -1  8   D -1  1 -1  -1 -1 -1 -1   B  2 -1 -1  -1 -1  E -1   5 -1  3  7  -1 -1 -1 -1  -1  E  3  5   4 -1  D -1  -1 -1  8 -1  -1  7 -1 -1   2 -1 -1  9  -1  A  5 -1  -1  1 -1 -1  D  4 -1 -1  -1 -1 -1  B  -1  6 -1 -1  -1 -1 -1 -1  -1  B  9  2  -1 -1  0  7  -1  8 -1 -1  -1 -1  D -1  1 -1 -1 -1  -1  9 -1 -1   B -1 -1 -1   2  A  C  6  -1 -1 -1  9  -1  7 -1  0  -1 -1  8 -1  -1  D -1 -1  -1  D -1 -1  -1 -1 -1 -1  -1  2  7 -1  -1  5 -1 -1  -1 -1 -1 -1  -1 -1 -1  6   9 -1 -1 -1   B -1 -1  E  C -1 -1 -1  -1  0 -1  4  -1  7 -1 -1  -1  3  5  8  -1 -1 -1 -1  -1  B  7 -1  -1  C -1 -1  -1 -1 -1 -1  -1  0  6  3   F -1 -1  C   1 -1 -1 -1   7  2 -1  A  -1  2 -1  8   6  5 -1  D   E -1 -1 -1  -1 -1 -1  0";//hexa-50
    //"A -1 -1 -1   2  5 -1  F  -1 -1 -1 -1  -1 -1  D -1  C -1  9 -1  -1 -1  0  4  -1  D -1 -1  -1  2 -1 -1  -1 -1  D  B  -1  8 -1  6   A  4 -1  E  -1  7 -1 -1  8 -1  4 -1  -1 -1 -1  C   7  F  9 -1   6 -1 -1 -1  -1  F -1  9  -1  4 -1 -1  -1  2  0 -1  -1  6 -1  8  -1 -1  5  6  -1 -1 -1 -1  -1  7 -1  D  -1 -1 -1 -1  1 -1 -1 -1  -1 -1  B  5  -1  8  E -1  -1 -1 -1 -1  -1 -1 -1  E  -1 -1 -1 -1  -1  1 -1 -1   5 -1  F -1  3  2 -1 -1  -1 -1  4 -1  -1  B -1  F  -1 -1  0 -1  0 -1 -1 -1  -1  C -1 -1  -1 -1  8 -1  -1  F  A -1  -1  5  E -1   1 -1  D -1  -1 -1 -1 -1  -1 -1 -1 -1  -1  8  7 -1  -1 -1 -1 -1  -1  0  C -1   1 -1 -1  9  B -1 -1 -1   9  D -1 -1  -1 -1  2  5  -1 -1 -1 -1  -1 -1 -1  A   6  1 -1  0  -1 -1 -1 -1  -1  8 -1 -1  -1 -1 -1  7  -1 -1  5 -1  -1  E  A  C   2  1 -1 -1  -1 -1 -1 -1   C  A -1  2  -1 -1 -1 -1  -1  9  B  0";//hexa-21
    //"-1  2 -1 -1  B  F -1 -1  -1 -1  E -1  8 -1 -1 -1  -1 -1  C  B  8  D  A -1  F -1 -1 -1  -1 -1 -1  9  -1 -1  9 -1  -1  3  E  4  -1 -1  8  B  D -1 -1  C  -1  F -1 -1  C -1 -1 -1  -1  9  2  4  -1 -1 -1  E  6  B  8 -1  -1 -1 -1  E  -1 -1 -1 -1  4 -1  C  0  4  A -1 -1  -1  9 -1 -1  -1  E -1 -1  -1  D -1  F  E -1 -1 -1  4 -1 -1 -1  -1 -1 -1  8  -1 -1  2 -1  -1 -1  F  C  -1  B  2 -1  1 -1  7  5  -1 -1  E -1  B  C -1  E  -1 -1 -1  F  -1 -1 -1 -1  -1  7  0  6  7 -1 -1  F  9  E -1 -1  D  A -1 -1  5 -1 -1  4  -1 -1  A -1  -1 -1  1 -1  -1 -1 -1 -1  B -1 -1 -1  -1 -1 -1 -1  -1 -1 -1 -1  -1 -1  4  9  -1 -1  8  3  -1  6 -1 -1  -1  A -1  B  -1 -1 -1 -1  7  F  5 -1  -1  7 -1 -1  F  5  9  6  A  C -1  E  -1 -1 -1 -1  -1 -1 -1 -1  D -1 -1 -1   5 -1  F -1  -1 -1 -1  A  -1 -1 -1 -1  -1 -1  3 -1  7  2  D  0  E -1 -1 -1";//hexa-20

    int** array=getInputArray(input);
    cout<<"Input array: "<<endl;
    for(int i=0;i<NUM_DIGITS;i++){
        for(int j=0;j<NUM_DIGITS;j++){
            if(array[i][j]!=-1)
                cout<<intToDigitChar(array[i][j])<<"\t";
            else
                cout<<array[i][j]<<"\t";
        }
        cout<<endl;
    }
    int heart[256];
    int puzzle[N][N];
    for(int i=0;i<16;i++){
        for(int j=0;j<16;j++){
            array[i][j]+=1;
            puzzle[i][j]=array[i][j];
        }
    }
    
    clock_t begin = clock();
    int** solution=solve_sudoku( puzzle );
    clock_t end = clock();
    cout<<"One Solution: "<<endl;
    for(int i=0;i<NUM_DIGITS;i++){
        for(int j=0;j<NUM_DIGITS;j++){
            cout<<intToDigitChar(solution[i][j]-1)<<"\t";
        }
        cout<<endl;
    }
    double time = double(end - begin) / CLOCKS_PER_SEC;
    cout << "The time spent for Dance Link: " << time << " seconds." << endl;
    if( !verify_puzzle( puzzle ) )
        cout<<"wrong answer!"<<endl;
    
}

