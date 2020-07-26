#pragma once
class SharedClass {
public:
    unsigned char Mark[8] = { 0x8e,0xb3,0x98,0xd2,0x58,0x8b,0x47,0x2d };
    //unsigned long Mark = 0x8eb398d2588b472d;
    unsigned long checksum = 0;
    unsigned char data1 = 0;
    unsigned char data2 = 0;
    unsigned long data3 = 0;
    unsigned long data4 = 0;
};
 static void SetDataFromWindow(HWND hwnd, unsigned long* Data) {

    const int textSize = 50;
    char text[textSize];
    unsigned long numberConverted_dword;
    if (GetWindowTextA(hwnd, text, textSize) != 0)
    {
        numberConverted_dword = _atol_l(text, 0);

        _ltoa_s(numberConverted_dword, text, 10);
        SetWindowTextA(hwnd, text);

        *Data = numberConverted_dword;
    }
}
 static void SetDataFromWindow(HWND hwnd,unsigned char* Data) {

    const int textSize = 50;
    char text[textSize];
    unsigned long numberConverted_dword;
    unsigned char numberConverted_byte;
    if (GetWindowTextA(hwnd, text, textSize) != 0)
    {
        numberConverted_dword = _atoi_l(text, 0);
        if (numberConverted_dword > 255)numberConverted_dword = 255;

        _itoa_s(numberConverted_dword, text, 10);
        SetWindowTextA(hwnd, text);

        numberConverted_byte = (char)numberConverted_dword;
        *Data = numberConverted_byte;
    }
}