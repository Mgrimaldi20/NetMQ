#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <WinSock2.h>
#include <MSWSock.h>

#pragma warning(push)
#pragma warning(disable: 6101)
#include <WS2tcpip.h>
#pragma warning(pop)

#include <iostream>
#include <string>

int main(int argc, char **argv)
{
	(int)argc;
	(char **)argv;

	WSADATA wsadata = { 0 };
	int res = WSAStartup(MAKEWORD(2, 2), &wsadata);
	if (res != 0)
	{
		std::cerr << "WSAStartup failed with error: " << res << std::endl;
		return 1;
	}

	struct addrinfo *result = nullptr, hints = { 0 };

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	res = getaddrinfo(nullptr, "8080", &hints, &result);
	if (res != 0)
	{
		std::cerr << "getaddrinfo failed with error: " << res << std::endl;
		WSACleanup();
		return 1;
	}

	SOCKET listensocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listensocket == INVALID_SOCKET)
	{
		std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	res = bind(listensocket, result->ai_addr, (int)result->ai_addrlen);
	if (res == SOCKET_ERROR)
	{
		std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
		closesocket(listensocket);
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);
	result = nullptr;

	if (listen(listensocket, SOMAXCONN) == SOCKET_ERROR)
	{
		std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
		closesocket(listensocket);
		WSACleanup();
		return 1;
	}

	SOCKET clientsocket = accept(listensocket, nullptr, nullptr);
	if (clientsocket == INVALID_SOCKET)
	{
		std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
		closesocket(listensocket);
		WSACleanup();
	}

	closesocket(listensocket);
	listensocket = INVALID_SOCKET;

	int sendres = 0, recvbuflen = 1024;
	std::string recvbuf(recvbuflen, '\0');

	do
	{
		res = recv(clientsocket, &recvbuf[0], recvbuflen, 0);		// receive data from the client
		if (res > 0)
		{
			std::cout << "Bytes received: " << res << std::endl;
			std::cout << "Data from client: " << recvbuf.c_str() << std::endl;

			sendres = send(clientsocket, recvbuf.c_str(), recvbuflen, 0);	// echo the data back to the client for now
			if (sendres == INVALID_SOCKET)
			{
				std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
				closesocket(clientsocket);
				WSACleanup();
				return 1;
			}

			std::cout << "Bytes sent: " << sendres << std::endl;
		}

		else if (res == SOCKET_ERROR)
		{
			std::cerr << "Recv failed with error: " << WSAGetLastError() << std::endl;
			closesocket(clientsocket);
			WSACleanup();
			return 1;
		}
	} while (res > 0);

	std::cout << "Closing connection..." << std::endl;

	res = shutdown(clientsocket, SD_SEND);
	if (res == SOCKET_ERROR)
	{
		std::cerr << "Shutdown failed with error: " << WSAGetLastError() << std::endl;
		closesocket(clientsocket);
		WSACleanup();
		return 1;
	}

	closesocket(clientsocket);
	WSACleanup();

	return 0;

	// initialize and configure the server

	// loop through the message queue and handle messages/commands

	// the client can publish to a topic or subscribe to a topic, or both

	// server will provide a few commands so far: conn, pub, sub, unsub, exit
}
