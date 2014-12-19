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

#include "Properties.h"

#include <fstream>

using namespace std;

Properties::Properties(const string &fileName)
{
	load(fileName);
}

void Properties::load(const string &fileName)
{
	map.clear();
	ifstream file(fileName.c_str());
	if (file.is_open()) {
		load(file);
		file.close();
	}
}

void Properties::load(istream &is)
{
	string line;

	while (is.good()) {
      getline(is, line);
		size_t pos = line.find('#');
		size_t size = (pos == string::npos) ? line.size() : pos;
		pos = line.find('=');
      if (pos != string::npos && pos < size) {
			string leftSide = line.substr(0, pos);
			string rightSide = line.substr(pos + 1, size - pos - 1);
			setProperty(leftSide, rightSide);
		} 
	}
}

void Properties::setProperty(const string &key, const string &value)
{
	map[key] = value;
}

string Properties::getProperty(const string &key)
{
	return map[key];
}

/*
int main(int argc, char* argv[])
{
	Properties properties;

	properties.load("TauArgus.properties");

	cout << properties.getProperty("1000") << endl;

	return 0;
}
*/
