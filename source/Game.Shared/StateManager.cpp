#include "pch.h"
#include "StateManager.h"

namespace DirectXGame
{
	StateManager* StateManager::sInstance = NULL;

	StateManager* StateManager::CreateInstance()
	{
		if (sInstance != NULL)return sInstance;
		else
			sInstance = new StateManager();
		return sInstance;
	}

	StateManager::GameState StateManager::getState()
	{
		return mGameState;
	}

	void StateManager::setState(StateManager::GameState state)
	{
		mGameState = state;
	}

	StateManager::ActivePlayers StateManager::getActivePlayers()
	{
		return mActivePlayers;
	}

	void StateManager::setActivePlayers(StateManager::ActivePlayers activePlayers)
	{
		mActivePlayers = activePlayers;
	}
}