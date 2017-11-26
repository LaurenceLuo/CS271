/*  
 *  CS271 Project Sudoku
 *  16*16 Version
 */
#include <stdio.h>
#include <cstdlib>
#include <math.h>
#include <time.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
#include <cstdlib>
#include <utility>
#include <random>
#include <queue>
#include <ctime>


using namespace std;

#define NUM_CELLS 256
#define NUM_DIGITS 16
#define NUM_SQRS 4

typedef enum {Row = 0, Column, Sqr} Type;

int digitCharToInt(char c){
    if(c >= '0' && c <= '9')
        return (int)c - (int)'0';
    if(c >= 'A' && c <= 'F')
        return (int)c - (int)'A' + 10;
    return -1;
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
            token = s.substr(0, pos);
            array[i][j]=digitCharToInt(token.at(0));
            s.erase(0, pos + 1);
        }
    }
    return array;
}

bool verify(int* count, int** array){
    int digit, i;
    for(int row = 0; row < NUM_DIGITS; ++row)
    {
        // Examine each cell in the row.
        for(i = 0; i < NUM_DIGITS; ++i)
            // Count the occurance of this digit.
            if(array[row][i] != -1)
                // If it's occured more than once, the board is invalid.
                if(++count[array[row][i]] > 1){
                    //cout<<"row wrong"<<endl;
                    return false;
                }
        
        // Clear the count array.
        for(i = 0; i < NUM_DIGITS; ++i)
            count[i] = 0;
    }
    
    for(int column = 0; column < NUM_DIGITS; ++column)
    {
        // Examine each cell in the column.
        for(i = 0; i < NUM_DIGITS; ++i)
            // Count the occurance of this digit.
            if(array[i][column] != -1)
                // If it's occured more than once, the board is invalid.
                if(++count[array[i][column]] > 1){
                    //cout<<"column wrong"<<endl;
                    return false;
                }
        
        // Clear the count array.
        for(i = 0; i < NUM_DIGITS; ++i)
            count[i] = 0;
    }
    
    for(int sqr = 0; sqr < NUM_DIGITS; ++sqr)
    {
        // Examine each cell in the square.
        for(i = 0; i < NUM_DIGITS; ++i)
            // Count the occurance of this digit.
            if(array[(sqr/4)*4+i/4][(sqr%4)*4+i%4] != -1)
                // If it's occured more than once, the board is invalid.
                if(++count[array[(sqr/4)*4+i/4][(sqr%4)*4+i%4]] > 1){
                    //cout<<"sqr wrong"<<endl;
                    return false;
                }
        
        // Clear the count array.
        for(i = 0; i < NUM_DIGITS; ++i)
            count[i] = 0;
    }
    return true;

}

bool solved(int** array){
    for(int row=0;row<NUM_DIGITS;row++){
        for(int col=0;col<NUM_DIGITS;col++){
            if(array[row][col]==-1)
                return false;
        }
    }
    return true;
}

bool FindUnassignedLocation(int** array, int &row, int &col)
{
    for (row = 0; row < NUM_DIGITS; row++)
        for (col = 0; col < NUM_DIGITS; col++)
            if (array[row][col] == -1)
                return true;
    return false;
}

bool UsedInRow(int** array, int row, int num)
{
    for (int col = 0; col < NUM_DIGITS; col++)
        if (array[row][col] == num)
            return true;
    return false;
}

bool UsedInCol(int** array, int col, int num)
{
    for (int row = 0; row < NUM_DIGITS; row++)
        if (array[row][col] == num)
            return true;
    return false;
}

bool UsedInBox(int** array, int boxStartRow, int boxStartCol, int num)
{
    for (int row = 0; row < 4; row++)
        for (int col = 0; col < 4; col++)
            if (array[row+boxStartRow][col+boxStartCol] == num)
                return true;
    return false;
}

bool isSafe(int** array, int row, int col, int num)
{
    /* Check if 'num' is not already placed in current row,
     current column and current 3x3 box */
    return !UsedInRow(array, row, num) &&
    !UsedInCol(array, col, num) &&
    !UsedInBox(array, row - row%4 , col - col%4, num);
}

bool solve(int** array){
    int row,col;
    if (!FindUnassignedLocation(array, row, col))
        return true; // success!
    for (int num = 0; num < NUM_DIGITS; num++)
    {
        // if looks promising
        if (isSafe(array, row, col, num))
        {
            // make tentative assignment
            array[row][col] = num;
            
            // return, if success, yay!
            if (solve(array))
                return true;
            
            // failure, unmake & try again
            array[row][col] = -1;
        }
    }
    return false;
}

int main(){
    string input="D C -1 -1 -1 7 2 0 1 E B -1 -1 -1 8 F -1 9 -1 6 B -1 5 -1 -1 C -1 F 2 -1 A -1 -1 0 -1 B 8 6 F -1 -1 3 7 A C -1 4 -1 -1 E 4 -1 3 -1 A -1 -1 6 -1 8 -1 1 B -1 F 6 -1 -1 -1 0 -1 -1 -1 -1 1 -1 -1 -1 D 9 1 3 -1 0 -1 -1 4 D 7 A -1 -1 8 -1 5 B -1 -1 -1 -1 5 -1 9 7 D B -1 C -1 -1 -1 -1 B -1 5 -1 -1 E -1 -1 -1 -1 2 -1 -1 4 -1 1 E -1 3 -1 -1 F -1 -1 -1 -1 6 -1 -1 2 -1 4 -1 -1 -1 -1 4 -1 E 8 5 2 -1 D -1 -1 -1 -1 5 2 -1 9 -1 -1 1 6 C F -1 -1 D -1 0 8 4 1 -1 -1 -1 5 -1 -1 -1 -1 A -1 -1 -1 F 6 -1 A 6 -1 7 -1 3 -1 -1 9 -1 1 -1 5 2 -1 -1 7 -1 3 1 8 0 -1 -1 D C B 6 -1 E -1 -1 5 -1 E 2 -1 D -1 -1 7 -1 6 1 -1 C -1 C B -1 -1 -1 A 6 F 3 5 0 -1 -1 -1 9 7";
    
    string solution1="D C A 5 9 7 2 0 1 E B 4 3 6 8 F 3 9 8 6 B 4 5 1 0 C D F 2 7 A E 2 0 1 B 8 6 F E 9 3 7 A C D 4 5 7 E 4 F 3 D A C 2 6 5 8 9 1 B 0 F 6 7 2 C 0 B 3 8 4 1 5 E A D 9 1 3 9 0 6 2 4 D 7 A F E 8 C 5 B A 4 E 8 5 1 9 7 D B 3 C F 0 6 2 B D 5 C F E 8 A 6 0 2 9 7 4 3 1 E 8 3 A D F C 9 B 1 6 0 5 2 7 4 6 F 0 7 4 B E 8 5 2 9 D A 3 1 C 5 2 B 9 A 3 1 6 C F 4 7 D E 0 8 4 1 C D 0 5 7 2 E 8 A 3 B 9 F 6 8 A 6 4 7 C 3 B F 9 E 1 0 5 2 D 9 7 2 3 1 8 0 5 4 D C B 6 F E A 0 5 F E 2 9 D 4 A 7 8 6 1 B C 3 C B D 1 E A 6 F 3 5 0 2 4 8 9 7";
    
    
    int** array=getInputArray(input);
    int* count=new int[NUM_DIGITS];
    for(int i=0;i<NUM_DIGITS;i++)
        count[i]=0;
    cout<<"Input array: "<<endl;
    for(int i=0;i<NUM_DIGITS;i++){
        for(int j=0;j<NUM_DIGITS;j++){
            cout<<array[i][j]<<"\t";
        }
        cout<<endl;
    }
    
    solve(array);
    if(!solved(array)){
        cout<<"No solution exists"<<endl;
        return -1;
    }
    
    cout<<"One solution: "<<endl;
    for(int i=0;i<NUM_DIGITS;i++){
        for(int j=0;j<NUM_DIGITS;j++){
            cout<<array[i][j]<<"\t";
        }
        cout<<endl;
    }

    return 0;
}
