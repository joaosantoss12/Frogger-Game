#include "threads.h"

BOOL WINAPI receiveGameMapFromServidor(LPVOID p) {
	SharedGame* sharedGame = (SharedGame*)p;

	while (TRUE) {
		WaitForSingleObject(sharedGame->hEventUpdateGame, INFINITE);
		WaitForSingleObject(sharedGame->hReadWriteMutexUpdateGame, INFINITE);

		system("cls"); 

		for (DWORD i = 0; i < MAX_PLAYERS; i++)
			if (sharedGame->gameData[i]->inGame)
				buildAndShowMap(sharedGame->gameData[i]);

		ReleaseMutex(sharedGame->hReadWriteMutexUpdateGame);

		Sleep(500);
	}
	return TRUE;
}

/*BOOL WINAPI checkCommandKeyPressed(LPVOID p) {
	SharedGame* sharedGame = (SharedGame*)p;

	while (TRUE) {
		if (GetAsyncKeyState('C')) {

			ResetEvent(sharedGame->hEventInGame);	// COLOCAR O JOGO EM "PAUSA"
			ReleaseSemaphore(sharedGame->hSemaforoWriteBufferCircular_OperadorServidor, 1, NULL);
			ReleaseMutex(sharedGame->hMutexBufferCircular_OperadorServidor);

			ReleaseMutex(sharedGame->hReadWriteMutexUpdateGame);

			_tprintf(TEXT("\n\n"));
		}

		Sleep(500);
	}
}*/

BOOL WINAPI sendCommandToServidor(LPVOID p) {
	SharedGame* sharedGame = (SharedGame*)p;
	BufferCell cell;

	while (TRUE) {
		_tprintf(TEXT(">> "));
		_getts_s(cell.message, MAX_TAM);

		WaitForSingleObject(sharedGame->hSemaforoWriteBufferCircular_OperadorServidor, INFINITE);
		WaitForSingleObject(sharedGame->hMutexBufferCircular_OperadorServidor, INFINITE);

		CopyMemory(&(sharedGame->bufferCircular_OperadorServidor->buffer[sharedGame->bufferCircular_OperadorServidor->wP]), &cell, sizeof(BufferCell));

		(sharedGame->bufferCircular_OperadorServidor->wP) += 1;

		if (sharedGame->bufferCircular_OperadorServidor->wP == MAX_TAM_BUFFER)
			sharedGame->bufferCircular_OperadorServidor->wP = 0;

		SetEvent(sharedGame->hEventInGame);		// VOLTAR AO JOGO APÓS ENVIAR O COMANDO

		ReleaseMutex(sharedGame->hMutexBufferCircular_OperadorServidor);
		ReleaseSemaphore(sharedGame->hSemaforoReadBufferCircular_OperadorServidor, 1, NULL);
	}
	return TRUE;
}

BOOL WINAPI closeAllThreads(LPVOID p) {
	ThreadsOperador* threadsOperador = (ThreadsOperador*)p;

	WaitForSingleObject(threadsOperador->hEventCloseAllThread, INFINITE);

	for (DWORD i = 0; i < N_THREADS_OPERADOR; i++)
		TerminateThread(threadsOperador->hThreads[i], 0);

	_tprintf(TEXT("\n\nO Servidor terminou a execução!\n"));
	Sleep(2000);

	return TRUE;
}