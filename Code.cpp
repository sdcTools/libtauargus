#include "General.h"
#include "Code.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCode::CCode()
{  
	Active = true;
	IsBogus = false;
}

CCode::~CCode()
{

}

