// Interfaz Grafica.cpp : Define el punto de entrada de la aplicación.
//

#include "framework.h"
#include "Interfaz Grafica.h"
#include "WindowsMessages.h"
#include "strsafe.h"
#include "PaintLEDS.h"
#include "Led.h"
#include "SwitchButton.h"

#define WM_APP_WIFI_CONNECTED WM_APP
#define ID_TIMER1 1001
#define TIMER1_MILLIS 1000
#include "WinSockServer3.h"
#include "UserInterface.h"

#define MAX_LOADSTRING 100
// Variables globales:
HINSTANCE       hInst;                                // instancia actual
WCHAR           szTitle[MAX_LOADSTRING];                  // Texto de la barra de título
WCHAR           szWindowClass[MAX_LOADSTRING];            // nombre de clase de la ventana principal
HWND            hwndMain;
WinSockServer3  WSS;
UserInterface   UI;
int portNumber = WinSockServer3::portNumber_default;
int             isConected = 0;
const int       LedTop = 200,LedLeft=500;

RECT            rectPicture3 = { LedLeft,LedTop,LedLeft+200,LedTop+200 };
const int       LedArrayLen = 4;
Led             LedArray[LedArrayLen];
const int       SwitchArrayLen = 4;
SwitchButton    SwitchArray[SwitchArrayLen];

// Declaraciones de funciones adelantadas incluidas en este módulo de código:
INT_PTR CALLBACK WndProc(HWND , UINT , WPARAM , LPARAM );
static WinSockServer3::WlanNotificationCallback_prv_Data CallBackData = { 0 };
void iniLedArrays(int left,int top,int margin,int diameter) {
    for (int i = 0; i < LedArrayLen; i++)
    {
        LedArray[i].on = true;
        LedArray[i].SetDimensions(diameter, diameter);
        LedArray[i].SetPos(left+(i*(diameter+margin)), top);
    }
    LedArray[0].color = RGB(0, 0, 255);
    LedArray[1].color = RGB(255, 255, 0);
    LedArray[2].color = RGB(255, 0, 0);
    LedArray[3].color = RGB(0, 255, 0);
}

void iniSwitchArrays(int left, int top, int margin, int width,int height) {
    for (int i = 0; i < LedArrayLen; i++)
    {
        SwitchArray[i].on = true;
        SwitchArray[i].SetDimensions(width, height);
        SwitchArray[i].SetPos(left + (i * (width + margin)), top);
    }
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    hInst = hInstance; 
    // Inicializar cadenas globales
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_INTERFAZGRAFICA, szWindowClass, MAX_LOADSTRING);

    hwndMain = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DIALOG1), NULL, WndProc);

    SetTimer(hwndMain, ID_TIMER1, TIMER1_MILLIS, NULL);

    WSS.EnumerarWifis(GetDlgItem(hwndMain, IDC_COMBO_ADAPTER), GetDlgItem(hwndMain, IDC_COMBO_WIFI),GetDlgItem(hwndMain, IDC_LIST_ADAPTER),GetDlgItem(hwndMain,IDC_LIST_WIFI2));

    ShowWindow(hwndMain, SW_SHOW);
    iniLedArrays(LedLeft, LedTop, 10, 20);
    iniSwitchArrays(LedLeft, LedTop+40, 10, 20,50);
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_INTERFAZGRAFICA));

    MSG msg;

    // Bucle principal de mensajes:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}
/// <summary>
/// Carga el Combo box con todas las IP's disponibles para trasferencia TCP/IP en el PC,
/// pero deja seleccionada la IP relacionada con el adaptador seleccionado en el ComboBox IDC_COMBO_ADAPTER
/// </summary>
/// <param name="hDlg">Manejador de la ventana principal</param>
void UpdateIPComboBox(HWND hDlg) {
    DWORD dwRetVal=0;
    IP_ADAPTER_INFO pAdapter_OUT = { 0 };
    IP_ADDR_STRING* ip_addr_str;
    ADDRINFOW result = { 0 };
    GUID guid = { 0 };
    LRESULT r1;

    /************conversion de char a wchar_T**************/
    wchar_t  comodin2[50];
    comodin2[0] = 0;
    size_t length;
    int size;

    //Cargamos en el LisBox del adaptador "IDC_LIST_IPs"  el listado de las IP's relaccionadas con el adaptador seleccionado
    int selected = SendMessage(GetDlgItem(hDlg, IDC_COMBO_ADAPTER), CB_GETCURSEL, 0, 0);
    if (WSS.GetSelectedGUIDFromInternalList(selected, &guid) != NULL) {
        if ((dwRetVal = WSS.FindAdapter(guid, &pAdapter_OUT)) == ERROR_SUCCESS) {
             r1 = SendMessage(GetDlgItem(hDlg, IDC_LIST_IPs), LB_RESETCONTENT, 0, 0);
            ip_addr_str = &pAdapter_OUT.IpAddressList;
            while (ip_addr_str) {
                length = 0;
                size = strlen(ip_addr_str->IpAddress.String);
                mbstowcs_s(&length, comodin2, ip_addr_str->IpAddress.String, size + 1);
                SendMessage(GetDlgItem(hDlg, IDC_LIST_IPs), LB_RESETCONTENT, 0, (LPARAM)comodin2);
                if (strcmp("0.0.0.0", ip_addr_str->IpAddress.String) != 0) {
                    r1 = SendMessage(GetDlgItem(hDlg, IDC_LIST_IPs), LB_ADDSTRING, 0, (LPARAM)comodin2);
                }
                ip_addr_str = ip_addr_str->Next;
            }
        }
        else {
             r1 = SendMessage(GetDlgItem(hDlg, IDC_LIST_IPs), LB_RESETCONTENT, 0, 0);
        }
    }
    //Carga el ComboBox_IP "IDC_COMBO_IP_PC" con todas las IP's devueltas por GetAddrInfoW() llamada desde 
    //de la funcion WSS.GetIPList()
    if ((dwRetVal=WSS.GetIPList(&result)) == ERROR_SUCCESS)
    {
    SendMessage(GetDlgItem(hDlg, IDC_COMBO_IP_PC), CB_RESETCONTENT, 0, 0);
    int last = WSS.GetIPNumberOfAdresses();
    int found = 0;
        for (int n = 0; n < last; n++)
        {
            wchar_t* IP = WSS.GetIPAddress(n);
            SendMessage(GetDlgItem(hDlg, IDC_COMBO_IP_PC), CB_ADDSTRING, 0, (LPARAM)IP);
            //Si una de las IP coincide con la relacionada con el adaptador inalambrico,
            //la memoriza en "found" para dejarla seleccionada en el comboBox
            if (!found)
            {
                if(lstrcmpiW(comodin2, IP)==0)found=n;
            }
        }
        SendMessage(GetDlgItem(hDlg, IDC_COMBO_IP_PC), CB_SETCURSEL,found, 0 );
    }
}
/// <summary>
/// Usa los dator guardados para presentar el listado de detalles de Wifi en funcion del ComboBox "Wifi".
///No actualiza los datos internos, simplemente presenta los datos cargados con EnumerarWifis();
/// Actualiza el estado (Enable/Disable) del botón conectar en funcion de las propiedades de la WiFi seleccionada
/// Actualiza el comboBox con las direcciones IP
/// </summary>
/// <param name="hDlg">Manejador de ventana del Cuadro de dialogo principal</param>
void UpdateWifiDetailsList(HWND hDlg) {

    int WifiIndex = SendMessage(GetDlgItem(hDlg, IDC_COMBO_WIFI), (UINT)CB_GETCURSEL,
        (WPARAM)0, (LPARAM)0);
    if (WifiIndex != CB_ERR)
    {
        WLAN_AVAILABLE_NETWORK* pBssEntry = WSS.GetWifiDetails(WifiIndex, GetDlgItem(hDlg, IDC_LIST_WIFI2));
        if (pBssEntry) {
            isConected = (pBssEntry->dwFlags & WLAN_AVAILABLE_NETWORK_CONNECTED);
            int hasProfile = (pBssEntry->dwFlags & WLAN_AVAILABLE_NETWORK_HAS_PROFILE);
            EnableWindow(GetDlgItem(hDlg, IDC_BTN_CONNECT_WIFI), (hasProfile && !isConected));
            UpdateIPComboBox(hDlg);
        }
    }
}

/// <summary>
/// Actualiza el listado de detalles del adaptador en función del ComboBox "Adaptador"
/// Pendiente de programar: Actualiza el listado de Redes Wifi detectadas en funcion del ComboBox "Adaptador"
/// Actualiza el listado de detalles de Wifi en funcion del ComboBox "Wifi"
/// Actualiza el estado (Enable/Disable) del botón conectar en funcion de las propiedades de la WiFi seleccionada
/// </summary>
/// <param name="hDlg">Manejador de ventana del Cuadro de dialogo principal</param>
void UpdateInterfaceDetialsList(HWND hDlg) {

    //***********Mostrar el contenido del adapter******************
    int AdapterIndex = SendMessage(GetDlgItem(hDlg, IDC_COMBO_ADAPTER), (UINT)CB_GETCURSEL,
        (WPARAM)0, (LPARAM)0);
    if (AdapterIndex != CB_ERR)  WSS.GetAdapterDetails(AdapterIndex, GetDlgItem(hDlg, IDC_LIST_ADAPTER));

    //****************Mostrar las caracteristicas de la wifi ************
    WSS.UpdateWifiList(AdapterIndex);
    UpdateWifiDetailsList(hDlg);
    
    //Actualmente, la única forma de actualizar en el listBox(IDC_LIST_ADAPTER) el estado 
    //del adaptador (conectado/desconectado) es con una llamada a Enumerar Wifis...
    //peroesto es bastante caótico, puesto que actualiza también todos los arreglos internos
    //(pendiente de programar un sistema mejor
    //WSS.EnumerarWifis(GetDlgItem(hDlg, IDC_COMBO_ADAPTER), GetDlgItem(hDlg, IDC_COMBO_WIFI));
}
/// <summary>
/// WindowProc callback function
/// </summary>
/// <param name="hDlg">Dialog Window handle</param>
/// <param name="message">WndProc message identifier</param>
/// <param name="wParam">WndProc first argument</param>
/// <param name="lParam">WndProc second argument</param>
/// <returns>WndProc return value depends of message identifier. Search "WindowProc callback function" for more info</returns>
INT_PTR CALLBACK WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    DWORD returnValue;
    int ret = 0;
    const char* translatedMessage = wmTranslation[message];
    wchar_t statusMessage[100];
    wchar_t DataString[10];
    SharedClass* local_sharedData = NULL;
    WinSockServer3::STATE status;
    wchar_t IP_string[50];
    wchar_t portString[10];
    WORD lwParam = LOWORD(wParam);
    WORD hwParam = HIWORD(wParam);

    //Connected to string************************
    sockaddr nameAddr = { 0 };
    int nameAddrLen = sizeof(sockaddr);
    char IPstring_connectedTo[25];
    wchar_t IPstring_connectedTo_w[25];
    wchar_t Port_connectedTo_w[25];

    //********************************************
    RECT rect = { 0 };
    RECT rectPicture = { 0 };
    RECT rectPicture2 = { 500,200,700,400 };
    TCHAR  ListItem[256];

    if (translatedMessage == NULL)
    {
        //translatedMessage = "UnknownMessage";
    }
    //OutputDebugStringA(translatedMessage);
    //if (message == CB_ADDSTRING)
    //    OutputDebugString(L"CB_ADDSTRING\n");
    //OutputDebugString(L"\n");
    //UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_TIMER:
        if (wParam == ID_TIMER1)
        {
            if (WSS.Who_is_my_socket == WinSockServer3::WHO_IS_MY_SOCKET::CLIENT)
            {
                if (WSS.ClientState == WinSockServer3::STATE::ATTEMPTING_CONNECT_TO_SERVER)
                {
                    IP_string[0] = 0;
                    portString[0] = 0;
                    if (SendDlgItemMessage(hDlg, IDC_RAD_IP_AUTO, BM_GETCHECK, 0, 0))
                    {
                        //IP Auto
                        GetWindowTextW(GetDlgItem(hDlg, IDC_COMBO_IP_PC), IP_string, sizeof(IP_string));
                    }
                    else
                    {
                        //IP Manual
                        GetWindowTextW(GetDlgItem(hDlg, IDC_EDIT_IP_MANUAL), IP_string, sizeof(IP_string));
                    }
                    GetWindowTextW(GetDlgItem(hDlg, IDC_EDIT_PC_PORT), portString, sizeof(portString));
                    //If the number is correct, we use it.
                    //If not, the older is used
                    unsigned int number = _wtoi(portString);
                    if ((number < 65535) && (number > 0)) {
                        portNumber = number;
                    }
                    WSS.Attemp_connect(IP_string, portNumber);
                }
            }
            WSS.testForEvents();
            //GetWindowRect(GetDlgItem(hDlg, IDC_PICTURE), &rectPicture);
            InvalidateRect(hDlg, &rectPicture3, TRUE);
            //UpdateWindow(hDlg);
            RedrawWindow(hDlg, &rectPicture3, NULL, RDW_NOCHILDREN| RDW_INVALIDATE);
            //UpdateWindow(hDlg);

            /*HWND hwndPic = GetDlgItem(hDlg, IDC_PICTURE);
            InvalidateRect(hwndPic, NULL, TRUE);
            InvalidateRgn(hwndPic, NULL, TRUE);
            UpdateWindow(hDlg);*/

            //InvalidateRect(hDlg,0, TRUE);
            switch (WSS.Who_is_my_socket)
            {
            case WinSockServer3::WHO_IS_MY_SOCKET::SERVER:
                lstrcpy(statusMessage, L" WORKING AS SERVER -");
                status = WSS.StateArray[0];
                local_sharedData = &WSS.sharedData[1];
                break;
            case WinSockServer3::WHO_IS_MY_SOCKET::CLIENT:
                lstrcpy(statusMessage, L" WORKING AS CLIENT -");
                status = WSS.ClientState;
                local_sharedData = &WSS.sharedDataClient;
                break;
            case WinSockServer3::WHO_IS_MY_SOCKET::UNDEFINED:
            default:
                lstrcpy(statusMessage, L" SOCKET NOT DEFINED ");
                status = WinSockServer3::STATE::NONE;
                local_sharedData = &WSS.sharedDataClient;
                break;
            }
            switch (status)
            {
            case WinSockServer3::STATE::CONNECTED:
                lstrcat(statusMessage, L" Connected to ");
                ret = getpeername(WSS.ClientSocket, &nameAddr, &nameAddrLen);
                WinSockServer3::socketaddress_to_string(&nameAddr, IPstring_connectedTo_w, Port_connectedTo_w);
                lstrcat(statusMessage, IPstring_connectedTo_w);
                lstrcat(statusMessage, L" - ");
                lstrcat(statusMessage, Port_connectedTo_w);
                EnableWindow(GetDlgItem(hDlg, IDC_BTN_SEND), TRUE);
                break;
            case WinSockServer3::STATE::ERROR_DETECTED:
                lstrcat(statusMessage, L" Error detected.");
                EnableWindow(GetDlgItem(hDlg, IDC_BTN_SEND), FALSE);
                break;
            case WinSockServer3::STATE::LISTENING:
                lstrcat(statusMessage, L" Listening");
                if (WSS.Who_is_my_socket == WinSockServer3::WHO_IS_MY_SOCKET::SERVER)
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_BTN_SEND), TRUE);
                    break;
                }
                if (WSS.Who_is_my_socket == WinSockServer3::WHO_IS_MY_SOCKET::CLIENT)
                {
                    EnableWindow(GetDlgItem(hDlg, IDC_BTN_SEND), FALSE);
                    break;
                }
                break;
            case WinSockServer3::STATE::NONE:
                lstrcat(statusMessage, L"");
                EnableWindow(GetDlgItem(hDlg, IDC_BTN_SEND), FALSE);
                break;
            case WinSockServer3::STATE::ATTEMPTING_CONNECT_TO_SERVER:
                lstrcat(statusMessage, L" Attempting connect to server ");
                lstrcat(statusMessage, IP_string);
                lstrcat(statusMessage, L" - ");
                wchar_t comodin[10];
                _itow_s(portNumber, comodin, 10);
                lstrcat(statusMessage, comodin);
                EnableWindow(GetDlgItem(hDlg, IDC_BTN_SEND), FALSE);
                break;
            }
            SetWindowText(GetDlgItem(hDlg, IDC_LABEL_STATUS), statusMessage);
            _itow_s(local_sharedData->data1, DataString, 10);
            SetWindowText(GetDlgItem(hDlg, IDC_RECV_DATA1), DataString);
            _itow_s(local_sharedData->data2, DataString, 10);
            SetWindowText(GetDlgItem(hDlg, IDC_RECV_DATA2), DataString);
            _itow_s(local_sharedData->data3, DataString, 10);
            SetWindowText(GetDlgItem(hDlg, IDC_RECV_DATA3), DataString);
            _itow_s(local_sharedData->data4, DataString, 10);
            SetWindowText(GetDlgItem(hDlg, IDC_RECV_DATA4), DataString);
        }
        //An application should return zero if it processes WM_TIMER.
        return 0;
    case WM_INITDIALOG:
    {
        CallBackData.hwnd = hDlg;
        CallBackData.MsgWifiConectedCode = WM_APP_WIFI_CONNECTED;

        SendMessage(GetDlgItem(hDlg, IDC_COMBO_SOCKET), (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)L"SERVIDOR");
        SendMessage(GetDlgItem(hDlg, IDC_COMBO_SOCKET), (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)L"CLIENTE");
        SendMessage(GetDlgItem(hDlg, IDC_COMBO_SOCKET), (UINT)CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
        //SendMessage(GetDlgItem(hDlg, IDC_RAD_IP_AUTO), BM_SETCHECK, BST_CHECKED, 0);
        SendMessage(GetDlgItem(hDlg, IDC_RAD_IP_MANUAL), BM_SETCHECK, BST_CHECKED, 0);
        _itow_s(WSS.portNumber_default, portString, 10);
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_PC_PORT), portString);
        SetWindowText(GetDlgItem(hDlg, IDC_EDIT_IP_MANUAL), L"192.168.1.32");

        HICON hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON1));
        SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
        //return TRUE to direct the system to set the keyboard focus to the control specified by wParam.
        return (INT_PTR)TRUE;
    }

    case WM_COMMAND:
    {
        switch (lwParam)
        {
        case IDOK:
        case IDCANCEL:
        {
            switch (hwParam)
            {
            case BN_CLICKED:
            {
                //EndDialog(hDlg, LOWORD(wParam));

                PostQuitMessage(0);
                //If an application processes WM_COMMAND, Return value should return zero.
                return 0;
            }
            default:
                return DefWindowProc(hDlg, message, wParam, lParam);
            }
        }
        case IDC_COMBO_ADAPTER:
        {
            switch (hwParam)
            {
            case CBN_SELCHANGE:
            {
                int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL,
                    (WPARAM)0, (LPARAM)0);
                (TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT,
                    (WPARAM)ItemIndex, (LPARAM)ListItem);
                //MessageBox(NULL, (LPCWSTR)ListItem, TEXT("Item Selected"), MB_OK);
                WSS.GetAdapterDetails(ItemIndex, GetDlgItem(hDlg, IDC_LIST_ADAPTER));
                //If an application processes WM_COMMAND, Return value should return zero.
                return 0;
            }
            default:
                return DefWindowProc(hDlg, message, wParam, lParam);
            }
        }
        case IDC_COMBO_WIFI:
        {
            switch (hwParam)
            {
            case CBN_SELCHANGE:
                // If the user makes a selection from the list:
                //   Send CB_GETCURSEL message to get the index of the selected list item.
                //   Send CB_GETLBTEXT message to get the item.
                //   Display the item in a messagebox.
            {
                int ItemIndex = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL,
                    (WPARAM)0, (LPARAM)0);
                (TCHAR)SendMessage((HWND)lParam, (UINT)CB_GETLBTEXT,
                    (WPARAM)ItemIndex, (LPARAM)ListItem);
                UpdateWifiDetailsList(hDlg);

                //If an application processes WM_COMMAND, Return value should return zero.
                return 0;
            }
            default:
                return DefWindowProc(hDlg, message, wParam, lParam);
            }
        }
        case IDC_BTN_UPDATE_WIFI:
        {
            OutputDebugString(L"IDC_BTN_UPDATE_WIFI\n");
            returnValue = WSS.EnumerarWifis(GetDlgItem(hDlg, IDC_COMBO_ADAPTER), GetDlgItem(hDlg, IDC_COMBO_WIFI), GetDlgItem(hDlg, IDC_LIST_ADAPTER), GetDlgItem(hDlg, IDC_LIST_WIFI2));
            if (returnValue == ERROR_SUCCESS)
            {
                UpdateInterfaceDetialsList(hDlg);
            }
            else
            {
                //Si ha habido algún error borramos los listados
                SendMessage(GetDlgItem(hDlg, IDC_LIST_ADAPTER), (UINT)LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
                SendMessage(GetDlgItem(hDlg, IDC_LIST_WIFI2), (UINT)LB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
            }
            //If an application processes WM_COMMAND, Return value should return zero.
            return 0;
        }
        case IDC_BTN_UPDATE_IP:
        {
            OutputDebugString(L"IDC_BTN_UPDATE_IP\n");
            UpdateIPComboBox(hDlg);
            //If an application processes WM_COMMAND, Return value should return zero.
            return 0;
        }
        case IDC_BTN_CONNECT_WIFI:
        {
            OutputDebugString(L"IDC_BTN_CONNECT_WIFI\n");
            int AdapterIndex = SendMessage(GetDlgItem(hDlg, IDC_COMBO_ADAPTER), (UINT)CB_GETCURSEL,
                (WPARAM)0, (LPARAM)0);
            if (AdapterIndex == CB_ERR) {
                SetWindowText(GetDlgItem(hDlg, IDC_LABEL_STATUS), L"Adaptador no seleccionado");
                //If an application processes WM_COMMAND, Return value should return zero.
                return 0;
            }
            int WifiIndex = SendMessage(GetDlgItem(hDlg, IDC_COMBO_WIFI), (UINT)CB_GETCURSEL,
                (WPARAM)0, (LPARAM)0);
            if (WifiIndex == CB_ERR) {
                SetWindowText(GetDlgItem(hDlg, IDC_LABEL_STATUS), L"Red Wifi no seleccionada");
                //If an application processes WM_COMMAND, Return value should return zero.
                return 0;
            }
            //WSS.ConnectWifi(AdapterIndex, WifiIndex);
            returnValue = WSS.ConnectWifi(AdapterIndex, WifiIndex, WSS.WlanNotificationCallback_prv, (PVOID)&CallBackData);
            if (returnValue == ERROR_SUCCESS) {

            }
            //Return value:If an application processes this message, it should return zero.
            return 0;
        }
        case IDC_BTN_CONNECT_SOCKET:
        {
            //Inicializa un SOCKET como server o como client dependiendo de la opcion seleccionada en el ComboBox IDC_COMBO_SOCKET
            OutputDebugString(L"IDC_BTN_CONNECT_SOCKET\n");
            IP_string[0] = 0;
            portString[0] = 0;
            if (SendDlgItemMessage(hDlg, IDC_RAD_IP_AUTO, BM_GETCHECK, 0, 0))
            {
                //IP Auto
                GetWindowTextW(GetDlgItem(hDlg, IDC_COMBO_IP_PC), IP_string, sizeof(IP_string));
            }
            else
            {
                //IP Manual
                GetWindowTextW(GetDlgItem(hDlg, IDC_EDIT_IP_MANUAL), IP_string, sizeof(IP_string));
            }
            GetWindowTextW(GetDlgItem(hDlg, IDC_EDIT_PC_PORT), portString, sizeof(portString));

            int ItemIndex = SendMessage(GetDlgItem(hDlg, IDC_COMBO_SOCKET), (UINT)CB_GETCURSEL,
                (WPARAM)0, (LPARAM)0);
            switch (ItemIndex)
            {
            case 1://CLIENT
                WSS.CreateClientSocket();
                break;
            case 0://SERVER
                WSS.CreateServerSocket(IP_string, portString);
                break;
            default:
                break;
            }
            //Return value:If an application processes this message, it should return zero.
            return 0;
        }

        case IDC_BTN_SEND:
        {

            SharedClass local_sharedData;
            SetDataFromWindow(GetDlgItem(hDlg, IDC_SEND_DATA1), &local_sharedData.data1);
            SetDataFromWindow(GetDlgItem(hDlg, IDC_SEND_DATA2), &local_sharedData.data2);
            SetDataFromWindow(GetDlgItem(hDlg, IDC_SEND_DATA3), &local_sharedData.data3);
            SetDataFromWindow(GetDlgItem(hDlg, IDC_SEND_DATA4), &local_sharedData.data4);
            switch (WSS.Who_is_my_socket)
            {
            case WinSockServer3::WHO_IS_MY_SOCKET::SERVER:
                WSS.SendDataServer(1, local_sharedData);
                break;
            case WinSockServer3::WHO_IS_MY_SOCKET::CLIENT:
                WSS.SendDataClient(local_sharedData);
                break;
            default:
                break;
            }
            //If an application processes WM_COMMAND, Return value should return zero.
            return 0;
        }
        default:
        {
            return DefWindowProc(hDlg, message, wParam, lParam);
        }
        }
    }
    case WM_APP_WIFI_CONNECTED:
    {
        UpdateInterfaceDetialsList(hDlg);
        UpdateIPComboBox(hDlg);
        return 0;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        //HDC hdc = BeginPaint(hDlg, &ps);
        unsigned char led = 15, Switch = 15;
        SetDataFromWindow(GetDlgItem(hDlg, IDC_SEND_DATA1), &led);
        switch (WSS.Who_is_my_socket)
        {
        case WinSockServer3::WHO_IS_MY_SOCKET::SERVER:
            local_sharedData = &WSS.sharedData[1];
            break;
        case WinSockServer3::WHO_IS_MY_SOCKET::CLIENT:
        default:
            local_sharedData = &WSS.sharedDataClient;
            break;
        }
        Switch = local_sharedData->data1 & 0xF;
        HWND hwndPicture = GetDlgItem(hDlg, IDC_PICTURE);
        HDC hdc = BeginPaint(hDlg, &ps);
        //HDC hdc = BeginPaint(hwndPicture, &ps);
        //PaintButtons(hdc, led, Switch, 200, 500);
        for (int i = 0; i < LedArrayLen; i++) {
            LedArray[i].on = (led & (1 << i)) != 0;
            LedArray[i].Paint(hdc);
        }
        for (int i = 0; i < SwitchArrayLen; i++) {
            SwitchArray[i].on = (Switch & (1 << i)) != 0;
            SwitchArray[i].Paint(hdc);
        }
        EndPaint(hwndPicture, &ps);
        //EndPaint(hDlg, &ps);
        //Return value:If an application processes this message, it should return zero.
        return 0;
    }
    case WM_SHOWWINDOW:
    {
        UpdateInterfaceDetialsList(hDlg);
        //Return value:If an application processes this message, it should return zero. 
        GetWindowRect(GetDlgItem(hDlg, IDC_COMBO_WIFI), &rect);
        SetWindowPos(GetDlgItem(hDlg, IDC_COMBO_WIFI), NULL, 0, 0, rect.right - rect.left, 200, SWP_NOMOVE | SWP_NOZORDER);
        GetWindowRect(GetDlgItem(hDlg, IDC_COMBO_SOCKET), &rect);
        SetWindowPos(GetDlgItem(hDlg, IDC_COMBO_SOCKET), NULL, 0, 0, rect.right - rect.left, 200, SWP_NOMOVE | SWP_NOZORDER);
        GetWindowRect(GetDlgItem(hDlg, IDC_COMBO_IP_PC), &rect);
        SetWindowPos(GetDlgItem(hDlg, IDC_COMBO_IP_PC), NULL, 0, 0, rect.right - rect.left, 200, SWP_NOMOVE | SWP_NOZORDER);

        return 0;
    }
    break;
    case WM_SIZE:
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        //Return value:If an application processes this message, it should return zero.
        return 0;
    default:
        return DefWindowProc(hDlg, message, wParam, lParam);
    }

    return DefWindowProc(hDlg, message, wParam, lParam);
}

