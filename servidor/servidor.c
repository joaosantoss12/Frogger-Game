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
	
	if (checkIfServerRunning() == TRUE) {
		_tprintf(TEXT("Já existe um servidor a correr!\n"));
		Sleep(2000);
		exit(-1);
	}
	else {
		_tprintf(TEXT("========================="));
		_tprintf(TEXT("\n|        SERVIDOR       |"));
		_tprintf(TEXT("\n=========================\n"));
	}
	
	return hAuxLibrary;
}

DWORD readRegistry(const TCHAR* keyName) {
	HKEY hKey;
	DWORD value;
	DWORD size = sizeof(DWORD);
	 
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGISTRY_PATH, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
		if (RegQueryValueEx(
			hKey,
			keyName,
			NULL,
			NULL,
			(LPBYTE)&value,
			&size
		) == ERROR_SUCCESS) {
			//_tprintf(TEXT("SUCESSO AO LER A CHAVE DO REGISTRY!\n"));
			RegCloseKey(hKey);
			return value;
		}
		else {
			RegCloseKey(hKey);
			_tprintf(TEXT("\nErro ao ler o valor da chave do Registry! [%d]"), GetLastError());

			if (keyName == "FaixasRodagem")
				return DEFAULT_VALUE_FAIXAS_RODAGEM;
			else if (keyName == "VelocidadeCarros")
				return DEFAULT_VALUE_VELOCIDADE_CARROS;
			else
				_tprintf(TEXT("\nNome da chave inválido!\n"));
			
		}
	}
	else {
		_tprintf(TEXT("\nErro ao abrir a chave do Registry! [%d]"),GetLastError());

		if (keyName == "FaixasRodagem")
			return DEFAULT_VALUE_FAIXAS_RODAGEM;
		else if (keyName == "VelocidadeCarros")
			return DEFAULT_VALUE_VELOCIDADE_CARROS;
		else
			_tprintf(TEXT("\nCHAVE VAZIA!\n"));
	
	}
}

void writeRegistry(const TCHAR* keyName, DWORD value) {
	HKEY hKey;

	if (RegCreateKeyEx(
		HKEY_CURRENT_USER,
		REGISTRY_PATH,
		0,
		NULL,
		REG_OPTION_VOLATILE,
		KEY_ALL_ACCESS,
		NULL,
		&hKey,
		NULL

	) == ERROR_SUCCESS) {
		if (RegSetValueEx(hKey, keyName, 0, REG_DWORD, (LPBYTE) & value, sizeof(DWORD)) == ERROR_SUCCESS) {
			//_tprintf(TEXT("CHAVE ESCRITA COM SUCESSO!\n"));
		}
		else
			_tprintf(TEXT("ERRO AO ESCREVER CHAVE!\n"));
		RegCloseKey(hKey);
	}
	else {
		_tprintf(TEXT("Erro ao criar/abrir chave do Registry! [%d]\n"), GetLastError());
	}
}

void initServer(int argc, TCHAR* argv[], SharedGame* sharedGame, PartilhaJogoServidorCliente* sharedGameServerClient) {

	if (argc == 3) {
		for (DWORD i = 0; i < MAX_PLAYERS; i++) {
			sharedGame->gameData[i]->nFaixasRodagem = _ttoi(argv[1]);
			sharedGame->gameData[i]->velocidadeInicialCarros = _ttoi(argv[2]);
		}
		

		writeRegistry(TEXT("FaixasRodagem"), sharedGame->gameData[0]->nFaixasRodagem);
		writeRegistry(TEXT("VelocidadeCarros"), sharedGame->gameData[0]->velocidadeInicialCarros);
	}
	else {
		for (DWORD i = 0; i < MAX_PLAYERS; i++) {
			sharedGame->gameData[i]->nFaixasRodagem = readRegistry(TEXT("FaixasRodagem"));
			sharedGame->gameData[i]->velocidadeInicialCarros = readRegistry(TEXT("VelocidadeCarros"));
		}
		
	}

	sharedGame->gameSettings.nLines = sharedGame->gameData[0]->nFaixasRodagem + 2;	// +1 DA PARTIDA e +1 DA CHEGADA
	sharedGame->gameSettings.nColumns = 20;

	// PREENCHER STRUCT PARTILHA JOGO ENTRE SERVER E CLIENTE
	for (DWORD i = 0; i < MAX_PLAYERS; i++){
		sharedGame->gameData[i]->idPlayer = i + 1;
		sharedGameServerClient->deveContinuar = 1;

		//sharedGameServerClient->deveContinuar = sharedGame->deveContinuar;
		sharedGameServerClient->hReadWriteMutexUpdateGame = sharedGame->hReadWriteMutexUpdateGame;
		sharedGameServerClient->gameData[i] = sharedGame->gameData[i];
	}
}

int _tmain(int argc, TCHAR* argv[]) {

#ifdef UNICODE
	int setmodeSTDIN = _setmode(_fileno(stdin), _O_WTEXT);
	int setmodeSTDOUT = _setmode(_fileno(stdout), _O_WTEXT);
	int setmodeSTDERR = _setmode(_fileno(stderr), _O_WTEXT);
#endif

	HINSTANCE hLibrary = openLib_checkServer();


	PFUNC_TypeBool_PointerSharedGame initSharedMemory_Sync;
	initSharedMemory_Sync = (PFUNC_TypeBool_PointerSharedGame) GetProcAddress(hLibrary, "initSharedMemory_Sync");

	SharedGame sharedGame;
	PartilhaJogoServidorCliente partilhaJogoClientes;

	if (!initSharedMemory_Sync(&sharedGame))
		exit(-1);

	initServer(argc, argv, &sharedGame, &partilhaJogoClientes);

	ThreadsServer threadsServer;
	threadsServer.hEventCloseAllThreads = sharedGame.hEventCloseAll;

	threadsServer.hThreads[0] = CreateThread(NULL, 0, updateMapGameToOperador, &sharedGame, 0, NULL);
	threadsServer.hThreads[1] = CreateThread(NULL, 0, inGameClock, &sharedGame, 0, NULL);
	threadsServer.hThreads[2] = CreateThread(NULL, 0, receiveCommandFromOperador, &sharedGame, 0, NULL);
	threadsServer.hThreads[3] = CreateThread(NULL, 0, executeCommandsServidor, &sharedGame, 0, NULL);

	HANDLE hFinishAllThreads = CreateThread(NULL, 0, closeAllThreads, &threadsServer, 0, NULL);

	if (threadsServer.hThreads[0] == NULL || threadsServer.hThreads[1] == NULL || threadsServer.hThreads[2] == NULL ||
		threadsServer.hThreads[3] == NULL || hFinishAllThreads == NULL) {
		_tprintf(TEXT("Não foi possivel criar as threads necessárias para o funcionamento do servidor!"));
		return 1;
	}

	// Testes
	partilhaJogoClientes.hEventUpdateGame = sharedGame.hEventUpdateGame;

	HANDLE hThreadNamedPipeTabuleiro = CreateThread(NULL, 0, criaNamedPipeParaClientesTabuleiroJogo, &partilhaJogoClientes, 0, NULL);
	HANDLE hThreadNamedPipe_ = CreateThread(NULL, 0, clienteConectaNamedPipeTabuleiro, &partilhaJogoClientes, 0, NULL);
	HANDLE hThreadNamedPipeMensagems_ = CreateThread(NULL, 0, clienteLerNamedPipeTabuleiro, &sharedGame, 0, NULL);
	// Fim testes

	//WaitForMultipleObjects(N_THREADS_SERVER, threadsServer.hThreads, TRUE, INFINITE);
	WaitForSingleObject(hFinishAllThreads, INFINITE);

	for (DWORD i = 0; i < N_THREADS_SERVER; i++)
		TerminateThread(threadsServer.hThreads[i], 0);

	TerminateThread(hThreadNamedPipeTabuleiro,0);
	TerminateThread(hThreadNamedPipe_,0);
	TerminateThread(hThreadNamedPipeMensagems_,0);
	

	for (DWORD i = 0; i < MAX_PLAYERS; i++) {
		CloseHandle(sharedGame.hMapFileGame[i]);
		UnmapViewOfFile(sharedGame.gameData[i]);
	}
	CloseHandle(sharedGame.hReadWriteMutexUpdateGame);
	CloseHandle(sharedGame.hEventUpdateGame);
	CloseHandle(sharedGame.hSemaforoSendGameUpdate);
	CloseHandle(sharedGame.hEventInGame);
	CloseHandle(sharedGame.hEventCloseAll);

	UnmapViewOfFile(sharedGame.bufferCircular_OperadorServidor);
	CloseHandle(sharedGame.hMutexBufferCircular_OperadorServidor);
	CloseHandle(sharedGame.hMapFileBufferCircular_OperadorServidor);
	CloseHandle(sharedGame.hSemaforoWriteBufferCircular_OperadorServidor);
	CloseHandle(sharedGame.hSemaforoReadBufferCircular_OperadorServidor);

	for (DWORD i = 0; i < N_THREADS_SERVER; i++)
		CloseHandle(threadsServer.hThreads[i]);

	CloseHandle(hFinishAllThreads);
	FreeLibrary(hLibrary);

	return 0;

}