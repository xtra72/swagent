#include <iostream>
#include <unistd.h>
#include "assert.h"
#include "exception.h"
#include "mqttclient.h"
#include "time2.h"
#include "utils.h"
#include "activeobject.h"

void	OnConnectCB(struct mosquitto *mosq, void *obj, int result);
void	OnDisconnectCB(struct mosquitto *mosq, void *obj, int rc);
void	OnPublishCB(struct mosquitto *mosq, void *obj, int mid);
void	OnSubscribeCB(struct mosquitto *_mosq, void *_obj, int mid, int qos_count, const int *granted_qos);
void	OnMessageCB(struct mosquitto *_mosq, void *_obj, const struct mosquitto_message *_message);
void	OnLogCB(struct mosquitto *mosq, void *obj, int level, const char *str);


static	int static_mid_ = 0;
static	bool	lib_init_ = false;

MQTTClient::MessageConnected::MessageConnected(std::string const& _receiver, std::string const& _sender)
: Message(MESSAGE_TYPE_CONNECTED, _receiver, _sender)
{
}

MQTTClient::MessageConnected::~MessageConnected()
{
}

MQTTClient::MessageDisconnected::MessageDisconnected(std::string const& _receiver, std::string const& _sender)
: Message(MESSAGE_TYPE_DISCONNECTED, _receiver, _sender)
{
}

MQTTClient::MessageDisconnected::~MessageDisconnected()
{
}

////////////////////////////////////////////////////////////////////////////////
//	Class	Publisher
////////////////////////////////////////////////////////////////////////////////
MQTTClient::Publisher::Publisher(std::string const& _topic)
: topic_(_topic), message_(""), mid_(++static_mid_), create_date_(), publish_date_(), qos_(1)
{
}

MQTTClient::Publisher::Publisher(std::string const& _topic, std::string const& _message)
: topic_(_topic), message_(_message), mid_(++static_mid_), create_date_(), publish_date_(), qos_(1)
{
}

MQTTClient::Publisher::Publisher(std::string const& _topic, JSONNode const& _message)
: topic_(_topic), message_(_message.write()), mid_(++static_mid_), create_date_(), publish_date_(), qos_(1)
{
}

std::string	const&	MQTTClient::Publisher::Topic(void)
{
	return	topic_;
}

std::string	const&	MQTTClient::Publisher::Message(void)
{
	return	message_;
}

bool	MQTTClient::Publisher::SetQoS(uint32_t _qos)
{
	if (2 < _qos)
	{
		return	false;
	}

	qos_ = _qos;

	return	true;
}

uint32_t	MQTTClient::Publisher::GetQoS()
{
	return	qos_;
}

void	MQTTClient::Publisher::SetMID(int _mid)
{
	mid_ = _mid;
}

int		MQTTClient::Publisher::GetMID()
{
	return	mid_;
}

void	MQTTClient::Publisher::Finished(void)
{
}

void	MQTTClient::Publisher::SetPublishDate(Date const& _date)
{
	publish_date_ = _date;
}

////////////////////////////////////////////////////////////////////////////////
//	Class	Subscriber
////////////////////////////////////////////////////////////////////////////////
MQTTClient::Subscriber::Subscriber(std::string const& _topic)
: topic_(_topic), mid_(++static_mid_), create_date_()
{
}

std::string const& MQTTClient::Subscriber::Topic()
{
	return	topic_;
}

void	MQTTClient::Subscriber::SetMID(int _mid)
{
	mid_ = _mid;
}

int		MQTTClient::Subscriber::GetMID()
{
	return	mid_;
}

void	MQTTClient::Subscriber::OnMessage(std::string const& _message)
{
}

MQTTClient::MQTTClient(std::string const& _id, Object* _parent)
: 	Object(_id, _parent),
	mosquitto_(NULL),
	protocol_version_(MQTT_PROTOCOL_V31),

    server_url_("localhost"),
    server_port_(1883),
	with_tls_(false),
	ca_cert_file_(""),

    keep_alive_(5*Time::SECOND),
    server_retry_interval_(60*Time::SECOND),
	server_connected_(false),
	first_connection_(true),
	server_retry_count_(0),

	client_id_(""),
	username_(""),
	password_(""),

	thread_(),
	stop_(false)
{
	if (!lib_init_)
	{
		mosquitto_lib_init();
		lib_init_ = true;
	}

	client_id_ = std::to_string(Date::GetCurrent().GetMicroseconds());

	properties_map_["url"] = ConfigURL;
	properties_map_["port"] = ConfigPort;
	properties_map_["client_id"] = ConfigClientID;
	properties_map_["username"] = ConfigUsername;
	properties_map_["password"] = ConfigPassword;
	properties_map_["with_tls"] = ConfigWithTLS;
	properties_map_["ca_cert_file"] = ConfigCACertFile;
}

MQTTClient::~MQTTClient()
{
	if (mosquitto_)
	{
		mosquitto_destroy(mosquitto_);
	}

	for(std::map<std::string, Subscriber*>::iterator it = subscribe_map_.begin(); it != subscribe_map_.end() ; it++)
	{
		delete it->second;
	 	subscribe_map_.erase(it);
	}

	for(std::list<Publisher*>::iterator it = publish_list_.begin(); it != publish_list_.end() ; it++)
	{
		delete *it;
	 	publish_list_.erase(it);
	}

}

bool	MQTTClient::Init(void)
{
	TRACE_DEBUG("MQTT Client init : " << client_id_.c_str());
	mosquitto_ = mosquitto_new(client_id_.c_str(), true, (void *)this);

	mosquitto_max_inflight_messages_set(mosquitto_, 20);

	mosquitto_opts_set(mosquitto_, MOSQ_OPT_PROTOCOL_VERSION, (void *)&protocol_version_);
	mosquitto_log_callback_set(mosquitto_, ::OnLogCB);
	mosquitto_publish_callback_set(mosquitto_, ::OnPublishCB);
	mosquitto_subscribe_callback_set(mosquitto_, ::OnSubscribeCB);
	mosquitto_connect_callback_set(mosquitto_, ::OnConnectCB);
	mosquitto_disconnect_callback_set(mosquitto_, ::OnDisconnectCB);
	mosquitto_message_callback_set(mosquitto_, ::OnMessageCB);

	mosquitto_loop_start(mosquitto_);
//	mosquitto_loop_forever(mosquitto_);
	
	return	true;
}

std::string MQTTClient::GetClientID(void)
{
	return	client_id_;
}

bool		MQTTClient::SetClientID(std::string const& _client_id)
{
	client_id_ = _client_id;

	return	true;	
}

std::string	MQTTClient::GetUsername(void)
{
	return	username_;
}

bool		MQTTClient::SetUsername(std::string const& _username)
{
	username_ = _username;

	return true;
}


std::string	MQTTClient::GetPassword(void)
{
	return	password_;
}

bool		MQTTClient::SetPassword(std::string const& _password)
{
	password_ = _password;

	return	true;
}

std::string	MQTTClient::GetServerURL(void)
{
	return	server_url_;
}

bool	MQTTClient::SetServerURL(std::string const& _url)
{
	server_url_ = _url;

	return	true;
}

uint16_t	MQTTClient::GetServerPort(void)
{
	return	server_port_;
}

bool		MQTTClient::SetServerPort(uint16_t _port)
{
	server_port_ = _port;

	return	true;
}

bool	MQTTClient::GetProperties(JSONNode& _properties)
{
	//Component::GetProperties(_properties);

	SetMemberValue(_properties, "client_id", client_id_);
	SetMemberValue(_properties, "url", server_url_);
	SetMemberValue(_properties, "port", server_port_);

	return	true;
}

bool	MQTTClient::Register(Subscriber* subscribe)
{
	ASSERT(subscribe);

	std::map<std::string, Subscriber*>::iterator it = subscribe_map_.find(subscribe->Topic());
	if (it != subscribe_map_.end())
	{
		return	false;	
	}

	subscribe_map_[subscribe->Topic()] = subscribe;
	TRACE_DEBUG("Subscribe registered : " << subscribe->Topic());

	return	true;
}

MQTTClient::Subscriber*	MQTTClient::GetSubscriber(std::string const& _topic)
{
	std::map<std::string, Subscriber*>::iterator it = subscribe_map_.find(_topic);
	if (it != subscribe_map_.end())
	{
		return	it->second;	
	}

	return	NULL;
}


void	MQTTClient::Start()
{
	
	if (!thread_.create(MQTTClient::ThreadMain, (void *)this))
	{
		TRACE_ERROR("Thread creation failed!");
	}
	else
	{
		TRACE_DEBUG("Thread started!");
	}
}

void	MQTTClient::Stop()
{
	thread_.join();
}

void*	MQTTClient::ThreadMain(void* _data)
{
	MQTTClient*	client= (MQTTClient*)_data;

	client->stop_ = false;
	while(!client->stop_)
	{
		client->Process();
	
		usleep(1000);	
	}

	return	NULL;
}

bool	MQTTClient::Connect()
{
	int	ret = MOSQ_ERR_SUCCESS;

	if (server_connected_)
	{
		return	true;
	}

	if (server_retry_count_ > 10)
	{
		if (mosquitto_)
		{
			mosquitto_destroy(mosquitto_);
			mosquitto_ = NULL;
		}

		server_retry_count_ = 0;
	}

	if (!mosquitto_)
	{
		Init();
	}

	if (server_retry_timeout_.RemainTime() == 0)
	{
		if (first_connection_)
		{
			if((username_.length() != 0) && (password_.length() != 0))
			{
				TRACE_DEBUG("MQTT USERNAME : " << username_);
				TRACE_DEBUG("MQTT PASSWORD : " << password_);
				ret = mosquitto_username_pw_set(mosquitto_, username_.c_str(), password_.c_str());
			}

			//
			// MQTT SSL SET
			if(with_tls_ && ca_cert_file_.length() != 0)
			{
				ret = mosquitto_tls_set(mosquitto_, ca_cert_file_.c_str(), NULL, NULL, NULL, NULL);
				if(ret == MOSQ_ERR_SUCCESS)
				{
					mosquitto_tls_opts_set(mosquitto_, 1, NULL, NULL);
					TRACE_DEBUG("MQTT TLS SET SUCCESS");
				}
				else
				{
					TRACE_ERROR("CAfile is Error");
				}

			}

			server_retry_count_ += 1;
			server_retry_timeout_.Set(Date::GetCurrent() + server_retry_interval_);
			ret = mosquitto_connect(mosquitto_, server_url_.c_str(), server_port_, keep_alive_);
			if (ret == MOSQ_ERR_SUCCESS)
			{
				TRACE_DEBUG("Broker connected : " << server_url_ << ":" << server_port_);
			}
			else
			{
				TRACE_ERROR("Failed to connect to server[" << server_url_ << "] : " << ret);
				TRACE_DEBUG("Try Connect to broker.");
			}
		}
		else
		{
			server_retry_count_ += 1;
			server_retry_timeout_.Set(Date::GetCurrent() + server_retry_interval_);
			ret = mosquitto_reconnect(mosquitto_);
			if (ret == MOSQ_ERR_SUCCESS)
			{
				TRACE_DEBUG("Broker connected : " << server_url_ << ":" << server_port_);
			}
			else
			{
				TRACE_ERROR("Failed to connect to server[" << server_url_ << "] : " << ret);
				TRACE_DEBUG("Try Connect to broker.");
			}
		}
	}

	return	(ret == MOSQ_ERR_SUCCESS);
}

bool	MQTTClient::Disconnect()
{
	if (server_connected_)
	{
		//mosquitto_loop_stop(mosquitto_, true);
		server_connected_ = false;
	}

	return	true;
}


bool	MQTTClient::Publish(Publisher* _publish)
{
	ASSERT(_publish);

	if (server_connected_)
	{
		int	mid;
		
		_publish->SetPublishDate(Date());
		if (mosquitto_publish(mosquitto_, &mid, _publish->Topic().c_str(), _publish->Message().length(), _publish->Message().c_str(), _publish->GetQoS(), false) != MOSQ_ERR_SUCCESS)
		{
			delete _publish;

			TRACE_ERROR("Publish failed");
			return	false;
		}

		_publish->SetMID(mid);
		TRACE_DEBUG("PUB[" << mid << "] : " << _publish->Topic());
		//TRACE_DEBUG("PUB[" << mid << "] : " << _publish->Topic() << " " << _publish->Message());
		publish_list_.push_back(_publish);
	}
	else 
	{
		TRACE_DEBUG("Server[" << server_url_ << ":" << server_port_ << " not connected");
	}

	return	true;

}

bool	MQTTClient::Send(uint8_t *_data, uint32_t _data_len)
{
	return	false;
}

void	MQTTClient::Preprocess(void)
{
}

void	MQTTClient::Process(void)
{
	if (!server_connected_)
	{
		Connect();
	}

	if (mosquitto_)
	{
		mosquitto_loop(mosquitto_, 1000, 0);
	}
}

void	MQTTClient::Postprocess(void)
{
}

void	MQTTClient::OnConnect(int result)
{
	ASSERT(mosquitto_);

	if (!server_connected_)
	{
		TRACE_INFO("Server[" << server_url_ << ":" << server_port_ << "] is connected.");

		if (first_connection_)
		{
			first_connection_ = false;
		}

		server_connected_ = true;
		server_retry_count_ = 0;

		for(std::map<std::string, Subscriber*>::iterator it = subscribe_map_.begin() ; it != subscribe_map_.end() ; it++)
		{
			int	mid  = it->second->GetMID();
			mosquitto_subscribe(mosquitto_, &mid, it->second->Topic().c_str(), 1);
		}

		ActiveObject*	active_object = dynamic_cast<ActiveObject*>(parent_);
		if (active_object)
		{
			active_object->Post(new MessageConnected(parent_->GetID(), GetID()));
		}
		else
		{
			TRACE_WARN("Parent is not exist!");
		}
	}
	else 
	{
		TRACE_INFO("Server already connected. : " << result);
	}
}

void	MQTTClient::OnDisconnect(int rc)
{
	if (server_connected_)
	{
		TRACE_INFO("Server disconnected. : " << rc);
		Disconnect();

		ActiveObject*	active_object = dynamic_cast<ActiveObject*>(parent_);
		if (active_object)
		{
			active_object->Post(new MessageDisconnected(parent_->GetID(), GetID()));
		}
	}
	else
	{
		TRACE_INFO("Server is not connected. : " << rc);
	}
}

void	MQTTClient::OnPublish(int mid)
{
	TRACE_DEBUG("PUB[" << mid << "] : completed.");
	for(std::list<Publisher*>::iterator it = publish_list_.begin(); it != publish_list_.end() ; it++)
	{
		if ((*it)->GetMID() == mid)
		{
			(*it)->Finished();

			delete *it;
	 		it = publish_list_.erase(it);
			break;
		}
	}
}

void	MQTTClient::OnSubscribe(int mid, int qos_count, const int *granted_qos)
{
	for(std::map<std::string, Subscriber*>::iterator it = subscribe_map_.begin() ; it != subscribe_map_.end() ; it++)
	{
		if (mid == it->second->GetMID())
		{
			TRACE_INFO("SUB[" << mid << "] : regisered.");
		}
	}
}

void	MQTTClient::OnMessage(const struct mosquitto_message *_message)
{
	ASSERT(_message);

	std::map<std::string, Subscriber*>::iterator it = subscribe_map_.find(_message->topic);
	if (it != subscribe_map_.end())
	{
		it->second->OnMessage((char*)_message->payload);
	}
}

void	MQTTClient::OnLog(int level, const char *str)
{
	//TRACE_DEBUG("Log : " << str);
}


bool	MQTTClient::ConfigURL(Object* _object, JSONNode const& _value)
{
	MQTTClient*	_client = dynamic_cast<MQTTClient*>(_object);

	_client->server_url_ = _value.as_string();

	return	true;
}

bool	MQTTClient::ConfigPort(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);

	MQTTClient*	_client = dynamic_cast<MQTTClient*>(_object);

	_client->server_port_ = _value.as_int();

	return	true;
}

bool	MQTTClient::ConfigClientID(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);

	MQTTClient*	_client = dynamic_cast<MQTTClient*>(_object);

	_client->client_id_ = _value.as_string();

	return	true;
}

bool	MQTTClient::ConfigUsername(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);

	MQTTClient*	_client = dynamic_cast<MQTTClient*>(_object);

	_client->username_ = _value.as_string();

	return	true;
}

bool	MQTTClient::ConfigPassword(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);

	MQTTClient*	_client = dynamic_cast<MQTTClient*>(_object);

	_client->password_ = _value.as_string();

	return	true;
}

bool	MQTTClient::ConfigWithTLS(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);

	MQTTClient*	_client = dynamic_cast<MQTTClient*>(_object);

	_client->with_tls_ = _value.as_bool();

	return	true;
}

bool	MQTTClient::ConfigCACertFile(Object* _object, JSONNode const& _value)
{
	ASSERT(_object);

	MQTTClient*	_client = dynamic_cast<MQTTClient*>(_object);

	_client->ca_cert_file_ = _value.as_string();

	return	true;
}

void	OnConnectCB(struct mosquitto *mosq, void *_obj, int result)
{
	ASSERT(_obj);

	MQTTClient*	client = (MQTTClient*)_obj;

	client->OnConnect(result);
}

void	OnDisconnectCB(struct mosquitto *mosq, void *_obj, int rc)
{
	ASSERT(_obj);

	MQTTClient*	client = (MQTTClient*)_obj;

	client->OnDisconnect(rc);
}

void	OnPublishCB(struct mosquitto *mosq, void *_obj, int mid)
{
	ASSERT(_obj);

	MQTTClient*	client = (MQTTClient*)_obj;

	client->OnPublish(mid);
}

void	OnSubscribeCB(struct mosquitto *_mosq, void *_obj, int mid, int qos_count, const int *granted_qos)
{
	ASSERT(_obj);

	MQTTClient*	client = (MQTTClient*)_obj;

	client->OnSubscribe(mid, qos_count, granted_qos);
}

void	OnMessageCB(struct mosquitto *_mosq, void *_obj, const struct mosquitto_message *_message)
{
	ASSERT(_obj);

	MQTTClient*	client = (MQTTClient*)_obj;

	client->OnMessage(_message);
}

void	OnLogCB(struct mosquitto *mosq, void *_obj, int level, const char *str)
{
	ASSERT(_obj);

	MQTTClient*	client = (MQTTClient*)_obj;

	client->OnLog(level, str);
}

std::ostream& operator<<(std::ostream& out, MQTTClient const& _client) 
{
	out << "Client ID : " << _client.client_id_ << std::endl;
	out << "Server URL : " << _client.server_url_ << std::endl;
	out << "Server Port : " << _client.server_port_ << std::endl;

	return	out;
}

