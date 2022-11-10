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
#include <string>
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
            //diff = std::min(diff, pos->first - i);
            diff = std::min(diff, pos->j - i);
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
            //diff = std::max(diff, pos->first - i);
            diff = std::max(diff, pos->j - i);
        }
    }
    maxDiff = diff;
}

/**
 * Reads information on ptable from file
 * File with ptable:
 *      free format with separator ";" 
 *      names on first line
 *      each next line contains (in this order)
 *      (int) i (int) j (double) pij (double) v (double) pij_ub
 * @param FileName
 * @return true if no error
 */    
 bool PTable::ReadFromFreqFile(std::string FileName){
    char line[MAXRECORDLENGTH];
    int i=0, i0=0;
    PTableRow row;
    FILE* ptable_in;
    P_ENTRY pentry;
     
    ptable_in = fopen(FileName.c_str(),"r");
    if (ptable_in == NULL){
        return false; // file does not exist
    }
    
    if (!fgets((char *)line, MAXRECORDLENGTH, ptable_in)) return false; // Disregard first line: contains only column names, not used
    if (!fgets((char *)line, MAXRECORDLENGTH, ptable_in)) return false; // read first line with real ptable data

    //row = PTableRow();
    row.clear();
    
    // parse first line
    i0 = atoi(strtok(line,";"));            // i
    pentry.j = atof(strtok(NULL,";"));      // j
    atof(strtok(NULL,";"));                 // p       // not used
    atof(strtok(NULL,";"));                 // diff    // not used    
    pentry.p_ub = atof(strtok(NULL,";"));   // p_ij_ub
    // save as first entry in row
    row.push_back(pentry);
            
    while (fgets((char*)line, MAXRECORDLENGTH, ptable_in)){
        // read new line
        i = atoi(strtok(line,";"));     // i
        if (i != i0){                   // if new i, save as row in Data
            Data.push_back(row);
            row.clear();                // start new row
            i0=i;
        }
        // parse new line
        pentry.j = atoi(strtok(NULL,";"));      // j
        atof(strtok(NULL,";"));                 // p       // not used
        atof(strtok(NULL,";"));                 // diff    // not used    
        pentry.p_ub = atof(strtok(NULL,";"));   // p_ij_ub
        // save as new entry in row
        row.push_back(pentry);
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
    printf("Data.size() = %u\n",Data.size());
    for(i = 0; i < (int) Data.size(); i++){
        fprintf(pout,"row %d:",i);
        for (pos=Data[i].begin();pos!=Data[i].end();++pos){
            fprintf(pout," (%d, %17.15lf)", pos->j, pos->p_ub);
        }
        fprintf(pout,"\n");
    }
    fclose(pout);
 }

  void PTable::Write(){
    int i;
    PTableRow::iterator pos;
    
    printf("Data.size() = %zu\n",Data.size());
    for(i = 0; i < (int) Data.size(); i++){
        printf("row %d:",i);
        for (pos=Data[i].begin();pos!=Data[i].end();++pos){
            printf(" (%d, %17.15lf)", pos->j, pos->p_ub);
        }
        printf("\n");
    }
 }
