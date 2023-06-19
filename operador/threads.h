#pragma once

#include "../library/utils.h"
#include "buildAndShowMap.h"

BOOL WINAPI receiveGameMapFromServidor(LPVOID p);
BOOL WINAPI sendCommandToServidor(LPVOID p);
BOOL WINAPI closeAllThreads(LPVOID p);

//BOOL WINAPI checkCommandKeyPressed(LPVOID p);