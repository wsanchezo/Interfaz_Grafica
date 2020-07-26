#pragma once
#include "framework.h"
class Electro {
	POINT pos = { 0 };
	RECT rectInClient = { 0 };
	RECT rectDimensions = { 0 };
public:	COLORREF color = 0;
public: void  SetPos(int x, int y) {
	pos.x = x;
	pos.y = y;
	rectInClient.left = x;
	rectInClient.top = y;
	rectInClient.right = x + rectDimensions.right;
	rectInClient.bottom = y + rectDimensions.bottom;
}
public: void SetDimensions(int width, int height) {
	rectDimensions.right = width;
	rectDimensions.bottom = height;
	rectInClient.right = pos.x + width;
	rectInClient.bottom = pos.y + height;
}
public:RECT* getRectInClient()
{
	return &rectInClient;
}
public:RECT* getRectDimensions()
{
	return &rectDimensions;
}
};