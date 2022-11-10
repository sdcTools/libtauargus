/*
* Argus Open Source
* Software to apply Statistical Disclosure Control techniques
* 
* Copyright 2014 Statistics Netherlands
* 
* This program is free software; you can redistribute it and/or 
* modify it under the terms of the European Union Public Licence 
* (EUPL) version 1.1, as published by the European Commission.
* 
* You can find the text of the EUPL v1.1 on
* https://joinup.ec.europa.eu/software/page/eupl/licence-eupl
* 
* This software is distributed on an "AS IS" basis without 
* warranties or conditions of any kind, either express or implied.
*/

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

