#ifndef	ACTIVE_OBJECT_H__
#define	ACTIVE_OBJECT_H__

#include <map>
#include "object.h"
#include "thread.h"
#include "locker.h"
#include "messagequeue.h"


class	ActiveObject : public Object
{
	typedef	bool	(*MessageHandler)(ActiveObject*, Message*);
public:
	ActiveObject(std::string const& _id = "", Object* _parent = NULL);
	~ActiveObject();

			bool	SetID(std::string const& _id);

	virtual	bool	Start(uint32_t _wait_for_init_time = 0);	// ms
	virtual	bool	Stop(bool _wait = true);

	virtual	bool	IsRunning(uint32_t _timeout = 0);

    virtual bool    Post(Message* _message);

 //   static  bool	IsIncludeIn(Object *_object);

protected:
    Thread          thread_; 
    bool            stop_;
    	double		loop_interval_;
	MessageQueue	message_queue_;
	std::map<uint32_t, MessageHandler>	message_handler_map_;

    Locker          locker_;
    Locker          activated_;
    Locker          waiting_for_initialization_;

	virtual	void	Preprocess();
	virtual	void	Process();
	virtual	void	MessageProcess();
	virtual	void	Postprocess();

	virtual	bool	OnMessage(Message* _message);
	virtual	bool	OnReceived(Message* _message);
	virtual	bool	OnSend(Message* _message);
	virtual	bool	OnSendConfirm(Message* _message);
	virtual	bool	OnServerRequest(Message* _message);
	virtual	bool	OnServerResponse(Message* _message);
	virtual	bool	OnReportStatus(Message* _message);
	virtual	bool	OnReportValue(Message* _message);
	virtual	bool	OnReportValues(Message* _message);
	virtual	bool	OnResponseReportValue(Message* _message);

	static	bool	Received(ActiveObject *_object, Message* _message);
	static	bool	Send(ActiveObject *_object, Message* _message);
	static	bool	SendConfirm(ActiveObject *_object, Message* _message);
	static	bool	ServerRequest(ActiveObject *_object, Message* _message);
	static	bool	ServerResponse(ActiveObject *_object, Message* _message);
	static	bool	ReportStatus(ActiveObject *_object, Message* _message);
	static	bool	ReportValue(ActiveObject *_object, Message* _message);
	static	bool	ReportValues(ActiveObject *_object, Message* _message);
	static	bool	ResponseReportValue(ActiveObject *_object, Message* _message);

    static  void*   ThreadMain(void* _object);
};

#endif
