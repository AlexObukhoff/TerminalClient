#include "MessageQueueServer.h"
#include "MessageQueueClient.h"

//----------------------------------------------------------------------------
IMessageQueueServer* createMessageQueueServer(const QString& aQueueName)
{
	return new MessageQueueServer(aQueueName);
}

//----------------------------------------------------------------------------
IMessageQueueServer* createMessageQueueServer(const QString& aQueueName, ILog* aLog)
{
	return new MessageQueueServer(aQueueName, aLog);
}

//----------------------------------------------------------------------------

IMessageQueueClient* createMessageQueueClient()
{
	return new MessageQueueClient();
}

//----------------------------------------------------------------------------
