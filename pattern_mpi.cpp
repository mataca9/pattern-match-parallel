// C++ program to implement wildcard
// pattern matching algorithm
#include "stdc++.h"
#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include "mpi.h"
using namespace std;

#define LINECOUNT 200
#define LINESIZE 200

char lineList[LINECOUNT][LINESIZE];
char result[LINECOUNT][LINESIZE];
char patternList[7][15] = {
    "*iaculis*",
    "*vulputate*",
    "*efficitur*",
    "*massa*",
    "*eleifend*",
    "*tristique*",
    "*tempor*"
};

// Function that matches input str with
// given wildcard pattern
bool strmatch(char str[], char pattern[], int n, int m)
{
    // empty pattern can only match with
    // empty string
    if (m == 0)
        return (n == 0);

    // lookup table for storing results of
    // subproblems
    bool lookup[n + 1][m + 1];

    // initailze lookup table to false
    memset(lookup, false, sizeof(lookup));

    // empty pattern can match with empty string
    lookup[0][0] = true;

    // Only '*' can match with empty string
    for (int j = 1; j <= m; j++)
        if (pattern[j - 1] == '*')
            lookup[0][j] = lookup[0][j - 1];

    // fill the table in bottom-up fashion
    for (int i = 1; i <= n; i++)
    {
        for (int j = 1; j <= m; j++)
        {
            // Two cases if we see a '*'
            // a) We ignore ‘*’ character and move
            //    to next  character in the pattern,
            //     i.e., ‘*’ indicates an empty sequence.
            // b) '*' character matches with ith
            //     character in input
            if (pattern[j - 1] == '*')
                lookup[i][j] = lookup[i][j - 1] ||
                               lookup[i - 1][j];

            // Current characters are considered as
            // matching in two cases
            // (a) current character of pattern is '?'
            // (b) characters actually match
            else if (pattern[j - 1] == '?' || str[i - 1] == pattern[j - 1])
                lookup[i][j] = lookup[i - 1][j - 1];

            // If characters don't match
            else
                lookup[i][j] = false;
        }
    }

    return lookup[n][m];
}

int main(int argc, char** argv)
{

    // Mpi vars
    int rank;
    int size;
    int source;
    int dest;
    int tag = 50;

    MPI_Status status; /* Status de retorno */
    
    MPI_Init (&argc , &argv);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Program vars
    int i, j;
    string line;
    int countLines, countPatterns;   
    char *pattern;
    std::ofstream output;
    ifstream file("file.txt");
    int chunkSize;

    countPatterns = sizeof(patternList) / sizeof(patternList[0]);

    // Set chunkSize
    chunkSize = (LINECOUNT / size);

    if (rank == 0) {

        if (file.is_open())
        {
            // Populates lineList from file
            i = 0;
            while (getline(file, line))
            {
                strcpy(lineList[i], line.c_str());
                i++;
            }
            countLines = i;
            file.close();

            // Send chucks
            for(int i=1; i<size; i++) {
                int begin = i * chunkSize;
                MPI_Send(&lineList[begin], chunkSize*LINESIZE, MPI_CHAR, i, tag, MPI_COMM_WORLD);
            }

            // Process local chunk
            for(i = 0; i < countPatterns; i++){
                for(j = 0; j < chunkSize; j++){
                    pattern = new char[strlen(patternList[i]) + 1];
                    strcpy(pattern, patternList[i]);
                    if (strmatch(lineList[j], pattern, strlen(lineList[j]), strlen(pattern))) {

                        // Concatenate other results for same line
                        stringstream result_stream;
                        result_stream << result[j] << "\nFound " << pattern;
                        string result_string = result_stream.str();
                        strcpy(result[j], result_string.c_str());
                    }
                }
            }

            // Receive result from other processes
            for(i = 1; i < size; i++) {
                int begin = i * chunkSize;
                MPI_Recv(&result[begin], chunkSize*LINESIZE, MPI_CHAR, i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            }
        } else {
            cout << "Unable to open file" << endl;
        }

    }
    else {
        // Receive a chunk
        MPI_Recv (&lineList, chunkSize*LINESIZE, MPI_CHAR, 0, tag, MPI_COMM_WORLD, &status);

        // Process the received chunk
        for(i = 0; i < countPatterns; i++){
            for(j = 0; j < chunkSize; j++){
                pattern = new char[strlen(patternList[i]) + 1];
                strcpy(pattern, patternList[i]);
                if (strmatch(lineList[j], pattern, strlen(lineList[j]), strlen(pattern))) {

                    // Concatenate other results for same line
                    stringstream result_stream;
                    result_stream << result[j] << "\nFound " << pattern;
                    string result_string = result_stream.str();
                    strcpy(result[j], result_string.c_str());
                }
            }
        }

        // Send processed chunk
		MPI_Send(result, chunkSize*LINESIZE, MPI_CHAR, 0, tag, MPI_COMM_WORLD);
    }

    if (rank == 0) {

        // Clear output
        remove("output.txt");
        output.open("output.txt", std::ios_base::app);

        // Throw result on output file
        for (i = 0; i < LINECOUNT; i++) {
            if (strlen(result[i]) > 0){
                output << "Line " << i << " : " << result[i] << "\n" << endl;
            }
        }
    }    

    MPI_Finalize();
    return 0;
}
