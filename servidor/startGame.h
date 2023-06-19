#pragma once

#include "../library/utils.h"

void startGame(GameData* gameData, GameSettings gameSettings);
void nextLevel(GameData* gameData);
void resetLevel(GameData* gameData);
void lostGame(GameData* gameData);
void pauseGame(GameData* gameData);
void resumeGame(GameData* gameData);
void moveFrogger(SharedGame* dados, TCHAR direction);