#include "..\\library\utils.h"
#include "threads.h"

HINSTANCE openLib_checkServer() {

	HINSTANCE hAuxLibrary;
	hAuxLibrary = LoadLibrary(FULL_PATH_DLL);

	if (hAuxLibrary == NULL) {
		_tprintf(TEXT("Erro ao abrir a DLL!\n"));
		exit(-1);
	}

	PFUNC_TypeBool_NoArguments checkIfServerRunning;
	checkIfServerRunning = (PFUNC_TypeBool_NoArguments)GetProcAddress(hAuxLibrary, "checkIfServerRunning");

	if (checkIfServerRunning() != TRUE) {
		_tprintf(TEXT("Não existe um servidor a correr!\n"));
		Sleep(2000);
		exit(-1);
	}
	else {
		_tprintf(TEXT("========================="));
		_tprintf(TEXT("\n|        OPERADOR       |"));
		_tprintf(TEXT("\n=========================\n"));
	}

	return hAuxLibrary;
}

int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	int setmodeSTDIN = _setmode(_fileno(stdin), _O_WTEXT);
	int setmodeSTDOUT = _setmode(_fileno(stdout), _O_WTEXT);
	int setmodeSTDERR = _setmode(_fileno(stderr), _O_WTEXT);
#endif

	HINSTANCE hLibrary = openLib_checkServer();

	PFUNC_TypeBool_PointerSharedGame initSharedMemory_Sync;
	initSharedMemory_Sync = (PFUNC_TypeBool_PointerSharedGame)GetProcAddress(hLibrary, "initSharedMemory_Sync");

	SharedGame sharedGame;

	if (!initSharedMemory_Sync(&sharedGame))
		exit(-1);

	ThreadsOperador threadsOperador;
	threadsOperador.hEventCloseAllThread = sharedGame.hEventCloseAll;

	threadsOperador.hThreads[0] = CreateThread(NULL, 0, receiveGameMapFromServidor, &sharedGame, 0, NULL);
	threadsOperador.hThreads[1] = CreateThread(NULL, 0, sendCommandToServidor, &sharedGame, 0, NULL);
	//threadsOperador.hThreads[2] = CreateThread(NULL, 0, checkCommandKeyPressed, &sharedGame, 0, NULL);

	HANDLE hFinishAllThreads = CreateThread(NULL, 0, closeAllThreads, &threadsOperador, 0, NULL);

	if (threadsOperador.hThreads[0] == NULL || threadsOperador.hThreads[1] == NULL || hFinishAllThreads == NULL) {
		_tprintf(TEXT("Não foi possível criar as threads necessárias para o funcionamento do operador!"));
		exit(-1);
	}

	WaitForSingleObject(N_THREADS_OPERADOR, threadsOperador.hThreads, TRUE, INFINITE);
	WaitForSingleObject(hFinishAllThreads, INFINITE);

	//for (DWORD i = 0; i < MAX_PLAYERS; i++) {
		//CloseHandle(sharedGame.hMapFileGame[i]);
		//UnmapViewOfFile(sharedGame.gameData[i]);
	//}
	//CloseHandle(sharedGame.hReadWriteMutexUpdateGame);
	//CloseHandle(sharedGame.hEventUpdateGame);
	//CloseHandle(sharedGame.hSemaforoSendGameUpdate);
	//CloseHandle(sharedGame.hEventInGame);
	//CloseHandle(sharedGame.hEventCloseAll);

	//UnmapViewOfFile(sharedGame.bufferCircular_OperadorServidor);
	//CloseHandle(sharedGame.hMutexBufferCircular_OperadorServidor);
	//CloseHandle(sharedGame.hMapFileBufferCircular_OperadorServidor);
	//CloseHandle(sharedGame.hSemaforoWriteBufferCircular_OperadorServidor);
	//CloseHandle(sharedGame.hSemaforoReadBufferCircular_OperadorServidor);

	for (DWORD i = 0; i < N_THREADS_OPERADOR; i++)
		CloseHandle(threadsOperador.hThreads[i]);

	CloseHandle(hFinishAllThreads);
	FreeLibrary(hLibrary);

	return 0;
}