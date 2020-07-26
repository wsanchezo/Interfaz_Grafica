#pragma once
#include "Electro.h"

class Led :public Electro {
public: bool on = true;
public:Led() {
    this->SetDimensions(10, 10);
}
public:void Paint(HDC hdc) {
    SelectObject(hdc, GetStockObject(DC_BRUSH));
    SetDCBrushColor(hdc, on ? color : 0);
    SelectObject(hdc, GetStockObject(DC_PEN));
    SetDCPenColor(hdc, RGB(127, 127, 127));
    Ellipse(hdc, getRectInClient()->left, getRectInClient()->top, getRectInClient()->right, getRectInClient()->bottom);

}

      //Switch the led status on<->off if mousePoint is inside RectInClient
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
