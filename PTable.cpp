/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PTable.cpp
 * Author: pwof
 * 
 * Created on May 31, 2018, 3:44 PM
 */

#include <map>
#include <cstdio>
#include <algorithm>
#include <cstring>
#include <vector>
#include "defines.h"

#include "PTable.h"

PTable::PTable() {
    maxNi = 0;
    minDiff = 0;
    maxDiff = 0;
}

PTable::PTable(const PTable& orig) {
}

PTable::~PTable() {
}

// PTable in file index i runs from 0 to MaxNi
void PTable::SetmaxNi(){
    maxNi = Data.size() - 1;
}

void PTable::SetminDiff(){
    int diff = 2140000000;
    int i;
    PTableRow::iterator pos;
        
    for(i = 0; i < (int) Data.size(); i++){
        for (pos=Data[i].begin();pos!=Data[i].end();++pos){
            diff = std::min(diff, pos->first - i);
        }
    }
    minDiff = diff;
}

void PTable::SetmaxDiff(){
    int diff = -2140000000;
    int i;
    PTableRow::iterator pos;
        
    for(i = 0; i < (int) Data.size(); i++){
        for (pos=Data[i].begin();pos!=Data[i].end();++pos){
            diff = std::max(diff, pos->first - i);
        }
    }
    maxDiff = diff;
}

/**
 * Reads information on ptable from file
 * File with ptable:
 *      free format with separator ";" 
 *      names on first line
 *      each next line starts with three values i, j and pij
 * @param FileName
 * @return true if no error
 */    
 bool PTable::ReadFromFile(const char* FileName){
    char line[MAXRECORDLENGTH];
    int i=0, j=0, i0=0;
    double bound;
    PTableRow row;
    PTableRow::iterator pos;
    FILE* ptable_in;
     
    ptable_in = fopen(FileName,"r");
    if (ptable_in == NULL){
        return false;
    }
    
    fgets((char *)line, MAXRECORDLENGTH, ptable_in); // Disregard first line: contains only names
    fgets((char *)line, MAXRECORDLENGTH, ptable_in);

    row = PTableRow();

    i0 = atoi(strtok(line,";"));
    j = atoi(strtok(NULL,";"));
    row[j] = atof(strtok(NULL,";"));

    while (fgets((char *)line, MAXRECORDLENGTH, ptable_in) != NULL){
        i = atoi(strtok(line,";"));
        if (i != i0){
            bound = 0;
            for (pos=row.begin();pos!=row.end();++pos){
                bound += pos->second;
                row[pos->first] = bound;
            }
            Data.push_back(row);
            row = PTableRow();
            i0=i;
        }
        j = atoi(strtok(NULL,";"));
        row[j] = atof(strtok(NULL,";"));
    }

    bound = 0;
    for (pos=row.begin();pos!=row.end();++pos){
        bound += pos->second;
        row[pos->first] = bound;
    }
    Data.push_back(row);
    SetmaxNi();
    SetminDiff();
    SetmaxDiff();
    fclose(ptable_in);
    return true;
}

 void PTable::WriteToFile(){
    int i;
    PTableRow::iterator pos;
    
    FILE *pout = fopen("ptable_read.txt","w");
    printf("Data.size() = %d\n",Data.size());
    for(i = 0; i < (int) Data.size(); i++){
        fprintf(pout,"row %d:",i);
        for (pos=Data[i].begin();pos!=Data[i].end();++pos){
            fprintf(pout," (%d, %17.15lf)", pos->first, pos->second);
        }
        fprintf(pout,"\n");
    }
    fclose(pout);
 }