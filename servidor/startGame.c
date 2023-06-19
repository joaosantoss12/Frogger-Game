#include "startGame.h"

void lostGame(GameData* gameData) {
	gameData->endGame = TRUE;
}

void resetLevel(GameData* gameData) {
	gameData->inactiveSeconds = 0;
	gameData->lost = FALSE;

	srand(time(NULL));


	// ================== FROGGER ==================
	DWORD initialFroggerColumn = (DWORD)(rand() % 20);
	gameData->gameMap[gameData->nLines - 1][initialFroggerColumn] = FROGGER;

	gameData->frogger.x = gameData->nLines - 1;
	gameData->frogger.y = initialFroggerColumn;
	gameData->frogger.xInicial = gameData->frogger.x;
	gameData->frogger.yInicial = gameData->frogger.y;
}

void nextLevel(GameData* gameData) {
	gameData->inactiveSeconds = 0;
	gameData->won = FALSE;

	if (gameData->secondsInGame >= 25)
		gameData->secondsInGame -= 5;

	gameData->points += 50 * gameData->level;

	gameData->level += 1;
	if (60 - (5 * gameData->level) < 25)	// LIMITE MINIMO 25 SEGUNDOS
		gameData->secondsInGame = 25;
	else
		gameData->secondsInGame = 60 - (5 * gameData->level);

	if(gameData->carrosPerLane < 8)
		gameData->carrosPerLane += 1;


	// PREENCHER MAPA COM DWORDs
	for (DWORD j = 0; j < gameData->nColumns; j++) {
		gameData->gameMap[0][j] = CHEGADA;
	}

	for (DWORD i = 1; i < gameData->nLines - 1; i++) {
		for (DWORD j = 0; j < gameData->nColumns; j++) {
			gameData->gameMap[i][j] = ESTRADA;
		}
	}

	srand(time(NULL));

	// ================== FROGGER ==================
	DWORD initialFroggerColumn = (DWORD)(rand() % 20);
	gameData->gameMap[gameData->nLines - 1][initialFroggerColumn] = FROGGER;

	gameData->frogger.x = gameData->nLines - 1;
	gameData->frogger.y = initialFroggerColumn;
	gameData->frogger.xInicial = gameData->frogger.x;
	gameData->frogger.yInicial = gameData->frogger.y;

	// ================== CARROS ==================
	//_tprintf(TEXT("\nNúmero Máximo de Carros por linha: "));
	//_getts_s(gameData->carrosPerLane, sizeof(gameData->carrosPerLane) / sizeof(TCHAR));
	DWORD numberOfCarsAdded = 0;
	BOOL successCarInMap = FALSE;

	for (DWORD i = 1; i < gameData->nLines - 1; i++) {	// DA 2ª LINHA ATÉ À PENÚLTIMA (ESTRADA)	
		DWORD randomDirection = rand() % 2;

		for (DWORD c = 1; c <= gameData->carrosPerLane; c++) {
			successCarInMap = FALSE;

			do {
				DWORD randomColumn = (DWORD)(rand() % 20);

				if (randomColumn != 0 &&									// SEM CARRO À DIREITA NEM ESQUERDA (SEM CONTAR COM 1ª E ÚLTIMA COLUNAS)
					randomColumn != (gameData->nColumns - 1) &&
					gameData->gameMap[i][randomColumn - 1] != CARRO &&
					gameData->gameMap[i][randomColumn + 1] != CARRO) {

					gameData->gameMap[i][randomColumn] = CARRO;

					gameData->cars[numberOfCarsAdded].x = i;	// LINHA DO CARRO
					gameData->cars[numberOfCarsAdded].y = randomColumn;	// COLUNA DO CARRO
					gameData->cars[numberOfCarsAdded].direction = randomDirection;	// DIREÇÃO DO CARRO

					numberOfCarsAdded++;
					successCarInMap = TRUE;
					continue;	// APAGAR
				}
				else if (randomColumn == 0) {
					if (gameData->gameMap[i][randomColumn + 1] != CARRO) {	// POSIÇÃO AO LADO DIREITO (ESQUERDO NAO EXISTE)
						gameData->gameMap[i][randomColumn] = CARRO;

						gameData->cars[numberOfCarsAdded].x = i;	// LINHA DO CARRO
						gameData->cars[numberOfCarsAdded].y = randomColumn;	// COLUNA DO CARRO
						gameData->cars[numberOfCarsAdded].direction = randomDirection;	// DIREÇÃO DO CARRO

						numberOfCarsAdded++;
						successCarInMap = TRUE;
						continue;	// APAGAR
					}
				}
				else if (randomColumn == (gameData->nColumns - 1)) {
					if (gameData->gameMap[i][randomColumn - 1] != CARRO) {	// POSIÇÃO AO LADO ESQUERDO (DIREITA NAO EXISTE)
						gameData->gameMap[i][randomColumn] = CARRO;

						gameData->cars[numberOfCarsAdded].x = i;	// LINHA DO CARRO
						gameData->cars[numberOfCarsAdded].y = randomColumn;	// COLUNA DO CARRO
						gameData->cars[numberOfCarsAdded].direction = randomDirection;	// DIREÇÃO DO CARRO

						numberOfCarsAdded++;
						successCarInMap = TRUE;
						continue;	// APAGAR
					}
				}
			} while (!successCarInMap);

		}
		gameData->carrosInGame = numberOfCarsAdded;
	}
}

void startGame(GameData* gameData, GameSettings gameSettings) {
	gameData->nLines = gameSettings.nLines;
	gameData->nColumns = gameSettings.nColumns;

	gameData->secondsInGame = 55;

	gameData->inactiveSeconds = 0;
	gameData->points = 0;

	gameData->carrosPerLane = 1;
	gameData->carrosStop = 0;

	gameData->inGame = TRUE;
	gameData->pausedGame = FALSE;

	gameData->won = FALSE;
	gameData->lost = FALSE;

	gameData->endGame = FALSE;

	gameData->level = 1;


	// PREENCHER MAPA COM DWORDs
	for (DWORD j = 0; j < gameData->nColumns; j++) {
		gameData->gameMap[0][j] = CHEGADA;
		gameData->gameMap[gameData->nLines - 1][j] = PARTIDA;
	}
	for (DWORD i = 1; i < gameData->nLines - 1; i++) {
		for (DWORD j = 0; j < gameData->nColumns; j++) {
			gameData->gameMap[i][j] = ESTRADA;
		}
	}

	srand(time(NULL));

	// ================== FROGGER ==================
	DWORD initialFroggerColumn = (DWORD)(rand() % 20);
	gameData->gameMap[gameData->nLines - 1][initialFroggerColumn] = FROGGER;

	gameData->frogger.x = gameData->nLines - 1;
	gameData->frogger.y = initialFroggerColumn;
	gameData->frogger.xInicial = gameData->frogger.x;
	gameData->frogger.yInicial = gameData->frogger.y;


	// ================== CARROS ==================
	//_tprintf(TEXT("\nNúmero Máximo de Carros por linha: "));
	//_getts_s(gameData->carrosPerLane, sizeof(gameData->carrosPerLane) / sizeof(TCHAR));
	DWORD numberOfCarsAdded = 0;
	BOOL successCarInMap = FALSE;

	for (DWORD i = 1; i < gameData->nLines - 1; i++) {	// DA 2ª LINHA ATÉ À PENÚLTIMA (ESTRADA)	
		DWORD randomDirection = rand() % 2;

		for (DWORD c = 1; c <= gameData->carrosPerLane; c++) {
			successCarInMap = FALSE;

			do {
				DWORD randomColumn = (DWORD)(rand() % 20);

				if (randomColumn != 0 &&									// SEM CARRO À DIREITA NEM ESQUERDA (SEM CONTAR COM 1ª E ÚLTIMA COLUNAS)
					randomColumn != (gameData->nColumns - 1) &&
					gameData->gameMap[i][randomColumn - 1] != CARRO &&
					gameData->gameMap[i][randomColumn + 1] != CARRO) {

					gameData->gameMap[i][randomColumn] = CARRO;

					gameData->cars[numberOfCarsAdded].x = i;	// LINHA DO CARRO
					gameData->cars[numberOfCarsAdded].y = randomColumn;	// COLUNA DO CARRO
					gameData->cars[numberOfCarsAdded].direction = randomDirection;	// DIREÇÃO DO CARRO

					numberOfCarsAdded++;
					successCarInMap = TRUE;
					continue;	// APAGAR
				}
				else if (randomColumn == 0) {
					if (gameData->gameMap[i][randomColumn + 1] != CARRO) {	// POSIÇÃO AO LADO DIREITO (ESQUERDO NAO EXISTE)
						gameData->gameMap[i][randomColumn] = CARRO;

						gameData->cars[numberOfCarsAdded].x = i;	// LINHA DO CARRO
						gameData->cars[numberOfCarsAdded].y = randomColumn;	// COLUNA DO CARRO
						gameData->cars[numberOfCarsAdded].direction = randomDirection;	// DIREÇÃO DO CARRO

						numberOfCarsAdded++;
						successCarInMap = TRUE;
						continue;	// APAGAR
					}
				}
				else if (randomColumn == (gameData->nColumns - 1)) {
					if (gameData->gameMap[i][randomColumn - 1] != CARRO) {	// POSIÇÃO AO LADO ESQUERDO (DIREITA NAO EXISTE)
						gameData->gameMap[i][randomColumn] = CARRO;

						gameData->cars[numberOfCarsAdded].x = i;	// LINHA DO CARRO
						gameData->cars[numberOfCarsAdded].y = randomColumn;	// COLUNA DO CARRO
						gameData->cars[numberOfCarsAdded].direction = randomDirection;	// DIREÇÃO DO CARRO

						numberOfCarsAdded++;
						successCarInMap = TRUE;
						continue;	// APAGAR
					}
				}
			} while (!successCarInMap);

		}
		gameData->carrosInGame = numberOfCarsAdded;
	}
}

void resumeGame(GameData* gameData) {
	gameData->pausedGame = FALSE;
}

void pauseGame(GameData* gameData) {
	gameData->pausedGame = TRUE;
}

void moveFrogger(SharedGame* dados, TCHAR direction) {

	if (dados->gameData[0]->frogger.x == dados->gameData[0]->nLines - 1)
		dados->gameData[0]->gameMap[dados->gameData[0]->frogger.x][dados->gameData[0]->frogger.y] = PARTIDA;
	else if(dados->gameData[0]->nLines - 1 > dados->gameData[0]->frogger.x > 0)
		dados->gameData[0]->gameMap[dados->gameData[0]->frogger.x][dados->gameData[0]->frogger.y] = ESTRADA;
	else if(dados->gameData[0]->frogger.x == 0)
		dados->gameData[0]->gameMap[dados->gameData[0]->frogger.x][dados->gameData[0]->frogger.y] = CHEGADA;

	switch (direction) {
		case 'U':		// CIMA
			if (!(dados->gameData[0]->gameMap[dados->gameData[0]->frogger.x - 1][dados->gameData[0]->frogger.y] == CARRO)) {
				dados->gameData[0]->frogger.x -= 1;
				dados->gameData[0]->inactiveSeconds = 0;
			}
			break;
		case 'D':		// BAIXO
			if (!(dados->gameData[0]->frogger.x == dados->gameData[0]->nLines - 1)) {
				if (!(dados->gameData[0]->gameMap[dados->gameData[0]->frogger.x + 1][dados->gameData[0]->frogger.y] == CARRO)) {
					dados->gameData[0]->frogger.x += 1;
					dados->gameData[0]->inactiveSeconds = 0;
				}
			}
			break;
		case 'R':		// DIREITA
			if (!(dados->gameData[0]->frogger.y == dados->gameData[0]->nColumns-1)) {
				if (!(dados->gameData[0]->gameMap[dados->gameData[0]->frogger.x][dados->gameData[0]->frogger.y + 1] == CARRO)) {
					dados->gameData[0]->frogger.y += 1;
					dados->gameData[0]->inactiveSeconds = 0;
				}
			}
			break;
		case 'L':		// ESQUERDA
			if (!(dados->gameData[0]->frogger.y == 0)) {
				if (!(dados->gameData[0]->gameMap[dados->gameData[0]->frogger.x][dados->gameData[0]->frogger.y - 1] == CARRO)) {
					dados->gameData[0]->frogger.y -= 1;
					dados->gameData[0]->inactiveSeconds = 0;
				}
			}
			break;

		case 'X':
			DWORD initialX = dados->gameData[0]->frogger.xInicial;
			DWORD initialY = dados->gameData[0]->frogger.yInicial;

			DWORD currentX = dados->gameData[0]->frogger.x;
			DWORD currentY = dados->gameData[0]->frogger.y;


			if (0 < dados->gameData[0]->frogger.x < dados->gameData[0]->nLines)
				dados->gameData[0]->gameMap[currentX][currentY] = ESTRADA;

			dados->gameData[0]->frogger.x = initialX;
			dados->gameData[0]->frogger.y = initialY;
	}

	DWORD x = dados->gameData[0]->frogger.x;
	DWORD y = dados->gameData[0]->frogger.y;

	dados->gameData[0]->gameMap[x][y] = FROGGER;
}

void moveCars(SharedGame* sharedGame) {
	for (DWORD c = 0; c < (sharedGame->gameData[0]->carrosInGame); c++) {	// 64 MAXIMO DE CARROS (EXPLICADO NO UTILS)

		if (sharedGame->gameData[0]->cars[c].direction == 0) {	// ESQUERDA

			if (sharedGame->gameData[0]->cars[c].y == 0) {	// VAI PARA A MARGEM DIREITA SE ESTIVER NA MARGEM ESQUERDA
				if (sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][sharedGame->gameData[0]->nColumns - 1] != OBSTÁCULO
					&& sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][sharedGame->gameData[0]->nColumns - 1] != CARRO) {

					// IF VERIFICA SE NA MARGEM OPOSTA SE ENCONTRA UM OBSTACULO OU CARRO PARA O IMPEDIR DE PROSSEGUIR
					sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][sharedGame->gameData[0]->cars[c].y] = ESTRADA;
					sharedGame->gameData[0]->cars[c].y = sharedGame->gameData[0]->nColumns - 1;//atualizar a cordenada do carro
				}
			}
			else {
				if (sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][sharedGame->gameData[0]->cars[c].y - 1] != OBSTÁCULO
					&& sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][sharedGame->gameData[0]->cars[c].y - 1] != CARRO) {

					// IF VERIFICA SE À SUA ESQUERDA SE ENCONTRA UM OBSTACULO OU CARRO PARA O IMPEDIR DE PROSSEGUIR
					sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][sharedGame->gameData[0]->cars[c].y] = ESTRADA;
					sharedGame->gameData[0]->cars[c].y = sharedGame->gameData[0]->cars[c].y - 1;
				}
			}
		}

		if (sharedGame->gameData[0]->cars[c].direction == 1) {	// DIREITA

			if (sharedGame->gameData[0]->cars[c].y == sharedGame->gameData[0]->nColumns - 1) { // VAI PARA A MARGEM ESQUERDA SE ESTIVER NA DIREITA
				if (sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][0] != OBSTÁCULO
					&& sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][0] != CARRO) {

					// IF VERIFICA SE NA MARGEM OPOSTA SE ENCONTRA UM OBSTACULO OU CARRO PARA O IMPEDIR DE PROSSEGUIR
					sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][sharedGame->gameData[0]->cars[c].y] = ESTRADA;
					sharedGame->gameData[0]->cars[c].y = 0;		// PRIMEIRA COLUNA (MARGEM ESQUERDA)
				}
			}
			else {
				if (sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][sharedGame->gameData[0]->cars[c].y + 1] != OBSTÁCULO
					&& sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][sharedGame->gameData[0]->cars[c].y + 1] != CARRO) {

					// IF VERIFICA SE À SUA DIREITA SE ENCONTRA UM OBSTACULO OU CARRO PARA O IMPEDIR DE PROSSEGUIR
					sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][sharedGame->gameData[0]->cars[c].y] = ESTRADA;
					sharedGame->gameData[0]->cars[c].y = sharedGame->gameData[0]->cars[c].y + 1;
				}
			}
		}


		sharedGame->gameData[0]->gameMap[sharedGame->gameData[0]->cars[c].x][sharedGame->gameData[0]->cars[c].y] = CARRO;

		if (sharedGame->gameData[0]->frogger.x == sharedGame->gameData[0]->cars[c].x && sharedGame->gameData[0]->frogger.y == sharedGame->gameData[0]->cars[c].y)
			sharedGame->gameData[0]->lost = TRUE;
	}
}