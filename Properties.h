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
