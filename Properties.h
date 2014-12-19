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

#if !defined Properties_h
#define Properties_h

#include <iostream>
#include <string>
#include <map>

class Properties
{
public:
	Properties(const std::string &fileName);
	void load(const std::string &fileName);
	void load(std::istream &is);
	void setProperty(const std::string &key, const std::string &value);
	std::string getProperty(const std::string &key);

private:
	std::map<std::string, std::string> map;
};

#endif
