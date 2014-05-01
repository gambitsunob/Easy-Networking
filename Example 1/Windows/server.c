// This is the Windows only version of UDP Networking
//		There is also a Unix version and Cross Platform version


/*  SOURCE: http://msdn.microsoft.com/en-us/library/windows/desktop/ms737629(v=vs.85).aspx
For historical reasons, the Windows.h header defaults to including the Winsock.h header
file for Windows Sockets 1.1. The declarations in the Winsock.h header file will conflict
with the declarations in the Winsock2.h header file required by Windows Sockets 2.0. The
WIN32_LEAN_AND_MEAN macro prevents the Winsock.h from being included by the Windows.h
header. */
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


// Default Server Variables
// The client will need to know both of these
#define DEFAULT_PORT "27015"
#define DEFAULT_BUFLEN 512


#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")

int main()
{
	// Initializes WinSock (WS2_32.dll)
	// Requests WinSock Version 2.2 via MAKEWORD(2, 2)
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		fprintf(stderr, "WSAStartup failed.\n");
		return 1;
	}

	// Initial structures
	struct addrinfo *result = NULL, *ptr = NULL, hints;

	// Setting up the "hints". This tells WinSock how it should be used
	ZeroMemory(&hints, sizeof (hints));
	hints.ai_family = AF_INET; // AF_INET => IPv4
	hints.ai_socktype = SOCK_STREAM; // SOCK_STREAM => Streaming Socket
	hints.ai_protocol = IPPROTO_UDP; // IPPROTO_UDP => UDP Protocol vs ex: IPPROTO_TCP => TCP Protocol
	// Using AI_PASSIVE lets the getaddrinfo method (below) use the INADDR_ANY (or any IP address)
	hints.ai_flags = AI_PASSIVE;

	// Resolve the local address and port to be used by the server
	int iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// This is the socket that the server listens on
	SOCKET ListenSocket = INVALID_SOCKET;
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);

	// This should only happen if an unsupported protocol/family was given
	if (ListenSocket == INVALID_SOCKET) {
		printf("Error at socket(): %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// The socket has been created, now it has to bind to the address and port
	// This will fail if the DEFAULT_PORT is already in use
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Address info is no longer needed now that the Socket has been bound
	freeaddrinfo(result);

	// Now that we have a socket, we listen to incoming messages
	if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("Listen failed with error: %ld\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}


	// Accept a client socket
	SOCKET ClientSocket = INVALID_SOCKET;
	ClientSocket = accept(ListenSocket, NULL, NULL);

	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}











	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;

	// Receive until the peer shuts down the connection
	do {

		iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
		if (iResult > 0) {
			printf("Bytes received: %d %d\n", iResult, *(int*)(recvbuf));

			// Echo the buffer back to the sender
			int iSendResult = send(ClientSocket, recvbuf, iResult, 0);
			if (iSendResult == SOCKET_ERROR) {
				printf("send failed: %d\n", WSAGetLastError());
				closesocket(ClientSocket);
				WSACleanup();
				return 1;
			}
			printf("Bytes sent: %d\n", iSendResult);
		}
		else if (iResult == 0) {
			printf("Connection closing...\n");
		}
		else {
			printf("recv failed: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}

	} while (iResult > 0);















	// shutdown the send half of the connection since no more data will be sent
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ClientSocket);
	WSACleanup();

	return 0;
}

