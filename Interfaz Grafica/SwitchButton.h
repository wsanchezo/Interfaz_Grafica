#pragma once
#include "framework.h"
#include "Electro.h"

class SwitchButton :public Electro {
public: bool on = true;
public:SwitchButton() {
	this->SetDimensions(10, 10);
}
public:void Paint(HDC hdc) {
    SelectObject(hdc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(hdc, on?color:0);
    SelectObject(hdc, GetStockObject(DC_PEN));
    SetDCPenColor(hdc, RGB(127, 127, 127));
    //Ellipse(hdc, getRectInClient()->left, getRectInClient()->top, getRectInClient()->right, getRectInClient()->bottom);
    //HPEN hWhitePen = GetStockObject(WHITE_PEN);
    SelectObject(hdc, GetStockObject(BLACK_PEN));

    MoveToEx(hdc, getRectInClient()->left, getRectInClient()->top, NULL);
    LineTo(hdc, getRectInClient()->left, getRectInClient()->top + 10);
    LineTo(hdc, getRectInClient()->left + 5, getRectInClient()->top + 10);

    MoveToEx(hdc, getRectInClient()->left + (on ? 5 : 10), getRectInClient()->top + (on ? 5 : 10), NULL);
    LineTo(hdc, getRectInClient()->left, getRectInClient()->top + 25);
    LineTo(hdc, getRectInClient()->left , getRectInClient()->bottom);

}

//Switch the button status on<->off if mousePoint is inside RectInClient
//if switch returns true, if mouse is outside returns false;
public:bool SwitchIfMouseIsInside(POINT mousePoint)
{
    if (PtInRect(this->getRectInClient(), mousePoint))
    {
        on = !on;
        return true;
    }
    return false;
}
};
