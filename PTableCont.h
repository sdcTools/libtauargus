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

#ifndef PTABLECONT_H
#define PTABLECONT_H

typedef std::map<double, double> PTableDRow;

class PTableCont {
public:
    PTableCont();
    PTableCont(const PTableCont& orig);
    virtual ~PTableCont();
    bool ReadFromFile(const char* FileName);
    std::map<int, PTableDRow> GetData(std::string type) {return Data[type];}
    void Write(std::string type);
    
private:
    // Data["even"] has even ptable info or "all" ptable info
    // Data["odd"] has odd ptable info
    std::map<std::string, std::map<int, PTableDRow> > Data;
};

#endif /* PTABLECONT_H */

