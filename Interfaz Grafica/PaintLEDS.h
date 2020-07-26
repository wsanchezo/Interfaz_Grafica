#pragma once
#include <Windows.h>

void PaintButtons(HDC hdc,int led,int Switch,int Top,int Left) {
    //PAINTSTRUCT ps;
    //HDC hdc = BeginPaint(hWnd_PaintButtons, &ps);
    // TODO: Agregar cualquier código de dibujo que use hDC aquí...

    int diame = 20;
    int separacion = 10;
    int top = Top;
    int MarginLeft = Left;
    int left = MarginLeft;

    int on1 = (led & 8) >> 3;
    int on2 = (led & 4) >> 2;
    int on3 = (led & 2) >> 1;
    int on4 = (led & 1);

    SelectObject(hdc, GetStockObject(DC_PEN));
    SetDCPenColor(hdc, RGB(0, 0, 0));

    SelectObject(hdc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(hdc, RGB(0, 100 * on1, 255 * on1));
    Ellipse(hdc, left, top, left + diame, top + diame);
    left += diame + separacion;

    SelectObject(hdc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(hdc, RGB(255 * on2, 255 * on2, 0));
    Ellipse(hdc, left, top, left + diame, top + diame);
    left += diame + separacion;

    SelectObject(hdc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(hdc, RGB(255 * on3, 0 * on3, 0));
    Ellipse(hdc, left, top, left + diame, top + diame);
    left += diame + separacion;

    SelectObject(hdc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(hdc, RGB(0, 255 * on4, 0));
    Ellipse(hdc, left, top, left + diame, top + diame);

    /****************************************************************************************/
    left = MarginLeft + (diame / 2);
    top += 50;
    MoveToEx(hdc, left, top, (LPPOINT)NULL);
    LineTo(hdc, left, top + 20);
    LineTo(hdc, left + 5, top + 20);
    MoveToEx(hdc, left, top + 40, (LPPOINT)NULL);
    LineTo(hdc, left, top + 60);

    left += diame + separacion;
    MoveToEx(hdc, left, top, (LPPOINT)NULL);
    LineTo(hdc, left, top + 20);
    LineTo(hdc, left + 5, top + 20);
    MoveToEx(hdc, left, top + 40, (LPPOINT)NULL);
    LineTo(hdc, left, top + 60);

    left += diame + separacion;
    MoveToEx(hdc, left, top, (LPPOINT)NULL);
    LineTo(hdc, left, top + 20);
    LineTo(hdc, left + 5, top + 20);
    MoveToEx(hdc, left, top + 40, (LPPOINT)NULL);
    LineTo(hdc, left, top + 60);

    left += diame + separacion;
    MoveToEx(hdc, left, top, (LPPOINT)NULL);
    LineTo(hdc, left, top + 20);
    LineTo(hdc, left + 5, top + 20);
    MoveToEx(hdc, left, top + 40, (LPPOINT)NULL);
    LineTo(hdc, left, top + 60);

    /****************************************************************************************/
    int dip1 = (Switch & 8) >> 3; dip1 = 1 - dip1;
    int dip2 = (Switch & 4) >> 2; dip2 = 1 - dip2;
    int dip3 = (Switch & 2) >> 1; dip3 = 1 - dip3;
    int dip4 = (Switch & 1); dip4 = 1 - dip4;

    left = MarginLeft + (diame / 2);
    MoveToEx(hdc, left, top + 40, (LPPOINT)NULL);
    LineTo(hdc, left + 5 + (5 * dip4), top + 17 + (3 * dip4));

    left += diame + separacion;
    MoveToEx(hdc, left, top + 40, (LPPOINT)NULL);
    LineTo(hdc, left + 5 + (5 * dip3), top + 17 + (3 * dip3));

    left += diame + separacion;
    MoveToEx(hdc, left, top + 40, (LPPOINT)NULL);
    LineTo(hdc, left + 5 + (5 * dip2), top + 17 + (3 * dip2));

    left += diame + separacion;
    MoveToEx(hdc, left, top + 40, (LPPOINT)NULL);
    LineTo(hdc, left + 5 + (5 * dip1), top + 17 + (3 * dip1));

    //EndPaint(hWnd_PaintButtons, &ps);


}
