/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PTable.h
 * Author: pwof
 */

#ifndef PTABLE_H
#define PTABLE_H

typedef struct{
    int j;
    double p_ub;
} P_ENTRY;

typedef std::vector<P_ENTRY> PTableRow;

class PTable {
public:
    PTable();
    PTable(const PTable& orig);
    virtual ~PTable();
    bool ReadFromFreqFile(std::string FileName);
    int GetmaxNi() {return maxNi;}
    int GetminDiff() {return minDiff;}
    int GetmaxDiff() {return maxDiff;}
    std::vector<PTableRow> GetData() {return Data;}
    void WriteToFile();
    void Write();
    
private:
    int maxNi;
    int minDiff;
    int maxDiff;
    std::vector<PTableRow> Data;
    
    void SetmaxNi();    
    void SetminDiff();
    void SetmaxDiff();

};

#endif /* PTABLE_H */

