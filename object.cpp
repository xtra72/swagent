#include <iomanip>
#include "object.h"
#include "assert.h"
#include "utils.h"

Object::Object(std::string const& _id, Object* _parent)
: id_(_id), name_(_id), trace(this), parent_(_parent)
{
	properties_map_["id"] = SetID;
	properties_map_["name"] = SetName;
	properties_map_["trace"] = SetTrace;
}

Object::~Object()
{
}

std::string const& Object::GetID()
{
    return  id_;    
}

bool	Object::SetID(std::string const& _id)
{
	id_ = _id;

	return	true;
}

std::string const& Object::GetName()
{
    return  name_;    
}

bool	Object::SetID(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);

	_object->SetID(_value.as_string());

	return	true;
}

bool	Object::SetName(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);

	_object->name_ = _value.as_string();

	return	true;
}

bool	Object::SetTrace(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);

	if (_value.type() != JSON_NODE)
	{
		TRACE_ERROR2(_object, "Invalid properties");
		return	false;
	}

	bool	enable;

	if (GetMemberValue(_value, "enable", enable))
	{
		_object->trace.SetEnable(enable);	
	}

	return	true;
}

bool	Object::Set(JSONNode const& _settings)
{
	for (auto itr = _settings.begin(); itr != _settings.end(); ++itr)
	{
		auto it = properties_map_.find(itr->name());
//		TRACE_INFO("Property[" << itr->name() << "]");
		if (it != properties_map_.end())
		{
			it->second(this, *itr);	
		}
		else
		{
//			TRACE_INFO("Property[" << itr->name() << "] not found.");
		}
	}

	return	true;
}

bool	Object::GetProperties(JSONNode&	_json)
{
	SetMemberValue(_json, "id", id_);
	SetMemberValue(_json, "name", name_);

	return	true;
}

/////////////////////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////////////////////
std::ostream& Object::Print(std::ostream& _out, uint32_t _indent, uint32_t _title_width) const
{
	PrintProperties(_out, _indent, _title_width);
	PrintOptions(_out, _indent, _title_width);

	return	_out;
}

std::ostream& Object::PrintProperties(std::ostream& _out, uint32_t _indent, uint32_t _title_width) const
{
	_out << std::setw(_indent + _title_width) << "Id" << " : " << id_ << std::endl;
	_out << std::setw(_indent + _title_width) << "Name" << " : " << name_ << std::endl;
	return	_out;
}

std::ostream& Object::PrintOptions(std::ostream& _out, uint32_t _indent, uint32_t _title_width) const
{
	return	_out;
}
