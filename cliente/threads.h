#pragma once
#include "..\\library\utils.h"

DWORD WINAPI getMapFromServer(LPVOID p);
void resetFroggerPosition(threadDataDrawMap* dados);
void sendMessage(TCHAR msg);