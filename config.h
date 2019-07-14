#ifndef CONFIG_H__
#define CONFIG_H__

#include <string>
#include "libjson/libjson.h"

struct Config
{
public:
	Config(std::string const& _file_name = "");
	~Config();

	bool	Load(int _argc, char* _argv[]);
	bool	Find(std::string const& _key, Config& _config);
	bool	Find(std::string const& _key, std::string& _value);
	bool	Find(std::string const& _key, uint32_t& _value);

	operator JSONNode() {	return	settings_; }
	Config& operator =(JSONNode& _node);

protected:
	std::string	file_name_;	
	JSONNode	settings_;

	friend std::ostream& operator<<(std::ostream& out, const Config& config) ;
};


#endif
