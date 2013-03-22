#if !defined AMPL_h
#define AMPL_h
#include "Table.h"
#include "DataCell.h"
#include "Variable.h"
#include "SubCodeList.h"



class CAMPL
{
public:
	bool WriteTableInAMPL(CTable *tab, CVariable *var, FILE *fd);
};
#endif
