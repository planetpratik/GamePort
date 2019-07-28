#pragma once
#include <cstdint>

namespace DirectXGame
{
	class StateManager
	{
	public:
		static StateManager* CreateInstance();
		static StateManager* GetInstance() { return sInstance; };
		~StateManager() = default;

		enum class GameState
		{
			MAIN_MENU,
			GAME_STARTED,
		};

		enum class ActivePlayers
		{
			PLAYER_ONE,
			PLAYER_TWO,
			PLAYER_ONE_AND_TWO
		};

		StateManager::GameState	getState();
		void		setState(StateManager::GameState state);
		StateManager::ActivePlayers	getActivePlayers();
		void		setActivePlayers(StateManager::ActivePlayers activePlayers);
	private:
		static StateManager* sInstance;
		StateManager() = default;
		GameState mGameState;
		ActivePlayers mActivePlayers;
		uint32_t		mCurrentState;
	};
}