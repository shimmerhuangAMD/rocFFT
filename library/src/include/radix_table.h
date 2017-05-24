/*******************************************************************************
 * Copyright (C) 2016 Advanced Micro Devices, Inc. All rights reserved.
 ******************************************************************************/

#pragma once
#if !defined( RADIX_TABLE_H )
#define RADIX_TABLE_H

#include <iostream>
#include <map>
#include <assert.h>

       /* radix table: tell the FFT algorithms; required by twiddle, passes, and kernel*/

        struct SpecRecord
        {
            size_t length;
            size_t workGroupSize;
            size_t numTransforms;
            size_t numPasses;
            size_t radices[12]; // Setting upper limit of number of passes to 12
        };

        SpecRecord specRecord[] = {
                //  Length, WorkGroupSize (thread block size), NumTransforms , NumPasses,  Radices
                //  vector<size_t> radices; NUmPasses = radices.size();
                //  Tuned for single precision on OpenCL stack; double precsion use the same table as single
                { 4096,           256,             1,         3,     16, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0 },//pow2
                { 2048,           256,             1,         4,     8, 8, 8, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
                { 1024,           128,             1,         4,     8, 8, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
                { 512,            64,              1,         3,     8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                { 256,            64,              1,         4,     4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
                { 128,            64,              4,         3,     8, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {  64,            64,              4,         3,     4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {  32,            64,             16,         2,     8, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {  16,            64,             16,         2,     4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {   8,            64,             32,         2,     4, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {   4,            64,             32,         2,     2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {   2,            64,             64,         1,     2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
/*
                {2187,           243,              1,         7,     3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0 },//pow3
                { 729,           243,              1,         6,     3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0 },
                { 243,           243,              3,         5,     3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0 },
                {  81,           243,              9,         4,     3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0 },
                {  27,           243,             27,         3,     3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {   9,           243,             81,         2,     3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {   3,           243,            243,         1,     3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {3125,           125,              1,         5,     5, 5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0 },//pow5
                { 625,           125,              1,         4,     5, 5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0 },
                { 125,           125,              5,         3,     5, 5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {  25,           125,             25,         2,     5, 5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {   5,           125,            125,         1,     5, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {2401,            49,              1,         4,     7, 7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0 },//pow7
                { 343,            49,              1,         3,     7, 7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {  40,            49,              7,         2,     7, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
                {   7,            49,             49,         1,     7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
*/
        };



    #define MAX_WORK_GROUP_SIZE 1024

    /* =====================================================================
       Calculate grid and thread blocks (work groups, work items)
           in kernel generator if no predefined table
       input: MAX_WORK_GROUP_SIZE, length
       output: workGroupSize, numTrans,
    =================================================================== */


    inline void DetermineSizes(const size_t &length, size_t &workGroupSize, size_t &numTrans)
    {
        assert(MAX_WORK_GROUP_SIZE >= 64);

        if(length == 1) // special case
        {
            workGroupSize = 64;
            numTrans = 64;
            return;
        }

        size_t baseRadix[] = {13,11,7,5,3,2}; // list only supported primes
        size_t baseRadixSize = sizeof(baseRadix)/sizeof(baseRadix[0]);

        size_t l = length;
        std::map<size_t, size_t> primeFactorsExpanded;
        for(size_t r=0; r<baseRadixSize; r++)
        {
            size_t rad = baseRadix[r];
            size_t e = 1;
            while(!(l%rad))
            {
                l /= rad;
                e *= rad;
            }

            primeFactorsExpanded[rad] = e;
        }

        assert(l == 1); // Makes sure the number is composed of only supported primes

        if        (primeFactorsExpanded[2] == length)    // Length is pure power of 2
        {
            if        (length >= 1024)    { workGroupSize = (MAX_WORK_GROUP_SIZE >= 256) ? 256 : MAX_WORK_GROUP_SIZE; numTrans = 1; }
            else if (length == 512)        { workGroupSize = 64; numTrans = 1; }
            else if    (length >= 16)        { workGroupSize = 64;  numTrans = 256/length; }
            else                        { workGroupSize = 64;  numTrans = 128/length; }
        }
        else if    (primeFactorsExpanded[3] == length) // Length is pure power of 3
        {
            workGroupSize = (MAX_WORK_GROUP_SIZE >= 256) ? 243 : 27;
            numTrans = length >= 3*workGroupSize ? 1 : (3*workGroupSize)/length;
        }
        else if    (primeFactorsExpanded[5] == length) // Length is pure power of 5
        {
            workGroupSize = (MAX_WORK_GROUP_SIZE >= 128) ? 125 : 25;
            numTrans = length >= 5*workGroupSize ? 1 : (5*workGroupSize)/length;
        }
        else if    (primeFactorsExpanded[7] == length) // Length is pure power of 7
        {
            workGroupSize = 49;
            numTrans = length >= 7*workGroupSize ? 1 : (7*workGroupSize)/length;
        }
        else if (primeFactorsExpanded[11] == length) // Length is pure power of 11
        {
            workGroupSize = 121;
            numTrans = length >= 11 * workGroupSize ? 1 : (11 * workGroupSize) / length;
        }
        else if (primeFactorsExpanded[13] == length) // Length is pure power of 13
        {
            workGroupSize = 169;
            numTrans = length >= 13 * workGroupSize ? 1 : (13 * workGroupSize) / length;
        }
        else
        {
            size_t leastNumPerWI = 1; // least number of elements in one work item
            size_t maxWorkGroupSize = MAX_WORK_GROUP_SIZE; // maximum work group size desired


            if        (primeFactorsExpanded[2] * primeFactorsExpanded[3] == length) {
                if (length % 12 == 0) {
                    leastNumPerWI = 12; maxWorkGroupSize = 128;
                } else {
                    leastNumPerWI =  6; maxWorkGroupSize = 256;
                }
            } else if (primeFactorsExpanded[2] * primeFactorsExpanded[5] == length) {
                if (length % 20 == 0) {
                    leastNumPerWI = 20; maxWorkGroupSize = 64;
                } else {
                    leastNumPerWI = 10; maxWorkGroupSize = 128;
                }
            } else if (primeFactorsExpanded[2] * primeFactorsExpanded[7] == length) {
                    leastNumPerWI = 14; maxWorkGroupSize = 64;
            } else if (primeFactorsExpanded[3] * primeFactorsExpanded[5] == length) {
                    leastNumPerWI = 15; maxWorkGroupSize = 128;
            } else if (primeFactorsExpanded[3] * primeFactorsExpanded[7] == length) {
                    leastNumPerWI = 21; maxWorkGroupSize = 128;
            } else if (primeFactorsExpanded[5] * primeFactorsExpanded[7] == length) {
                    leastNumPerWI = 35; maxWorkGroupSize = 64;
            } else if (primeFactorsExpanded[2] * primeFactorsExpanded[3] * primeFactorsExpanded[5] == length) {
                    leastNumPerWI = 30; maxWorkGroupSize = 64;
            } else if (primeFactorsExpanded[2] * primeFactorsExpanded[3] * primeFactorsExpanded[7] == length) {
                    leastNumPerWI = 42; maxWorkGroupSize = 60;
            } else if (primeFactorsExpanded[2] * primeFactorsExpanded[5] * primeFactorsExpanded[7] == length) {
                    leastNumPerWI = 70; maxWorkGroupSize = 36;
            } else if (primeFactorsExpanded[3] * primeFactorsExpanded[5] * primeFactorsExpanded[7] == length) {
                    leastNumPerWI =105; maxWorkGroupSize = 24;
            }
            else if (primeFactorsExpanded[2] * primeFactorsExpanded[11] == length) {
                leastNumPerWI = 22; maxWorkGroupSize = 128;
            }
            else if (primeFactorsExpanded[2] * primeFactorsExpanded[13] == length) {
                leastNumPerWI = 26; maxWorkGroupSize = 128;
            }
            else {
                    leastNumPerWI =210; maxWorkGroupSize = 12;
            }


            if (maxWorkGroupSize > MAX_WORK_GROUP_SIZE)
                maxWorkGroupSize = MAX_WORK_GROUP_SIZE;
            assert (leastNumPerWI > 0 && length % leastNumPerWI == 0);

            for (size_t lnpi = leastNumPerWI; lnpi <= length; lnpi += leastNumPerWI) {
                if (length % lnpi != 0) continue;

                if (length / lnpi <= MAX_WORK_GROUP_SIZE) {
                    leastNumPerWI = lnpi;
                    break;
                }
            }

            numTrans = maxWorkGroupSize / (length / leastNumPerWI);
            numTrans = numTrans < 1 ? 1 : numTrans;
            workGroupSize = numTrans * (length / leastNumPerWI);
        }

        assert(workGroupSize <= MAX_WORK_GROUP_SIZE);
    }

std::vector<size_t> GetRadices(size_t length);
void GetWGSAndNT(size_t length, size_t &workGroupSize, size_t &numTransforms);


#endif //defined( RADIX_TABLE_H )
