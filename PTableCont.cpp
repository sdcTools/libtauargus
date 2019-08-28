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
 *      each next line starts with three values i, j and pij
 * @param FileName
 * @return true if no error
 */    
 bool PTableCont::ReadFromFile(const char* FileName){
    char line[MAXRECORDLENGTH];
    int  i0, i;
    double j, pkum_o, dummy, bound, diff;
    std::string type;
    PTableDRow row;
    PTableDRow::iterator pos;
    FILE* ptable_in;
     
    ptable_in = fopen(FileName,"r");
    if (ptable_in == NULL){
        return false;
    }
    
    fgets((char *)line, MAXRECORDLENGTH, ptable_in); // Disregard first line: contains only names
    fgets((char *)line, MAXRECORDLENGTH, ptable_in);

    row = PTableDRow();
    
    i0 = atoi(strtok(line,";"));  // i
    j = atof(strtok(NULL,";"));   // j
    row[j] = atof(strtok(NULL,";"));  // p
    dummy = atof(strtok(NULL,";")); // p_kum_u
    pkum_o = atof(strtok(NULL,";")); // p_kum_o
    diff = atof(strtok(NULL,";"));  // diff
    type = strtok(NULL,"\n");       // type: even, odd or all
    
    while (fgets((char *)line, MAXRECORDLENGTH, ptable_in) != NULL){
        i = atoi(strtok(line,";"));
        if (i != i0){
            bound = 0;
            for (pos=row.begin();pos!=row.end();++pos){
                bound += pos->second;
                row[pos->first] = bound;
            }
            Data[type][i] = row;
            row = PTableDRow();
            i0=i;
        }
        j = atof(strtok(NULL,";"));
        row[j] = atof(strtok(NULL,";"));
        dummy = atof(strtok(NULL,";")); // p_kum_u
        pkum_o = atof(strtok(NULL,";")); // p_kum_o
        diff = atof(strtok(NULL,";"));  // diff
        type = strtok(NULL,"\n");       // type: even, odd or all
    }

    bound = 0;
    for (pos=row.begin();pos!=row.end();++pos){
        bound += pos->second;
        row[pos->first] = bound;
    }
    Data[type][i]=row;

    fclose(ptable_in);
    return true;
}

 void PTableCont::WriteToFile(){
    /*int i;
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
    fclose(pout);*/
 }

void PTableCont::Write(std::string type){
    PTableDRow::iterator pos;
    std::map<int,PTableDRow>::iterator rowpos;
    
    printf("Data[%s].size() = %d\n", type.c_str(),Data[type].size());
    
    for (rowpos=Data[type].begin();rowpos!=Data[type].end();++rowpos){
        printf("row %d:",rowpos->first);
        for (pos=rowpos->second.begin();pos!=rowpos->second.end();++pos){
            printf(" (%3.1lf, %17.15lf)", pos->first, pos->second);
        }
        printf("\n");
    }
}