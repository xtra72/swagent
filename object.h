#ifndef	OBJECT_H__
#define	OBJECT_H__

#include <string>
#include <map>
#include "libjson/libjson.h"
#include "trace.h"

typedef	bool (*SetProperty)(Object* object, JSONNode const&);
class	Object 
{
public:
			Object(std::string const& _id = "", Object* _parent = NULL);
	virtual	~Object();

    std::string const& 	GetID();
    virtual	bool		SetID(std::string const& _id);
    std::string const& 	GetName();

	bool		Set(JSONNode const& _settings);

	virtual	bool	GetProperties(JSONNode&	_json);

	virtual	std::ostream& 	Print(std::ostream& _out, uint32_t _indent = 0, uint32_t _title_width = 8) const;  
	virtual	std::ostream& 	PrintProperties(std::ostream& _out, uint32_t _indent = 0, uint32_t _title_width = 8) const;  
	virtual	std::ostream& 	PrintOptions(std::ostream& _out, uint32_t _indent = 0, uint32_t _title_width = 8) const;  

protected:
	std::string	id_;
    std::string name_;

	std::map<std::string, SetProperty>	properties_map_;
    Trace       trace;
	Object*		parent_;

	static	bool	SetID(Object* _object, JSONNode const& _value);
	static	bool	SetName(Object* _object, JSONNode const& _value);
	static	bool	SetTrace(Object* _object, JSONNode const& _value);

	friend	Trace;
};

#endif
