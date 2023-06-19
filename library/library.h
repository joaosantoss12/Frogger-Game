#pragma once
#include "utils.h"

#ifdef LIBRARY_EXPORTS
#define DLL_IMP_API __declspec(dllexport)
#else
#define DLL_IMP_API __declspec(dllimport)
#endif

DLL_IMP_API BOOL checkIfServerRunning();
DLL_IMP_API BOOL initSharedMemory_Sync(SharedGame* sharedGame);