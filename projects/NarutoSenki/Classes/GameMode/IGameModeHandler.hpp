#pragma once
#include "Core/Hero.hpp"
#include "GameLayer.h"
#include "LoadLayer.h"
#include "SelectLayer.h"

class IGameModeHandler;

enum GameMode : uint32_t
{
	OneVsOne,
	Classic,
	FourVsFour,
	HardCore_4Vs4,
	Boss,
	Clone,
	Deathmatch,
	RandomDeathmatch,
	__Internal_Max_Length
};

struct GameModeData
{
public:
	string title;
	string description;
	bool isLocked;
	bool useMask2;
	bool hasSelected;
};

struct GameData
{
	bool enableGear = true;
	bool isHardCore = true;
	bool isRandomChar = false;
	bool use4v4SpawnLayout = false;
	Group playerGroup;
};

class IGameModeHandler
{
	friend class GameLayer;
	friend class GameModeLayer;
	friend class SelectLayer;

private:
	vector<HeroData> heroDataVector;
	int mapId = 0;
	bool enableKonohaTowers = true;
	bool enableAkatsukiTowers = true;
	bool skipInitFlogs = false;
	bool enableKonohaFlogs = false;
	bool enableAkatsukiFlogs = false;
	float flogSpawnDuration = 15.0f;
	bool isLimitFlog = false;
	int maxFlogWaves;
	bool enableHeroReborn = true;

	void Internal_GameOver()
	{
		onGameOver();
		gd = {};
		clearHeroArray();
		resetCheats();
	}

protected:
	SelectLayer *selectLayer = nullptr;
	GameData gd;
	int oldCheats = -1;
	vector<string> heroVector;
	Group playerGroup;

public:
	const uint8_t kMaxCharCount = 4;
	const int kDefaultMap = 1;
	const Group kDefaultGroup = Group::Konoha;

	virtual void init() = 0;
	virtual void onInitHeros() = 0;
	virtual void onGameStart() = 0;
	virtual void onGameOver() = 0;
	virtual void onCharacterInit(CharacterBase *c) = 0;
	virtual void onCharacterDead(CharacterBase *c) = 0;
	virtual void onCharacterReborn(CharacterBase *c) = 0;

	inline const GameData &getGameData() { return gd; }
	inline int getOldCheats() { return oldCheats; }
	inline void setOldCheats(int val) { oldCheats = val; }
	inline void resetCheats()
	{
		if (oldCheats != -1)
		{
			Cheats = oldCheats;
			oldCheats = -1;
		}
	}
	inline const vector<HeroData> &getHerosArray() { return heroDataVector; }

protected:
	void clearHeroArray()
	{
		heroDataVector.clear();
		heroVector.clear();
	}

	void setMap(int id) { this->mapId = id; }

	void setTower(bool enableKonohaTowers, bool enableAkatsukiTowers)
	{
		this->enableKonohaTowers = enableKonohaTowers;
		this->enableAkatsukiTowers = enableAkatsukiTowers;
	}

	void setFlog(bool enableKonohaFlogs, bool enableAkatsukiFlogs, float duration = 15.0f, int maxWaves = -1)
	{
		this->enableKonohaFlogs = enableKonohaFlogs;
		this->enableAkatsukiFlogs = enableAkatsukiFlogs;
		this->flogSpawnDuration = (duration < 5 || duration > 60) ? 15.0f : duration;
		this->isLimitFlog = maxWaves >= 0;
		this->maxFlogWaves = maxWaves;
	}

	void setHero(bool enableReborn) { this->enableHeroReborn = enableReborn; }

	inline void addHero(const char *name, Role role, Group group, uint32_t lv = 1)
	{
		heroDataVector.push_back({name, role, group});
		heroVector.push_back(name);
	}

	inline void addHeros(int count, const char *name, Role role, Group group, uint32_t lv = 1)
	{
		for (int i = 0; i < count; i++)
			addHero(name, role, group, lv);
	}

	inline void addKonohaHero(const vector<string> &heros, const char *name, Role role, uint32_t lv = 1)
	{
		addHero(name, Role::Com, Group::Konoha, lv);
	}

	inline void addAkatsukiHero(const vector<string> &heros, const char *name, Role role, uint32_t lv = 1)
	{
		addHero(name, Role::Com, Group::Akatsuki, lv);
	}

	inline void initHeros(uint32_t playerCount, uint32_t enemyCount)
	{
		initHeros(playerCount, enemyCount, nullptr, kDefaultGroup);
	}

	void initHeros(uint8_t playerCount, uint8_t enemyCount,
				   const char *playerSelect, Group playerGroup,
				   const char *com1Select = nullptr, const char *com2Select = nullptr, const char *com3Select = nullptr)
	{
		clearHeroArray();
		playerCount = playerCount > kMaxCharCount ? kMaxCharCount : playerCount;
		enemyCount = enemyCount > kMaxCharCount ? kMaxCharCount : enemyCount;
		setRand();
		int team = random(2);
		playerGroup = team > 0 ? Group::Akatsuki : Group::Konoha;
		this->gd.playerGroup = playerGroup;
		this->playerGroup = playerGroup;
		uint8_t totalCount = playerCount + enemyCount;
		uint8_t comCount = totalCount > 0 ? totalCount - 1 : 0;
		uint8_t comOfPlayerGroupCount = playerCount > 0 ? playerCount - 1 : 0;

		if (playerSelect)
			selectLayer->_playerSelect = playerSelect;
		if (com1Select)
			selectLayer->_com1Select = com1Select;
		if (com2Select)
			selectLayer->_com2Select = com2Select;
		if (com3Select)
			selectLayer->_com3Select = com3Select;

		string customEnemy[4];
		for (int j = 0; j < 4; j++)
			customEnemy[j] = takeCustomEnemy(j);

		string tmpChar;
		if (selectLayer->_playerSelect)
		{
			tmpChar = selectLayer->_playerSelect;
		}
		else
		{
			auto selcetList = selectLayer->_selectList;
			int count = selcetList->count();
			setRand();
			int index = random(count);
			tmpChar = ((CCString *)selcetList->objectAtIndex(index))->m_sString;
			do
			{
				setRand();
				index = random(count);
				tmpChar = ((CCString *)selcetList->objectAtIndex(index))->m_sString;
			} while (tmpChar == "None");
			selectLayer->_playerSelect = tmpChar.c_str();
			gd.isRandomChar = true;
		}

		heroDataVector.push_back({tmpChar, Role::Player, playerGroup});
		heroVector.push_back(tmpChar);

		vector<string> realHeroVector;
		for (size_t i = 0; i < kHeroNum; i++)
		{
			if (is_same(selectLayer->_playerSelect, kHeroList[i]))
				continue;
			if (selectLayer->_com1Select && is_same(selectLayer->_com1Select, kHeroList[i]))
				continue;
			if (selectLayer->_com2Select && is_same(selectLayer->_com2Select, kHeroList[i]))
				continue;
			if (selectLayer->_com3Select && is_same(selectLayer->_com3Select, kHeroList[i]))
				continue;
			bool isCustomEnemy = false;
			for (int j = 0; j < 4; j++)
				if (!customEnemy[j].empty() && customEnemy[j] == kHeroList[i])
					isCustomEnemy = true;
			if (isCustomEnemy)
				continue;
			realHeroVector.push_back(kHeroList[i]);
		}

		string hero;
		for (int i = 0; i < comCount; i++)
		{
			if (i < comOfPlayerGroupCount)
			{
				if (i == 0 && selectLayer->_com1Select)
					hero = selectLayer->_com1Select;
				else if (i == 1 && selectLayer->_com2Select)
					hero = selectLayer->_com2Select;
				else if (i == 2 && selectLayer->_com3Select)
					hero = selectLayer->_com3Select;
				else
				{
					int count = realHeroVector.size();
					setRand();
					int index = random(count);
					if (index == count)
						index = realHeroVector.size() - 1;
					hero = realHeroVector.at(index);
					realHeroVector.erase(std::find(realHeroVector.begin(), realHeroVector.end(), hero));
				}
				heroDataVector.push_back({hero, Role::Com, team > 0 ? Group::Akatsuki : Group::Konoha});
				heroVector.push_back(hero);
			}
			else
			{
				int enemyIndex = i - comOfPlayerGroupCount;
				if (enemyIndex >= 0 && enemyIndex < 4 && !customEnemy[enemyIndex].empty())
					hero = customEnemy[enemyIndex];
				else
				{
					int count = realHeroVector.size();
					setRand();
					int index = random(count);
					if (index == count)
						index = realHeroVector.size() - 1;
					hero = realHeroVector.at(index);
					realHeroVector.erase(std::find(realHeroVector.begin(), realHeroVector.end(), hero));
				}
				heroDataVector.push_back({hero, Role::Com, team > 0 ? Group::Konoha : Group::Akatsuki});
				heroVector.push_back(hero);
			}
		}
	}

	inline void setPlayerTeamByGroup(Group group) { gd.playerGroup = group; }

	static inline const char *getRandomHero()
	{
		setRand();
		return kHeroList[random(kHeroNum)];
	}

	static inline const char *getRandomHeroExcept(const char *except)
	{
		if (except)
		{
			setRand();
			int i = random(kHeroNum);
			while (is_same(kHeroList[i], except))
				i = random(kHeroNum);
			return kHeroList[i];
		}
		return kHeroList[random(kHeroNum)];
	}

	static inline const char *getRandomHeroExceptAll(const vector<string> &excepts, const char *defaultChar = "Naruto")
	{
		auto _begin = excepts.begin();
		auto _end = excepts.end();
		int idx;
		for (size_t i = 0; i < kHeroNum * 2; i++)
		{
			setRand();
			idx = random(kHeroNum);
			if (std::find(_begin, _end, kHeroList[idx]) == _end)
				return kHeroList[idx];
		}
		return defaultChar;
	}

	// Used by Random Deathmatch: returns nullptr only if all heroes are occupied.
	static inline const char *getRandomHeroExceptAllOrNull(const vector<string> &excepts)
	{
		vector<const char *> availableHeroes;
		availableHeroes.reserve(kHeroNum);

		for (size_t i = 0; i < kHeroNum; i++)
		{
			if (std::find(excepts.begin(), excepts.end(), kHeroList[i]) == excepts.end())
				availableHeroes.push_back(kHeroList[i]);
		}

		if (availableHeroes.empty())
			return nullptr;

		setRand();
		size_t index = (size_t)random((int)availableHeroes.size());
		if (index >= availableHeroes.size())
			index = availableHeroes.size() - 1;

		return availableHeroes[index];
	}

	inline const char *getSelectedOrRandomHero()
	{
		return selectLayer->_playerSelect ? selectLayer->_playerSelect : getRandomHero();
	}

	inline const char *getSelectOrRandomHeroExcept(const char *except)
	{
		return selectLayer->_playerSelect ? selectLayer->_playerSelect : getRandomHeroExcept(except);
	}

	inline const char *getSelectOrRandomHeroExceptAll(const vector<string> &excepts, const char *defaultChar = "Naruto")
	{
		return selectLayer->_playerSelect ? selectLayer->_playerSelect : getRandomHeroExceptAll(excepts, defaultChar);
	}

	static inline Group getRandomGroup()
	{
		setRand();
		return random(2) == 0 ? Group::Konoha : Group::Akatsuki;
	}

	static inline string takeCustomEnemy(int index)
	{
		static const char *kKeys[4] = {"customEnemy1", "customEnemy2", "customEnemy3", "customEnemy4"};
		if (index < 0 || index > 3)
			return "";
		auto ud = UserDefault::sharedUserDefault();
		string value = ud->getStringForKey(kKeys[index]);
		if (!value.empty())
			ud->setStringForKey(kKeys[index], "");
		return value;
	}

	static inline void getRandomGroups(Group &groupA, Group &groupB)
	{
		setRand();
		int team = random(2) == 0;
		groupA = team == 0 ? Group::Konoha : Group::Akatsuki;
		groupB = team == 0 ? Group::Akatsuki : Group::Konoha;
	}
};
