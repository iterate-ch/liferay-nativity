/**
 * Copyright (c) 2000-2013 Liferay, Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 */

#include "CommunicationSocket.h"

#include <WinSock2.h>
#include <Ws2def.h>
#include <windows.h>
#include <memory>
#include "RegistryUtil.h"

using namespace std;

#define DEFAULT_BUFLEN 4096

bool connect(int &port, SOCKET &clientSocket, sockaddr_in &clientService, bool retry) {
	clientSocket = INVALID_SOCKET;
	clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET) {
		int error = WSAGetLastError();

		return false;
	}

	clientService.sin_family = AF_INET;
	clientService.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	clientService.sin_port = htons(port);

	HRESULT hResult = connect(clientSocket, (SOCKADDR*)&clientService, sizeof(clientService));

	if (FAILED(hResult)) {
		int error = WSAGetLastError();

		closesocket(clientSocket);
		if (retry) {
			unique_ptr<int> tempPort = make_unique<int>();

			if (!RegistryUtil::ReadRegistry(REGISTRY_ROOT_KEY, REGISTRY_PORT, tempPort.get())) {
				return false;
			}

			port = *tempPort;
			return connect(port, clientSocket, clientService, false);
		}
		else {
			return false;
		}
	}

	return true;
}

CommunicationSocket::CommunicationSocket() : _port(0)
{
	WSADATA wsaData;

	HRESULT hResult = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (FAILED(hResult))
	{
		int error = WSAGetLastError();
	}
}

CommunicationSocket::~CommunicationSocket()
{
	WSACleanup();
}

bool CommunicationSocket::ReceiveResponseOnly(wstring* message)
{
	SOCKET clientSocket;
	struct sockaddr_in clientService;

	if (!connect(_port, clientSocket, clientService, true)) {
		return false;
	}

	int bytesRead;

	do
	{
		char rec_buf[DEFAULT_BUFLEN];
		bytesRead = recv(clientSocket, rec_buf, DEFAULT_BUFLEN, MSG_WAITALL);

		if (bytesRead > 0)
		{
			// Java sending UTF-16LE stream is already in correct format for wstring/wchar_t
			message->append((wchar_t*)rec_buf, bytesRead / 2);
		}
	} while (bytesRead > 0);

	HRESULT hResult = shutdown(clientSocket, SD_BOTH);

	if (FAILED(hResult))
	{
		int error = WSAGetLastError();

		closesocket(clientSocket);

		return false;
	}

	closesocket(clientSocket);

	return true;
}

bool CommunicationSocket::SendMessageReceiveResponse(const wchar_t* message, wstring* response)
{
	SOCKET clientSocket;
	struct sockaddr_in clientService;

	if (!connect(_port, clientSocket, clientService, true)) {
		return false;
	}

	size_t result = send(clientSocket, (char*)message, (int)(wcslen(message) * 2), 0);

	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		closesocket(clientSocket);

		return false;
	}

	// shutdown the connection since no more data will be sent

	result = shutdown(clientSocket, SD_SEND);

	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		closesocket(clientSocket);

		return false;
	}

	int bytesRead;

	do
	{
		char rec_buf[DEFAULT_BUFLEN];
		bytesRead = recv(clientSocket, rec_buf, DEFAULT_BUFLEN, MSG_WAITALL);

		if (bytesRead > 0)
		{
			// Java sending UTF-16LE stream is already in correct format for wstring/wchar_t
			response->append((wchar_t*)rec_buf, bytesRead / 2);
		}
	} while (bytesRead > 0);

	result = shutdown(clientSocket, SD_BOTH);

	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		closesocket(clientSocket);

		return false;
	}

	closesocket(clientSocket);

	return true;
}
