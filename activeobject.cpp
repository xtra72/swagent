#include <iostream>
#include <unistd.h>
#include "assert.h"
#include "activeobject.h"
#include "timer.h"

ActiveObject::ActiveObject(std::string const& _id, Object* _parent)
: 	Object(_id, _parent), 
	thread_(), 
	stop_(true), 
	loop_interval_(1000), 
	message_queue_(_id),
	locker_(), 
	activated_(), 
	waiting_for_initialization_()
{
	message_handler_map_[Message::TYPE_PACKET_RECEIVED]		= Received;
	message_handler_map_[Message::TYPE_PACKET_SEND]			= Send;
	message_handler_map_[Message::TYPE_PACKET_SEND_CONFIRM] = SendConfirm;
	message_handler_map_[Message::TYPE_SERVER_REQUEST]	= ServerRequest;
	message_handler_map_[Message::TYPE_SERVER_RESPONSE] = ServerResponse;
	message_handler_map_[Message::TYPE_REPORT_STATUS] 	= ReportStatus;
	message_handler_map_[Message::TYPE_REPORT_VALUE] 	= ReportValue;
	message_handler_map_[Message::TYPE_REPORT_VALUES] 	= ReportValues;
	message_handler_map_[Message::TYPE_RESPONSE_REPORT_VALUE] 	= ResponseReportValue;
}

ActiveObject::~ActiveObject()
{
}

bool	ActiveObject::SetID(std::string const& _id)
{
	if (id_ != _id)
	{
		id_ = _id;

		message_queue_.ChangeReceiverID(id_);
	}

	return	true;
}

bool	ActiveObject::Start(uint32_t _timeout)	// ms 
{
    bool	ret_value = true;

    if (locker_.TryLock(_timeout))
    {
        if (activated_.TryLock(0))
        {
            activated_.Unlock();

            waiting_for_initialization_.Lock();
            if (!thread_.create(ActiveObject::ThreadMain, (void *)this))
            {
                TRACE_ERROR("Thread creation failed.");
            }
            else
            {
                if (waiting_for_initialization_.TryLock(_timeout))
                {
                    TRACE_INFO("Started.");
                }
                else
                {
                    TRACE_ERROR("Exceeded initialization wait time.");
                }
            }
            waiting_for_initialization_.Unlock();
        }
        else
        {
            TRACE_INFO("Already activated.");
            ret_value = false;
        }

        locker_.Unlock();
    }
    else
    {
        TRACE_INFO("Exceeded the start initialization time.");
        ret_value = false;
    }

	return	ret_value;

}

bool	ActiveObject::Stop(bool _wait)
{
	stop_  = true;

	return	true;
}

bool	ActiveObject::IsRunning(uint32_t _timeout)
{
	return	!stop_ ;
}

void* ActiveObject::ThreadMain(void* _data)
{
   	Timer	loop_timer;
	ActiveObject* object = (ActiveObject*)_data;

	TRACE_INFO2(object, "Thread started.");
	object->activated_.Lock();

	TRACE_INFO2(object, "Preprocess.");
	object->Preprocess();
	object->stop_ = false;
	TRACE_INFO2(object, "Initialization completed : " << object->GetID());
	object->waiting_for_initialization_.Unlock();

	loop_timer.Set(Date::GetCurrent());

	while(!object->stop_)
	{
		loop_timer.Add(object->loop_interval_);

		object->Process();
		usleep(loop_timer.RemainTime());
	}

	object->Postprocess();
	object->activated_.Unlock();

}

bool	ActiveObject::Post(Message* _message)
{
	message_queue_.Push(_message);

	return	true;
}

bool	ActiveObject::OnMessage(Message* _message)
{
	TRACE_ERROR("Unknown message[" << _message->GetType() << "," << _message->GetSender() << "]");

	return	true;
}

bool	ActiveObject::OnReceived(Message* _message)
{
	TRACE_ERROR("Unknown message[" << _message->GetType() << "," << _message->GetSender() << "]");

	return	true;
}

bool	ActiveObject::OnSend(Message* _message)
{
	TRACE_ERROR("Unknown message[" << _message->GetType() << "," << _message->GetSender() << "]");

	return	true;
}

bool	ActiveObject::OnSendConfirm(Message* _message)
{
	TRACE_ERROR("Unknown message[" << _message->GetType() << "," << _message->GetSender() << "]");

	return	true;
}

bool	ActiveObject::OnServerRequest(Message* _message)
{
	TRACE_ERROR("Unknown message[" << _message->GetType() << "," << _message->GetSender() << "]");

	return	true;
}

bool	ActiveObject::OnServerResponse(Message* _message)
{
	TRACE_ERROR("Unknown message[" << _message->GetType() << "," << _message->GetSender() << "]");

	return	true;
}

bool	ActiveObject::OnReportStatus(Message* _message)
{
	TRACE_ERROR("Unknown message[" << _message->GetType() << "," << _message->GetSender() << "]");

	return	true;
}

bool	ActiveObject::OnReportValue(Message* _message)
{
	TRACE_ERROR("Unknown message[" << _message->GetType() << "]");

	return	true;
}

bool	ActiveObject::OnReportValues(Message* _message)
{
	TRACE_ERROR("Unknown message[" << _message->GetType() << "]");

	return	true;
}

bool	ActiveObject::OnResponseReportValue(Message* _message)
{
	TRACE_ERROR("Unknown message[" << _message->GetType() << "]");

	return	true;
}

void	ActiveObject::Preprocess()
{
}

void	ActiveObject::Process()
{
	MessageProcess();
}

void	ActiveObject::MessageProcess()
{
	Message*	message = message_queue_.Pop();
	if (message != NULL)
	{
		auto handler = message_handler_map_.find(message->GetType());
		if (handler != message_handler_map_.end())
		{
			TRACE_INFO("MSG[" << message->GetID() << "] : received - " << message->GetType());
			handler->second(this, message);
		}
		else
		{
			OnMessage(message);
		}

		delete message;
	}
}

void	ActiveObject::Postprocess()
{
}

//bool	ActiveObject::IsIncludeIn(Object *_object)
//{
//	return	dynamic_cast<ActiveObject*>(_object) != NULL;
//}

bool	ActiveObject::Received(ActiveObject *_object, Message* _message)
{
	ASSERT(_object);

	return	_object->OnReceived(_message);
}

bool	ActiveObject::Send(ActiveObject *_object, Message* _message)
{
	ASSERT(_object);

	return	_object->OnSend(_message);
}

bool	ActiveObject::SendConfirm(ActiveObject *_object, Message* _message)
{
	ASSERT(_object);

	return	_object->OnSendConfirm(_message);
}

bool	ActiveObject::ServerRequest(ActiveObject *_object, Message* _message)
{
	ASSERT(_object);

	return	_object->OnServerRequest(_message);
}

bool	ActiveObject::ServerResponse(ActiveObject *_object, Message* _message)
{
	ASSERT(_object);

	return	_object->OnServerResponse(_message);
}

bool	ActiveObject::ReportStatus(ActiveObject *_object, Message* _message)
{
	ASSERT(_object);

	return	_object->OnReportStatus(_message);
}

bool	ActiveObject::ReportValue(ActiveObject *_object, Message* _message)
{
	ASSERT(_object);

	return	_object->OnReportValue(_message);
}

bool	ActiveObject::ReportValues(ActiveObject *_object, Message* _message)
{
	ASSERT(_object);

	return	_object->OnReportValues(_message);
}

bool	ActiveObject::ResponseReportValue(ActiveObject *_object, Message* _message)
{
	ASSERT(_object);

	return	_object->OnResponseReportValue(_message);
}
