// C++ program to implement wildcard
// pattern matching algorithm
#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <string>
#include <omp.h> 
#include <iomanip>
using namespace std;

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

int main()
{
    vector<string> patternList, lineList;
    int th_id, nthreads, i, j;
    string line;
    string pattern;
    std::ofstream output;
    ifstream file("file.txt");
    double starttime, stoptime;
    
    // Set patterns
    patternList.push_back("*olar*");
    patternList.push_back("*c*rto*");

    // Clear output
    remove("output.txt");
    output.open("output.txt", std::ios_base::app);

    // starttime
    starttime = omp_get_wtime();

    // omp_set_num_threads(4);

    if (file.is_open())
    {
        // Populates lineList from file
        while (getline(file, line))
        {
            lineList.push_back(line);
        }
        file.close();

        #pragma omp parallel private(i, j, th_id, nthreads)
        {
            nthreads = omp_get_num_threads();
            for(i = 0; i < lineList.size(); i++){
                line = lineList.at(i);
                #pragma omp for
                    for(j = 0; j < patternList.size(); j++){
                        char *pattern = new char[patternList.at(j).length() + 1];
                        strcpy(pattern, patternList.at(j).c_str());
                        if (strmatch((char*)line.c_str(), pattern, line.length(), strlen(pattern))){
                            th_id = omp_get_thread_num();
                            output << "Match '" << pattern << "' in line " << i << " Thd: " << th_id << " of " << nthreads << endl;
                        }
                    }
            }
        }

        stoptime = omp_get_wtime();

        cout << "Tempo de execução:"
            << std::fixed << std::setw(11)
            << std::setprecision( 6 )
            << stoptime-starttime
            << " segundos." << endl;
    }
    else {
        cout << "Unable to open file" << endl;
    }

    return 0;
}
