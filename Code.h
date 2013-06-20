#if !defined Code_h
#define Code_h

/*
Code      Parent nChildren IsBogus Level

13             Y         1       N     1 
  130          Y         1       Y     2
    1300       N         0       Y     3
*/

class CCode  
{
public:
	bool IsParent;    // nChildren >= 1
	bool IsBogus;     // only child of a parent
	bool Active;      // if so, always all descendants from an IsParent-item
	int Level;        // 0 = total, 1, 2 ... 
	int nChildren;    // number of childs 
	CCode();
	virtual ~CCode();
};

#endif // Code_h

