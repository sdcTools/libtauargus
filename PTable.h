/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   PTable.h
 * Author: pwof
 *
 * Created on May 31, 2018, 3:44 PM
 */

#ifndef PTABLE_H
#define PTABLE_H

class PTable {
    typedef std::map<int,double> PTableRow;
public:
    PTable();
    PTable(const PTable& orig);
    virtual ~PTable();
    bool ReadFromFile(const char* FileName);
    int GetmaxNi() {return maxNi;}
    int GetminDiff() {return minDiff;}
    int GetmaxDiff() {return maxDiff;}
    std::vector<PTableRow> GetData() {return Data;}
    
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

