/*
 * The contents of this file are subject to the Mozilla Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is Vision.
 *
 * The Initial Developer of the Original Code is The Vision Team.
 * Portions created by The Vision Team are
 * Copyright (C) 1999-2010 The Vision Team.	All Rights
 * Reserved.
 *
 * Contributor(s): Wade Majors <wade@ezri.org>
 *								 Rene Gollent
 *								 Todd Lair
 *								 Andrew Bazan
 */


#include "NetworkManager.h"
#include "VisionMessages.h"
#include "Vision.h"

#include <String.h>

#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>

NetworkManager::NetworkManager(void)
	: BLooper("Big Brother"),
		fShuttingDown(false)
{
	for (uint32 i = 0; i < sizeof(fPollFDs) / sizeof(pollfd); i++)
	{
		fPollFDs[i].fd = -1;
	}

	fPollThread = spawn_thread(Overlord, "Overlord", B_LOW_PRIORITY, this);
	if (fPollThread >= B_OK)
	{
		resume_thread(fPollThread);
	}
	else
	{
		printf("Thread error: %" B_PRId32 "\n", fPollThread);
	}
}

NetworkManager::~NetworkManager(void)
{
	// TODO: shut down all threads / sockets
}

bool
NetworkManager::QuitRequested(void)
{
	fShuttingDown = true;
	_SocketLock();
	int32 index = -1;
	while (!fSockets.empty())
	{
		index = _IndexForSocket(fSockets.begin()->first);
		_HandleDisconnect(fSockets.begin()->first, index);
	}
	_SocketUnlock();

	return true;
}

void
NetworkManager::MessageReceived(BMessage *message)
{
	switch (message->what)
	{
		case M_CREATE_CONNECTION:
		{
			_HandleConnect(message);
		}
		break;

		case M_CREATE_LISTENER:
		{
			_HandleBind(message);
		}
		break;

		case M_DESTROY_CONNECTION:
		{
			int32 sock = -1;
			if (message->FindInt32("connection", &sock) == B_OK)
			{
				int32 index = _IndexForSocket(sock);
				if (index >= 0)
				{
					_HandleDisconnect(sock, index);
				}
			}
		}
		break;


		case M_SEND_CONNECTION_DATA:
		{
			_HandleSend(message);
		}
		break;

		default:
			BLooper::MessageReceived(message);
			break;
	}
}

int32
NetworkManager::Overlord(void *data)
{
	NetworkManager *manager = reinterpret_cast<NetworkManager *>(data);
	while (!manager->fShuttingDown)
	{
		int result = 0;
		manager->_SocketLock();

		result = poll(manager->fPollFDs, manager->fSockets.size(), 500);
		if (result <= 0)
		{
			manager->_SocketUnlock();
			continue;
		}
		for (uint32 i = 0; i < manager->fSockets.size(); i++)
		{
			if (manager->fPollFDs[i].revents & (POLLIN | POLLPRI))
			{
				if (manager->fListeners.find(manager->fPollFDs[i].fd) != manager->fListeners.end())
				{
					manager->_HandleAccept(manager->fPollFDs[i].fd, i);
				}
				else
				{
					manager->_HandleReceive(manager->fPollFDs[i].fd, i);
				}
			}
			else if (manager->fPollFDs[i].revents &
				(POLLERR | POLLHUP | POLLNVAL))
			{
				manager->_HandleDisconnect(manager->fPollFDs[i].fd, i);
			}
			else if (manager->fPollFDs[i].revents != 0)
			{
				printf("Unexpected poll flags for socket %d: %d\n", manager->fPollFDs[i].fd, manager->fPollFDs[i].revents);
			}
		}
		manager->_SocketUnlock();
	}

	return B_OK;
}

int32
NetworkManager::ConnectionHandler(void *data)
{
	BMessage *message = reinterpret_cast<BMessage *>(data);
	BString hostname, port;

	BMessenger target;
	message->FindMessenger("target", &target);

	BMessenger msgr(network_manager);

	bigtime_t timeout = 0;
	if (message->FindInt64("timeout", &timeout) == B_OK && timeout > 0)
	{
		while (timeout > 0)
		{
			snooze(500000);
			timeout -= 500000;
			// bail if the network manager's not around any more
			if (msgr.LockTarget() != B_OK)
			{
				delete message;
				return B_OK;
			}
			else
			{
				network_manager->Unlock();
			}
		}
	}

	BMessage reply(M_CONNECTION_CREATED);
	if (message->FindString("port", &port) == B_OK && message->FindString("hostname", &hostname) == B_OK)
	{
		struct addrinfo *info;
		struct addrinfo hints;
		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_ADDRCONFIG;
		int result = getaddrinfo(hostname.String(), port.String(), &hints, &info);
		int32 sock = -1;
		if (result == 0)
		{
			for (struct addrinfo *current = info; current != NULL; current = current->ai_next)
			{
				sock = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
				if (sock < 0)
				{
					result = sock;
					continue;
				}
				result = connect(sock, current->ai_addr, current->ai_addrlen);
				if (result < 0)
				{
					close(sock);
					sock = -1;
					continue;
				}
				break;
			}
			freeaddrinfo(info);
		}

		reply.AddInt32("status", result);
		if (result == 0 && sock >= 0)
		{
			if (msgr.LockTarget())
			{
				network_manager->_SocketLock();
				int32 index = network_manager->fSockets.size();
				network_manager->fSockets[sock] = target;
				network_manager->fPollFDs[index].fd = sock;
				network_manager->fPollFDs[index].events = POLLIN | POLLERR;
				reply.AddInt32("connection", sock);
				network_manager->_SocketUnlock();
				network_manager->Unlock();
			}
			else
			{
				close(sock);
				reply.ReplaceInt32("status", B_BAD_VALUE);
			}
		}
	}
	else
	{
		reply.AddInt32("status", B_BAD_DATA);
	}
	target.SendMessage(&reply);

	delete message;

	return B_OK;
}

void
NetworkManager::_HandleSend(const BMessage *data)
{
	int32 sock = -1;
	const void *sendBuffer = NULL;
	ssize_t size = -1;
	if (data->FindInt32("connection", &sock) == B_OK
		&& data->FindData("data", B_RAW_TYPE, &sendBuffer, &size) == B_OK)
	{
		int result = send(sock, sendBuffer, size, 0);
		if (result < 0)
		{
			_HandleDisconnect(sock, _IndexForSocket(sock));
		}
	}
}

void
NetworkManager::_HandleReceive(int sock, uint32 index)
{
	char recvbuffer[16384];
	int result = recv(sock, recvbuffer, sizeof(recvbuffer), 0);
	if (result <= 0)
	{
		// handle as a disconnect
		_HandleDisconnect(sock, index);
		return;
	}

	std::map<int, BMessenger>::iterator it = fSockets.find(sock);
	if (it == fSockets.end())
	{
		// TODO: log error, this should never happen
		return;
	}

	BMessage msg(M_CONNECTION_DATA_RECEIVED);
	msg.AddInt32("connection", sock);
	msg.AddData("data", B_RAW_TYPE, recvbuffer, result);

	it->second.SendMessage(&msg);
}

void
NetworkManager::_HandleAccept(int sock, uint32 /* index */)
{
	struct sockaddr_storage data;
	socklen_t datasize = sizeof(data);
	int client = accept(sock, (sockaddr *)&data, &datasize);
	if (client >= 0)
	{
		std::map<int, BMessenger>::iterator it = fSockets.find(sock);
		if (it == fSockets.end())
		{
			close(client);
			// TODO: log error, this should never happen
			return;
		}
		char namebuf[200];
		char ipbuf[100];
		memset(namebuf, 0, sizeof(namebuf));
		memset(ipbuf, 0, sizeof(ipbuf));
		getnameinfo((sockaddr *)&data, datasize, namebuf, sizeof(namebuf),
			NULL, 0, 0);
		getnameinfo((sockaddr *)&data, datasize, ipbuf, sizeof(ipbuf),
			NULL, 0, NI_NUMERICHOST);
		_SocketLock();
		int32 index = fSockets.size();
		fSockets[client] = it->second;
		fPollFDs[index].fd = client;
		fPollFDs[index].events = POLLIN | POLLERR;
		_SocketUnlock();
		BMessage msg(M_CONNECTION_ACCEPTED);
		msg.AddInt32("connection", sock);
		msg.AddInt32("client", client);
		msg.AddString("address", ipbuf);
		if (strlen(namebuf) > 0)
		{
			msg.AddString("name", namebuf);
		}
		it->second.SendMessage(&msg);
	}
}

void
NetworkManager::_HandleBind(const BMessage *message)
{
	BMessenger target;

	if (message->FindMessenger("target", &target) != B_OK)
	{
		return;
	}

	BMessage reply(M_LISTENER_CREATED);
	BString interface, port;
	if (message->FindString("port", &port) != B_OK)
	{
		reply.AddInt32("status", B_BAD_DATA);
		target.SendMessage(&reply);
		return;
	}

	struct addrinfo *info = NULL;
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
	hints.ai_socktype = SOCK_STREAM;

	int32 sock = -1;

	int result = getaddrinfo(NULL, port.String(), &hints, &info);
	if (result == 0)
	{
		for (struct addrinfo *current = info; current != NULL; current = current->ai_next)
		{
			sock = socket(current->ai_family, current->ai_socktype, current->ai_protocol);
			if (sock < 0)
			{
				result = sock;
				continue;
			}
			int opt = 1;
			setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

			result = bind(sock, current->ai_addr, current->ai_addrlen);
			if (result < 0)
			{
				close(sock);
				sock = -1;
				continue;
			}

			result = listen(sock, SOMAXCONN);
			if (result < 0)
			{
				close(sock);
				sock = -1;
				continue;
			}
			break;
		}
		freeaddrinfo(info);
	}

	reply.AddInt32("status", result);
	if (result == 0)
	{
		_SocketLock();
		int32 index = network_manager->fSockets.size();
		fSockets[sock] = target;
		fPollFDs[index].fd = sock;
		fPollFDs[index].events = POLLIN | POLLERR;
		fListeners.insert(sock);
		_SocketUnlock();
		reply.AddInt32("connection", sock);
	}

	target.SendMessage(&reply);
}

void
NetworkManager::_HandleConnect(const BMessage *message)
{
	BMessenger target;

	if (message->FindMessenger("target", &target) != B_OK)
	{
		return;
	}

	BString threadName;
	vision_app->GetThreadName(THREAD_S, threadName);
	thread_id connector = spawn_thread(ConnectionHandler, threadName.String(),
		B_LOW_PRIORITY, new BMessage(*message));

	if (connector < B_OK)
	{
		BMessage reply(M_CONNECTION_CREATED);
		reply.AddInt32("status", connector);
		target.SendMessage(&reply);
	}
	else
	{
		resume_thread(connector);
	}
}

void
NetworkManager::_HandleDisconnect(int sock, uint32 index)
{
	std::map<int, BMessenger>::iterator it = fSockets.find(sock);
	if (it == fSockets.end())
	{
		// TODO: log error, this should never happen
		return;
	}
	BMessage msg(M_CONNECTION_DISCONNECT);
	msg.AddInt32("connection", sock);
	it->second.SendMessage(&msg);
	_CleanupSocket(sock, index);
}

void
NetworkManager::_CleanupSocket(int sock, uint32 index)
{
	std::map<int, BMessenger>::iterator it = fSockets.find(sock);
	if (it == fSockets.end())
	{
		// TODO: log error, this should never happen
		return;
	}
	std::set<int>::iterator sit = fListeners.find(sock);
	if (sit != fListeners.end())
	{
		fListeners.erase(sit);
	}
	if (index < fSockets.size() - 1)
	{
		memmove(&fPollFDs[index], &fPollFDs[index + 1],
			sizeof(pollfd) * (fSockets.size() - index));
	}
	close(sock);
	fSockets.erase(it);
}

int32
NetworkManager::_IndexForSocket(int sock)
{
	_SocketLock();
	for (uint32 i = 0; i < fSockets.size(); i++)
	{
		if (fPollFDs[i].fd == sock)
		{
			_SocketUnlock();
			return i;
		}
	}
	_SocketUnlock();
	return -1;
}
void
NetworkManager::_SocketLock(void)
{
	fSocketLock.Lock();
}

void
NetworkManager::_SocketUnlock(void)
{
	fSocketLock.Unlock();
}
