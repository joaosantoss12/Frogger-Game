#include "threads.h"
#include "startGame.h"
#include "commandsOperador.h"

BOOL WINAPI criaNamedPipeParaClientesTabuleiroJogo(LPVOID p) {

	PartilhaJogoServidorCliente* partilhaJogo = (PartilhaJogoServidorCliente*)p;

	_tprintf(TEXT("Á espera da conexão de um jogador...\n"));

	for (int i = 0; i < MAX_PLAYERS; ++i) {

		HANDLE hPipe;
		HANDLE hEventTemp;

		hPipe = CreateNamedPipe(
			PIPE_NAME_MAP,
			PIPE_ACCESS_OUTBOUND | FILE_FLAG_OVERLAPPED,
			PIPE_WAIT | PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE
			, MAX_PLAYERS,
			sizeof(GameData),
			sizeof(GameData),
			NMPWAIT_USE_DEFAULT_WAIT,
			NULL
		);
		if (hPipe == INVALID_HANDLE_VALUE) {
			_tprintf(TEXT("Error 1 %d.\n"), GetLastError());
			exit(-1);
		}

		hEventTemp = CreateEvent(NULL, TRUE, FALSE, NULL);
		if (hEventTemp == NULL) {
			_tprintf(TEXT("Error 2 %d.\n"), GetLastError());
			exit(-1);
		}

		ZeroMemory(&partilhaJogo->hPipes[i].overlap, sizeof(&partilhaJogo->hPipes[i].overlap));
		partilhaJogo->hPipes[i].hInstancia = hPipe;
		partilhaJogo->hPipes[i].overlap.hEvent = hEventTemp;
		partilhaJogo->hPipes[i].active = FALSE;
		partilhaJogo->hEvents[i] = hEventTemp;

		if (ConnectNamedPipe(hPipe, &partilhaJogo->hPipes[i].overlap)) {
			_tprintf(TEXT("Error 3 %d.\n"), GetLastError());
			exit(-1);
		}
	}

	DWORD numClientes = 0, offset, nBytes;
	while (partilhaJogo->deveContinuar && numClientes < MAX_PLAYERS) {

		offset = WaitForMultipleObjects(MAX_PLAYERS, partilhaJogo->hEvents, FALSE, INFINITE);
		DWORD i = offset - WAIT_OBJECT_0;

		if (i >= 0 && i < MAX_PLAYERS) {
			if (GetOverlappedResult(partilhaJogo->hPipes[i].hInstancia, &partilhaJogo->hPipes[i].overlap, &nBytes, FALSE)) {
				ResetEvent(partilhaJogo->hEvents[i]);
				WaitForSingleObject(partilhaJogo->hReadWriteMutexUpdateGame, INFINITE);
				partilhaJogo->hPipes[i].active = TRUE;
				ReleaseMutex(partilhaJogo->hReadWriteMutexUpdateGame);
			}
			numClientes++;
		}
	}


	return TRUE;
}

DWORD WINAPI clienteConectaNamedPipeTabuleiro(LPVOID p) {
	PartilhaJogoServidorCliente* dados = (PartilhaJogoServidorCliente*)p;
	TCHAR buf[MAX_TAM];
	TCHAR read[MAX_TAM];
	DWORD n;
	BOOL ret;

	BOOL connected = FALSE;

	do {

		WaitForSingleObject(dados->hEventUpdateGame, INFINITE);

		//for (int i = 0; i < MAX_PLAYERS; ++i) {
			WaitForSingleObject(dados->hReadWriteMutexUpdateGame, INFINITE);

			if (dados->hPipes[0].active) {
				if (!WriteFile(dados->hPipes[0].hInstancia, dados->gameData[0], sizeof(GameData), &n, NULL)) {	
					_tprintf(TEXT("Cliente desconectou!\n"));
					dados->gameData[0]->inGame = FALSE;
				}
				else {
					if (!connected) {
						_tprintf(TEXT("Jogador conectado!\n"));
						connected = TRUE;
					}
				}
			}
			ReleaseMutex(dados->hReadWriteMutexUpdateGame);
		//}
	} while (dados->deveContinuar == 1);

	// Desligar todas as instancias de named pipes
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (!DisconnectNamedPipe(dados->hPipes[i].hInstancia)) {
			_tprintf(_T("[ERRO] Desligar o pipe!"));
			return -1;
		}
	}

	return 0;
}

DWORD WINAPI clienteLerNamedPipeTabuleiro(LPVOID p) {
	SharedGame* dados = (SharedGame*)p;

	HANDLE hPipe;
	hPipe = CreateNamedPipe(
		PIPE_NAME_MESSAGES, PIPE_ACCESS_DUPLEX, PIPE_WAIT | PIPE_TYPE_MESSAGE, PIPE_READMODE_MESSAGE,
		PIPE_UNLIMITED_INSTANCES, sizeof(TCHAR) * MAX_TAM, sizeof(TCHAR) * MAX_TAM, 0, NULL
	);

	if (hPipe == INVALID_HANDLE_VALUE) {
		_tprintf(TEXT("[ERROR] Ao criar o pipe!"));
		exit(-1);
	}

	while(TRUE) {
		TCHAR msg;
		DWORD bytesRead;

		if (ReadFile(hPipe, &msg, sizeof(TCHAR), &bytesRead, NULL)) {

			if (msg == 'S') {
				startGame(dados->gameData[0], dados->gameSettings, 1);
				SetEvent(dados->hEventInGame);
				ReleaseSemaphore(dados->hSemaforoSendGameUpdate, 1, NULL);
			}
			if (!dados->gameData[0]->inGame || dados->gameData[0]->pausedGame)
				continue;

			moveFrogger(dados,msg);
		}
	}
}



BOOL WINAPI updateMapGameToOperador(LPVOID p) {
	SharedGame* sharedGame = (SharedGame*)p;

	while (TRUE) {
		WaitForSingleObject(sharedGame->hSemaforoSendGameUpdate, INFINITE);//espera pelo semaforo//atulização do mapa de jogo esta disponivel

		SetEvent(sharedGame->hEventUpdateGame);//notificar outras thread que o mapa foi atualizado
		//Sleep(500);	//dar tempo às threads 
		ResetEvent(sharedGame->hEventUpdateGame);
	}
	return TRUE;
}

BOOL WINAPI inGameClock(LPVOID p) {
	SharedGame* sharedGame = (SharedGame*)p;
	BOOL anyChanges = FALSE;   //variavel para thread
	BOOL updatedAfterPause = FALSE;

	while (TRUE) {

		WaitForSingleObject(sharedGame->hEventInGame, INFINITE);//esperar pelo evento

		WaitForSingleObject(sharedGame->hReadWriteMutexUpdateGame, INFINITE);

		//for (DWORD i = 0; i < MAX_PLAYERS; i++) {

			if (!sharedGame->gameData[0]->inGame || sharedGame->gameData[0]->pausedGame)		// NAO PRECISAMOS SABER POIS VAI ESTAR SEM O EVENTO EventInGame
				continue;																		// MAS DEIXA PARA NÃO DAR MAIS TRABALHO CASO DÊ ASNEIRA

			if (sharedGame->gameData[0]->won) 
				nextLevel(sharedGame->gameData[0]);

			if(sharedGame->gameData[0]->lost)
				resetLevel(sharedGame->gameData[0]);


			if (sharedGame->gameData[0]->frogger.x == 0)	// check win
				sharedGame->gameData[0]->won = TRUE;


			if (sharedGame->gameData[0]->carrosStop > 0) {			// VERIFICAR SE A VARIÁVEL DE PARAR OS CARROS EM SEGUNDOS É > 0
				--sharedGame->gameData[0]->carrosStop;
			}
			else {
				if(sharedGame->gameData[0]->level < 5)
					moveCars(sharedGame);
				else if (sharedGame->gameData[0]->level >= 5) {
					moveCars(sharedGame);
					moveCars(sharedGame);
				}
			}

			

			sharedGame->gameData[0]->secondsInGame -= 1;	// TEMPO VAI DIMINUINDO
			sharedGame->gameData[0]->inactiveSeconds += 1;

			if (sharedGame->gameData[0]->secondsInGame == 0) {
				lostGame(sharedGame->gameData[0]);
				ResetEvent(sharedGame->hEventInGame);
			}

			if (sharedGame->gameData[0]->inactiveSeconds == 10) {
				
				
				DWORD initialX = sharedGame->gameData[0]->frogger.xInicial;
				DWORD initialY = sharedGame->gameData[0]->frogger.yInicial;

				DWORD currentX = sharedGame->gameData[0]->frogger.x;
				DWORD currentY = sharedGame->gameData[0]->frogger.y;


				if (0 < sharedGame->gameData[0]->frogger.x < sharedGame->gameData[0]->nLines)
					sharedGame->gameData[0]->gameMap[currentX][currentY] = ESTRADA;

				sharedGame->gameData[0]->frogger.x = initialX;
				sharedGame->gameData[0]->frogger.y = initialY;

				currentX = sharedGame->gameData[0]->frogger.x;
				currentY = sharedGame->gameData[0]->frogger.y;

				sharedGame->gameData[0]->gameMap[currentX][currentY] = FROGGER;
				
				sharedGame->gameData[0]->inactiveSeconds = 0;
			}
			
			anyChanges = TRUE;
		//}

		ReleaseMutex(sharedGame->hReadWriteMutexUpdateGame);//para que o operador o use

		if (anyChanges) {
			ReleaseSemaphore(sharedGame->hSemaforoSendGameUpdate, 1, NULL);//abrir o semaforo para atualizar
		}


		Sleep(1000);
	}
	return TRUE;
}



BOOL WINAPI receiveCommandFromOperador(LPVOID p) {
	SharedGame* sharedGame = (SharedGame*)p;

	while (TRUE) {
		WaitForSingleObject(sharedGame->hSemaforoReadBufferCircular_OperadorServidor, INFINITE);
		WaitForSingleObject(sharedGame->hMutexBufferCircular_OperadorServidor, INFINITE);

		WaitForSingleObject(sharedGame->hReadWriteMutexUpdateGame, INFINITE);//esperar que o jogo não esteja a ser atualizado //servidor atualizar e operador mostrar o jogo atualizado

		checkCommandFromOperador(sharedGame, sharedGame->bufferCircular_OperadorServidor->buffer[sharedGame->bufferCircular_OperadorServidor->rP].message);

		ReleaseMutex(sharedGame->hReadWriteMutexUpdateGame);

		(sharedGame->bufferCircular_OperadorServidor->rP) += 1;

		if (sharedGame->bufferCircular_OperadorServidor->rP == MAX_TAM_BUFFER)
			sharedGame->bufferCircular_OperadorServidor->rP = 0;

		ReleaseMutex(sharedGame->hMutexBufferCircular_OperadorServidor);
		ReleaseSemaphore(sharedGame->hSemaforoWriteBufferCircular_OperadorServidor, 1, NULL);//liberta um espal+ço no semaoforo para a escrita

	}
	return TRUE;
}


BOOL WINAPI executeCommandsServidor(LPVOID p) {
	SharedGame* sharedGame = (SharedGame*)p;
	BufferCell cell;

	TCHAR* command;

	while (TRUE) {

		_tprintf(TEXT(">> "));
		_getts_s(cell.message, MAX_TAM);

		command = cell.message;

		WaitForSingleObject(sharedGame->hReadWriteMutexUpdateGame, INFINITE);

		if (_tcscmp(command, TEXT("exit")) == 0) {
			SetEvent(sharedGame->hEventCloseAll);
		}
		//else if (_tcscmp(command, TEXT("start")) == 0) {

			//for (DWORD i = 0; i < MAX_PLAYERS; i++) {   SINGLEPLAYER OU MULTIPLAYER

				//if (!sharedGame->gameData[0]->inGame) {
					//startGame(sharedGame->gameData[0], sharedGame->gameSettings);//inicialização

					//if (i == 0) {
						//SetEvent(sharedGame->hEventInGame);
						//ReleaseSemaphore(sharedGame->hSemaforoSendGameUpdate, 1, NULL);
					//}

					//break;
				//}
			//}
		//}
		else if (_tcscmp(command, TEXT("pause")) == 0) {
			if (sharedGame->gameData[0]->inGame && !sharedGame->gameData[0]->pausedGame) {
				pauseGame(sharedGame->gameData[0]);
				ResetEvent(sharedGame->hEventInGame);
			}
		}
		else if (_tcscmp(command, TEXT("resume")) == 0) {
			if (sharedGame->gameData[0]->inGame && sharedGame->gameData[0]->pausedGame) {
				resumeGame(sharedGame->gameData[0]);
				SetEvent(sharedGame->hEventInGame);
			}
		}
		else if (_tcscmp(command, TEXT("restart")) == 0) {
			if (sharedGame->gameData[0]->inGame) {
				startGame(sharedGame->gameData[0], sharedGame->gameSettings);
			}
		}

		ReleaseMutex(sharedGame->hReadWriteMutexUpdateGame);
	}
	return TRUE;
}

BOOL WINAPI closeAllThreads(LPVOID p) {
	ThreadsServer* threadsServer = (ThreadsServer*)p;

	WaitForSingleObject(threadsServer->hEventCloseAllThreads, INFINITE);

	return TRUE;
}
