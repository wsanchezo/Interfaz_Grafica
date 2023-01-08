#pragma once
#include <Windows.h>
#include <stdlib.h>
#include <winsock2.h>			//for WSAStartup()
#include <ws2tcpip.h>			//for getaddrinfo()
#include <Iphlpapi.h>			// for GetAdaptersAddresses()
#include <Wlanapi.h>			//for WlanEnumInterfaces
#include <objbase.h>			//for StringFromGUID2
#include <Commctrl.h>			//for CB_SETMINVISIBLE
#include <vector>
#include "SharedClass.h"

// Need to link with Ws2_32.lib, Mswsock.lib, Advapi32.lib, and Iphlpapi.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Wlanapi.lib")	//for WlanEnumInterfaces
#pragma comment(lib,"Ole32.lib")	//for StringFromGUID2
#pragma comment(lib, "Rpcrt4.lib")	//for  UuidFromString / UuidFromStringA / UuidFromStringW

#define WSS_IP_STRING_SIZE 256

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))

class WinSockServer3 {
	//******************************  STRINGS  *****************************************************
public: static const int SEND_PHOTO_MAIN_DEFAULT_STRING_SIZE = 256;
public: static const int MAX_IP_ADRESSES = 10;
public: static const int MAX_IP_STRING_LENGHT = 25;
protected: wchar_t ServiceName[SEND_PHOTO_MAIN_DEFAULT_STRING_SIZE]= { 0 };
protected: wchar_t ComputerName[MAX_COMPUTERNAME_LENGTH + 1] = { 0 }; // obtener nombre la computadora
protected: wchar_t IP[MAX_IP_ADRESSES][SEND_PHOTO_MAIN_DEFAULT_STRING_SIZE];
protected: wchar_t PORT[MAX_IP_ADRESSES][SEND_PHOTO_MAIN_DEFAULT_STRING_SIZE];

		 //*************** STATUS **********************************************************
public: enum class STATE { NONE, CONNECTED, LISTENING, ERROR_DETECTED, ATTEMPTING_CONNECT_TO_SERVER };
public:STATE            ClientState = STATE::NONE;
public:BOOL             bConnected = FALSE;
	//****************************** SOCKET MANAGEMENT ***********************************************

public: enum class WHO_IS_MY_SOCKET {SERVER,CLIENT,UNDEFINED};
public: WHO_IS_MY_SOCKET Who_is_my_socket = WHO_IS_MY_SOCKET::UNDEFINED;
protected: 	ADDRINFOW 	hints;
public: 	SOCKET ClientSocket = INVALID_SOCKET, ListenSocket= INVALID_SOCKET;
protected: 	WSAEVENT NetworkEvent_Listen, NetworkEvent_Client;
protected: int WeGotIP = false;
//ip_client_string=direccion IP del cliente. obtenida por getpeername() dentro de WinSockAccept()
//funcion llamada en respuesta al mensaje FD_ACCEPT de WndProc()
protected: wchar_t ip_client_string[WSS_IP_STRING_SIZE];
public: wchar_t* getIP_string(wchar_t* buffer) {
	lstrcpyW(buffer, ip_client_string);
	return buffer;
}
protected: int IP_Adresses_avaliables = 0;
DWORD  bufCharCount = MAX_COMPUTERNAME_LENGTH + 1;
ADDRINFOW* ptr = NULL;
public:static const  unsigned  int portNumber_default = 27015;
	//****************************** WIFI ENUMERATION AND CONNECTION *******************************

	//******************************  OUTPUT   *****************************************************
enum class OUTPUT_t { DEBUG, LISTBOX};
OUTPUT_t OUTPUT = OUTPUT_t::DEBUG;
HWND							ListBox_forOutput = NULL;
HANDLE							hClientHandle = NULL;
PWLAN_INTERFACE_INFO_LIST		pIfList = NULL;
//PWLAN_INTERFACE_INFO			pIfInfo = NULL;

PWLAN_AVAILABLE_NETWORK_LIST	pBssList = NULL;
//PWLAN_AVAILABLE_NETWORK			pBssEntry = NULL;

//****************************** SOCKET ****************************************************
protected: BOOL         WSAIniciated = FALSE;
public:static const int IPString_Lenght = 50;
public:      wchar_t IPString[IPString_Lenght] = { 0 };
public:      wchar_t PortString[IPString_Lenght] = { 0 };
protected: WSADATA wsaData = { 0 };
protected: const int ServerIndex = 0;
public: unsigned int ClientIndex = 1;
	  //***************************** EVENTS *****************************************************
public:static const int TIME_OUT_FOR_EVENTS = 50;//50 milliseconds
public:int TimeOutForEvents = TIME_OUT_FOR_EVENTS;
protected: DWORD EventTotal = 1;
//*******************************  CLIENTS ARRAYS********************************************
public: static const int DATA_BUFSIZE = 512;
protected: WSAEVENT EventArray[WSA_MAXIMUM_WAIT_EVENTS];
protected: SOCKET SocketArray[WSA_MAXIMUM_WAIT_EVENTS];
public: sockaddr AddressArray[WSA_MAXIMUM_WAIT_EVENTS];
public: unsigned char BufferReceived[DATA_BUFSIZE + 1][WSA_MAXIMUM_WAIT_EVENTS] = { 0 };
public:int ReceivedBytes[WSA_MAXIMUM_WAIT_EVENTS] = { 0 };
public: BOOL OverflowAlert[WSA_MAXIMUM_WAIT_EVENTS] = { FALSE };
public: STATE StateArray[WSA_MAXIMUM_WAIT_EVENTS] = { STATE::NONE };
	  //******************************* SEND -RECIEVE data ****************************************
protected: static const int SendBufferSize = 512;
protected: char SendBuffer[SendBufferSize];
protected: int SendBytes = 0;
public: SharedClass sharedData[WSA_MAXIMUM_WAIT_EVENTS];
public:unsigned char sharedDataBuffer[sizeof(SharedClass)];
	  //************************** SEND RECIEVE ********************************************

public: unsigned char            BufferReceivedClient[DATA_BUFSIZE + 1] = { 0 };
public:unsigned short              ReceivedBytesClient = { 0 };
public: BOOL            OverflowAlertClient = { FALSE };
public:SharedClass      sharedDataClient;
 //***************************** Various ******************************************************
protected: int iResult = 0;
public: int lastWSAError = 0;
protected: static const int ErrorBufferLen = 1000;
protected: wchar_t lpBuffer[ErrorBufferLen];


public: WinSockServer3() {

}
/// <summary>
/// Inicializa el sistema de Sockets de Windows mediante una llamada a WSAStartup()
/// </summary>
/// <returns>0 si ha funcionado. WSA error code if it has fail</returns>
protected: int IniWSA() {
	// Obtiene el nombre de la computadora, necesario para la 
	//funcion GetAddrInfoW()

	int iResult = 0;

	if (!WSAIniciated)
	{
		//Inicializa el sistema de sockets de windows
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			XTrace(L"WSAStartup() failed with error:", iResult);
		}
		else
		{
			WSAIniciated = true;
			XTrace(L"WSAStartup() success");
		}
	}
	return iResult;
}

/// <summary>
/// Inicializa el sistema WinsockDLL de windows con una llamada a WSAStartup()
/// Recibe un listado de las IP disponibles con GetAddrInfoW()
/// Las IP's son guardadas en el arreglo privado ipstringbuffer[]
/// </summary>
/// <returns>WSAError code</returns>
public: int GetIPList(addrinfoW* result) {
	// Initialize Winsock**************************************************************************************

	wchar_t comodin[256], comodin2[256];
	int iResult = 0;
	const wchar_t* s;
	//Obtener el IP de la computadora
	int i = 0;
	int inet = 0;
	//    struct sockaddr_in6 *sockaddr_ipv6;
	LPSOCKADDR sockaddr_ip;

	INT iRetval;
	wchar_t ipstringbuffer[46];
	DWORD ipbufferlength = 46;


	// Obtiene el nombre de la computadora, necesario para la 
	//funcion GetAddrInfoW()
	ComputerName[0] = 0;
	bufCharCount = MAX_COMPUTERNAME_LENGTH + 1;
	if (!GetComputerNameW(ComputerName, &bufCharCount))
	{
		XTrace(s = L"GetComputerName failed");
		DWORD lastError=GetLastError();
		XTrace(WindowsErrorToString(lastError));
	}

	if (!WSAIniciated)
	{
		//Inicializa el sistema de sockets de windows
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			XTrace(s = L"WSAStartup() failed with error:", iResult);
			return iResult;
		}
		WSAIniciated = true;
		XTrace(s = L"WSAStartup() success");
	}
		//Obtiene un listado de las direcciones IP**********************************************************************************

		ZeroMemory(&hints, sizeof(hints));
		ZeroMemory(&ServiceName, SEND_PHOTO_MAIN_DEFAULT_STRING_SIZE);
		hints.ai_family = AF_INET;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_flags = AI_PASSIVE;

		iResult = GetAddrInfoW(
			ComputerName,
			ServiceName,
			&hints,
			&result);
		if (iResult != 0) {
			XTrace(L"getaddrinfo() failed with error:", iResult);
			XTrace(this->WindowsErrorToString(iResult));
			WSACleanup();
			return iResult;
		}
		XTrace(s = L"getaddrinfo() success");
		// Retrieve each address and print out the hex bytes
		for (ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			//XTrace(L"GetAddrInfoW response ", i++);
			//XTrace(L"\tFlags: ", ptr->ai_flags);
			i++;
			_itow_s(i, comodin2, 10);
			lstrcpyW(comodin, comodin2);
			lstrcatW(comodin, L" Family: ");

			inet = ptr->ai_family;
			switch (ptr->ai_family) {
			case AF_UNSPEC:
				lstrcatW(comodin, L"Unspecified");
				break;
			case AF_INET:
			{
				lstrcatW(comodin, L"AF_INET (IPv4)");
				sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
				// The buffer length is changed by each call to WSAAddresstoString
				// So we need to set it for each iteration through the loop for safety
				ipbufferlength = 46;
				iRetval = WSAAddressToStringW(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL,
					ipstringbuffer, &ipbufferlength);
				if (iRetval)
					XTrace(L"WSAAddressToString failed with ", WSAGetLastError());
				else
					lstrcatW(comodin, L" IP:");
				lstrcatW(comodin, ipstringbuffer);
				SaveIpAddress(ipstringbuffer);
				break;
			}
			case AF_INET6:
			{
				lstrcatW(comodin, L"AF_INET6 (IPv6)");;
				inet = 6;
				// the InetNtop function is available on Windows Vista and later
				// sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
				// printf("\tIPv6 address %s\n",
				//    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

				// We use WSAAddressToString since it is supported on Windows XP and later
				sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
				// The buffer length is changed by each call to WSAAddresstoString
				// So we need to set it for each iteration through the loop for safety
				ipbufferlength = 46;
				iRetval = WSAAddressToStringW(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL,
					ipstringbuffer, &ipbufferlength);
				if (iRetval)
					XTrace(L"WSAAddressToString failed with ", WSAGetLastError());
				else
					lstrcatW(comodin, L" IP:");
				lstrcatW(comodin, ipstringbuffer);
				SaveIpAddress(ipstringbuffer);
				break;
			}
			default:
				lstrcatW(comodin, L"Other ");
				_itow_s(ptr->ai_family, comodin2, 10);
				lstrcatW(comodin, comodin2);
				break;
			}
			XTrace(comodin);
			lstrcpyW(comodin, L"\tSocket type: ");
			switch (ptr->ai_socktype) {
			case 0:
				lstrcatW(comodin, L"Unspecified");
				break;
			case SOCK_STREAM:
				lstrcatW(comodin, L"SOCK_STREAM (stream)");
				break;
			case SOCK_DGRAM:
				lstrcatW(comodin, L"SOCK_DGRAM (datagram)");
				break;
			case SOCK_RAW:
				lstrcatW(comodin, L"SOCK_RAW (raw)");
				break;
			case SOCK_RDM:
				lstrcatW(comodin, L"SOCK_RDM (reliable message datagram)");
				break;
			case SOCK_SEQPACKET:
				lstrcatW(comodin, L"SOCK_SEQPACKET (pseudo-stream packet)");
				break;
			default:
				_itow_s(ptr->ai_socktype, comodin2, 10);
				lstrcatW(comodin, comodin2);
				break;
			}
			//XTrace(comodin);

			lstrcpyW(comodin, L"\tProtocol: ");
			switch (ptr->ai_protocol) {
			case 0:
				lstrcatW(comodin, L"Unspecified");
				break;
			case IPPROTO_TCP:
				lstrcatW(comodin, L"IPPROTO_TCP (TCP)");
				break;
			case IPPROTO_UDP:
				lstrcatW(comodin, L"IPPROTO_UDP (UDP) ");
				break;
			default:
				_itow_s(ptr->ai_protocol, comodin2, 10);
				lstrcatW(comodin, comodin2);
				break;
			}
			//XTrace(comodin);
			//XTrace(L"\tLength of this sockaddr: ", (int)ptr->ai_addrlen);
			//XTrace(L"\tCanonical name: ", ptr->ai_canonname);
		}
		//GetAdaptersInformation(); 
		//GetInterfaceInformation();
		FreeAddrInfoW(result);
	

	lstrcpyW(this->ip_client_string, L"Not connected");
	return iResult;
}
/// <summary>
/// No implementada todavía
/// </summary>
public:void WinSockClientIni(wchar_t* IPString, wchar_t* PortString) {

}
/// <summary>
/// Inicializa el sistema WinsockDLL de windows con una llamada a WSAStartup()
/// Crea un Socket básico con () y lo almacena en ListenSocket
/// </summary>
public:int WinSockServerIni2(wchar_t* IPString, wchar_t* PortString) {

	ADDRINFOW* result = NULL;
	int iResult;
	// Initialize Winsock**************************************************************************************
	if (!WSAIniciated)
	{
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			XTrace(L"WSAStartup() failed with error:", iResult);
			WSAIniciated = false;
			return iResult;
		}
		else
		{
			WSAIniciated = true;
			XTrace(L"WSAStartup() success");
		}
	}
	// Resolve the server address and port**************************************************************************************

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	iResult = GetAddrInfoW(
		IPString,
		PortString,
		&hints,
		&result);
	if (iResult != 0) {
		XTrace(L"getaddrinfo() failed with error:", iResult);
		XTrace(this->WindowsErrorToString(iResult));
		WSACleanup();
		return false;
	}
	else
	{
		XTrace(L"getaddrinfo() success");
	}
	// Create the socket *************************************************************************************
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		XTrace(L"socket() failed with error:", iResult = WSAGetLastError());
		XTrace(L"Error:", this->WindowsErrorToString(iResult));
		FreeAddrInfoW(result);
		WSACleanup();
		return false;
	}
	else
	{
		XTrace(L"socket() success");
	}
}

protected:void closeAllSockets()
{
	closesocket(ClientSocket);
	Who_is_my_socket = WHO_IS_MY_SOCKET::UNDEFINED;
	ClientState = STATE::NONE;

	for (unsigned int index = 0; index < (ClientIndex - 1); index++)
	{
	closesocket(SocketArray[index]);
	WSACloseEvent(EventArray[index]);
	StateArray[0] = STATE::NONE;
		ReceivedBytes[0] = 0;
		OverflowAlert[0] = FALSE;
	}
	ClientIndex = 1;
}
	  /// <summary>
	  /// Inicializa WinSock2 de windows.
	  /// Inicializa el socket cliente.
	  /// Inicializa el evento FD_CONNECT|FD_CLOSE|FD_READ|FD_WRITE.
	  /// No altera el STATUS del objeto WSA_non_blocking_Client
	  /// </summary>
	  /// <returns>TRUE si el socket está inicializado y enlazado con el Evento.
	  /// FALSE si algo ha fallado. lastWSAError guarda el ultimo error generado</returns>
public: int CreateClientSocket() {

	lastWSAError = 0;
	// socket() data *******************************************************************
	int iFamily = AF_INET;
	int iType = SOCK_STREAM;
	int iProtocol = IPPROTO_TCP;
	// Initialize Winsock*****************************************************************
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		lastWSAError = WSAGetLastError();
		XTrace(L"WSAStartup failed, returned= %d. lastWSAError=%u\n", iResult, lastWSAError);
		ClientState = STATE::ERROR_DETECTED;
		Who_is_my_socket = WHO_IS_MY_SOCKET::UNDEFINED;
		return FALSE;
	}
	// Create socket****************************************************************
	closeAllSockets();
	ClientSocket = socket(iFamily, iType, iProtocol);
	if (ClientSocket == INVALID_SOCKET)
	{
		lastWSAError = WSAGetLastError();
		XTrace(L"socket function failed with error = %d\n", lastWSAError);
		ClientState = STATE::ERROR_DETECTED;
		Who_is_my_socket = WHO_IS_MY_SOCKET::UNDEFINED;
		return FALSE;
	}
	// Associate event types  FD_READ|FD_WRITE| FD_CONNECT |FD_CLOSE*****************************************
		// with the listening socket and NewEvent
		// Create new event
	EventArray[0] = WSACreateEvent();
	iResult = WSAEventSelect(ClientSocket, EventArray[0], FD_READ  | FD_CONNECT | FD_CLOSE);
	if (iResult == SOCKET_ERROR) {
		lastWSAError = WSAGetLastError();
		XTrace(L"WSAEventSelect failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		closesocket(ClientSocket);
		WSACleanup();
		ClientState = STATE::ERROR_DETECTED;
		return FALSE;
	}
	ClientState =STATE:: ATTEMPTING_CONNECT_TO_SERVER;
	Who_is_my_socket = WHO_IS_MY_SOCKET::CLIENT;
	return TRUE;
}

	  /// <summary>
	  /// Initialize SERVER.
	  /// Initialize WSA subsystem.
	  /// Initializes Server Socket and listens to requesting clients
	  /// Initializes Server Events
	  /// </summary>
	  /// <param name="IPString">Server IP</param>
	  /// <param name="port">Server Port</param>
	  /// <returns>True if succeed. FALSE if fails, lastWSAError saves the WSAGelLastError() value
	  /// this function does alter the content of "status" ERROR_DETECTED: if socket cannot be created
	  /// status=LISTENING if has been created</returns>
public: BOOL CreateServerSocket(wchar_t* IPString, wchar_t* strPort) {
	lastWSAError = 0;
	// socket() data --------------------
	int iFamily = AF_INET;
	int iType = SOCK_STREAM;
	int iProtocol = IPPROTO_TCP;
	//  bind() data ---------------------
	sockaddr_in service = { 0 };
	IN_ADDR in_addr = { 0 };
	//  Listen() data -------------------
	int    backlog = 0;
	// Initialize Winsock*****************************************************************

	lastWSAError = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (lastWSAError != 0) {
		XTrace(L"WSAStartup failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		StateArray[ServerIndex] = STATE::ERROR_DETECTED;
		Who_is_my_socket = WHO_IS_MY_SOCKET::UNDEFINED;
		return FALSE;
	}

	WSAIniciated = TRUE;
	// Create socket****************************************************************
	closeAllSockets();
	SocketArray[ServerIndex] = socket(iFamily, iType, iProtocol);
	if (SocketArray[ServerIndex] == INVALID_SOCKET)
	{
		lastWSAError = WSAGetLastError();
		XTrace(L"socket failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		StateArray[ServerIndex] = STATE::ERROR_DETECTED;
		Who_is_my_socket = WHO_IS_MY_SOCKET::UNDEFINED;
		return FALSE;
	}
	// Set non-blocking mode****************************************************************
	//-------------------------
	// Set the socket I/O mode: In this case FIONBIO
	// enables or disables the blocking mode for the 
	// socket based on the numerical value of iMode.
	// If iMode = 0, blocking is enabled; 
	// If iMode != 0, non-blocking mode is enabled.

	u_long iMode = 1;
	iResult = ioctlsocket(SocketArray[ServerIndex], FIONBIO, &iMode);
	if (iResult == SOCKET_ERROR)
	{
		lastWSAError = WSAGetLastError();
		XTrace(L"ioctlsocket failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		StateArray[ServerIndex] = STATE::ERROR_DETECTED;
		Who_is_my_socket = WHO_IS_MY_SOCKET::UNDEFINED;
		return FALSE;
	}
	// Bind the socket to a specific port of an IP adress***********************************
	int port=_wtoi(strPort);
	if ((iResult = InetPton(AF_INET, IPString, &in_addr)) != 1) {
		//The InetPton function returns a value of 0 if the pAddrBuf parameter points to a string
		//that is not a valid IPv4 dotted - decimal string or a valid IPv6 address string.
		//Otherwise, a value of - 1 is returned, and a specific error code can be retrieved by 
		//calling the WSAGetLastError() for extended error information.
		if (iResult == 0) {
			//WSAEFAULT=The system detected an invalid pointer address.
			lastWSAError = WSAEFAULT;
			XTrace(L"InetPton failed: IPString is not a valid IP");
			closesocket(SocketArray[ServerIndex]);
			WSACleanup();
			StateArray[ServerIndex] = STATE::ERROR_DETECTED;
			Who_is_my_socket = WHO_IS_MY_SOCKET::UNDEFINED;
			return FALSE;
		}
		lastWSAError = WSAGetLastError();
		XTrace(L"InetPton failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		closesocket(SocketArray[ServerIndex]);
		WSACleanup();
		StateArray[ServerIndex] = STATE::ERROR_DETECTED;
		Who_is_my_socket = WHO_IS_MY_SOCKET::UNDEFINED;
		return FALSE;

	}
	service.sin_family = AF_INET;
	service.sin_addr = in_addr;
	service.sin_port = htons(port);
	SOCKADDR* prtSOCKADDR = (SOCKADDR*)&service;
	wchar_t IPString2[20];
	wchar_t PortString2[20];
	socketaddress_to_string(prtSOCKADDR, IPString2, PortString2);
	iResult = bind(SocketArray[ServerIndex], (SOCKADDR*)&service, sizeof(service));
	if (iResult == SOCKET_ERROR) {
		lastWSAError = WSAGetLastError();
		XTrace(L"bind failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		closesocket(SocketArray[ServerIndex]);
		WSACleanup();
		StateArray[ServerIndex] = STATE::ERROR_DETECTED;
		Who_is_my_socket = WHO_IS_MY_SOCKET::UNDEFINED;
		return FALSE;
	}
	//The IP and port received are saved in variables of the WSA_non_blocking object
	wcscpy_s(this->IPString, this->IPString_Lenght, IPString);
	_itow_s(port, PortString, 10);
	//-------------------------
	// Associate event types FD_ACCEPT and FD_CLOSE*****************************************
	// with the listening socket and NewEvent
	// Create new event
	EventArray[0] = WSACreateEvent();
	iResult = WSAEventSelect(SocketArray[ServerIndex], EventArray[0], FD_ACCEPT | FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE);
	if (iResult == SOCKET_ERROR) {
		lastWSAError = WSAGetLastError();
		XTrace(L"WSAEventSelect failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		closesocket(SocketArray[ServerIndex]);
		WSACleanup();
		StateArray[ServerIndex] = STATE::ERROR_DETECTED;
		Who_is_my_socket = WHO_IS_MY_SOCKET::UNDEFINED;
		return FALSE;
	}
	// Listen *********************************************************************
	iResult = listen(SocketArray[ServerIndex], 1);
	if (iResult == SOCKET_ERROR) {
		lastWSAError = WSAGetLastError();
		XTrace(L"listen failed: %d. %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		closesocket(SocketArray[ServerIndex]);
		WSACleanup();
		StateArray[ServerIndex] = STATE::ERROR_DETECTED;
		Who_is_my_socket = WHO_IS_MY_SOCKET::UNDEFINED;
		return FALSE;
	}
	XTrace(L"listen succeed\n");
	StateArray[ServerIndex] = STATE::LISTENING;
	Who_is_my_socket = WHO_IS_MY_SOCKET::SERVER;
	return TRUE;
}

/// <summary>
/// Inicializa el sistema WinsockDLL de windows con una llamada a WSAStartup()
/// Traduce IPString y PortString con una llamada a GetAddrInfoW()
/// Crea un socket, lo bindea, y lo pone en escucha
/// Inicializa los eventos con WSACreateEvent
/// </summary>
/// <param name="IPString"></param>
/// <param name="PortString"></param>
/// <param name="MY_MESSAGE_NOTIFICATION"></param>
/// <returns></returns>
public:	   BOOL WinSockServerIni(wchar_t* IPString, wchar_t* PortString, u_int MY_MESSAGE_NOTIFICATION)
{
	int iResult;
	int ErrorReturned;
	ADDRINFOW* result = NULL;
	// Initialize Winsock**************************************************************************************
	if (!WSAIniciated)
	{
		iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
		if (iResult != 0) {
			XTrace(L"WSAStartup() failed with error:", iResult);
			return false;
		}
		else
		{
			WSAIniciated = true;
			XTrace(L"WSAStartup() success");
		}
	}
	// Resolve the server address and port**************************************************************************************

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	iResult = GetAddrInfoW(
		IPString,
		PortString,
		&hints,
		&result);
	if (iResult != 0) {
		XTrace(L"getaddrinfo() failed with error:", iResult);
		XTrace(this->WindowsErrorToString(iResult));
		WSACleanup();
		return false;
	}
	else
	{
		XTrace(L"getaddrinfo() success");
	}
	// Create a SOCKET for listen**************************************************************************************
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		XTrace(L"socket() failed with error:", iResult = WSAGetLastError());
		XTrace(L"Error:", this->WindowsErrorToString(iResult));
		FreeAddrInfoW(result);
		WSACleanup();
		return false;
	}
	else
	{
		XTrace(L"socket() success");
	}


	// Setup the TCP listening socket**************************************************************************************
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		XTrace(L"bind() failed with error:", iResult = WSAGetLastError());
		XTrace(L"Error:", this->WindowsErrorToString(iResult));
		FreeAddrInfoW(result);
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}
	else
	{
		XTrace(L"bind() success");
	}

	FreeAddrInfoW(result);




	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		XTrace(L"listen() failed with error:", iResult);
		closesocket(ListenSocket);
		WSACleanup();
		return false;
	}
	else
	{
		XTrace(L"listen() success");
	}

	//-------------------------
	// Create new event
	NetworkEvent_Listen = WSACreateEvent();
	NetworkEvent_Client = WSACreateEvent();
	//-------------------------
	// Associate event types FD_ACCEPT FD_CLOSE| FD_READ | FD_CONNECT
	// with the listening socket and NewEvent
	//WSAEventSelect(ListenSocket, NetworkEvent, FD_ACCEPT | FD_CLOSE | FD_READ | FD_CONNECT);
	return true;
}
public:	BOOL ClientIni(wchar_t* IPString, wchar_t* PortString)
{

	ADDRINFOW* result = NULL;
	int iResult = 0;
	size_t length = 0;
	//int ErrorReturned;
	//int size;
	const int DefaultStringSize = 256;
	//	char IPString[DefaultStringSize];
	//	char PortString[DefaultStringSize];
	//hostent* CurrentSocketHost;
	wchar_t CurrentSocketIP[DefaultStringSize];
	// Initialize Winsock**************************************************************************************
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		XTrace(L"WSAStartup failed with error:", iResult);
		return false;
	}
	// Resolve the server address and port**************************************************************************************
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = GetAddrInfoW(
		IPString,
		PortString,
		&hints,
		&result);
	if (iResult != 0) {
		XTrace(L"getaddrinfo failed with error:", iResult);
		WSACleanup();
		return false;
	}
	// Attempt to connect to an address until one succeeds**************************************************************************************
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		// Create a SOCKET for connecting to server
		ClientSocket = socket(ptr->ai_family, ptr->ai_socktype,
			ptr->ai_protocol);
		if (ClientSocket == INVALID_SOCKET) {
			XTrace(L"socket failed with error:", iResult);
			WSACleanup();
			return false;
		}
		// Get the local host information 
		ADDRINFOW CurrentSocketHost2;
		CurrentSocketHost2.ai_addr = 0;
		ADDRINFOW* Temp = &CurrentSocketHost2;
		PADDRINFOW* Temp2 = &Temp;
		//PADDRINFOW* CurrentSocketHost2;
		int retVal = GetAddrInfoW(NULL, NULL, NULL, Temp2);
		lstrcpyW(CurrentSocketIP, L"");
		//InetNtopW(AF_INET, (void*)&CurrentSocketHost->h_addr_list, CurrentSocketIP, WSS_IP_STRING_SIZE);

		wchar_t ip[WSS_IP_STRING_SIZE];
		wchar_t ip2[WSS_IP_STRING_SIZE];
		wchar_t port2[WSS_IP_STRING_SIZE];
		InetNtopW(AF_INET, &((sockaddr_in*)ptr->ai_addr)->sin_addr, ip, WSS_IP_STRING_SIZE);
		socketaddress_to_string(ptr->ai_addr, ip2, port2);
		//EM->AddMessage(L"host IP: ", CurrentSocketIP);
		XTrace(L"testing for connect IP: ", ip);
		this->XTrace(L"testing for connect IP:%s  - port:%s\n", ip2,port2);
		// Connect to server.**************************************************************************************
		iResult = connect(ClientSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
		if (iResult == SOCKET_ERROR) {
			closesocket(ClientSocket);
			ClientSocket = INVALID_SOCKET;
			continue;
		}
		break;
	}
	FreeAddrInfoW(result);

	if (ClientSocket == INVALID_SOCKET) {
		XTrace(L"Unable to connect to server!");
		WSACleanup();
		return false;
	}

	return true;
}
public: int testForEvents() {
	switch (Who_is_my_socket)
	{
	case WHO_IS_MY_SOCKET::SERVER:
		return testForServerEvents();
	case WHO_IS_MY_SOCKET::CLIENT:
		return testForClientEvents();
	}
	return FALSE;
}

	  /// <summary>
	  /// Evalua los Eventos registrados y reacciona apropiadamente.
	  /// FD_CONNECT (conexion rechazada para no bloquear). coloca STATUS en REQUESTING
	  /// FD_CONNECT (conexion aceptada por el servidor). coloca STATUS en CONNECTED y bConected=true,
	  ///permitiendo la transmision de datos.
	  /// FD_CLOSE Si la conexion se cierra. Destruye el socket cliente y crea uno nuevo para ponerlo
	  /// nuevamente en modo "escucha". Actualiza el STATUS a NONE y bConnected a false;
	  /// </summary>
	  /// <returns>Devuelve cero si ha funconado, SOCKET_ERROR si ha fallado</returns>
protected: int testForClientEvents() {

	lastWSAError = 0;
	WSANETWORKEVENTS NetworkEvents = { 0 };
	// Wait for one of the sockets to receive I/O notification and
	DWORD Event = WSAWaitForMultipleEvents(
		EventTotal,             //The number of event object handles in the array pointed to by lphEvents. 
		EventArray,             //A pointer to an array of event object handles.              
		FALSE,                  // If FALSE, the function returns when any of the event objects is signaled.
		TimeOutForEvents,       //The time-out interval, in milliseconds.
		FALSE                   //If FALSE, the thread is not placed in an alertable wait state and I/O completion routines are not executed.
	);
	switch (Event)
	{
	case WSA_WAIT_FAILED:
	{
		lastWSAError = WSAGetLastError();
		XTrace(L"WSAWaitForMultipleEvents() failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		return SOCKET_ERROR;
	}
	case WSA_WAIT_IO_COMPLETION:
		XTrace(L"WSAWaitForMultipleEvents() WSA_WAIT_IO_COMPLETION\n");
		return 0;
	case WSA_WAIT_TIMEOUT:
		XTrace(L"WSAWaitForMultipleEvents() WSA_WAIT_TIMEOUT\n");
		return 0;
	default:
		break;
	}
	XTrace(L"WSAWaitForMultipleEvents() is pretty damn OK!\n");
	iResult = WSAEnumNetworkEvents(
		ClientSocket,          //A descriptor identifying the socket.
		EventArray[Event - WSA_WAIT_EVENT_0],           //An optional handle identifying an associated event object to be reset.
		&NetworkEvents);                                //A structure that is filled with a record of network events that occurred and any associated error codes.
	if (iResult == SOCKET_ERROR)
	{
		lastWSAError = WSAGetLastError();
		XTrace(L"WSAEnumNetworkEvents() failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		return SOCKET_ERROR;
	}
	if (NetworkEvents.lNetworkEvents == 0) {
		XTrace(L"WSAEnumNetworkEvents() se ha disparado. Pero el evento no está definido.\n");
		return 0;
	}
	if (NetworkEvents.lNetworkEvents & FD_CLOSE) {
		XTrace(L"FD_CLOSE\n");
		closesocket(ClientSocket);
		WSACloseEvent(EventArray[0]);
		CreateClientSocket();
		bConnected = FALSE;
		ClientState = STATE::ATTEMPTING_CONNECT_TO_SERVER;
		return 0;
	}
	if (NetworkEvents.lNetworkEvents & FD_CONNECT) {
		if (NetworkEvents.iErrorCode[FD_CONNECT_BIT] != 0) {
			XTrace(L"FD_CONNECT conexion rechazada\n");
			bConnected = FALSE;
			ClientState = STATE::ATTEMPTING_CONNECT_TO_SERVER;
			return 0;
		}
		else
		{
			XTrace(L"FD_CONNECT\n");
			bConnected = TRUE;
			ClientState = STATE::CONNECTED;
			return 0;
		}
	}
	if (NetworkEvents.lNetworkEvents & FD_READ) {
		if (NetworkEvents.iErrorCode[FD_READ_BIT] != 0) {
			XTrace(L"FD_READ ha devuelto error %u:\n", NetworkEvents.iErrorCode[FD_READ_BIT]);
			return 0;
		}
		else {
			XTrace(L"FD_READ\n");
			FD_READ_response();
			return 0;
		}
	}
	return 0;
	return TRUE;
}
protected: int testForServerEvents() {
	lastWSAError = 0;
	WSANETWORKEVENTS NetworkEvents = { 0 };
	// Wait for one of the sockets to receive I/O notification and
	DWORD Event = WSAWaitForMultipleEvents(
		ClientIndex,             //The number of event object handles in the array pointed to by lphEvents. 
		EventArray,             //A pointer to an array of event object handles.              
		FALSE,                  // If FALSE, the function returns when any of the event objects is signaled.
		TimeOutForEvents,       //The time-out interval, in milliseconds.
		FALSE                   //If FALSE, the thread is not placed in an alertable wait state and I/O completion routines are not executed.
	);
	switch (Event)
	{
	case WSA_WAIT_FAILED:
	{
		lastWSAError = WSAGetLastError();
		XTrace(L"WSAWaitForMultipleEvents() failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		return SOCKET_ERROR;
	}
	case WSA_WAIT_IO_COMPLETION:
		XTrace(L"WSAWaitForMultipleEvents() WSA_WAIT_IO_COMPLETION\n");
		return 0;
	case WSA_WAIT_TIMEOUT:
		XTrace(L"WSAWaitForMultipleEvents() WSA_WAIT_TIMEOUT\n");
		return 0;
	default:
		break;
	}
	iResult = WSAEnumNetworkEvents(
		SocketArray[Event - WSA_WAIT_EVENT_0],          //A descriptor identifying the socket.
		EventArray[Event - WSA_WAIT_EVENT_0],           //An optional handle identifying an associated event object to be reset.
		&NetworkEvents);                                //A structure that is filled with a record of network events that occurred and any associated error codes.
	if (iResult == SOCKET_ERROR)
	{
		lastWSAError = WSAGetLastError();
		XTrace(L"WSAEnumNetworkEvents() failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));

		return SOCKET_ERROR;
	}
	if (NetworkEvents.lNetworkEvents & FD_ACCEPT) {
		XTrace(L"SocketArray[%u]FD_ACCEPT\n", Event - WSA_WAIT_EVENT_0);
		return FD_ACCEPT_response();
	}
	if (NetworkEvents.lNetworkEvents & FD_CLOSE) {
		XTrace(L"SocketArray[%u]FD_CLOSE\n", Event - WSA_WAIT_EVENT_0);
		FD_CLOSE_response(Event - WSA_WAIT_EVENT_0);
		return TRUE;
	}
	if (NetworkEvents.lNetworkEvents & FD_READ) {
		XTrace(L"SocketArray[%u]FD_READ\n", Event - WSA_WAIT_EVENT_0);
		FD_READ_response(Event - WSA_WAIT_EVENT_0);
		return TRUE;
	}
	XTrace(L"SocketArray[%u]FD_XXXX WSAEnumNetworkEvents() ha devuelto:%u\n", Event - WSA_WAIT_EVENT_0, NetworkEvents.lNetworkEvents);
	return TRUE;
}

		 /// <summary>
		 /// FD_ACCEPT event response.
		 /// Accept incoming client. Inicialize events, update state arrays and clear Buffers for send/recv data
		 /// </summary>
		 /// <returns>TRUE if succeed. FALSE if fails.
		 /// In case of error, lastWSAError stores the value returned by WSAGetLastError().
		 ///</returns>
protected: BOOL FD_ACCEPT_response() {

	lastWSAError = 0;
	//Create a new ClientSocket to accept the requested conection
	if (ClientIndex >= WSA_MAXIMUM_WAIT_EVENTS)
	{
		XTrace(L"FD_ACCEPT: Top Client conections researched %u:\n", ClientIndex);
		return FALSE;
	}
	int sizeAddress = sizeof(AddressArray[ClientIndex]);
	SocketArray[ClientIndex] = accept(SocketArray[ServerIndex], &AddressArray[ClientIndex], &sizeAddress);
	if (SocketArray[ClientIndex] == INVALID_SOCKET) {
		lastWSAError = WSAGetLastError();
		XTrace(L"accept() failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		return FALSE;
	}
	// Associate event types FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE*****************************************
	// with the listening socket and NewEvent
	// Create new event
	EventArray[ClientIndex] = WSACreateEvent();
	iResult = WSAEventSelect(SocketArray[ClientIndex], EventArray[ClientIndex], FD_CONNECT | FD_CLOSE | FD_READ | FD_WRITE);
	if (iResult == SOCKET_ERROR) {

		lastWSAError = WSAGetLastError();
		XTrace(L"WSAEventSelect failed in SocketArray[%u] with error %u: %s\n", ClientIndex, lastWSAError, WindowsErrorToString(lastWSAError));
		closesocket(SocketArray[ClientIndex]);
		return FALSE;
	}
	//Update Arrays to new client
	StateArray[ServerIndex] = STATE::LISTENING;
	StateArray[ClientIndex] = STATE::CONNECTED;
	BufferReceived[ClientIndex][0] = 0;
	ReceivedBytes[ClientIndex] = 0;
	OverflowAlert[ClientIndex] = FALSE;
	ClientIndex++;
	XTrace(L"accept succeed\n");
	return TRUE;
}

		 /// <summary>
		 /// FD_CLOSES event response.
		 /// Closes the client socket. And update the internal Arrays to fill in the gap.
		 /// </summary>
		 /// <param name="SocketArrayIndex">Index of the closing socket in the internal SocketArray[]</param>
protected:void FD_CLOSE_response(int SocketArrayIndex) {

	closesocket(SocketArray[SocketArrayIndex]);
	WSACloseEvent(EventArray[SocketArrayIndex]);

	for (unsigned int index = SocketArrayIndex; index < (ClientIndex - 1); index++)
	{
		SocketArray[index] = SocketArray[index + 1];
		EventArray[index] = EventArray[index + 1];
		AddressArray[index] = AddressArray[index + 1];
		StateArray[index] = StateArray[index + 1];
		ReceivedBytes[index] = ReceivedBytes[index + 1];
		OverflowAlert[index] = OverflowAlert[index + 1];
		for (unsigned int byte = 0; byte < WSA_MAXIMUM_WAIT_EVENTS + 1; byte++) {
			BufferReceived[index][byte] = BufferReceived[index + 1][byte];
		}
	}
	ClientIndex--;
}

		 /// <summary>
		 /// FD_READ event response.
		 /// Read the incoming data and put a zero at the end
		 /// activates or deactivates the OverflowAlert if the incoming data is bigger or smaller than buffer
		 /// </summary>
protected:int FD_READ_response() {
	ReceivedBytesClient = recv(ClientSocket, (char*)BufferReceivedClient, DATA_BUFSIZE, 0);
	if (ReceivedBytesClient == SOCKET_ERROR) {

		lastWSAError = WSAGetLastError();
		XTrace(L"recv() failed with error %u: %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		return SOCKET_ERROR;
	}
	if (ReceivedBytesClient >= DATA_BUFSIZE)
	{
		OverflowAlertClient = TRUE;
	}
	else {

		OverflowAlertClient = FALSE;
	}
	//El tamaño del buffer es DATA_BUFSIZE+1 para poder colocar un cero al final
	BufferReceivedClient[ReceivedBytesClient] = 0;
	GetBinaryDataFromSocketBuffer(&sharedDataClient);
	return 0;
}

		 /// <summary>
		 /// FD_READ event response.
		 /// Read the incoming data and put a zero at the end
		 /// activates or deactivates the OverflowAlert if the incoming data is bigger or smaller than buffer
		 /// </summary>
		 /// <param name="SocketArrayIndex">Index of the receiving socket in the internal SocketArray[]</param>
protected:void FD_READ_response(int SocketArrayIndex) {
	ReceivedBytes[SocketArrayIndex] = recv(SocketArray[SocketArrayIndex], (char*)BufferReceived[SocketArrayIndex], DATA_BUFSIZE, 0);
	if (ReceivedBytes[SocketArrayIndex] >= DATA_BUFSIZE)
	{
		OverflowAlert[SocketArrayIndex] = TRUE;
	}
	else {
		OverflowAlert[SocketArrayIndex] = FALSE;
	}
	//El tamaño del buffer es DATA_BUFSIZE+1 para poder colocar un cero al final
	BufferReceived[SocketArrayIndex][ReceivedBytes[SocketArrayIndex]] = 0;
	//Obtiene los dator binarios del buffer leido y los guarda en la variable miembro sharedData.
	GetBinaryDataFromSocketBuffer(SocketArrayIndex, &(this->sharedData[SocketArrayIndex]));
}
		 /// <summary>
		 /// Extrae los datos recibidos de una transmision TCP, elimina la "basura" al principio 
		 /// y al final de la transmision
		 ///, verifica su fiabilidad y los clasifica en la clase SharedClass
		 /// el tamaño máximo para el buffer sería de 65535, puesto que usamos un "unsigned short" como indice
		 /// </summary>
		 /// <param name="BufferArrayIndex">Indice del Socket usado</param>
		 /// <returns>TRUE if succeed.There is new data avaliable in member variable "sharedData"
		 ///</returns>

		 /// <summary>
		 /// Extrae los datos recibidos de una transmision TCP, elimina la "basura" al principio 
		 /// y al final de la transmision
		 ///, verifica su fiabilidad y los clasifica en la clase SharedClass
		 /// el tamaño máximo para el buffer sería de 65535, puesto que usamos un "unsigned short" como indice
		 /// </summary>
		 /// <param name="BufferArrayIndex">Indice del Socket usado</param>
		 /// <returns>TRUE if succeed.There is new data avaliable in member variable "sharedData"
		 ///</returns>
protected:bool GetBinaryDataFromSocketBuffer(SharedClass* receivedSharedObject)
{
	bool returnValue = false;
	unsigned short mark_index = 0, buffer_index = 0, sharedDataStart = 0;
	unsigned short i, endLoop;
	unsigned long checksum = 0;
	SharedClass sharedData;

	if (ReceivedBytesClient < sizeof(SharedClass))
		return false;
	while (buffer_index < ReceivedBytesClient)
	{
		while (BufferReceivedClient[buffer_index] == sharedData.Mark[mark_index])
		{
			mark_index++;
			buffer_index++;
			if (mark_index == (sizeof(sharedData.Mark)))
			{
				//Marca encontrada
				if ((ReceivedBytesClient - buffer_index) <= (sizeof(SharedClass) - sizeof(sharedData.Mark)))
				{
					//Hay datos suficientes en el buffer para albergar un objeto SharedData
					i = buffer_index;
					sharedData.checksum = BufferReceivedClient[i++];
					sharedData.checksum += BufferReceivedClient[i++] << 8;
					sharedData.checksum += BufferReceivedClient[i++] << 16;
					sharedData.checksum += BufferReceivedClient[i++] << 24;

					sharedData.data1 = BufferReceivedClient[i++];
					sharedData.data2 = BufferReceivedClient[i++];

					sharedData.data3 = BufferReceivedClient[i++];
					sharedData.data3 += BufferReceivedClient[i++] << 8;
					sharedData.data3 += BufferReceivedClient[i++] << 16;
					sharedData.data3 += BufferReceivedClient[i++] << 24;

					sharedData.data4 = BufferReceivedClient[i++];
					sharedData.data4 += BufferReceivedClient[i++] << 8;
					sharedData.data4 += BufferReceivedClient[i++] << 16;
					sharedData.data4 += BufferReceivedClient[i++] << 24;
				//Verificar checksum
				endLoop = i;
				for (i = buffer_index + sizeof(sharedData.checksum), checksum = 0; i < endLoop; i++)
				{
					checksum += BufferReceivedClient[i];
				}
				if (checksum == sharedData.checksum)
				{
					//los datos son válidos
					receivedSharedObject->checksum = sharedData.checksum;
					receivedSharedObject->data1 = sharedData.data1;
					receivedSharedObject->data2 = sharedData.data2;
					receivedSharedObject->data3 = sharedData.data3;
					receivedSharedObject->data4 = sharedData.data4;
					returnValue = true;
					//En este punto devería comprobar si hay espacio en la clase para alvergar otro paquete entrante
					//que pueda estar en la cola del buffer
					//if(hay espacio en la clase OBDC_Server para más datos entrantes)
					//buffer_index = i;
					//else
					return true;
				}
				else
				{
					//Si no se supera el checksum, existe la posibilidad de que quede un paquete integro 
					// en la cola del buffer y que éste empiece justo donde acaba el sharedData.Mark anterior
					//así que no es necesario actualizar buffer_index (ya está apuntando al final de dicho sharedData.Mark)
					//, solo restarle 1 para poder continuar con el bucle
					buffer_index--;
					break;
				}
				}
			}
			else if (buffer_index >= ReceivedBytesClient)
			{
				return returnValue;
			}
		}
		mark_index = 0;
		buffer_index++;

	}
	// buffer ends
	return returnValue;
}
protected:bool GetBinaryDataFromSocketBuffer(int BufferArrayIndex, SharedClass* receivedSharedObject)
{
	bool returnValue = false;
	unsigned short mark_index = 0, buffer_index = 0, sharedDataStart = 0;
	unsigned short i, endLoop;
	unsigned long checksum = 0;
	SharedClass sharedData;

	if (ReceivedBytes[BufferArrayIndex] < sizeof(SharedClass))
		return false;
	while (buffer_index < ReceivedBytes[BufferArrayIndex])
	{
		while (BufferReceived[BufferArrayIndex][buffer_index] == sharedData.Mark[mark_index])
		{
			mark_index++;
			buffer_index++;
			if (mark_index == (sizeof(sharedData.Mark)))
			{
				//Marca encontrada
				if ((ReceivedBytes[BufferArrayIndex] - buffer_index) <= (sizeof(SharedClass) - sizeof(sharedData.Mark)))
				{
					//Hay datos suficientes en el buffer para albergar un objeto SharedData
					i = buffer_index;
					sharedData.checksum = BufferReceived[BufferArrayIndex][i++];
					sharedData.checksum += BufferReceived[BufferArrayIndex][i++] << 8;
					sharedData.checksum += BufferReceived[BufferArrayIndex][i++] << 16;
					sharedData.checksum += BufferReceived[BufferArrayIndex][i++] << 24;

					sharedData.data1 = BufferReceived[BufferArrayIndex][i++];
					sharedData.data2 = BufferReceived[BufferArrayIndex][i++];

					sharedData.data3 = BufferReceived[BufferArrayIndex][i++];
					sharedData.data3 += BufferReceived[BufferArrayIndex][i++] << 8;
					sharedData.data3 += BufferReceived[BufferArrayIndex][i++] << 16;
					sharedData.data3 += BufferReceived[BufferArrayIndex][i++] << 24;

					sharedData.data4 = BufferReceived[BufferArrayIndex][i++];
					sharedData.data4 += BufferReceived[BufferArrayIndex][i++] << 8;
					sharedData.data4 += BufferReceived[BufferArrayIndex][i++] << 16;
					sharedData.data4 += BufferReceived[BufferArrayIndex][i++] << 24;
				}
				//Verificar checksum

				endLoop = i;
				for (i = buffer_index + sizeof(sharedData.checksum), checksum = 0; i < endLoop; i++)
				{
					checksum += BufferReceived[BufferArrayIndex][i];
				}
				if (checksum == sharedData.checksum)
				{
					//los datos son válidos
					receivedSharedObject->checksum = sharedData.checksum;
					receivedSharedObject->data1 = sharedData.data1;
					receivedSharedObject->data2 = sharedData.data2;
					receivedSharedObject->data3 = sharedData.data3;
					receivedSharedObject->data4 = sharedData.data4;
					returnValue = true;
					//En este punto devería comprobar si hay espacio en la clase para alvergar otro paquete entrante
					//que pueda estar en la cola del buffer
					//if(hay espacio en la clase OBDC_Server para más datos entrantes)
					//buffer_index = i;
					//else
					return true;
				}
				else
				{
					//Si no se supera el checksum, existe la posibilidad de que quede un paquete integro 
					// en la cola del buffer y que éste empiece justo donde acaba el sharedData.Mark anterior
					//así que no es necesario actualizar buffer_index (ya está apuntando al final de dicho sharedData.Mark)
					//, solo restarle 1 para poder continuar con el bucle
					buffer_index--;
					break;
				}
			}
			else if (buffer_index >= ReceivedBytes[BufferArrayIndex])
			{
				return returnValue;
			}
		}
		mark_index = 0;
		buffer_index++;

	}
	// buffer ends
	return returnValue;
}
/// <summary>
/// Actualiza los arreglos pIfList con la lista de los Adaptadores de Red inalambricos
/// y el arreglo pBssList con el listado de las redes wifis detectadas por el primer adaptador 
/// de esa lista.
/// </summary>
/// <param name="ComboAdapter">Manejador del ComboBox que se rellenará con el listado de adaptadores a wifi. Puede ser NULL</param>
/// <param name="ComboWifi">Manejador del ComboBox que se rellenará con el listado de redes wifi. Puede ser NULL</param>
/// <param name="ListAdapter">Manejador del Lisbox que se rellenará con detalles del primer adaptador a wifi. Puede ser NULL</param>
/// <param name="ListWifi">Manejador del ComboBox que se rellenará con detalles de la primera red wifi. Puede ser NULL</param>
/// <returns>ERROR_SUCCESS si ha funcionado, otro error de WlanAPI si algo a ido mal</returns>
public: int EnumerarWifis(HWND ComboAdapter, HWND ComboWifi, HWND ListAdapter, HWND ListWifi) {

	PWLAN_INTERFACE_INFO			pIfInfo = NULL;
	PWLAN_AVAILABLE_NETWORK			pBssEntry = NULL;
	DWORD dwCurVersion = 0;
	DWORD dwResult = 0;
	DWORD dwRetVal = 0;
	int iRet = 0;
	WCHAR GuidString[40] = { 0 };

	unsigned int i, j, k;

	int iRSSI = 0;
	/* variables used for WlanEnumInterfaces  */

	//Borra el contenido de los ComboBox y ListBox
	if (ComboAdapter != NULL) SendMessage(ComboAdapter, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	if (ComboWifi != NULL) SendMessage(ComboWifi, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	if (ListAdapter != NULL) SendMessage(ListAdapter, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	if (ListWifi != NULL) SendMessage(ListWifi, (UINT)CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);

	//Se inicializa el sistema WlanAPI solo si no se ha inicializado en una llamada prévia a EnumerarWifis()
	if (hClientHandle == NULL) {
		dwResult = WlanOpenHandle(WLAN_API_MAKE_VERSION(2, 0), NULL, &dwCurVersion, &hClientHandle);
		if (dwResult != ERROR_SUCCESS) {
			XTrace(L"WlanOpenHandle failed with error: ", dwResult);
			// FormatMessage can be used to find out why the function failed
			WlanOpenHandle_ShowError(dwResult);
			return dwResult;
		}
	}

	//Si ha habido una llamada prévia a WlanEnumInterfaces, se borra la memoria de la llamada anterior
	if (pIfList != NULL) {
		WlanFreeMemory(pIfList);
		pIfList = NULL;
	}

	//Se guardan en el arreglo global "pIfList" los dispositivos mediante los cuales se puede acceder a una Wifi.
	dwResult = WlanEnumInterfaces(hClientHandle, NULL, &pIfList);
	if (dwResult != ERROR_SUCCESS) {
		XTrace(L"WlanEnumInterfaces failed with error: %u\n", dwResult);
		// FormatMessage can be used to find out why the function failed
		return dwResult;
	}
	else {
		XTrace(L"Num Entries: ", pIfList->dwNumberOfItems);
		XTrace(L"Current Index: ", pIfList->dwIndex);
		for (i = 0; i < (int)pIfList->dwNumberOfItems; i++) {
			if ((i == 0) && (ListAdapter != NULL)) {
				this->OUTPUT = OUTPUT_t::LISTBOX;
				this->ListBox_forOutput = ListAdapter;
			}
			pIfInfo = (WLAN_INTERFACE_INFO*)&pIfList->InterfaceInfo[i];
			iRet = StringFromGUID2(pIfInfo->InterfaceGuid, (LPOLESTR)&GuidString, 39);
			if (iRet == 0)
				XTrace(L"StringFromGUID2 failed\n");
			else {
				XTrace(L"  InterfaceGUID[%d]=%s", i, GuidString);
			}
			XTrace(L"  Interface Description[%d]=%s", i,pIfInfo->strInterfaceDescription);
			if (ComboAdapter != NULL)
			{
				SendMessage(ComboAdapter, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)pIfInfo->strInterfaceDescription);

				SendMessage(ComboAdapter, CB_SETMINVISIBLE, (WPARAM)9, (LPARAM)0);
				SendMessage(ComboAdapter, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			}
			switch (pIfInfo->isState) {
			case wlan_interface_state_not_ready:
				XTrace(L"  Interface State[%i]:\t Not ready",i);
				break;
			case wlan_interface_state_connected:
				XTrace(L"  Interface State[%i]:\t Connected", i);
				break;
			case wlan_interface_state_ad_hoc_network_formed:
				XTrace(L"  Interface State[%i]:\t First node in a ad hoc network",i);
				break;
			case wlan_interface_state_disconnecting:
				XTrace(L"  Interface State[%i]:\t Disconnecting",i);
				break;
			case wlan_interface_state_disconnected:
				XTrace(L"  Interface State[%i]:\t Not connected",i);
				break;
			case wlan_interface_state_associating:
				XTrace(L"  Interface State[%i]:\t Attempting to associate with a network",i);
				break;
			case wlan_interface_state_discovering:
				XTrace(L"  Interface State[%i]:\t Auto configuration is discovering settings for the network",i);
				break;
			case wlan_interface_state_authenticating:
				XTrace(L"  Interface State[%i]:\t In process of authenticating",i);
				break;
			default:
				XTrace(L"  Interface State[%i]:\t Unknown state: %i", i, pIfInfo->isState);
				break;
			}

			this->OUTPUT = OUTPUT_t::DEBUG;
		}
	}

	//Si ha habido una llamada prévia a WlanGetAvailableNetworkList, se borra la memoria de la llamada anterior
	if (pBssList != NULL) {
		WlanFreeMemory(pBssList);
		pBssList = NULL;
	}
	//Se guardan en el arreglo global "pBssList" las Wifis detectadas
	dwResult = WlanGetAvailableNetworkList(hClientHandle,
		&pIfInfo->InterfaceGuid,
		0,
		NULL,
		&pBssList);
	if (dwResult != ERROR_SUCCESS) {
		XTrace(L"WlanGetAvailableNetworkList failed with error:",
			dwResult);
		return dwResult;
		// You can use FormatMessage to find out why the function failed
	}

	//Muestra los detalles de todas las wifis en la ventana del depurador
	XTrace(L"\n");
	XTrace(L"WLAN_AVAILABLE_NETWORK_LIST for this interface");

	XTrace(L"  Num Entries: %i", pBssList->dwNumberOfItems);

	for (j = 0; j < pBssList->dwNumberOfItems; j++) {
		if ((j == 0) && (ListWifi != NULL)) {
			this->OUTPUT = OUTPUT_t::LISTBOX;
			this->ListBox_forOutput = ListWifi;
		}
		pBssEntry =
			(WLAN_AVAILABLE_NETWORK*)&pBssList->Network[j];

		XTrace(L"  Profile Name[%i]:%s ", j, pBssEntry->strProfileName);

		if (pBssEntry->dot11Ssid.uSSIDLength == 0)
		{
			XTrace(L"  SSID[%i]:-NONAME-", j);
			if (ComboWifi != NULL)
			{
				SendMessage(ComboWifi, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)"-NONAME-");

				SendMessage(ComboWifi, CB_SETMINVISIBLE, (WPARAM)9, (LPARAM)0);
				SendMessage(ComboWifi, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			}
		}
		else {
			wchar_t SSID[DOT11_SSID_MAX_LENGTH + 1];
			for (k = 0; k < pBssEntry->dot11Ssid.uSSIDLength; k++) {
				SSID[k] = pBssEntry->dot11Ssid.ucSSID[k];
			}
			SSID[k] = 0;
			XTrace(L"  SSID[%i]:%s", j, SSID);
			if (ComboWifi != NULL)
			{
				SendMessage(ComboWifi, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)SSID);

				SendMessage(ComboWifi, CB_SETMINVISIBLE, (WPARAM)9, (LPARAM)0);
				SendMessage(ComboWifi, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
			}
		}
		switch (pBssEntry->dot11BssType) {
		case dot11_BSS_type_infrastructure:
			XTrace(L"  BSS Network type[%i]\t Infrastructure:%i\n",j, pBssEntry->dot11BssType);
			break;
		case dot11_BSS_type_independent:
			XTrace(L"  BSS Network type[%i]:\t Infrastructure:%i\n", j,pBssEntry->dot11BssType);
			break;
		default:
			XTrace(L"  BSS Network type[%i]:\t Infrastructure:Other (%i)\n", j,pBssEntry->dot11BssType);
			break;
		}

		XTrace(L"  Number of BSSIDs[%i]:\t %i\n", j, pBssEntry->uNumberOfBssids);

		if (pBssEntry->bNetworkConnectable)
			XTrace(L"  Connectable[%i]:\t Yes\n", j);
		else {
			XTrace(L"  Connectable[%i]:\t No\n", j);
			XTrace(L"  Not connectable WLAN_REASON_CODE value[[%i]:\t%i \n", j,
				pBssEntry->wlanNotConnectableReason);
		}

		XTrace(L"  Number of PHY types supported[%i]:\t %i \n", j, pBssEntry->uNumberOfPhyTypes);

		if (pBssEntry->wlanSignalQuality == 0)
			iRSSI = -100;
		else if (pBssEntry->wlanSignalQuality == 100)
			iRSSI = -50;
		else
			iRSSI = -100 + (pBssEntry->wlanSignalQuality / 2);

		XTrace(L"  Signal Quality[%i]:\t %i  (RSSI:%i  dBm)", j, pBssEntry->wlanSignalQuality, iRSSI);

		if (pBssEntry->bSecurityEnabled)
			XTrace(L"  Security Enabled[%i]:\t Yes\n",j);
		else
			XTrace(L"  Security Enabled[%i]:\t No\n", j);

		switch (pBssEntry->dot11DefaultAuthAlgorithm) {
		case DOT11_AUTH_ALGO_80211_OPEN:
			XTrace(L"  Default AuthAlgorithm[%i]=\t802.11 Open (code:%i)", j, pBssEntry->dot11DefaultAuthAlgorithm);
			break;
		case DOT11_AUTH_ALGO_80211_SHARED_KEY:
			XTrace(L"  Default AuthAlgorithm[%i]=\t802.11 Shared (code:%i)", j, pBssEntry->dot11DefaultAuthAlgorithm);
			break;
		case DOT11_AUTH_ALGO_WPA:
			XTrace(L"  Default AuthAlgorithm[%i]=\tWPA (code:%i)", j, pBssEntry->dot11DefaultAuthAlgorithm);
			break;
		case DOT11_AUTH_ALGO_WPA_PSK:
			XTrace(L"  Default AuthAlgorithm[%i]=\tWPA-PSK (code:%i)", j, pBssEntry->dot11DefaultAuthAlgorithm);
			break;
		case DOT11_AUTH_ALGO_WPA_NONE:
			XTrace(L"  Default AuthAlgorithm[%i]=\tWPA-None (code:%i)", j, pBssEntry->dot11DefaultAuthAlgorithm);
			break;
		case DOT11_AUTH_ALGO_RSNA:
			XTrace(L"  Default AuthAlgorithm[%i]=\tRSNA (code:%i)", j, pBssEntry->dot11DefaultAuthAlgorithm);
			break;
		case DOT11_AUTH_ALGO_RSNA_PSK:
			XTrace(L"  Default AuthAlgorithm[%i]=\tRSNA with PSK (code:%i)", j, pBssEntry->dot11DefaultAuthAlgorithm);
			break;
		default:
			XTrace(L"  Default AuthAlgorithm[%i]=\tOther (code:%i)", j, pBssEntry->dot11DefaultAuthAlgorithm);
			break;
		}

		switch (pBssEntry->dot11DefaultCipherAlgorithm) {
		case DOT11_CIPHER_ALGO_NONE:
			XTrace(L"  Default CipherAlgorithm[%i]: None (0x%X)", j, pBssEntry->dot11DefaultCipherAlgorithm);
			break;
		case DOT11_CIPHER_ALGO_WEP40:
			XTrace(L"  Default CipherAlgorithm[%i]: WEP-40 (0x%X)", j, pBssEntry->dot11DefaultCipherAlgorithm);
			break;
		case DOT11_CIPHER_ALGO_TKIP:
			XTrace(L"  Default CipherAlgorithm[%i]: TKIP (0x%X)", j, pBssEntry->dot11DefaultCipherAlgorithm);
			break;
		case DOT11_CIPHER_ALGO_CCMP:
			XTrace(L"  Default CipherAlgorithm[%i]: CCMP (0x%X)", j, pBssEntry->dot11DefaultCipherAlgorithm);
			break;
		case DOT11_CIPHER_ALGO_WEP104:
			XTrace(L"  Default CipherAlgorithm[%i]: WEP-104 (0x%X)", j, pBssEntry->dot11DefaultCipherAlgorithm);
			break;
		case DOT11_CIPHER_ALGO_WEP:
			XTrace(L"  Default CipherAlgorithm[%i]: WEP (0x%X)", j, pBssEntry->dot11DefaultCipherAlgorithm);
			break;
		default:
			XTrace(L"  Default CipherAlgorithm[%i]: Other (0x%X)", j, pBssEntry->dot11DefaultCipherAlgorithm);
			break;
		}

		XTrace(L"  Flags[%i]:\t 0x%X",j, pBssEntry->dwFlags);
		if (pBssEntry->dwFlags) {
			if (pBssEntry->dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED)
				XTrace(L" - Currently connected");
			if (pBssEntry->dwFlags & WLAN_AVAILABLE_NETWORK_HAS_PROFILE)
			{
				XTrace(L" - Has profile");
			}
		}
		XTrace(L"\n");


		this->OUTPUT = OUTPUT_t::DEBUG;
	}

	return ERROR_SUCCESS;
}

/// <summary>
/// Actualiza el contenido del Arreglo pBssList[] con las Wifis detectadas por el Adaptador 
/// guardado en la posicion "AdapterIndex" del arreglo pIfList->InterfaceInfo[] que se ha rellenado previamente
/// con una llamada a WinSockSocket3::EnumerarWifis().
/// UpdateWifiList() usa la función WlanGetAvailableNetworkList() de WlanAPI de windows.
/// </summary>
/// <param name="AdapterIndex">numero del indice del arreglo pIfInfo[] donde se ha memorizado el adaptador de Wifi, 
/// normalmente AdapteIndex es igual a cero, puesto que solo suele
/// haber un adaptador Wifi en un PC.</param>
public: DWORD UpdateWifiList(int AdapterIndex) {
	DWORD dwResult = NULL;
	PWLAN_INTERFACE_INFO			pIfInfo = NULL;
	//Si ha habido una llamada prévia a WlanGetAvailableNetworkList, se borra la memoria de la llamada anterior
	if (pIfList == NULL) {
		return ERROR_INVALID_PARAMETER;
	}
	if (pBssList != NULL) {
		WlanFreeMemory(pBssList);
		pBssList = NULL;
	}
	pIfInfo = &pIfList->InterfaceInfo[AdapterIndex];
	//Se guardan en el arreglo global "pBssList" las Wifis detectadas
	dwResult = WlanGetAvailableNetworkList(hClientHandle,
		&pIfInfo->InterfaceGuid,
		0,
		NULL,
		&pBssList);
	if (dwResult != ERROR_SUCCESS) {
		XTrace(L"WlanGetAvailableNetworkList failed with error:",
			dwResult);
		return dwResult;
		// You can use FormatMessage to find out why the function failed
	}
	return ERROR_SUCCESS;
}
/// <summary>
/// Muestra los detalles del Adaptador Wifi en un control ListBox. 
/// </summary>
/// <param name="AdapterIndex">numero de adaptador de Wifi del arreglo pIfList->InterfaceInfo[] que se ha rellenado previamente
/// con una llamada a WinSockSocket3::EnumerarWifis()., normalmente AdapteIndex es igual a cero, puesto que solo suele
/// haber un adaptador Wifi en un PC.</param>
/// <param name="ListBox">Manejador de ventana del control Lisbox al que se enviarán mensajes "LB_RESETCONTENT" y "LB_ADDSTRING"
/// para mostrar los detalles del adaptador Wifi. Si es NULL, los detalles se envían a la ventana de depuración</param>
public: void GetAdapterDetails(int AdapterIndex, HWND ListBox)
{
	PWLAN_INTERFACE_INFO			pIfInfo = NULL;
	//Prepara el ListBox
	if (ListBox != NULL) {
		ListBox_forOutput = ListBox;
		OUTPUT = OUTPUT_t::LISTBOX;
		SendMessage(ListBox_forOutput, LB_RESETCONTENT, 0, 0);
	}

	WCHAR GuidString[40] = { 0 };
	if (pIfList == NULL)return;
	pIfInfo = (WLAN_INTERFACE_INFO*)&pIfList->InterfaceInfo[AdapterIndex]; 
	//XTrace(L"  Interface Index[%i]", AdapterIndex);
	int iRet = StringFromGUID2(pIfInfo->InterfaceGuid, (LPOLESTR)&GuidString, 39);
	// For c rather than C++ source code, the above line needs to be
	// iRet = StringFromGUID2(&pIfInfo->InterfaceGuid, (LPOLESTR) &GuidString, 39); 
	if (iRet == 0)
		XTrace(L"StringFromGUID2 failed\n");
	else {
		XTrace(L"  InterfaceGUID[%i]=\t%s", AdapterIndex, GuidString);
	}
	XTrace(L"  Interface Description[%i]=\t%s", AdapterIndex,
		pIfInfo->strInterfaceDescription);
	XTrace(L"\n");
	switch (pIfInfo->isState) 
	{
	case wlan_interface_state_not_ready:
		XTrace(L"  Interface State[%i]=\tNot ready", AdapterIndex);
		break;
	case wlan_interface_state_connected:
		XTrace(L"  Interface State[%i]=\tConnected", AdapterIndex);
		break;
	case wlan_interface_state_ad_hoc_network_formed:
		XTrace(L"  Interface State[%i]=\tFirst node in a ad hoc network", AdapterIndex);
		break;
	case wlan_interface_state_disconnecting:
		XTrace(L"  Interface State[%i]=\tDisconnecting", AdapterIndex);
		break;
	case wlan_interface_state_disconnected:
		XTrace(L"  Interface State[%i]=\tNot connected", AdapterIndex);
		break;
	case wlan_interface_state_associating:
		XTrace(L"  Interface State[%i]=\tAttempting to associate with a network", AdapterIndex);
		break;
	case wlan_interface_state_discovering:
		XTrace(L"  Interface State[%i]=\tAuto configuration is discovering settings for the network", AdapterIndex);
		break;
	case wlan_interface_state_authenticating:
		XTrace(L"  Interface State[%i]=\tIn process of authenticating", AdapterIndex);
		break;
	default:
		XTrace(L"  Interface State[%i]=\tUnknown state: %i", AdapterIndex, pIfInfo->isState);
		break;

	}
	OUTPUT = OUTPUT_t::DEBUG;
}

/// <summary>
/// Muestra los detalles de la red WiFi en un control ListBox. 
/// </summary>
/// <param name="AdapterIndex">numero de Wifi del arreglo pBssList->Network[] que se ha rellenado previamente
/// con una llamada a WinSockSocket3::EnumerarWifis()., o con WinSockSocket3::UpdateWifiList()</param>
/// <param name="ListBox">Manejador de ventana del control Lisbox al que se enviarán mensajes "LB_RESETCONTENT" y "LB_ADDSTRING"
/// para mostrar los detalles de la red Wifi. Si es NULL, los detalles se envían a la ventana de depuración</param>
/// <returns>Una estructura WLAN_AVAILABLE_NETWORK con los detalles de la Wifi solicitada</returns>
public: WLAN_AVAILABLE_NETWORK* GetWifiDetails(int wifiIndex,HWND ListBox) {

	PWLAN_AVAILABLE_NETWORK			pBssEntry = NULL;
	//Prepara el ListBox
	if (ListBox != NULL)
	{
		ListBox_forOutput = ListBox;
		OUTPUT = OUTPUT_t::LISTBOX;
		SendMessage(ListBox_forOutput, LB_RESETCONTENT, 0, 0);
	}

	unsigned int k;
	int iRSSI = 0;

	if (pBssList == NULL) {
		XTrace(L"GetWifiDetails() Error - pBssList == NULL. No se ha inicializado el sistema con una llamada a EnumerarWifis()");
		return 0;
	}
	/* variables used for WlanEnumInterfaces  */
	pBssEntry =
		(WLAN_AVAILABLE_NETWORK*)&pBssList->Network[wifiIndex];

	XTrace(L"  Profile Name[%i]:%s", wifiIndex, pBssEntry->strProfileName);

	if (pBssEntry->dot11Ssid.uSSIDLength == 0)
		XTrace(L"  SSID[%i]:-NONAME-", wifiIndex);
	else {
		wchar_t SSID[DOT11_SSID_MAX_LENGTH + 1];
		for (k = 0; k < pBssEntry->dot11Ssid.uSSIDLength; k++) {
			SSID[k] = pBssEntry->dot11Ssid.ucSSID[k];
		}
		SSID[k] = 0;
		XTrace(L"  SSID[%i]:%s", wifiIndex, SSID);
	}
	switch (pBssEntry->dot11BssType) {
	case dot11_BSS_type_infrastructure:
		XTrace(L"  [%i]BSS Network type:\t Infrastructure:%i", wifiIndex, pBssEntry->dot11BssType);
		break;
	case dot11_BSS_type_independent:
		XTrace(L"  [%i]BSS Network type:\t independent:%i", wifiIndex, pBssEntry->dot11BssType);
		break;
	default:
		XTrace(L"Other (%i)", pBssEntry->dot11BssType);
		break;
	}

	XTrace(L"[%i]  Number of BSSIDs:\t %i", wifiIndex, pBssEntry->uNumberOfBssids);

	if (pBssEntry->bNetworkConnectable)
		XTrace(L"  Connectable:Yes\n");
	else {
		XTrace(L"  Connectable:No\n");
		XTrace(L"  Not connectable WLAN_REASON_CODE value:%i",
			pBssEntry->wlanNotConnectableReason);
	}

	XTrace(L"  Number of PHY types supported[%i]:%ld", wifiIndex, pBssEntry->uNumberOfPhyTypes);

	if (pBssEntry->wlanSignalQuality == 0)
		iRSSI = -100;
	else if (pBssEntry->wlanSignalQuality == 100)
		iRSSI = -50;
	else
		iRSSI = -100 + (pBssEntry->wlanSignalQuality / 2);
	XTrace(L"  Signal Quality[%i]:\t %i  (RSSI:%i  dBm)",wifiIndex , pBssEntry->wlanSignalQuality, iRSSI);

	if (pBssEntry->bSecurityEnabled)
		XTrace(L"  Security Enabled[%i]:\t Yes\n", wifiIndex);
	else
		XTrace(L"  Security Enabled[%i]:\t No\n", wifiIndex);

	switch (pBssEntry->dot11DefaultAuthAlgorithm) {
	case DOT11_AUTH_ALGO_80211_OPEN:
		XTrace(L"  Default AuthAlgorithm[%i]=\t802.11 Open (code:%i)", wifiIndex, pBssEntry->dot11DefaultAuthAlgorithm);
		break;
	case DOT11_AUTH_ALGO_80211_SHARED_KEY:
		XTrace(L"  Default AuthAlgorithm[%i]=\t802.11 Shared (code:%i)", wifiIndex, pBssEntry->dot11DefaultAuthAlgorithm);
		break;
	case DOT11_AUTH_ALGO_WPA:
		XTrace(L"  Default AuthAlgorithm[%i]=\tWPA (code:%i)", wifiIndex, pBssEntry->dot11DefaultAuthAlgorithm);
		break;
	case DOT11_AUTH_ALGO_WPA_PSK:
		XTrace(L"  Default AuthAlgorithm[%i]=\tWPA-PSK (code:%i)", wifiIndex, pBssEntry->dot11DefaultAuthAlgorithm);
		break;
	case DOT11_AUTH_ALGO_WPA_NONE:
		XTrace(L"  Default AuthAlgorithm[%i]=\tWPA-None (code:%i)", wifiIndex, pBssEntry->dot11DefaultAuthAlgorithm);
		break;
	case DOT11_AUTH_ALGO_RSNA:
		XTrace(L"  Default AuthAlgorithm[%i]=\tRSNA (code:%i)", wifiIndex, pBssEntry->dot11DefaultAuthAlgorithm);
		break;
	case DOT11_AUTH_ALGO_RSNA_PSK:
		XTrace(L"  Default AuthAlgorithm[%i]=\tRSNA with PSK (code:%i)", wifiIndex, pBssEntry->dot11DefaultAuthAlgorithm);
		break;
	default:
		XTrace(L"  Default AuthAlgorithm[%i]=\tOther (code:%i)", wifiIndex, pBssEntry->dot11DefaultAuthAlgorithm);
		break;
	}


	switch (pBssEntry->dot11DefaultCipherAlgorithm) {
	case DOT11_CIPHER_ALGO_NONE:
		XTrace(L"  Default CipherAlgorithm[%i]: None (0x%X)", wifiIndex, pBssEntry->dot11DefaultCipherAlgorithm);
		break;
	case DOT11_CIPHER_ALGO_WEP40:
		XTrace(L"  Default CipherAlgorithm[%i]: WEP-40 (0x%X)", wifiIndex, pBssEntry->dot11DefaultCipherAlgorithm);
		break;
	case DOT11_CIPHER_ALGO_TKIP:
		XTrace(L"  Default CipherAlgorithm[%i]: TKIP (0x%X)", wifiIndex, pBssEntry->dot11DefaultCipherAlgorithm);
		break;
	case DOT11_CIPHER_ALGO_CCMP:
		XTrace(L"  Default CipherAlgorithm[%i]: CCMP (0x%X)", wifiIndex, pBssEntry->dot11DefaultCipherAlgorithm);
		break;
	case DOT11_CIPHER_ALGO_WEP104:
		XTrace(L"  Default CipherAlgorithm[%i]: WEP-104 (0x%X)", wifiIndex, pBssEntry->dot11DefaultCipherAlgorithm);
		break;
	case DOT11_CIPHER_ALGO_WEP:
		XTrace(L"  Default CipherAlgorithm[%i]: WEP (0x%X)", wifiIndex, pBssEntry->dot11DefaultCipherAlgorithm);
		break;
	default:
		XTrace(L"  Default CipherAlgorithm[%i]: Other (0x%X)", wifiIndex, pBssEntry->dot11DefaultCipherAlgorithm);
		break;
	}

	XTrace(L"  Flags[%i]:\t 0x%X", wifiIndex, pBssEntry->dwFlags);
	if (pBssEntry->dwFlags) {
		if (pBssEntry->dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED)
			XTrace(L" - Currently connected");
		if (pBssEntry->dwFlags & WLAN_AVAILABLE_NETWORK_HAS_PROFILE)
		{
			XTrace(L" - Has profile");
		}
	}
	XTrace(L"\n");


	OUTPUT = OUTPUT_t::DEBUG;
	return pBssEntry;
}
/// <summary>
/// Guarda la IP en formato string en el arreglo interno IP[]. Permite un numero máximo de IP's= MAX_IP_ADRESSES
/// </summary>
/// <param name="newIpAddress">IP en formato String</param>
protected: void SaveIpAddress(wchar_t* newIpAddress)
{
	if (newIpAddress)
	{
		if (IP_Adresses_avaliables < MAX_IP_ADRESSES)
		{
			lstrcpyW(IP[IP_Adresses_avaliables], newIpAddress);

			IP_Adresses_avaliables++;
		}
		else
		{
			XTrace(L"Maximun number of allowed IP's reached.");
			XTrace(L"Discarting:%s", newIpAddress);
		}
	}
}
/// <summary>
/// Obtiene el GUID del adaptador guardado en el arreglo  pIfList->InterfaceInfo[index].
/// llamar a EnumerarWifis() para actualizar este arreglo
/// </summary>
/// <param name="index">Posicion dentro del arreglo</param>
/// <param name="guidOUT">puntero a una estructura GUID donde se guardará la informacion solicitada</param>
/// <returns>puntero a GUID si ha funcionado. NULL si el indice está fuera del rango o si no se ha 
/// llamado previamente a EnumerarWifis()</returns>
public:GUID* GetSelectedGUIDFromInternalList(int index,GUID* guidOUT) {

	PWLAN_INTERFACE_INFO			pIfInfo = NULL;

	if (pIfList == NULL)return NULL;
	if (index >= pIfList->dwNumberOfItems)return NULL;
	pIfInfo = (WLAN_INTERFACE_INFO*)&pIfList->InterfaceInfo[index];

	guidOUT->Data1 = pIfInfo->InterfaceGuid.Data1;
	guidOUT->Data2 = pIfInfo->InterfaceGuid.Data2;
	guidOUT->Data3 = pIfInfo->InterfaceGuid.Data3;
	for (int n = 0; n < 8; n++) {
		guidOUT->Data4[n] = pIfInfo->InterfaceGuid.Data4[n];
	}

	return guidOUT;
}
/// <summary>
/// Utiliza la funcion GetAdaptersInfo() para obtener la IP relacionada con un adaptador Wifi
/// </summary>
/// <param name="guid">GUID del adaptador solicitado</param>
/// <param name="pAdapter_OUT">Estructura donde se guarda la informacion solicitada</param>
/// <returns>ERROR_SUCCESS si ha funcionado.ERROR_NOT_FOUND si no se ha encontrado.ERROR_BUFFER_OVERFLOW si no se podido reservar memoria</returns>
public:DWORD FindAdapter(GUID guid,IP_ADAPTER_INFO* pAdapter_OUT) {
	/* variables used for GetAdapterInfo */
	IP_ADAPTER_INFO* pAdapterInfo;
	IP_ADAPTER_INFO* pAdapter;
	ULONG ulOutBufLen;
	DWORD dwRetVal;
	//************************************ MEMORY ALLOCATION ******************************************
	pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		XTrace(L"Error allocating memory needed to call GetAdapterInfo");
		return ERROR_BUFFER_OVERFLOW;
	}
	ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			XTrace(L"Error allocating memory needed to call GetAdapterInfo");
			return ERROR_BUFFER_OVERFLOW;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) != NO_ERROR) {
		XTrace(L"GetAdaptersInfo failed with error %i", dwRetVal);
		if (pAdapterInfo)
			FREE(pAdapterInfo);
		return dwRetVal;
	}

	pAdapter = pAdapterInfo;
	//***************************************************************************************************
	CLSID clsid;
	while (pAdapter) {
		//************************CLSI****************
		size_t length = 0; wchar_t AdapterName_16bits[100];
		int size = strlen(pAdapter->AdapterName);
		mbstowcs_s(&length, AdapterName_16bits, pAdapter->AdapterName, size + 1);
		CLSIDFromString(AdapterName_16bits, &clsid);
		if (IsEqualGUID(guid, clsid))
		{
			memcpy_s(pAdapter_OUT, sizeof(IP_ADAPTER_INFO), pAdapter, sizeof(IP_ADAPTER_INFO));
			if (pAdapterInfo)
				FREE(pAdapterInfo);
			return ERROR_SUCCESS;
		}
		
		pAdapter = pAdapter->Next;
	}
	pAdapter_OUT = NULL; 
	if (pAdapterInfo)
		FREE(pAdapterInfo);
	return ERROR_NOT_FOUND;
}
///Conversor de integro a cadena
public:template <class T,class U>
U* Int2Hex(T lnumber, U* buffer)
{
	const char* ref = "0123456789ABCDEF";
	T hNibbles = (lnumber >> 4);

	unsigned char* b_lNibbles = (unsigned char*)&lnumber;
	unsigned char* b_hNibbles = (unsigned char*)&hNibbles;

	U* pointer = buffer + (sizeof(lnumber) *2);

	*pointer = 0;
	do {
		*--pointer = ref[(*b_lNibbles++) & 0xF];
		*--pointer = ref[(*b_hNibbles++) & 0xF];
	} while (pointer > buffer);

	return buffer;
}
/// <summary>
/// Devuelve la IP en el Arreglo interno "IP[]" tras la llamada a GetIPList().
/// Esta funcion usa la API  Windows Sockets 2
/// </summary>
/// <returns>Ip almacenada en la posicion "index" del arreglo IP[]. Llamar a  GetIPList() para actualizar.</returns>
public: wchar_t* GetIPAddress(int index)
{
	if (index < IP_Adresses_avaliables)
	{
		return IP[index];
	}
	return NULL;
}
/// <summary>
/// Numero de Direcciones alamcenadas en el Arreglo interno "IP[]" tras la llamada a GetIPList().
/// Esta funcion usa la API  Windows Sockets 2
/// </summary>
/// <returns>Ips almacenadas en memoria. Llamar a  GetIPList() para actualizar.</returns>
public: int GetIPNumberOfAdresses()
{
	return IP_Adresses_avaliables;
}
/// <summary>
/// Estructura para ser enviada a WlanNotificationCallback_prv
/// </summary>
public: struct WlanNotificationCallback_prv_Data
{
	HWND hwnd;
	ULONG MsgWifiConectedCode;
};
/// <summary>
/// Método Callback por defecto para ser usado en el método WlanRegisterNotification() de WinSockServer::ConnectWifi()
/// </summary>
/// <param name="Arg1">PWLAN_NOTIFICATION_DATA Informacion relativa al tipo de notificacion recibida</param>
/// <param name="Arg2">WlanNotificationCallback_prv_Data* Estructura para enviar la notificacion 
/// wlan_notification_acm_connection_complete a un procedimiento de ventana a traves de 
/// PostMessageA(Arg2->hwnd, Arg2->MsgWifiConectedCode,(WPARAM)0,(LPARAM) 0);
/// Este argumento puede ser NULL</param>
/// <returns></returns>
public:static void WINAPI WlanNotificationCallback_prv(PWLAN_NOTIFICATION_DATA Arg1, PVOID Arg2)
{
	BOOL ret;
	struct WlanNotificationCallback_prv_Data* Arg=NULL;
	if (Arg2 != NULL)
	{
		Arg = (WlanNotificationCallback_prv_Data*)Arg2;
	}

	OutputDebugString(L"WlanNotificationCallback_prv() - ");
	switch (Arg1->NotificationSource) {
	case WLAN_NOTIFICATION_SOURCE_NONE:
		OutputDebugString(L"WLAN_NOTIFICATION_SOURCE_NONE \n");
		break;
	case WLAN_NOTIFICATION_SOURCE_ONEX:
		OutputDebugString(L"WLAN_NOTIFICATION_SOURCE_ONEX - ");
		break;
	case WLAN_NOTIFICATION_SOURCE_ACM:
		OutputDebugString(L"WLAN_NOTIFICATION_SOURCE_ACM - ");
		break;
	case WLAN_NOTIFICATION_SOURCE_MSM:
		OutputDebugString(L"WLAN_NOTIFICATION_SOURCE_MSM - ");
		break;
	case WLAN_NOTIFICATION_SOURCE_SECURITY:
		OutputDebugString(L"WLAN_NOTIFICATION_SOURCE_SECURITY \n");
		break;
	case WLAN_NOTIFICATION_SOURCE_IHV:
		OutputDebugString(L"WLAN_NOTIFICATION_SOURCE_IHV  \n");
		break;
	case WLAN_NOTIFICATION_SOURCE_HNWK:
		OutputDebugString(L"WLAN_NOTIFICATION_SOURCE_HNWK - ");
		break;
	case WLAN_NOTIFICATION_SOURCE_ALL:
		OutputDebugString(L"WLAN_NOTIFICATION_SOURCE_ALL \n");
		break;
	default:
		OutputDebugString(L"WLAN_NOTIFICATION_SOURCE_UNKNOW\n");
		break;
	}

	if (Arg1->NotificationSource == WLAN_NOTIFICATION_SOURCE_ONEX)
	{
		switch (Arg1->NotificationCode)
		{
		case 1://OneXNotificationTypeResultUpdate:
			OutputDebugString(L" OneXNotificationTypeResultUpdate\n");
			break;
		case 2://OneXNotificationTypeAuthRestarted:
			OutputDebugString(L" OneXNotificationTypeAuthRestarted\n");
			break;
		case 3://OneXNotificationTypeEventInvalid:
			OutputDebugString(L" OneXNotificationTypeEventInvalid \n");
			break;
		default:
			break;
		}
	}
	if (Arg1->NotificationSource == WLAN_NOTIFICATION_SOURCE_ACM)
	{
		switch (Arg1->NotificationCode)
		{
		case wlan_notification_acm_autoconf_enabled:
			OutputDebugString(L"wlan_notification_acm_autoconf_enabled \n");
			break;
		case wlan_notification_acm_autoconf_disabled:
			OutputDebugString(L"wlan_notification_acm_autoconf_disabled  \n");
			break;
		case wlan_notification_acm_background_scan_enabled:
			OutputDebugString(L"wlan_notification_acm_background_scan_enabled \n");
			break;
		case wlan_notification_acm_background_scan_disabled:
			OutputDebugString(L" wlan_notification_acm_background_scan_disabled  \n");
			break;
		case wlan_notification_acm_bss_type_change:
			OutputDebugString(L" wlan_notification_acm_bss_type_change\n");
			break;
		case wlan_notification_acm_power_setting_change:
			OutputDebugString(L" wlan_notification_acm_power_setting_change \n");
			break;
		case wlan_notification_acm_scan_complete:
			OutputDebugString(L" wlan_notification_acm_scan_complete \n");
			break;
		case wlan_notification_acm_scan_fail:
			OutputDebugString(L" wlan_notification_acm_scan_fail \n");
			break;
		case wlan_notification_acm_connection_start:
			OutputDebugString(L" wlan_notification_acm_connection_start \n");
			break;
		case wlan_notification_acm_connection_complete:
			OutputDebugString(L" wlan_notification_acm_connection_complete \n");
			if(Arg2!=NULL){ret=PostMessageA(Arg->hwnd, Arg->MsgWifiConectedCode,(WPARAM)0,(LPARAM) 0);}
			break;
		case wlan_notification_acm_connection_attempt_fail:
			OutputDebugString(L" wlan_notification_acm_connection_attempt_fail \n");
			break;
		case wlan_notification_acm_filter_list_change:
			OutputDebugString(L" wlan_notification_acm_filter_list_change \n");
			break;
		case wlan_notification_acm_interface_arrival:
			OutputDebugString(L"wlan_notification_acm_interface_arrival  \n");
			break;
		case wlan_notification_acm_interface_removal:
			OutputDebugString(L" wlan_notification_acm_interface_removal \n");
			break;
		case wlan_notification_acm_profile_change:
			OutputDebugString(L" wlan_notification_acm_profile_change \n");
			break;
		case wlan_notification_acm_profile_name_change:
			OutputDebugString(L" wlan_notification_acm_profile_name_change \n");
			break;
		case wlan_notification_acm_profiles_exhausted:
			OutputDebugString(L" wlan_notification_acm_profiles_exhausted \n");
			break;
		case wlan_notification_acm_network_not_available:
			OutputDebugString(L" wlan_notification_acm_network_not_available \n");
			break;
		case wlan_notification_acm_network_available:
			OutputDebugString(L" wlan_notification_acm_network_available\n");
			break;
		case wlan_notification_acm_disconnecting:
			OutputDebugString(L" wlan_notification_acm_disconnecting \n");
			break;
		case wlan_notification_acm_disconnected:
			OutputDebugString(L" wlan_notification_acm_disconnected\n");
			break;
		case wlan_notification_acm_adhoc_network_state_change:
			OutputDebugString(L" wlan_notification_acm_adhoc_network_state_change \n");
			break;
		case wlan_notification_acm_profile_unblocked:
			OutputDebugString(L" wlan_notification_acm_profile_unblocked\n");
			break;
		case wlan_notification_acm_screen_power_change:
			OutputDebugString(L" wlan_notification_acm_screen_power_change \n");
			break;
		case wlan_notification_acm_profile_blocked:
			OutputDebugString(L" wlan_notification_acm_profile_blocked \n");
			break;
		case wlan_notification_acm_scan_list_refresh:
			OutputDebugString(L" wlan_notification_acm_scan_list_refresh \n");
			break;
		default:
			OutputDebugString(L" wlan_notification_acm_unknow \n");
			break;
		}
	}
	if (Arg1->NotificationSource == WLAN_NOTIFICATION_SOURCE_MSM)
	{
		switch (Arg1->NotificationCode)
		{
		case wlan_notification_msm_associating:
			OutputDebugString(L" wlan_notification_msm_associating\n");
			break;
		case wlan_notification_msm_associated:
			OutputDebugString(L" wlan_notification_msm_associated \n");
			break;
		case wlan_notification_msm_authenticating:
			OutputDebugString(L" wlan_notification_msm_authenticating \n");
			break;
		case wlan_notification_msm_connected:
			OutputDebugString(L" wlan_notification_msm_connected \n");
			break;
		case wlan_notification_msm_roaming_start:
			OutputDebugString(L" wlan_notification_msm_roaming_start \n");
			break;
		case wlan_notification_msm_roaming_end:
			OutputDebugString(L" wlan_notification_msm_roaming_end \n");
			break;
		case wlan_notification_msm_radio_state_change:
			OutputDebugString(L"  wlan_notification_msm_radio_state_change \n");
			break;
		case wlan_notification_msm_signal_quality_change:
			OutputDebugString(L"  wlan_notification_msm_signal_quality_change \n");
			break;
		case wlan_notification_msm_disassociating:
			OutputDebugString(L"  wlan_notification_msm_disassociating\n");
			break;
		case wlan_notification_msm_disconnected:
			OutputDebugString(L"  wlan_notification_msm_disconnected \n");
			break;
		case wlan_notification_msm_peer_join:
			OutputDebugString(L"  wlan_notification_msm_peer_join \n");
			break;
		case wlan_notification_msm_peer_leave:
			OutputDebugString(L"  wlan_notification_msm_peer_leave \n");
			break;
		case wlan_notification_msm_adapter_removal:
			OutputDebugString(L"  wlan_notification_msm_adapter_removal \n");
			break;
		case wlan_notification_msm_adapter_operation_mode_change:
			OutputDebugString(L" wlan_notification_msm_adapter_operation_mode_change  \n");
			break;
		default:
			OutputDebugString(L"wlan_notification_msm_unknow\n");
			break;
		}
	}
	if (Arg1->NotificationSource == WLAN_NOTIFICATION_SOURCE_HNWK)
	{
		switch (Arg1->NotificationCode)
		{
		case wlan_hosted_network_state_change:
			OutputDebugString(L" wlan_hosted_network_state_change \n");
			break;
		case wlan_hosted_network_peer_state_change:
			OutputDebugString(L" wlan_hosted_network_peer_state_change \n");
			break;
		case wlan_hosted_network_radio_state_change:
			OutputDebugString(L" wlan_hosted_network_radio_state_change \n");
			break;
		default:
			OutputDebugString(L"wlan_hosted_network_unknow \n");
			break;
		}
	}
}
/// <summary>
/// Connects to a Wifi.
/// Esta funcion utiliza llamadas previas a WlanApi para obtener referencias a redes Wifis,
/// por lo que previamente se debe llamar a la función EnumerarWifis() para obtener un manejador hClientHandle 
/// y rellenar estructuras como pIfList y pBssList
/// La funcion regresa inmediatamente. Para notificar el exito o el fracaso en la conexion se registra 
///la Callback WlanNotificationCallback_prv
/// </summary>
/// <param name="WifiIndex">Indice del Array pBssList[WifiIndex] obtenido en la llamada de la WlanAPI "WlanGetAvailableNetworkList()" 
///  a la que se llama en el interior de WinSockServer3::EnumerarWifis()</param>
///	<param name="WlanNotificationCallback">Metodo Callback que recibirá las notificaciones "conectado", "desconectado", etc...
/// puede usarse la función por defecto WinSockServer::WlanNotificationCallback_prv()</param>
///	<param name="pCallbackContext">puntero a memoria que recibira WlanNotificationCallback como argumento Arg1</param>
/// <returns>ERROR_SUCCESS si ha funcionado, ERROR_INVALID_PARAMETER si no se ha llamado a EnumerarWifis() previamente. Otros errores propios de WlanAPI</returns>
public:DWORD ConnectWifi(int WifiAdapter, int WifiIndex, WLAN_NOTIFICATION_CALLBACK WlanNotificationCallback, PVOID pCallbackContext)
{
	DWORD dwPrevNotifSource;

	if (hClientHandle == NULL) {
		XTrace(L"ConnectWifi() hClientHandle == NULL - Falto una llamada a EnumerarWifis() de inicialización\n");
		return ERROR_INVALID_PARAMETER;
	}
	//The WlanRegisterNotification function is used to register 
	//and unregister notifications on all wireless interfaces.
	DWORD returnValue=WlanRegisterNotification(
		hClientHandle,					//hClientHandle-The client's session handle, obtained by a previous call to the WlanOpenHandle function.
		WLAN_NOTIFICATION_SOURCE_ALL,	//The notification sources to be registered. These flags may be combined.
		TRUE,							//bIgnoreDuplicate, If set to TRUE, a notification will not be sent to the client if it is identical to the previous one.
		WlanNotificationCallback,	//funcCallback,notification callback function.	This parameter can be NULL if the dwNotifSource parameter is set to WLAN_NOTIFICATION_SOURCE_NONE
		pCallbackContext,				//A pointer to the client context that will be passed to the callback function with the notification.
		NULL,							//Reserved for future use. Must be set to NULL.
		&dwPrevNotifSource				//A pointer to the previously registered notification sources.
	);
	switch (returnValue)
	{
	case ERROR_SUCCESS:
		XTrace(L"WlanRegisterNotification Callback returns OK\n");
		break;
	case ERROR_INVALID_PARAMETER:
		XTrace(L"WlanRegisterNotification Callback returns ERROR_INVALID_PARAMETER\n");
		return returnValue;
	case ERROR_INVALID_HANDLE:
		XTrace(L"WlanRegisterNotification Callback returns ERROR_INVALID_HANDLE\n");
		return returnValue;
	case ERROR_NOT_ENOUGH_MEMORY:
		XTrace(L"WlanRegisterNotification Callback returns ERROR_NOT_ENOUGH_MEMORY\n");
		return returnValue;
	default:
		XTrace(L"WlanRegisterNotification Callback returns UNKNOW ERRROR:%i\n",returnValue);
		return returnValue;
	}
	//MostrarWifi(WifiIndex);

	returnValue = ConnectWifi_byProfile(WifiAdapter,   WifiIndex);

	return returnValue;
}

/// <summary>
/// Utiliza el método WlanConnect de WlanAPI para conectar a una red a la que el equipo 
/// tenga permiso al haberse conectado previamente y guardado el PASSWORD
/// Es necesario llamar a EnumerarWifis antes de llamar a esta función
/// </summary>
/// <param name="WifiIndex">Indice del arreglo donde WlanGetAvailableNetworkList() ha guardado la red Wifi</param>
/// <returns></returns>
protected:DWORD ConnectWifi_byProfile(int AdapterIndex,int WifiIndex) {
	DWORD returnValue= ERROR_SUCCESS;

	PWLAN_INTERFACE_INFO			pIfInfo = pIfList[AdapterIndex].InterfaceInfo;
	if (pIfInfo == NULL) {
		XTrace(L"ConnectWifi_byProfile() - WinSockServer3 no Iniciado. Es necesario llamar a EnumerarWifis()\n");
		return ERROR_INVALID_PARAMETER;
	}

	WLAN_CONNECTION_PARAMETERS ConnectionParameters;
	memset(&ConnectionParameters, 0, sizeof(WLAN_CONNECTION_PARAMETERS));
	ConnectionParameters.wlanConnectionMode = wlan_connection_mode_profile;
   //Specifies the profile being used for the connection.
	ConnectionParameters.strProfile = pBssList->Network[WifiIndex].strProfileName;
	//the BSS type of the network.
	ConnectionParameters.dot11BssType = dot11_BSS_type_any;//  pBssList->Network[WifiIndex].dot11BssType;
	returnValue = WlanConnect(
		hClientHandle,			//The client's session handle, 
								//returned by a previous call to the WlanOpenHandle function.
		&pIfInfo->InterfaceGuid,//pInterfaceGuid,			//The GUID of the interface to use for the connection.
		&ConnectionParameters,	//Pointer to a WLAN_CONNECTION_PARAMETERS structure that specifies 
								//the connection type, mode, network profile, 
								//SSID that identifies the network, and other parameters.
		NULL                    //pReserved
	);
	switch (returnValue)
	{
	case ERROR_SUCCESS:
		OutputDebugString(L"ConnectWifi_byProfile()-WlanConnect()-conectado\n");
		break;
	case ERROR_INVALID_PARAMETER:
		XTrace(L"ConnectWifi_byProfile()-WlanConnect()-ERROR_INVALID_PARAMETER");
		return returnValue;
	case ERROR_INVALID_HANDLE:
		XTrace(L"ConnectWifi_byProfile()-WlanConnect()-ERROR_INVALID_HANDLE");
		return returnValue;
	case ERROR_ACCESS_DENIED:
		XTrace(L"ConnectWifi_byProfile()-WlanConnect()-ERROR_ACCESS_DENIED");
		return returnValue;
	default:
		XTrace(L"ConnectWifi_byProfile()-WlanConnect()-UNKNOW ERROR:", returnValue);
		return returnValue;
	}
	return ERROR_SUCCESS;
}

/// <summary>
/// My personal version of inet_ntop()
/// </summary>
/// <param name="address">IP structure to retrieve a string</param>
/// <param name="IPString">buffer where the ip will be stored</param>
/// <param name="PortString">buffer where the port will be stored</param>
/// <returns>Puntero a IPString</returns>
public: static wchar_t* socketaddress_to_string(sockaddr* address, wchar_t* IPString, wchar_t* PortString) {
	int source = 0, dest = 0;
	wchar_t byte_string[4] = { 0 };
	wchar_t word_string[6] = { 0 };
	for (int nByte = 2; nByte < 6; nByte++) {
		_itow_s((unsigned char)address->sa_data[nByte], byte_string, 10);
		source = 0;
		while (byte_string[source] != 0)
		{
			IPString[dest] = byte_string[source]; source++; dest++;
		}
		if (nByte < 5)IPString[dest++] = L'.';
		else IPString[dest] = 0;
	}

	WORD hByte = (unsigned char)(address->sa_data[0]);
	WORD lByte = (unsigned char)(address->sa_data[1]);
	WORD port = (hByte * 256) + lByte;
	_itow_s(port, word_string, 10);
	source = 0;
	while (word_string[source] != 0) {
		PortString[source] = word_string[source];
		source++;
	}
	PortString[source] = 0;
	return IPString;
}

/// <summary>
/// printf() style debugging
/// https://stackoverflow.com/questions/15240/
/// </summary>
/// <param name="lpszFormat">Debugging text</param>
void XTrace0(LPCTSTR lpszText)
{
	switch (OUTPUT)
	{
	case OUTPUT_t::LISTBOX:
	{

		SendMessage(ListBox_forOutput, LB_ADDSTRING, 0,
			(LPARAM)lpszText);
	}
	default:
	{
		::OutputDebugString(lpszText);
	}
	}
}

/// <summary>
/// printf() style debugging
/// https://stackoverflow.com/questions/15240/
/// </summary>
/// <param name="lpszFormat">-Debugging text</param>
/// <param name="">.... parameters in _vstprintf_s() style</param>
void XTrace(LPCTSTR lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);
	int nBuf;
	TCHAR szBuffer[512];
	nBuf = _vstprintf_s(szBuffer, 511, lpszFormat, args);
	switch (OUTPUT)
	{
	case OUTPUT_t::LISTBOX:
	{

	SendMessage(ListBox_forOutput, LB_ADDSTRING, 0,
	(LPARAM)szBuffer);
	}
	default:
	{
	::OutputDebugString(szBuffer);
		break;
	}
	}
	va_end(args);
}

public:wchar_t* WindowsErrorToString(int ErrorCode) {

	wchar_t* lpBuffer;

	if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		ErrorCode,
		0,
		(wchar_t*)&lpBuffer,
		1000,
		NULL) == 0)
	{
		XTrace(L"Error with FormatMessage\n");
	}
	return lpBuffer;
}
public:DWORD WlanOpenHandle_ShowError(DWORD returnValue)
{
	switch (returnValue)
	{
	case ERROR_SUCCESS:
		XTrace(L"WlanOpenHandle returns OK\n");
		return returnValue;
	case ERROR_INVALID_PARAMETER:
		XTrace(L"WlanOpenHandle returns ERROR_INVALID_PARAMETER\n");
		return returnValue;
	case ERROR_NOT_ENOUGH_MEMORY:
		XTrace(L"WlanOpenHandle returns ERROR_NOT_ENOUGH_MEMORY\n");
		return returnValue;
	case ERROR_REMOTE_SESSION_LIMIT_EXCEEDED:
		XTrace(L"WlanOpenHandle returns ERROR_REMOTE_SESSION_LIMIT_EXCEEDED\n");
		return returnValue;
	default:
		XTrace(L"WlanOpenHandle returns UNKNOW ERRROR:\n", returnValue);
		return returnValue;
	}
}

	  /// <summary>
	  /// Initiates a connection to the SERVER on the specified IP and ports.
	  /// If it fails to connect, but the connect command runs successfully,
	  /// put STATUS in "REQUESTING" and WSA_non_blocking_Client goes on hold
	  /// of the triggering of an event "FD_CONNECT". The method returns immediately,
	  /// DOES NOT block the application, but the application must
	  /// call WSAnb_Client.Attemp_connect () to try again and
	  /// WSAnb_Client.testForEvents () regularly (with a WM_TIMER, for example)
	  /// to react to the FD_CONNECT event that will occur when the Server accepts the
	  /// connection requested.
	  /// Attemp_connect () modifies STATUS- CONNECTED if it has connected to the first one or NONE
	  /// if it has been rejected by time to not block the application
	  /// </summary>
	  /// <param name="IPString">IP v4 of the server to which you want to connect</param>
	  /// <param name="port">Port of the IP to which you want to connect</param>
	  /// <returns>TRUE if connected or rejected to not block. FALSE if something has failed.
	  /// lastWSAError saves the last value of WSAGetLastError. between the possible values of lastWSAError,
	  /// WSAEWOULDBLOCK indicates that the connection attempt to not block has been canceled
	  /// This function alters the content of "state" variable. 
	  /// state=CONNECTED. Connection has been made with a server.
	  /// sate=LISTENING. The connection has not been made, probably because the server is not available.
	  /// Try again in a little while.
	  /// state=ERROR_DETECTED The requested IP or port is not in the valid format. O connect () has returned another error
	  ///</returns>
public: BOOL Attemp_connect(wchar_t* IPString, int port) {
	lastWSAError = 0;
	sockaddr_in  ClientAdress = { 0 };
	int          ClientAdressLen;
	if (bConnected)
	{
		closesocket(ClientSocket);
		if (!CreateClientSocket())return false;
	}
	IN_ADDR in_addr = { 0 };
	//Guarda el string IP u el Puerto recibidos
	//actualiza el estado de la clase
	wcscpy_s(this->IPString, this->IPString_Lenght, IPString);
	_itow_s(port, this->PortString, 10);
	if (InetPton(AF_INET, IPString, &in_addr) != 1) {
		//The InetPton function returns a value of 0 if the pAddrBuf parameter points to a string
		//that is not a valid IPv4 dotted - decimal string or a valid IPv6 address string.
		//Otherwise, a value of - 1 is returned, and a specific error code can be retrieved by 
		//calling the WSAGetLastError() for extended error information.
		if (iResult == 0) {
			//WSAEFAULT=The system detected an invalid pointer address.
			lastWSAError = WSAEFAULT;
			XTrace(L"InetPton failed: IPString is not a valid IP");
			ClientState = STATE::ERROR_DETECTED;
			return FALSE;
		}
		lastWSAError = WSAGetLastError();
		XTrace(L"InetPton error %u\n", lastWSAError);
		ClientState = STATE::ERROR_DETECTED;
		return FALSE;

	}
	ClientAdressLen = sizeof(ClientAdress);
	ClientAdress.sin_family = AF_INET;
	ClientAdress.sin_addr = in_addr;
	ClientAdress.sin_port = htons(port);
	iResult = connect(ClientSocket, (SOCKADDR*)&ClientAdress, ClientAdressLen);
	if (iResult == SOCKET_ERROR) {
		lastWSAError = WSAGetLastError();
		XTrace(L"connect failed with error %u : %s\n", lastWSAError, WindowsErrorToString(lastWSAError));
		switch (lastWSAError)
		{
		case WSAEWOULDBLOCK:
			bConnected = FALSE;
			ClientState = STATE::ATTEMPTING_CONNECT_TO_SERVER;
			return TRUE;
		default:
			ClientState = STATE::ERROR_DETECTED;
			return FALSE;
		}
	}
	bConnected = TRUE;
	ClientState = STATE::CONNECTED;
	XTrace(L"Connected to server.\n");
	return TRUE;

}

public:BOOL SendDataClient(SharedClass local_sharedData) {
	//Prepare checksum**********************************
	unsigned long bufferSize = sizeof(SharedClass);

	//local_sharedData.data1 = 1;
	//local_sharedData.data2 = 2;
	//local_sharedData.data3 = 0x20100804;
	//local_sharedData.data4 = 0x00008040;


	unsigned long checksum = local_sharedData.data1 + local_sharedData.data2;

	unsigned long* pointer = &local_sharedData.data3;
	unsigned char* myByte = (unsigned char*)pointer;

	checksum += (myByte[0]);
	checksum += (myByte[1]);
	checksum += (myByte[2]);
	checksum += (myByte[3]);

	pointer = &local_sharedData.data4;
	myByte = (unsigned char*)pointer;

	checksum += (myByte[0]);
	checksum += (myByte[1]);
	checksum += (myByte[2]);
	checksum += (myByte[3]);

	local_sharedData.checksum = checksum;
	int i = 0;
	//Prepare buffer**********************************
	sharedDataBuffer[i] = local_sharedData.Mark[0];
	sharedDataBuffer[++i] = local_sharedData.Mark[1];
	sharedDataBuffer[++i] = local_sharedData.Mark[2];
	sharedDataBuffer[++i] = local_sharedData.Mark[3];
	sharedDataBuffer[++i] = local_sharedData.Mark[4];
	sharedDataBuffer[++i] = local_sharedData.Mark[5];
	sharedDataBuffer[++i] = local_sharedData.Mark[6];
	sharedDataBuffer[++i] = local_sharedData.Mark[7];


	sharedDataBuffer[++i] = checksum & 0xFF;
	sharedDataBuffer[++i] = (checksum >> 8) & 0xFF;
	sharedDataBuffer[++i] = (checksum >> 16) & 0xFF;
	sharedDataBuffer[++i] = (checksum >> 24) & 0xFF;

	sharedDataBuffer[++i] = local_sharedData.data1;
	sharedDataBuffer[++i] = local_sharedData.data2;

	sharedDataBuffer[++i] = local_sharedData.data3 & 0xFF;
	sharedDataBuffer[++i] = (local_sharedData.data3 >> 8) & 0xFF;
	sharedDataBuffer[++i] = (local_sharedData.data3 >> 16) & 0xFF;
	sharedDataBuffer[++i] = (local_sharedData.data3 >> 24) & 0xFF;

	sharedDataBuffer[++i] = local_sharedData.data4 & 0xFF;
	sharedDataBuffer[++i] = (local_sharedData.data4 >> 8) & 0xFF;
	sharedDataBuffer[++i] = (local_sharedData.data4 >> 16) & 0xFF;
	sharedDataBuffer[++i] = (local_sharedData.data4 >> 24) & 0xFF;

	//Send buffer**********************************
	lastWSAError = 0;
	int bytesSend = 0;
	if (bConnected) {
		bytesSend = send(ClientSocket, (char*)sharedDataBuffer, (sizeof(sharedDataBuffer)), 0);
		if (bytesSend == SOCKET_ERROR) {
			lastWSAError = WSAGetLastError();
			XTrace(L"Error sending Data. Code: %u = %s", lastWSAError, WindowsErrorToString(lastWSAError));
			return FALSE;
		}
		return TRUE;
	}
	return FALSE;
}

public:bool SendDataServer(unsigned int socketIndex, SharedClass local_sharedData) {

	lastWSAError = 0;
	int bytesSend = 0;
	unsigned char sendBuffer[sizeof(SharedClass)];

	PrepareBuffer(local_sharedData, sendBuffer);
	//SendBuffer
	if ((socketIndex > 0) && (socketIndex < ClientIndex)) {
		bytesSend = send(SocketArray[socketIndex], (char*)sendBuffer, sizeof(sendBuffer), 0);
		if (bytesSend == SOCKET_ERROR) {
			lastWSAError = WSAGetLastError();
			XTrace(L"Error al enviar texto. Codigo: %u = %s", lastWSAError, WindowsErrorToString(lastWSAError));
			return FALSE;
		}
		return TRUE;
	}
	else
	{
		XTrace(L"WSA_non_blocking::SendText() error: socketIndex out of bounds\n");
		return FALSE;
	}
	return false;

}

public:bool PrepareBuffer(SharedClass local_sharedData, unsigned char sendBuffer[sizeof(SharedClass)]) {
	//Prepare checksum**********************************
	unsigned long bufferSize = sizeof(SharedClass);

	//local_sharedData.data1 = 1;
	//local_sharedData.data2 = 2;
	//local_sharedData.data3 = 0x20100804;
	//local_sharedData.data4 = 0x00008040;


	unsigned long checksum = local_sharedData.data1 + local_sharedData.data2;

	unsigned long* pointer = &local_sharedData.data3;
	unsigned char* myByte = (unsigned char*)pointer;

	checksum += (myByte[0]);
	checksum += (myByte[1]);
	checksum += (myByte[2]);
	checksum += (myByte[3]);

	pointer = &local_sharedData.data4;
	myByte = (unsigned char*)pointer;

	checksum += (myByte[0]);
	checksum += (myByte[1]);
	checksum += (myByte[2]);
	checksum += (myByte[3]);

	local_sharedData.checksum = checksum;
	int i = 0;
	//Prepare buffer**********************************
	sendBuffer[i] = local_sharedData.Mark[0];
	sendBuffer[++i] = local_sharedData.Mark[1];
	sendBuffer[++i] = local_sharedData.Mark[2];
	sendBuffer[++i] = local_sharedData.Mark[3];
	sendBuffer[++i] = local_sharedData.Mark[4];
	sendBuffer[++i] = local_sharedData.Mark[5];
	sendBuffer[++i] = local_sharedData.Mark[6];
	sendBuffer[++i] = local_sharedData.Mark[7];


	sendBuffer[++i] = checksum & 0xFF;
	sendBuffer[++i] = (checksum >> 8) & 0xFF;
	sendBuffer[++i] = (checksum >> 16) & 0xFF;
	sendBuffer[++i] = (checksum >> 24) & 0xFF;

	sendBuffer[++i] = local_sharedData.data1;
	sendBuffer[++i] = local_sharedData.data2;

	sendBuffer[++i] = local_sharedData.data3 & 0xFF;
	sendBuffer[++i] = (local_sharedData.data3 >> 8) & 0xFF;
	sendBuffer[++i] = (local_sharedData.data3 >> 16) & 0xFF;
	sendBuffer[++i] = (local_sharedData.data3 >> 24) & 0xFF;

	sendBuffer[++i] = local_sharedData.data4 & 0xFF;
	sendBuffer[++i] = (local_sharedData.data4 >> 8) & 0xFF;
	sendBuffer[++i] = (local_sharedData.data4 >> 16) & 0xFF;
	sendBuffer[++i] = (local_sharedData.data4 >> 24) & 0xFF;

	return true;
}

};
