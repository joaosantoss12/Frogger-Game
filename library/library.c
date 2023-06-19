#include "library.h"

BOOL checkIfServerRunning() {
	HANDLE returnCreateMutex = CreateMutex(
		NULL,
		FALSE,
		"checkFroggerServerMutex"
	);

	if (returnCreateMutex == NULL)
		return TRUE;

	else if (GetLastError() == ERROR_ALREADY_EXISTS) 
		return TRUE;
		
	
	return FALSE;
}

BOOL initSharedMemory_Sync(SharedGame* sharedGame) {

	for (DWORD i = 0; i < MAX_PLAYERS; i++) {
		TCHAR sharedMemoryName[MAX_TAM];
		swprintf(sharedMemoryName, MAX_TAM, TEXT("%s-%d"), SHARED_MEMORY_GAME, i + 1);

		sharedGame->hMapFileGame[i] = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			sizeof(GameData),
			sharedMemoryName
		);
		if (sharedGame->hMapFileGame[i] == NULL) {
			_tprintf(TEXT("ERROR: CreateFileMapping [%d]"), GetLastError());
			return FALSE;
		}

		sharedGame->gameData[i] = MapViewOfFile(
			sharedGame->hMapFileGame[i],
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			sizeof(GameData)
		);
		if (sharedGame->gameData[i] == NULL) {
			_tprintf(TEXT("ERROR: MapViewOfFile [%d]"), GetLastError());
			CloseHandle(sharedGame->hMapFileGame[i]);
			return FALSE;
		}

		sharedGame->hReadWriteMutexUpdateGame = CreateMutex(
			NULL,
			FALSE,
			"updateFroggerMapMutex"
		);
		if (sharedGame->hReadWriteMutexUpdateGame == NULL) {
			_tprintf(TEXT("ERROR: CreateMutex [%d]\n"), GetLastError());

			for (DWORD i = 0; i < MAX_PLAYERS; i++) {
				UnmapViewOfFile(sharedGame->gameData[i]);
				CloseHandle(sharedGame->hMapFileGame[i]);
			}
			
			return FALSE;
		}

		sharedGame->hEventUpdateGame = CreateEvent(
			NULL,
			TRUE,
			FALSE,
			"updateFroggerMapEvent"
		);
		if (sharedGame->hEventUpdateGame == NULL) {
			_tprintf(TEXT("ERROR: CreateEvent [%d]\n"), GetLastError());

			for (DWORD i = 0; i < MAX_PLAYERS; i++) {
				UnmapViewOfFile(sharedGame->gameData[i]);
				CloseHandle(sharedGame->hMapFileGame[i]);
			}
			CloseHandle(sharedGame->hReadWriteMutexUpdateGame);
			return FALSE;
		}

		sharedGame->hEventInGame = CreateEvent(
			NULL,
			TRUE,
			FALSE,
			"anyInGameMatchesEvent"
		);
		if (sharedGame->hEventInGame == NULL) {
			_tprintf(TEXT("ERROR: CreateEvent [%d]\n"), GetLastError());

			for (DWORD i = 0; i < MAX_PLAYERS; i++) {
				UnmapViewOfFile(sharedGame->gameData[i]);
				CloseHandle(sharedGame->hMapFileGame[i]);
			}
			CloseHandle(sharedGame->hReadWriteMutexUpdateGame);
			CloseHandle(sharedGame->hEventUpdateGame);
			return FALSE;
		}

		sharedGame->hSemaforoSendGameUpdate = CreateSemaphore(
			NULL,
			0,
			1,
			"shareGameMapSemaphore"
		);
		if (sharedGame->hSemaforoSendGameUpdate == NULL) {
			_tprintf(TEXT("ERROR: CreateSemaphore [%d]"), GetLastError());

			for (DWORD i = 0; i < MAX_PLAYERS; i++) {
				UnmapViewOfFile(sharedGame->gameData[i]);
				CloseHandle(sharedGame->hMapFileGame[i]);
			}
			CloseHandle(sharedGame->hReadWriteMutexUpdateGame);
			CloseHandle(sharedGame->hEventUpdateGame);
			CloseHandle(sharedGame->hEventInGame);

			return FALSE;
		}


		// BUFFER CIRCULAR OPERADOR PARA SERVER
		sharedGame->hMapFileBufferCircular_OperadorServidor = CreateFileMapping(
			INVALID_HANDLE_VALUE,
			NULL,
			PAGE_READWRITE,
			0,
			sizeof(GameData),
			"sharedMemoryCircularOperadorToServer"
		);

		if (sharedGame->hMapFileBufferCircular_OperadorServidor == NULL) {
			_tprintf(TEXT("ERROR: CreateFileMapping [%d]\n"), GetLastError());
			for (DWORD i = 0; i < MAX_PLAYERS; i++) {
				UnmapViewOfFile(sharedGame->gameData[i]);
				CloseHandle(sharedGame->hMapFileGame[i]);
			}

			CloseHandle(sharedGame->hSemaforoSendGameUpdate);
			CloseHandle(sharedGame->hReadWriteMutexUpdateGame);
			CloseHandle(sharedGame->hEventUpdateGame);
			CloseHandle(sharedGame->hEventInGame);

			return FALSE;
		}

		sharedGame->bufferCircular_OperadorServidor = (BufferCircular*)MapViewOfFile(
			sharedGame->hMapFileBufferCircular_OperadorServidor,
			FILE_MAP_ALL_ACCESS,
			0,
			0,
			sizeof(BufferCircular)
		);

		if (sharedGame->bufferCircular_OperadorServidor == NULL) {
			_tprintf(TEXT("ERROR: MapViewOfFile [%d]\n"), GetLastError());
			for (DWORD i = 0; i < MAX_PLAYERS; i++) {
				UnmapViewOfFile(sharedGame->gameData[i]);
				CloseHandle(sharedGame->hMapFileGame[i]);
			}
			CloseHandle(sharedGame->hSemaforoSendGameUpdate);
			CloseHandle(sharedGame->hReadWriteMutexUpdateGame);
			CloseHandle(sharedGame->hEventUpdateGame);
			CloseHandle(sharedGame->hEventInGame);
			CloseHandle(sharedGame->hMapFileBufferCircular_OperadorServidor);

			return FALSE;
		}

		sharedGame->hMutexBufferCircular_OperadorServidor = CreateMutex(
			NULL,
			FALSE,
			"mutexCircularOperadorToServer"
		);

		if (sharedGame->hMutexBufferCircular_OperadorServidor == NULL) {
			_tprintf(TEXT("ERROR: CreateMutex [%d]\n"), GetLastError());

			for (DWORD i = 0; i < MAX_PLAYERS; i++) {
				UnmapViewOfFile(sharedGame->gameData[i]);
				CloseHandle(sharedGame->hMapFileGame[i]);
			}
			CloseHandle(sharedGame->hSemaforoSendGameUpdate);
			CloseHandle(sharedGame->hReadWriteMutexUpdateGame);
			CloseHandle(sharedGame->hEventUpdateGame);
			CloseHandle(sharedGame->hEventInGame);
			UnmapViewOfFile(sharedGame->bufferCircular_OperadorServidor);
			CloseHandle(sharedGame->hMapFileBufferCircular_OperadorServidor);

			return FALSE;
		}

		sharedGame->hSemaforoWriteBufferCircular_OperadorServidor = CreateSemaphore(
			NULL,
			CIRCULAR_BUFFER_SIZE,
			CIRCULAR_BUFFER_SIZE,
			"semaforoWriteCircularOperadorToServidor"
		);
		if (sharedGame->hSemaforoWriteBufferCircular_OperadorServidor == NULL) {
			_tprintf(TEXT("ERROR: CreateSemaphore [%d]\n"), GetLastError());
			for (DWORD i = 0; i < MAX_PLAYERS; i++) {
				UnmapViewOfFile(sharedGame->gameData[i]);
				CloseHandle(sharedGame->hMapFileGame[i]);
			}
			CloseHandle(sharedGame->hSemaforoSendGameUpdate);
			CloseHandle(sharedGame->hMutexBufferCircular_OperadorServidor);
			CloseHandle(sharedGame->hReadWriteMutexUpdateGame);
			CloseHandle(sharedGame->hEventUpdateGame);
			CloseHandle(sharedGame->hEventInGame);
			UnmapViewOfFile(sharedGame->bufferCircular_OperadorServidor);
			CloseHandle(sharedGame->hMapFileBufferCircular_OperadorServidor);

			return FALSE;
		}

		sharedGame->hSemaforoReadBufferCircular_OperadorServidor = CreateSemaphore(
			NULL,
			0,
			CIRCULAR_BUFFER_SIZE,
			"semaforoReadCirculaOperadorToServidor"
		);
		if (sharedGame->hSemaforoReadBufferCircular_OperadorServidor == NULL) {
			_tprintf(TEXT("ERROR: CreateSemaphore [%d]\n"), GetLastError());
			for (DWORD i = 0; i < MAX_PLAYERS; i++) {
				CloseHandle(sharedGame->hMapFileGame[i]);
				UnmapViewOfFile(sharedGame->gameData[i]);
			}
			CloseHandle(sharedGame->hSemaforoSendGameUpdate);
			CloseHandle(sharedGame->hMutexBufferCircular_OperadorServidor);
			CloseHandle(sharedGame->hReadWriteMutexUpdateGame);
			CloseHandle(sharedGame->hEventUpdateGame);
			CloseHandle(sharedGame->hEventInGame);
			UnmapViewOfFile(sharedGame->bufferCircular_OperadorServidor);
			CloseHandle(sharedGame->hMapFileBufferCircular_OperadorServidor);
			CloseHandle(sharedGame->hSemaforoWriteBufferCircular_OperadorServidor);

			return FALSE;
		}

		sharedGame->hEventCloseAll = CreateEvent(
			NULL,
			TRUE,
			FALSE,
			"closeAllEvent"
		);

		if (sharedGame->hEventCloseAll == NULL) {
			_tprintf(TEXT("ERROR: CreateEvent [%d]\n"), GetLastError());
			for (DWORD i = 0; i < MAX_PLAYERS; i++) {
				CloseHandle(sharedGame->hMapFileGame[i]);
				UnmapViewOfFile(sharedGame->gameData[i]);
			}
			CloseHandle(sharedGame->hSemaforoSendGameUpdate);
			CloseHandle(sharedGame->hMutexBufferCircular_OperadorServidor);
			CloseHandle(sharedGame->hReadWriteMutexUpdateGame);
			CloseHandle(sharedGame->hEventUpdateGame);
			CloseHandle(sharedGame->hEventInGame);
			UnmapViewOfFile(sharedGame->bufferCircular_OperadorServidor);
			CloseHandle(sharedGame->hMapFileBufferCircular_OperadorServidor);
			CloseHandle(sharedGame->hSemaforoWriteBufferCircular_OperadorServidor);
			CloseHandle(sharedGame->hSemaforoReadBufferCircular_OperadorServidor);

			return FALSE;
		}
	}


}