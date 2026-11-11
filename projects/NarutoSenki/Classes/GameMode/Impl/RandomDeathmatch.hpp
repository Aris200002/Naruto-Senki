#pragma once
#include "Core/Provider.hpp"
#include "Core/Utils/UnitEx.hpp"
#include "GameMode/IGameModeHandler.hpp"

class ModeRandomDeathmatch : public IGameModeHandler
{
public:
    const uint8_t kRebornCount = 40;
    const uint8_t kHeroAmount = 3;

private:
    uint8_t konohaRebornCount;
    uint8_t akatsukiRebornCount;
    uint8_t konohaLiveCount;
    uint8_t akatsukiLiveCount;

public:
    void init()
    {
        CCLOG("Enter Random Deathmatch mode.");
        konohaRebornCount = kRebornCount;
        akatsukiRebornCount = kRebornCount;
        konohaLiveCount = kHeroAmount;
        akatsukiLiveCount = kHeroAmount;
        gd.isHardCore = true;
    }

    void onInitHeros()
    {
        initHeros(kHeroAmount, kHeroAmount);
    }

    void onGameStart() {}
    void onGameOver() {}
    void onCharacterInit(CharacterBase *c) {}

    void onCharacterDead(CharacterBase *c)
    {
        if (c->isPlayerOrCom())
        {
            uint8_t &count = c->isKonohaGroup()
                ? konohaRebornCount
                : akatsukiRebornCount;

            uint8_t &liveCount = c->isKonohaGroup()
                ? konohaLiveCount
                : akatsukiLiveCount;

            if (count == 0)
            {
                c->enableReborn = false;

                if (liveCount == 0)
                    getGameLayer()->onGameOver(c->getGroup() != playerGroup);
            }
            else
            {
                count--;
                liveCount--;

                setRandomHero(c);

                c->unschedule(
                    schedule_selector(CharacterBase::resumeAction)
                );
            }
        }
    }

    void onCharacterReborn(CharacterBase *c)
    {
        if (c->isPlayerOrCom())
        {
            uint8_t liveCount = c->isKonohaGroup()
                ? konohaLiveCount++
                : akatsukiLiveCount++;
        }

        if (c->changeCharId > -1)
        {
            auto gameLayer = getGameLayer();
            auto newCharName = heroVector.at(c->changeCharId);

            if (c->getName() == newCharName)
            {
                c->changeCharId = -1;
                return;
            }

            // Pertahankan objek lama selama proses pergantian karakter.
            c->retain();

            CCNotificationCenter::sharedNotificationCenter()
                ->removeAllObservers(c);

            c->stopAllActions();

            auto monsters = c->getMonsterArray();

            for (auto mo : monsters)
            {
                if (auto heroMo = dynamic_cast<Hero *>(mo))
                {
                    heroMo->dealloc();
                }
                else
                {
                    CCNotificationCenter::sharedNotificationCenter()
                        ->removeAllObservers(mo);

                    mo->stopAllActions();
                    mo->unscheduleAllSelectors();
                    mo->removeFromParent();
                }
            }

            c->removeAllClones();

            std::erase(gameLayer->_CharacterArray, c);

            gameLayer->clearAllFlogsMainTarget(c);

            if (c->_shadow)
                c->_shadow->removeFromParent();

            if (c->getMaster())
                c->getMaster()->removeMon(c);

            c->removeFromParent();

            LoadLayer::perloadCharIMG(newCharName);

            auto oldCharName = c->getName();

            if (std::find(
                    heroVector.begin(),
                    heroVector.end(),
                    oldCharName
                ) == heroVector.end())
            {
                LoadLayer::unloadCharIMG(c);
            }

            auto hudLayer = gameLayer->getHudLayer();

            auto newChar = gameLayer->addHero(
                newCharName,
                c->getRole(),
                c->getGroup(),
                c->getSpawnPoint(),
                c->getCharId()
            );

            bool isPlayer = newChar->isPlayer();

            newChar->setCoin(c->getCoin());
            newChar->setCKR(c->getCKR());
            newChar->setCKR2(c->getCKR2());
            newChar->setEXP(c->getEXP());
            newChar->setLV(c->getLV());
            newChar->setKillNum(c->getKillNum());

            newChar->setWalkSpeed(224);
            newChar->setGearArray(c->getGearArray());

            newChar->changeHPbar();
            newChar->updateDataByLVOnly();

            for (auto gear : c->getGearArray())
            {
                if (gear == GearType::Gear00)
                {
                    newChar->_isCanGear00 = true;
                }
                else if (gear == GearType::Gear01)
                {
                    newChar->gearCKRValue = 25;
                }
                else if (gear == GearType::Gear02)
                {
                    newChar->isAttackGainCKR = true;
                }
                else if (gear == GearType::Gear03)
                {
                    newChar->_isCanGear03 = true;
                }
                else if (gear == GearType::Gear04)
                {
                    if (newChar->getTempAttackValue1())
                    {
                        newChar->setTempAttackValue1(
                            newChar->getTempAttackValue1() + 160
                        );
                    }

                    newChar->setNAttackValue(
                        newChar->getNAttackValue() + 160
                    );

                    newChar->hasArmorBroken = true;
                }
                else if (gear == GearType::Gear05)
                {
                    newChar->isGearCD = true;

                    newChar->_sAttackCD1 -= 5;
                    newChar->_sAttackCD2 -= 5;
                    newChar->_sAttackCD3 -= 5;
                }
                else if (gear == GearType::Gear06)
                {
                    newChar->_isCanGear06 = true;
                }
                else if (gear == GearType::Gear07)
                {
                    newChar->gearRecoverValue = 3000;

                    if (isPlayer)
                    {
                        hudLayer->item1Button->setCD(3000);
                        hudLayer->item1Button->_isColdChanged = true;
                    }
                }
                else if (gear == GearType::Gear08)
                {
                    newChar->setMaxHPValue(
                        newChar->getMaxHP() + 6000
                    );

                    newChar->hasArmor = true;
                }
            }

            newChar->_deadNum = c->_deadNum;
            newChar->_flogNum = c->_flogNum;
            newChar->isBaseDanger = c->isBaseDanger;

            if (isPlayer)
            {
                gameLayer->currentPlayer = newChar;

                hudLayer->updateSkillButtons();
                hudLayer->resetSkillButtons();

                hudLayer->status_hpbar->setOpacity(255);
                hudLayer->status_expbar->setOpacity(255);

                hudLayer->setHPLose(newChar->getHpPercent());
            }
            else if (newChar->isCom())
            {
                newChar->doAI();
            }

            c->changeCharId = -1;

            // Lepaskan referensi sementara setelah semua data selesai disalin.
            c->release();
        }
    }

private:
    void setRandomHero(CharacterBase *c)
    {
        int index = getIndexByHero(c);

        if (index < 0 || index >= (int)heroVector.size())
        {
            CCLOGERROR(
                "Invalid hero index for %s",
                c->getName().c_str()
            );

            return;
        }

        // Jangan memilih hero yang sedang digunakan karakter lain.
        const char *newChar = getRandomHeroExceptAllOrNull(heroVector);

        if (newChar == nullptr)
        {
            CCLOGERROR("No available hero for Random Deathmatch");

            c->changeCharId = -1;
            c->enableReborn = false;

            return;
        }

        c->changeCharId = index;

        // Hero lama kembali tersedia untuk kematian berikutnya.
        heroVector.at(index) = newChar;
    }

    inline int getIndexByHero(CharacterBase *c)
    {
        if (c->getCharId() == 0 ||
            c->getCharId() > heroVector.size())
        {
            return -1;
        }

        return c->getCharId() - 1;
    }
};
