// File: SmithWaterman.cpp
// Original Author: Connor Skennerton
// --------------------------------------------------------------------
//
// OVERVIEW:
//
// Modified smithwaterman implementation
// 
// --------------------------------------------------------------------
//  Copyright  2011 Michael Imelfort and Connor Skennerton
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.
// --------------------------------------------------------------------
//
//                        A
//                       A B
//                      A B R
//                     A B R A
//                    A B R A C
//                   A B R A C A
//                  A B R A C A D
//                 A B R A C A D A
//                A B R A C A D A B 
//               A B R A C A D A B R  
//              A B R A C A D A B R A 
//
// --------------------------------------------------------------------

//////////////////////////////////////////////
// Simple Ends-Free Smith-Waterman Algorithm
//
// You will be prompted for input sequences
// Penalties and match scores are hard-coded
//
// Program does not perform multiple tracebacks if 
// it finds several alignments with the same score
//
// By Nikhil Gopal
// Similar implementation here: https://wiki.uni-koeln.de/biologicalphysics/index.php/Implementation_of_the_Smith-Waterman_local_alignment_algorithm
//////////////////////////////////////////////

// system includes
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <cmath>
#include <sys/time.h>
#include <map>
#include <exception>

// local includes
#include "SmithWaterman.h"
#include "SeqUtils.h"
#include "LoggerSimp.h"
#include "PatternMatcher.h"
#include "Exception.h"
double findMax(double a, double b, double c, double d, int * index)
{
    //-----
    // find the biggest out of these guys!
    // if all are equal it will return a
    //
    if(b > a)
    {
        if(c > d)
        {
            if(c > b) 
            { 
                *index = 2; 
                return c; 
            }
            else 
            { 
                *index = 1; 
                return b; 
            }
        }
        else
        {
            if(d > b)
            { 
                *index = 3; 
                return d; 
            }
            else 
            { *index = 1; 
                return b; 
            }
        }
    }
    else
    {
        if(c > d)
        {
            if(c > a) 
            { 
                *index = 2; 
                return c; 
            }
            else 
            { 
                *index = 0; 
                return a; 
            }
        }
        else
        {
            if(d > a) 
            { 
                *index = 3; 
                return d; 
            }
            else 
            { 
                *index = 0; 
                return a; 
            }
        }
    }
}

stringPair smithWaterman(std::string& seqA, std::string& seqB )
{
    //-----
    // The SW you've grown to know and love
    //
    int waste1, waste2;

    return smithWaterman(seqA, seqB, &waste1, &waste2, 0, (int)seqA.length(), 0);
}

stringPair smithWaterman(std::string& seqA, std::string& seqB, int * aStartAlign, int * aEndAlign, int aStartSearch, int aSearchLen)
{
    //-----
    // Same as below but ignore similarity
    //
    return smithWaterman(seqA, seqB, aStartAlign, aEndAlign, aStartSearch, aSearchLen, 0);
}

stringPair smithWaterman(std::string& seqA, std::string& seqB, int * aStartAlign, int * aEndAlign, int aStartSearch, int aSearchLen, double similarity)
{
    //-----
    // Trickle-ier verison of the original version of the smith waterman algorithm I found in this file
    // 
    // This function is seqA centric, it will align ALL of seqB to the parts of seqA which lie INCLUSIVELY
    // between aStartSearch and aEndSearch. It will return the UNIMPUTED alignment strings for A and B respectively
    // in the stringPair variable AND it also stores the start and end indexes used to cut the seqA substring in the 
    // two int references  aStartAlign, aEndAlign 
    // 
    // Hoi!
    //
    // initialize some variables
    int length_seq_B = (int)seqB.length();
    
    // initialize matrix
    double ** matrix = new double*[aSearchLen+1];
    int ** I_i = new int*[aSearchLen+1];
    int ** I_j = new int*[aSearchLen+1];
    
    for(int i=0;i<=aSearchLen;i++)
    {
        double * tmp_mat = new double[length_seq_B+1];
        int * tmp_i = new int[length_seq_B+1];
        int * tmp_j = new int[length_seq_B+1];

        matrix[i] = tmp_mat;
        I_i[i] = tmp_i;
        I_j[i] = tmp_j;
        
        for(int j=0;j<=length_seq_B;j++)
        {
            matrix[i][j]=0;
        }
    }
    
      
    //start populating matrix
    double matrix_max = -1;
    int i_max = 0, j_max = 0;
    for (int i=1;i<=aSearchLen;i++)
    {
        for(int j=1;j<=length_seq_B;j++)
        {
            int index = -1;
            matrix[i][j] = findMax(   matrix[i-1][j-1] + SW_SIM_SCORE(seqA[i-1 + aStartSearch],seqB[j-1]), \
                                      matrix[i-1][j] + SW_GAP, \
                                      matrix[i][j-1] + SW_GAP, \
                                      0, \
                                      &index
                                  );

            if(matrix[i][j] > matrix_max)
            {
                matrix_max = matrix[i][j];
                i_max = i;
                j_max = j;
            }
            
            switch(index)
            {
                case 0:
                    I_i[i][j] = i-1;
                    I_j[i][j] = j-1;
                    break;
                case 1:
                    I_i[i][j] = i-1;
                    I_j[i][j] = j;
                    break;
                case 2:
                    I_i[i][j] = i;
                    I_j[i][j] = j-1;
                    break;
                case 3:
                    I_i[i][j] = i;
                    I_j[i][j] = j;
                    break;
                default:
                    throw crispr::exception( __FILE__, __LINE__, __PRETTY_FUNCTION__,"Index never set in findMax");
            }
        }
    }
    
    int current_i = i_max;
    int current_j = j_max;
    int next_i = I_i[current_i][current_j];
    int next_j = I_j[current_i][current_j];
    
    while((next_j!=0) && (next_i!=0) && ((current_i!=next_i) || (current_j!=next_j)))
    {
        current_i = next_i;
        current_j = next_j;
        next_i = I_i[current_i][current_j];
        next_j = I_j[current_i][current_j];
    }
    
    // record the start and end of seqA
    current_i--;
    current_j--;
    if(0 > current_j) { current_j = 0; } 
    if(0 > current_i) { current_i = 0; } 
    
    *aStartAlign = current_i+ aStartSearch;
    *aEndAlign = (*aStartAlign) + i_max - current_i - 1;
    
    // we don't need these guys anymore
    for(int i=0;i<=aSearchLen;i++)
    {
        delete [] matrix[i];
        delete [] I_i[i];
        delete [] I_j[i];
    }
    delete [] matrix;
    delete [] I_i;
    delete [] I_j;
    
    // calculate similarity and return substrings is need be
    std::string a_ret;
    std::string b_ret;
    try {
        a_ret = seqA.substr(current_i+ aStartSearch, i_max - current_i + aStartSearch);
    } catch (std::exception& e) {
        throw (crispr::exception( __FILE__, __LINE__, __PRETTY_FUNCTION__, e.what()));
    }
    try {
        b_ret = seqB.substr(current_j, j_max - current_j);
    } catch (std::exception& e) {
        throw (crispr::exception( __FILE__, __LINE__, __PRETTY_FUNCTION__, e.what()));
    }
    if(0 != similarity)
    {
        
        double similarity_ld = 1.0 - (PatternMatcher::levenstheinDistance(a_ret, b_ret) /(double)a_ret.length()); 
        if(similarity_ld >= similarity)
        {
            return std::pair<std::string, std::string>(a_ret, b_ret); 
        }
        else
        {
            // no go joe
            *aStartAlign = 0;
            *aEndAlign = 0;
            return std::pair<std::string, std::string>("","");
        }
    }
    else
    {
        // just return the substrings
        return std::pair<std::string, std::string>(a_ret, b_ret);
    }
}







