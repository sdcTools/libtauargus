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

#include "PTableCont.h"

PTableCont::PTableCont() {
}

PTableCont::PTableCont(const PTableCont& orig) {
}

PTableCont::~PTableCont() {
}

/**
 * Reads information on ptable from file
 * File with ptable:
 *      free format with separator ";" 
 *      names on first line
 *      each next line contains (in this order)
 *      (int) i (double) j (double) pij (double) v (double) pij_ub (string) type
 * @param FileName
 * @return true if no error
 */    
 bool PTableCont::ReadFromFile(std::string FileName){
    char line[MAXRECORDLENGTH];
    int  i0, i;
    double j, p_ub, pij;
    std::string type;
    PTableDRow row;
    FILE* ptable_in;
     
    ptable_in = fopen(FileName.c_str(),"r");
    if (ptable_in == NULL){
        return false; // file does not exist
    }
    
    if (!fgets((char *)line, MAXRECORDLENGTH, ptable_in)) return false; // Disregard first line: contains only column names
    if (!fgets((char *)line, MAXRECORDLENGTH, ptable_in)) return false;

    //row = PTableDRow();
    row.clear();
    
    // read first line with real data
    i0 = atoi(strtok(line,";"));    // i
    j = atof(strtok(NULL,";"));     // j
    pij = atof(strtok(NULL,";"));   // p
    atof(strtok(NULL,";"));         // diff    // not used    
    p_ub = atof(strtok(NULL,";"));  // p_kum_o
    type = strtok(NULL,"\n");       // type: even, odd or all
    row[j][0] = pij;
    row[j][1] = p_ub;
    
    while (fgets((char *)line, MAXRECORDLENGTH, ptable_in)){
        i = atoi(strtok(line,";"));  // i
        if (i != i0){ // New entry for lookup value, so start new row
            Data[type][i0] = row;
            row.clear();
            i0 = i;
        }
        j = atof(strtok(NULL,";"));    // j
        pij = atof(strtok(NULL,";"));  // p
        atof(strtok(NULL,";"));        // diff    // not used
        p_ub = atof(strtok(NULL,";")); // p_kum_o
        type = strtok(NULL,"\n");      // type: even, odd or all
        row[j][0] = pij;
        row[j][1] = p_ub;
    }
    Data[type][i] = row;
        
    fclose(ptable_in);
    return true;
 }        

// For debugging purposes 
/*void PTableCont::Write(std::string type){
    PTableDRow::iterator pos;
    std::map<int,PTableDRow>::iterator rowpos;
    
    printf("Data[%s].size() = %lld\n", type.c_str(),Data[type].size());
    
    for (rowpos=Data[type].begin();rowpos!=Data[type].end();++rowpos){
        printf("row %d:",rowpos->first);
        for (pos=rowpos->second.begin();pos!=rowpos->second.end();++pos){
            printf(" (%3.1lf, %10.8lf, %10.8lf)", pos->first, pos->second[0], pos->second[1]);
        }
        printf("\n");
    }
}*/