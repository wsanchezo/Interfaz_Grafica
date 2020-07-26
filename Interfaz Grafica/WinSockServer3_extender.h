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

// Need to link with Ws2_32.lib, Mswsock.lib, Advapi32.lib, and Iphlpapi.lib
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Wlanapi.lib")	//for WlanEnumInterfaces
#pragma comment(lib,"Ole32.lib")	//for StringFromGUID2

#define WinSockSocket_extender_WSS_IP_STRING_SIZE 256
#define WinSockSocket_extender_COMODIN_LENGHT 1024

#define WinSockSocket_extender_MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define WinSockSocket_extender_FREE(x) HeapFree(GetProcessHeap(), 0, (x))

/// <summary>
/// WinSockSocket_extender incorpora las funciones de prueba de WinSockSocket3 que han resultado 
/// poco útiles.
/// </summary>
class WinSockSocket_extender {


	//******************************  OUTPUT   *****************************************************
	enum class OUTPUT_t { DEBUG, LISTBOX };
	OUTPUT_t OUTPUT = OUTPUT_t::DEBUG;
	HWND ListBox_forOutput = NULL;
	//****************************** WIFI ENUMERATION AND CONNECTION *******************************

	HANDLE							hClientHandle = NULL;
	PWLAN_INTERFACE_INFO_LIST		pIfList = NULL;
	//PWLAN_INTERFACE_INFO			pIfInfo = NULL;

	PWLAN_AVAILABLE_NETWORK_LIST	pBssList = NULL;
	//PWLAN_AVAILABLE_NETWORK			pBssEntry = NULL;


	//********************************** VARIAS *******************************************************
//comodin reservado para this->Debug()
	wchar_t comodin[WinSockSocket_extender_COMODIN_LENGHT];
	/// <summary>
	/// En un intento de obtener el IP relaccionado con una determinada red wifi
	/// se ha testeado el funcionamiento del método WlanHostedNetworkQueryStatus() de WlanAPI
	/// pero ha resultado ser útil solo para listar algunas características de la red SoftAP primaria
	/// </summary>
	/// <returns>ERROR_SUCCESS si no ha habido ningún error</returns>
public: DWORD GetIPFromHostedWifi() {
	PWLAN_HOSTED_NETWORK_STATUS pWlanHostedNetworkStatus = NULL;
	NET_LUID InterfaceLuid = { 0 };
	MIB_IPINTERFACE_ROW Row = { 0 };

	DWORD returnValue = WlanHostedNetworkQueryStatus(
		hClientHandle,
		&pWlanHostedNetworkStatus,
		NULL
	);

	switch (returnValue)
	{
	case ERROR_SUCCESS:
		this->Debug(L"GetIPFromWifi- WlanHostedNetworkQueryStatus()  returns OK\n");
		break;
	case ERROR_INVALID_HANDLE:
		this->Debug(L"GetIPFromWifi returns ERROR_INVALID_HANDLE\n");
		return ERROR_INVALID_HANDLE;
	case ERROR_INVALID_PARAMETER:
		this->Debug(L"GetIPFromWifi returns ERROR_INVALID_PARAMETER\n");
		return ERROR_INVALID_PARAMETER;
	case ERROR_INVALID_STATE:
		this->Debug(L"GetIPFromWifi returns ERROR_INVALID_STATE\n");
		return ERROR_INVALID_STATE;
	case ERROR_SERVICE_NOT_ACTIVE:
		this->Debug(L"GetIPFromWifi returns ERROR_SERVICE_NOT_ACTIVE\n");
		return ERROR_SERVICE_NOT_ACTIVE;
	default:
		this->Debug(L"GetIPFromWifi returns UNKNOW ERRROR:\n", returnValue);
		return returnValue;
	}

	returnValue = ConvertInterfaceGuidToLuid(
		&pWlanHostedNetworkStatus->IPDeviceID,
		&InterfaceLuid
	);
	if (returnValue != 0)
	{
		this->Debug(L"WlanGetProfile()- ConvertInterfaceGuidToLuid returns STATUS_INVALID_PARAMETER\n");
		return ERROR_INVALID_PARAMETER;
	}
	Row.InterfaceLuid = InterfaceLuid;
	Row.Family = AF_INET;

	returnValue = GetIpInterfaceEntry(
		&Row
	);
	switch (returnValue)
	{
	case 0:
		this->Debug(L"GetIPFromWifi- GetIpInterfaceEntry  returns OK\n");
		break;
	case STATUS_INVALID_PARAMETER:
		this->Debug(L"GetIPFromWifi- GetIpInterfaceEntry returns STATUS_INVALID_PARAMETER\n");
		return ERROR_INVALID_PARAMETER;
		//case STATUS_NOT_FOUND:
		//	this->Debug(L"GetIPFromWifi- GetIpInterfaceEntry returns STATUS_NOT_FOUND\n");
		//	return STATUS_NOT_FOUND;
	default:
		this->Debug(L"GetIPFromWifi returns UNKNOW ERRROR:\n", returnValue);
		return ERROR_ERRORS_ENCOUNTERED;
	}
}

/// <summary>
/// Obtiene el xml del archivo que guarda el password (por así decirlo) en el equipo
/// </summary>
/// <param name="WifiIndex">Indice del Array pBssList[WifiIndex] obtenido en la llamada de la WlanAPI "WlanGetAvailableNetworkList()" 
///  a la que se llama en el interior de WinSockServer3::EnumerarWifis()</param>
/// <returns>Cero si falla, puntero LPWSTR al texto XML si funciona.
///The caller is responsible for calling the WlanFreeMemory() function to free the memory 
///allocated for the buffer pointer to by the pstrProfileXml 
///parameter when the buffer is no longer needed.</returns>
public:LPWSTR GetProfile(int InterfaceIndex, int WifiIndex) {

	LPWSTR strProfileXml;
	DWORD* pdwFlags = NULL;
	DWORD dwGrantedAccess = WLAN_WRITE_ACCESS;
	this->Debug(L"Dentro de WinSockServer3::GetProfile()\n");
	if (hClientHandle == NULL) {

		this->Debug(L"GetProfile() hClientHandle == NULL - Falto una llamada a EnumerarWifis() de inicialización\n");
		return 0;
	}
	if (pIfList == NULL) {

		this->Debug(L"GetProfile() pIfList == NULL - Falto una llamada a EnumerarWifis() de inicialización\n");
		return 0;
	}
	if (pBssList == NULL) {

		this->Debug(L"GetProfile() pBssList == NULL - Falto una llamada a EnumerarWifis() de inicialización\n");
		return 0;
	}

	PWLAN_INTERFACE_INFO	EntryIfInfo = pIfList[InterfaceIndex].InterfaceInfo;
	PWLAN_AVAILABLE_NETWORK	Entry = pBssList[WifiIndex].Network;
	DWORD returnValue = WlanGetProfile(
		hClientHandle,				//The client's session handle, obtained by a previous call to the WlanOpenHandle function.
		&EntryIfInfo->InterfaceGuid,//The GUID of the wireless interface. A list of the GUIDs for wireless interfaces on the local computer can be retrieved using the WlanEnumInterfaces function.
		Entry->strProfileName,		//strProfileName_2 The name of the profile.
		NULL,						//Reserved for future use. Must be set to NULL.
		&strProfileXml,				//A string that is the XML representation of the queried profile. There is no predefined maximum string length.
		pdwFlags,					//On input, a pointer to the address location used to provide additional information about the request. 
									//If this parameter is NULL on input, then no information on profile
									//flags will be returned. On output, a pointer to the address
									//location used to receive profile flags.
		&dwGrantedAccess			//The access mask of the all-user profile.
									//WLAN_WRITE_ACCESS - WLAN_EXECUTE_ACCESS - WLAN_READ_ACCESS
	);

	switch (returnValue)
	{
	case ERROR_SUCCESS:
		this->Debug(L"WlanGetProfile  returns OK\n");
		break;
	case ERROR_INVALID_PARAMETER:
		this->Debug(L"WlanGetProfile returns ERROR_INVALID_PARAMETER\n");
		return 0;
	case ERROR_INVALID_HANDLE:
		this->Debug(L"WlanGetProfile returns ERROR_INVALID_HANDLE\n");
		return 0;
	case ERROR_NOT_ENOUGH_MEMORY:
		this->Debug(L"WlanGetProfile returns ERROR_NOT_ENOUGH_MEMORY\n");
		return 0;
	case ERROR_ACCESS_DENIED:
		this->Debug(L"WlanGetProfile returns ERROR_ACCESS_DENIED\n");
		return 0;
	case ERROR_NOT_FOUND:
		this->Debug(L"WlanGetProfile returns ERROR_NOT_FOUND\n");
		return 0;
	default:
		this->Debug(L"WlanGetProfile returns UNKNOW ERRROR:\n", returnValue);
		return 0;
	}

}
	  /// <summary>
	  /// Versión de prueba. Utilizar ConnectWifi_byProfile();
	  /// puede resultar interesante para repasar como se rellenan estructuras como 
	  /// DOT11_BSSID_LIST 
	  /// NDIS_OBJECT_HEADER
	  /// </summary>
	  /// <param name="WifiIndex"></param>
	  /// <returns></returns>
protected: DWORD ConnectWifi_prv(int AdapterIndex, int WifiIndex) {

	OutputDebugString(L"Dentro de ConnectWifi_prv()\n");

	PWLAN_INTERFACE_INFO			pIfInfo = NULL;
	PWLAN_AVAILABLE_NETWORK			pBssEntry = NULL;
	unsigned int k = 0, n = 0;
	if (hClientHandle == NULL) {
		this->Debug(L"ConnectWifi_prv()-ERROR_INVALID_PARAMETER");
		return ERROR_INVALID_PARAMETER;
	}
	if (pIfList == NULL) {
		this->Debug(L"GetWifiDetails() Error - pIfList == NULL. No se ha inicializado el sistema con una llamada a EnumerarWifis()");
		return ERROR_INVALID_PARAMETER;
	}
	if (pBssList == NULL) {
		this->Debug(L"GetWifiDetails() Error - pBssList == NULL. No se ha inicializado el sistema con una llamada a EnumerarWifis()");
		return ERROR_INVALID_PARAMETER;
	}
	pIfInfo = pIfList[AdapterIndex].InterfaceInfo;
	pBssEntry = (WLAN_AVAILABLE_NETWORK*)&pBssList->Network[WifiIndex];
	ULONG uSSIDLength = pBssEntry->dot11Ssid.uSSIDLength;
	UCHAR ucSSID[DOT11_SSID_MAX_LENGTH + 1];

	for (k = 0; k < pBssEntry->dot11Ssid.uSSIDLength; k++) {
		ucSSID[k] = pBssEntry->dot11Ssid.ucSSID[k];
	}
	//specifies the SSID of the network to connect to.
	DOT11_SSID  Dot11Ssid = { 0 };
	Dot11Ssid.uSSIDLength = uSSIDLength;
	for (n = 0; n < uSSIDLength; n++) Dot11Ssid.ucSSID[n] = ucSSID[n];

	PWLAN_BSS_LIST pWlanBssList; DOT11_MAC_ADDRESS Bssid;
	DWORD returnValue = WlanGetNetworkBssList(
		hClientHandle,
		&pIfInfo->InterfaceGuid,
		NULL,//&Dot11Ssid,
		dot11_BSS_type_any,
		TRUE,
		NULL,
		&pWlanBssList
	);
	if (pWlanBssList->dwNumberOfItems > 0) {
		for (int n = 0; n < 6; n++) Bssid[n] = pWlanBssList->wlanBssEntries[0].dot11Bssid[n];
	}
	else
	{
		this->Debug(L"ConnectWifi_prv()-WlanGetNetworkBssList no ha podido obtener direccion MAC del dispositivo que transmite la Wifi especificada\n");
		return ERROR_INVALID_PARAMETER;
	}
	WLAN_BSS_ENTRY Entry = { 0 };
	returnValue = 0;
	for (unsigned int n = 0; n < pWlanBssList->dwNumberOfItems; n++)
	{
		Entry = pWlanBssList->wlanBssEntries[n];
		if (uSSIDLength == Entry.dot11Ssid.uSSIDLength) {
			returnValue = 1;
			for (unsigned int n = 0; n < uSSIDLength; n++) {
				if (Entry.dot11Ssid.ucSSID[n] != ucSSID[n]) { returnValue = 0; break; }
			}
		}
		if (returnValue == 1)break;
	}
	if (returnValue == 1) {
		this->Debug(L"ConnectWifi_prv()Mac encontrada\n");
	}
	else
	{
		this->Debug(L"ConnectWifi_prv()Mac NO encontrada\n");
	}
	NDIS_OBJECT_HEADER Header = { 0 };
	Header.Type = NDIS_OBJECT_TYPE_DEFAULT;//Type.Specifies the type of NDIS object that a structure describes.
	Header.Revision = DOT11_BSSID_LIST_REVISION_1; //Revision.Specifies the revision number of this structure.
	Header.Size = sizeof(DOT11_BSSID_LIST);
	//Header.Size = (ushort)System.Runtime.InteropServices.Marshal.SizeOf(typeof(Wlan.DOT11_BSSID_LIST));
	const ULONG NumEntries = 1;

	//Pointer to a DOT11_BSSID_LIST structure 
	//that contains the list of basic service set (BSS) 
	//identifiers desired for the connection.
	DOT11_BSSID_LIST    DesiredBssidList = { 0 };
	DesiredBssidList.Header = Header;
	DesiredBssidList.uNumOfEntries = NumEntries;
	DesiredBssidList.uTotalNumOfEntries = NumEntries;
	for (int n = 0; n < 6; n++) DesiredBssidList.BSSIDs[0][n] = Entry.dot11Bssid[n];

	WLAN_CONNECTION_PARAMETERS ConnectionParameters;
	//wlanConnectionMode=						A WLAN_CONNECTION_MODE value that specifies the mode of connection.
	//wlan_connection_mode_profile				A profile will be used to make the connection.
	//wlan_connection_mode_temporary_profile	A temporary profile will be used to make the connection.
	//wlan_connection_mode_discovery_secure		Secure discovery will be used to make the connection.
	//wlan_connection_mode_discovery_unsecure	Unsecure discovery will be used to make the connection.
	//wlan_connection_mode_auto					The connection is initiated by the wireless service automatically using a persistent profile.
	//wlan_connection_mode_invalid				Not used.

	memset(&ConnectionParameters, 0, sizeof(WLAN_CONNECTION_PARAMETERS));
	ConnectionParameters.wlanConnectionMode = wlan_connection_mode_profile;// wlan_connection_mode_auto;
   //Specifies the profile being used for the connection.
	ConnectionParameters.strProfile = pBssList->Network[WifiIndex].strProfileName;
	//the BSS type of the network.
	ConnectionParameters.dot11BssType = dot11_BSS_type_any;//  pBssList->Network[WifiIndex].dot11BssType;
	//the SSID of the network to connect to.
	//When set to NULL, all SSIDs in the profile will be tried. 
	//ConnectionParameters.pDot11Ssid = NULL; //&Dot11Ssid;
	//contains the list of basic service set (BSS)desired for the connection.
	//ConnectionParameters.pDesiredBssidList = 0,//&DesiredBssidList;			
	//dwFlags=															The following table shows flags used to specify the connection parameters.
	//WLAN_CONNECTION_HIDDEN_NETWORK									Connect to the destination network even if the destination is a hidden network.A hidden network does not broadcast its SSID.Do not use this flag if the destination network is an ad - hoc network.If the profile specified by strProfile is not NULL, then this flag is ignoredand the nonBroadcast profile element determines whether to connect to a hidden network.
	//WLAN_CONNECTION_ADHOC_JOIN_ONLY									Do not form an ad - hoc network.Only join an ad - hoc network if the network already exists.Do not use this flag if the destination network is an infrastructure network.
	//WLAN_CONNECTION_IGNORE_PRIVACY_BIT								Ignore the privacy bit when connecting to the network.Ignoring the privacy bit has the effect of ignoring whether packets are encrypted and ignoring the method of encryption used.Only use this flag when connecting to an infrastructure network using a temporary profile.
	//WLAN_CONNECTION_EAPOL_PASSTHROUGH									Exempt EAPOL traffic from encryption and decryption.This flag is used when an application must send EAPOL traffic over an infrastructure network that uses Open authentication and WEP encryption.This flag must not be used to connect to networks that require 802.1X authentication.This flag is only valid when wlanConnectionMode is set to wlan_connection_mode_temporary_profile.Avoid using this flag whenever possible.
	//WLAN_CONNECTION_PERSIST_DISCOVERY_PROFILE							Automatically persist discovery profile on successful connection completion.This flag is only valid for wlan_connection_mode_discovery_secure or wlan_connection_mode_discovery_unsecure.The profile will be saved as an all user profile, with the name generated from the SSID using WlanUtf8SsidToDisplayName.If there is already a profile with the same name, a number will be appended to the end of the profile name.The profile will be saved with manual connection mode, unless WLAN_CONNECTION_PERSIST_DISCOVERY_PROFILE_CONNECTION_MODE_AUTO is also specified.
	//WLAN_CONNECTION_PERSIST_DISCOVERY_PROFILE_CONNECTION_MODE_AUTO	To be used in conjunction with WLAN_CONNECTION_PERSIST_DISCOVERY_PROFILE.The discovery profile will be persisted with automatic connection mode.
	//WLAN_CONNECTION_PERSIST_DISCOVERY_PROFILE_OVERWRITE_EXISTING		To be used in conjunction with WLAN_CONNECTION_PERSIST_DISCOVERY_PROFILE.The discovery profile will be persisted and attempt to overwrite an existing profile with the same name.
	//ConnectionParameters.dwFlags = 0;
		//WLAN_CONNECTION_HIDDEN_NETWORK| 
		//WLAN_CONNECTION_ADHOC_JOIN_ONLY;

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
		OutputDebugString(L"ConnectWifi_prv()-WlanConnect()-conectado\n");
		break;
	case ERROR_INVALID_PARAMETER:
		this->Debug(L"ConnectWifi_prv()-WlanConnect()-ERROR_INVALID_PARAMETER");
		return returnValue;
	case ERROR_INVALID_HANDLE:
		this->Debug(L"ConnectWifi_prv()-WlanConnect()-ERROR_INVALID_HANDLE");
		return returnValue;
	case ERROR_ACCESS_DENIED:
		this->Debug(L"ConnectWifi_prv()-WlanConnect()-ERROR_ACCESS_DENIED");
		return returnValue;
	default:
		this->Debug(L"ConnectWifi_prv()-WlanConnect()-UNKNOW ERROR:", returnValue);
		return returnValue;
	}
	return ERROR_SUCCESS;
}

/// <summary>
/// Muestra en la pantalla de depuración el nombre de la Wifi
/// </summary>
/// <param name="WifiIndex">indice del arreglo BssList->Network[] donde se ha memorizado la Wifi</param>
/// <returns>ERROR_SUCCESS si funciona, ERROR_INVALID_PARAMETER si no se ha inicializado pBssList</returns>
public: DWORD MostrarWifi(int WifiIndex) {

	PWLAN_AVAILABLE_NETWORK			pBssEntry = NULL;
	if (pBssList == NULL)
		return ERROR_INVALID_PARAMETER;
	pBssEntry = (WLAN_AVAILABLE_NETWORK*)&pBssList->Network[WifiIndex];
	UCHAR SSID[DOT11_SSID_MAX_LENGTH + 1];
	wchar_t wSSID[DOT11_SSID_MAX_LENGTH + 1];
	unsigned int k;
	for (k = 0; k < pBssEntry->dot11Ssid.uSSIDLength; k++) {
		SSID[k] = pBssEntry->dot11Ssid.ucSSID[k];
		wSSID[k] = pBssEntry->dot11Ssid.ucSSID[k];
	}
	SSID[k] = 0;
	wSSID[k] = 0;
	this->Debug(L"Wifi()  SSID[", WifiIndex, L"]:", wSSID);
	return ERROR_SUCCESS;
}
/// <summary>
/// GetAdaptersInfo_Test se implementó para testear el funcionamiento de GetIpAddrTable()
// de la API "IP Helper" de Windows
///https://docs.microsoft.com/en-us/windows/win32/iphlp/managing-ip-addresses-using-getipaddrtable
/// </summary>
/// <returns>ERROR_SUCCESS si no ha habido error</returns>
public:DWORD GetIpAddrTable_Test() {
	//
	MIB_IPADDRTABLE* pIPAddrTable;
	DWORD            dwSize = 0;
	DWORD            dwRetVal;
	ULONG            ulOutBufLen;
	unsigned int     i;

	IP_INTERFACE_INFO* pInterfaceInfo;

	/********************************************************* IP ***********************************************************/
	//Allocate memory for the structure.
	pIPAddrTable = (MIB_IPADDRTABLE*)malloc(sizeof(MIB_IPADDRTABLE));

	//Make an initial call to GetIpAddrTable to get the size needed into the dwSize variable.
	//This call to the function is meant to fail, and is used to ensure that the dwSize variable 
	//specifies a size sufficient for holding all the information returned to pIPAddrTable.
	if ((dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0)) == ERROR_INSUFFICIENT_BUFFER) {
		free(pIPAddrTable);
		pIPAddrTable = (MIB_IPADDRTABLE*)malloc(dwSize);

		dwRetVal = GetIpAddrTable(pIPAddrTable, &dwSize, 0);
	}
	if (dwRetVal != NO_ERROR) {
		this->Debug(L"Investigate()-GetIpAddrTable call failed with %d\n", dwRetVal);
		return dwRetVal;
	}

	/********************************************************* IF ***********************************************************/

	pInterfaceInfo = (IP_INTERFACE_INFO*)malloc(sizeof(IP_INTERFACE_INFO));
	ulOutBufLen = sizeof(IP_INTERFACE_INFO);
	//This call to the function is meant to fail, and is used to ensure that the ulOutBufLen variable
	//specifies a size sufficient for holding all the information returned to pInfo.This is a common 
	//programming model in IP Helper for data structuresand functions of this type.
	if ((dwRetVal = GetInterfaceInfo(pInterfaceInfo, &ulOutBufLen)) == ERROR_INSUFFICIENT_BUFFER) {
		free(pInterfaceInfo);
		pInterfaceInfo = (IP_INTERFACE_INFO*)malloc(ulOutBufLen);

		dwRetVal = GetInterfaceInfo(pInterfaceInfo, &ulOutBufLen);
	}
	if (dwRetVal != NO_ERROR) {
		this->Debug(L"Investigate()-GetInterfaceInfo call failed with %d\n", dwRetVal);
		return dwRetVal;
	}
	/********************************************************* SHOW ***********************************************************/



	const int cadena_len = 100;
	wchar_t cadena[cadena_len];
	for (i = 0; i < pIPAddrTable->dwNumEntries; i++)
	{
		this->Debug(L"**************************************************************\n");
		this->Debug(L"IP Address:         ", pIPAddrTable->table[i].dwAddr, L" = ", InetNtopW(AF_INET, &pIPAddrTable->table[i].dwAddr, cadena, cadena_len));
		this->Debug(L"IP Mask:            ", pIPAddrTable->table[i].dwMask, L" = ", InetNtopW(AF_INET, &pIPAddrTable->table[i].dwMask, cadena, cadena_len));
		this->Debug(L"IF Index:           ", pIPAddrTable->table[i].dwIndex);
		this->Debug(L"Broadcast Addr:     ", pIPAddrTable->table[i].dwBCastAddr);
		this->Debug(L"Re-assembly size:   ", pIPAddrTable->table[i].dwReasmSize);
		this->Debug(L"");
		//InetNtopW(AF_INET, &pIPAddrTable->table[i].dwAddr, comodin, COMODIN_LENGHT);
	}


	this->Debug(L"  Num Adapters: ", pInterfaceInfo->NumAdapters);
	for (i = 0; i < (unsigned int)pInterfaceInfo->NumAdapters; i++) {
		this->Debug(L"  Adapter Index[", i, L"]", pInterfaceInfo->Adapter[i].Index);
		this->Debug(L"  Adapter Name[", i, L"]", pInterfaceInfo->Adapter[i].Name);
	}

	if (pIPAddrTable)
		free(pIPAddrTable);
	return ERROR_SUCCESS;
}

/// <summary>
/// GetAdaptersInfo_Test se implementó para testear el funcionamiento de GetAdaptersInfo()
// de la API "IP Helper" de Windows
///https://docs.microsoft.com/en-us/windows/win32/api/iphlpapi/nf-iphlpapi-getadaptersinfo
/// </summary>
/// <returns>ERROR_SUCCESS si no ha habido error</returns>
public:DWORD GetAdaptersInfo_Test() {
	/* variables used for GetAdapterInfo */
	IP_ADAPTER_INFO* pAdapterInfo;
	IP_ADAPTER_INFO* pAdapter;

	/* variables used to print DHCP time info */
	struct tm newtime;
	char buffer[32];
	errno_t error;

	/* Some general variables */
	const int cadena_len = 100;
	wchar_t cadena[cadena_len];
	ULONG ulOutBufLen;
	DWORD dwRetVal;
	unsigned int i;

	this->Debug(L"------------------------");
	this->Debug(L"This is GetAdaptersInfo");
	this->Debug(L"------------------------");

	pAdapterInfo = (IP_ADAPTER_INFO*)WinSockSocket_extender_MALLOC(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL) {
		this->Debug(L"Error allocating memory needed to call GetAdapterInfo");
		return ERROR_BUFFER_OVERFLOW;
	}
	ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		WinSockSocket_extender_FREE(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)WinSockSocket_extender_MALLOC(ulOutBufLen);
		if (pAdapterInfo == NULL) {
			this->Debug(L"Error allocating memory needed to call GetAdapterInfo");
			return ERROR_BUFFER_OVERFLOW;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) != NO_ERROR) {
		this->Debug(L"GetAdaptersInfo failed with error ", dwRetVal);
		if (pAdapterInfo)
			WinSockSocket_extender_FREE(pAdapterInfo);
		return dwRetVal;
	}

	pAdapter = pAdapterInfo;
	while (pAdapter) {
		this->Debug(L"\tAdapter Name: \t", pAdapter->AdapterName);
		this->Debug(L"\tAdapter Desc: \t", pAdapter->Description);

		/***************************************************************************************/
		wchar_t buf[5], buf2[20];
		buf2[0] = 0;
		for (i = 0; i < (int)pAdapter->AddressLength; i++) {
			if (i == (pAdapter->AddressLength - 1))
			{
				Int2Hex((byte)pAdapter->Address[i], buf);
				lstrcatW(buf2, buf);
				//printf("%.2X\n", (int)pAdapter->Address[i]);
			}
			else
			{
				Int2Hex((byte)pAdapter->Address[i], buf);
				lstrcatW(buf2, buf);
				lstrcatW(buf2, L".");
				//printf("%.2X-", (int)pAdapter->Address[i]);
			}
		}

		this->Debug(L"\tAdapter Addr: \t", buf2);
		/***************************************************************************************/

		this->Debug(L"\tIP Address: \t",
			pAdapter->IpAddressList.IpAddress.String);
		this->Debug(L"\tIP Mask: \t\t", pAdapter->IpAddressList.IpMask.String);

		this->Debug(L"\tGateway: \t\t", pAdapter->GatewayList.IpAddress.String);

		if (pAdapter->DhcpEnabled) {
			this->Debug(L"\tDHCP Enabled: \tYes");
			this->Debug(L"\tDHCP Server: \t", pAdapter->DhcpServer.IpAddress.String);

			/* Display local time */
			error = _localtime32_s(&newtime, (__time32_t*)&pAdapter->LeaseObtained);
			if (error)
				this->Debug(L"\tLease Obtained:\tInvalid Argument to _localtime32_s");

			else {
				// Convert to an ASCII representation 
				error = asctime_s(buffer, 32, &newtime);
				if (error)
					this->Debug(L"\tLease Obtained:\tInvalid Argument to asctime_s");
				else
					/* asctime_s returns the string terminated by \n\0 */
					this->Debug(L"\tLease Obtained:\t", buffer);
			}

			error = _localtime32_s(&newtime, (__time32_t*)&pAdapter->LeaseExpires);
			if (error)
				this->Debug(L"\tLease Expires:\tInvalid Argument to _localtime32_s");
			else {
				// Convert to an ASCII representation 
				error = asctime_s(buffer, 32, &newtime);
				if (error)
					this->Debug(L"\tLease Expires:\tInvalid Argument to asctime_s");
				else
					/* asctime_s returns the string terminated by \n\0 */
					this->Debug(L"\tLease Expires:\t", buffer);
			}
		}
		else
			this->Debug(L"\tDHCP Enabled: \tNo");

		if (pAdapter->HaveWins) {
			this->Debug(L"\tHave Wins: \tYes");
			this->Debug(L"\tPrimary Wins Server: \t", pAdapter->PrimaryWinsServer.IpAddress.String);
			this->Debug(L"\tSecondary Wins Server: \t", pAdapter->SecondaryWinsServer.IpAddress.String);
		}
		else
			this->Debug(L"\tHave Wins: \tNo");

		this->Debug(L"\n");
		pAdapter = pAdapter->Next;
	}
	return ERROR_SUCCESS;
}


public:template <class T, class U>
	U* Int2Hex(T lnumber, U* buffer)
{
	const char* ref = "0123456789ABCDEF";
	T hNibbles = (lnumber >> 4);

	unsigned char* b_lNibbles = (unsigned char*)&lnumber;
	unsigned char* b_hNibbles = (unsigned char*)&hNibbles;

	U* pointer = buffer + (sizeof(lnumber) << 1);

	*pointer = 0;
	do {
		*--pointer = ref[(*b_lNibbles++) & 0xF];
		*--pointer = ref[(*b_hNibbles++) & 0xF];
	} while (pointer > buffer);

	return buffer;
}
public:wchar_t* Debug(const wchar_t* message) {
	int pos;
	switch (OUTPUT)
	{
	case OUTPUT_t::LISTBOX:
		pos = (int)SendMessage(ListBox_forOutput, LB_ADDSTRING, 0,
			(LPARAM)message);
		break;
	case OUTPUT_t::DEBUG:
	default:
		lstrcpyW(comodin, message);
		lstrcatW(comodin, L"\n");
		OutputDebugString(comodin);
		break;
	}
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, wchar_t* message2) {


	lstrcpyW(comodin, message);
	lstrcatW(comodin, message2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(wchar_t* message, char* message2) {
	wchar_t  comodin2[1024];
	size_t length = 0;
	int size = strlen(message2);
	mbstowcs_s(&length, comodin2, message2, size + 1);

	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, const char* message2) {
	wchar_t  comodin2[1024];
	size_t length = 0;
	int size = strlen(message2);
	mbstowcs_s(&length, comodin2, message2, size + 1);

	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2, int number2) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);
	_itow_s(number2, comodin2, 10);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2, int number2, const wchar_t* message3) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);
	_itow_s(number2, comodin2, 10);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message3);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2, int number2, const wchar_t* message3, int number3) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);
	_itow_s(number2, comodin2, 10);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message3);
	_itow_s(number3, comodin2, 10);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2, int number2, const wchar_t* message3, int number3, const wchar_t* message4) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);
	_itow_s(number2, comodin2, 10);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message3);
	_itow_s(number3, comodin2, 10);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message4);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, const wchar_t* message2, const wchar_t* message3) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	lstrcatW(comodin, message2);
	lstrcatW(comodin, message3);
	Debug(comodin);
	return comodin;
}
public:wchar_t* Debug(const wchar_t* message, int number, int number2) {
	wchar_t  comodin2[1024];
	_itow_s(number, comodin2, 10);
	lstrcpyW(comodin, message);
	lstrcatW(comodin, comodin2);
	_itow_s(number2, comodin2, 10);
	lstrcatW(comodin, comodin2);
	Debug(comodin);
	return comodin;
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
		Debug(L"Error with FormatMessage\n");
	}
	return lpBuffer;
}
public:DWORD WlanOpenHandle_ShowError(DWORD returnValue)
{
	switch (returnValue)
	{
	case ERROR_SUCCESS:
		this->Debug(L"WlanOpenHandle returns OK\n");
		return returnValue;
	case ERROR_INVALID_PARAMETER:
		this->Debug(L"WlanOpenHandle returns ERROR_INVALID_PARAMETER\n");
		return returnValue;
	case ERROR_NOT_ENOUGH_MEMORY:
		this->Debug(L"WlanOpenHandle returns ERROR_NOT_ENOUGH_MEMORY\n");
		return returnValue;
	case ERROR_REMOTE_SESSION_LIMIT_EXCEEDED:
		this->Debug(L"WlanOpenHandle returns ERROR_REMOTE_SESSION_LIMIT_EXCEEDED\n");
		return returnValue;
	default:
		this->Debug(L"WlanOpenHandle returns UNKNOW ERRROR:\n", returnValue);
		return returnValue;
	}
}
};