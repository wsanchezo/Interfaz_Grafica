#pragma once
#include <Windows.h>
#include <Commctrl.h>
class UserInterface {
    // load the combobox with item list.  
// Send a CB_ADDSTRING message to load each item

    TCHAR Planets[9][10] =
    {
        TEXT("Mercury"), TEXT("Venus"), TEXT("Terra"), TEXT("Mars"),
         TEXT("Saturn"),TEXT("Jupiter"), TEXT("Uranus"), TEXT("Neptune"),
        TEXT("Pluto??")
    };

    TCHAR A[16];
    int  k = 0;
public: void TestRellenarCombo(HWND comboHWnd) {
    memset(&A, 0, sizeof(A));
    for (k = 0; k <= 8; k += 1)
    {
        wcscpy_s(A, sizeof(A) / sizeof(TCHAR), (TCHAR*)Planets[k]);

        // Add string to combobox.
        SendMessage(comboHWnd, (UINT)CB_ADDSTRING, (WPARAM)0, (LPARAM)A);
    }

    // Send the CB_SETCURSEL message to display an initial item 
    //  in the selection field  
    SendMessage(comboHWnd, CB_SETMINVISIBLE, (WPARAM)9, (LPARAM)0);
    SendMessage(comboHWnd, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
}
};