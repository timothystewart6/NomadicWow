/*
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2011-2015 ArkCORE <http://www.arkania.net/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
SDName: Instance_Shadowfang_Keep
SD%Complete: 90
SDComment:
SDCategory: Shadowfang Keep
EndScriptData */

#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "shadowfang_keep.h"
#include "TemporarySummon.h"

enum Apothecary
{
    ACTION_SPAWN_CRAZED         = 3
};

enum Yells
{
    SAY_BOSS_DIE_AD         = 4,
    SAY_BOSS_DIE_AS         = 3,
    SAY_ARCHMAGE            = 0
};

enum Creatures
{
    NPC_ASH                 = 3850,
    NPC_ADA                 = 3849,
    NPC_ARCHMAGE_ARUGAL     = 4275,
    NPC_ARUGAL_VOIDWALKER   = 4627
};

enum GameObjects
{
    GO_COURTYARD_DOOR       = 18895, //door to open when talking to NPC's
    GO_SORCERER_DOOR        = 18972, //door to open when Fenrus the Devourer
    GO_ARUGAL_DOOR          = 18971  //door to open when Wolf Master Nandos
};

enum Spells
{
    SPELL_ASHCROMBE_TELEPORT    = 15742
};

const Position SpawnLocation[] =
{
    {-148.199f, 2165.647f, 128.448f, 1.026f},
    {-153.110f, 2168.620f, 128.448f, 1.026f},
    {-145.905f, 2180.520f, 128.448f, 4.183f},
    {-140.794f, 2178.037f, 128.448f, 4.090f},
    {-138.640f, 2170.159f, 136.577f, 2.737f}
};
class instance_shadowfang_keep : public InstanceMapScript
{
public:
    instance_shadowfang_keep() : InstanceMapScript("instance_shadowfang_keep", 33) { }

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_shadowfang_keep_InstanceMapScript(map);
    }

    struct instance_shadowfang_keep_InstanceMapScript : public InstanceScript
    {
        instance_shadowfang_keep_InstanceMapScript(Map* map) : InstanceScript(map) { }

        uint32 m_auiEncounter[MAX_ENCOUNTER];
        std::string str_data;

        uint64 uiAshGUID;
        uint64 uiAdaGUID;
        uint64 uiArchmageArugalGUID;

        uint64 DoorCourtyardGUID;
        uint64 DoorSorcererGUID;
        uint64 DoorArugalGUID;

        uint64 fryeGUID;
        uint64 hummelGUID;
        uint64 baxterGUID;
        uint32 spawnCrazedTimer;

        uint64 LordWaldenGUID;
        uint64 uiBaronAshbury;
        uint64 uiBaronSilverlaine;
        uint64 uiCommanderSpringvale;
        uint64 uiLordGodfrey;

        uint8 uiPhase;
        uint16 uiTimer;

        void Initialize() override
        {
            memset(&m_auiEncounter, 0, sizeof(m_auiEncounter));

            uiAshGUID = 0;
            uiAdaGUID = 0;
            uiArchmageArugalGUID = 0;

            DoorCourtyardGUID = 0;
            DoorSorcererGUID = 0;
            DoorArugalGUID = 0;

            fryeGUID = 0;
            hummelGUID = 0;
            baxterGUID = 0;

            LordWaldenGUID = 0;
            uiBaronAshbury = 0;
            uiBaronSilverlaine = 0;
            uiCommanderSpringvale = 0;
            uiLordGodfrey = 0;

            uiPhase = 0;
            uiTimer = 0;
        }

        void OnCreatureCreate(Creature* creature) override
        {
            switch (creature->GetEntry())
            {
                case NPC_ASH: uiAshGUID = creature->GetGUID(); break;
                case NPC_ADA: uiAdaGUID = creature->GetGUID(); break;
                case NPC_ARCHMAGE_ARUGAL: uiArchmageArugalGUID = creature->GetGUID(); break;
                case NPC_FRYE: fryeGUID = creature->GetGUID(); break;
                case NPC_HUMMEL: hummelGUID = creature->GetGUID(); break;
                case NPC_BAXTER: baxterGUID = creature->GetGUID(); break;
                case BOSS_BARON_ASHBURY: uiBaronAshbury = creature->GetGUID();        break;
                case BOSS_BARON_SILVERLAINE: uiBaronSilverlaine = creature->GetGUID();    break;
                case BOSS_COMMANDER_SPRINGVALE: uiCommanderSpringvale = creature->GetGUID(); break;
                case BOSS_LORD_GODFREY: uiLordGodfrey = creature->GetGUID();         break;
                case BOSS_LORD_WALDEN: LordWaldenGUID = creature->GetGUID(); break;
            }
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            switch (go->GetEntry())
            {
                case GO_COURTYARD_DOOR:
                    DoorCourtyardGUID = go->GetGUID();
                    if (m_auiEncounter[0] == DONE)
                        HandleGameObject(0, true, go);
                    break;
                case GO_SORCERER_DOOR:
                    DoorSorcererGUID = go->GetGUID();
                    if (m_auiEncounter[2] == DONE)
                        HandleGameObject(0, true, go);
                    break;
                case GO_ARUGAL_DOOR:
                    DoorArugalGUID = go->GetGUID();
                    if (m_auiEncounter[3] == DONE)
                        HandleGameObject(0, true, go);
                    break;
            }
        }

        void DoSpeech()
        {
            Creature* pAda = instance->GetCreature(uiAdaGUID);
            Creature* pAsh = instance->GetCreature(uiAshGUID);

            if (pAda && pAda->IsAlive() && pAsh && pAsh->IsAlive())
            {
                pAda->AI()->Talk(SAY_BOSS_DIE_AD);
                pAsh->AI()->Talk(SAY_BOSS_DIE_AS);
            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case TYPE_FREE_NPC:
                    if (data == DONE)
                        DoUseDoorOrButton(DoorCourtyardGUID);
                    m_auiEncounter[0] = data;
                    break;
                case TYPE_RETHILGORE:
                    if (data == DONE)
                        DoSpeech();
                    m_auiEncounter[1] = data;
                    break;
                case DATA_LORD_WALDEN_EVENT:
                    if (data == DONE)
                        DoUseDoorOrButton(DoorSorcererGUID);
                    m_auiEncounter[2] = data;
                    break;
                case TYPE_NANDOS:
                    if (data == DONE)
                        DoUseDoorOrButton(DoorArugalGUID);
                    m_auiEncounter[3] = data;
                    break;
                case TYPE_CROWN:
                    if (data == NOT_STARTED)
                        spawnCrazedTimer = urand(7000, 14000);
                    m_auiEncounter[4] = data;
                    break;
                case DATA_BARON_ASHBURY_EVENT:
                    m_auiEncounter[5] = data;
                    break;
                case DATA_BARON_SILVERLAINE_EVENT:
                    m_auiEncounter[6] = data;
                    break;
                case DATA_COMMANDER_SPRINGVALE_EVENT:
                    m_auiEncounter[7] = data;
                    break;
                case DATA_LORD_GODFREY_EVENT:
                    m_auiEncounter[8] = data;
                    break;
                default:
                    break;
            }

            if (data == DONE)
            {
                OUT_SAVE_INST_DATA;

                std::ostringstream saveStream;
                saveStream << m_auiEncounter[0] << ' ' << m_auiEncounter[1] << ' ' << m_auiEncounter[2] << ' ' << m_auiEncounter[3] << ' ' << m_auiEncounter[4] << ' ' << m_auiEncounter[5] << ' ' << m_auiEncounter[6] << ' ' << m_auiEncounter[7] << ' ' << m_auiEncounter[8];

                str_data = saveStream.str();

                SaveToDB();
                OUT_SAVE_INST_DATA_COMPLETE;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            switch (type)
            {
                case TYPE_FREE_NPC:
                    return m_auiEncounter[0];
                case TYPE_RETHILGORE:
                    return m_auiEncounter[1];
                case DATA_LORD_WALDEN_EVENT:
                    return m_auiEncounter[2];
                case TYPE_NANDOS:
                    return m_auiEncounter[3];
                case TYPE_CROWN:
                    return m_auiEncounter[4];
                case DATA_BARON_ASHBURY_EVENT:
                    return m_auiEncounter[5];
                case DATA_BARON_SILVERLAINE_EVENT:
                    return m_auiEncounter[6];
                case DATA_COMMANDER_SPRINGVALE_EVENT:
                    return m_auiEncounter[7];
                case DATA_LORD_GODFREY_EVENT:
                    return m_auiEncounter[8];
            }
            return 0;
        }

        uint64 GetData64(uint32 id) const
        {
            switch(id)
            {
                case DATA_DOOR:   return DoorCourtyardGUID;
                case DATA_FRYE:   return fryeGUID;
                case DATA_HUMMEL: return hummelGUID;
                case DATA_BAXTER: return baxterGUID;
                case DATA_LORD_WALDEN: return LordWaldenGUID;
                case DATA_BARON_ASHBURY: return uiBaronAshbury;
                case DATA_BARON_SILVERLAINE: return uiBaronSilverlaine;
                case DATA_COMMANDER_SPRINGVALE: return uiCommanderSpringvale;
                case DATA_LORD_GODFREY: return uiLordGodfrey;
            }
            return 0;
        }

        std::string GetSaveData()
        {
            return str_data;
        }

        void Load(const char* in) override
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);

            std::istringstream loadStream(in);
            loadStream >> m_auiEncounter[0] >> m_auiEncounter[1] >> m_auiEncounter[2] >> m_auiEncounter[3] >> m_auiEncounter[4] >> m_auiEncounter[5] >> m_auiEncounter[6] >> m_auiEncounter[7] >> m_auiEncounter[8];

            for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
            {
                if (m_auiEncounter[i] == IN_PROGRESS)
                    m_auiEncounter[i] = NOT_STARTED;
            }

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        void Update(uint32 uiDiff)
        {
            if (GetData(TYPE_CROWN) == IN_PROGRESS)
            {
                if (spawnCrazedTimer <= uiDiff)
                {
                    if (Creature* hummel = instance->GetCreature(hummelGUID))
                        hummel->AI()->DoAction(ACTION_SPAWN_CRAZED);
                    spawnCrazedTimer = urand(2000, 5000);
                }
                else
                    spawnCrazedTimer -= uiDiff;
            }

            if (GetData(TYPE_FENRUS) != DONE)
                return;

            Creature* pArchmage = instance->GetCreature(uiArchmageArugalGUID);

            if (!pArchmage || !pArchmage->IsAlive())
                return;

            if (uiPhase)
            {
                if (uiTimer <= uiDiff)
                {
                    switch (uiPhase)
                    {
                        case 1:
                        {
                            Creature* summon = pArchmage->SummonCreature(pArchmage->GetEntry(), SpawnLocation[4], TEMPSUMMON_TIMED_DESPAWN, 10000);
                            summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                            summon->SetReactState(REACT_DEFENSIVE);
                            summon->CastSpell(summon, SPELL_ASHCROMBE_TELEPORT, true);
                            summon->AI()->Talk(SAY_ARCHMAGE);
                            uiTimer = 2000;
                            uiPhase = 2;
                            break;
                        }
                        case 2:
                        {
                            pArchmage->SummonCreature(NPC_ARUGAL_VOIDWALKER, SpawnLocation[0], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000);
                            pArchmage->SummonCreature(NPC_ARUGAL_VOIDWALKER, SpawnLocation[1], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000);
                            pArchmage->SummonCreature(NPC_ARUGAL_VOIDWALKER, SpawnLocation[2], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000);
                            pArchmage->SummonCreature(NPC_ARUGAL_VOIDWALKER, SpawnLocation[3], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 60000);
                            uiPhase = 0;
                            break;
                        }

                    }
                } else uiTimer -= uiDiff;
            }
        }
    };

};

void AddSC_instance_shadowfang_keep()
{
    new instance_shadowfang_keep();
}
