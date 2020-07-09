#pragma once
//===================================================================================
#include "sdk.h"
#include "baseapi.h"

#define make_ptr( Type, dwBase, dwOffset ) ((Type*)((DWORD)dwBase + (DWORD)dwOffset))

class c_offsets
{
private:
	RecvTable* pFindTable(const char * szTableName)
	{
		handle_structs::client_class * pClass = ints.chlclient->get_all_classes();
		do
		{
			RecvTable * pRecvTable = pClass->table;
			if (!pRecvTable)
				continue;
			if (!strcmp(pRecvTable->GetName(), szTableName))
			{
				return pRecvTable;
			}
			pClass = pClass->nextclass;
		}
		while (pClass);
		return NULL;
	}

	RecvProp* pFindProp(RecvTable * pRecvTable, const char * szPropertyName, int * x)
	{
		if (!pRecvTable) return NULL;
		for (int i = 0; i < pRecvTable->GetNumProps(); i++)
		{
			RecvProp * pRecvProp = pRecvTable->GetProp(i);
			if (!pRecvProp)
				continue;
			if (!strcmp(pRecvProp->GetName(), szPropertyName))
			{
				*x = pRecvProp->get_offset();
				return pRecvProp;
			}
		}
		return NULL;
	}

	int iFindOffset(RecvTable * pRecvTable, const char * szPropertyName)
	{
		int a = NULL;
		pFindProp(pRecvTable, szPropertyName, &a);
		return a;
	}

	int offset_DT_LocalPlayerExclusive__m_iAmmo,
	    offset_DT_LocalPlayerExclusive__m_nTickBase,
	    offset_DT_LocalPlayerExclusive__m_vecVelocity,

	    offset_DT_BasePlayer__m_lifeState,
	    offset_DT_BasePlayer__m_fFlags,
	    offset_DT_BasePlayer__m_iHealth,
	    offset_DT_Witch__m_wanderRage,
	    offset_DT_BaseEntity__m_iTeamNum,
	    offset_DT_BaseEntity__m_vecOrigin,
	    offset_DT_BaseCombatCharacter__m_hActiveWeapon,
	    offset_DT_BaseAnimating__m_nSequence,
	    offset_DT_Infected__m_vecOrigin;

public:
	void c_offsets::dump_netvars()
	{
		handle_structs::client_class * pClass = ints.chlclient->get_all_classes();
		do
		{
			RecvTable * pRecvTable = pClass->table;
			if (!pRecvTable)
				continue;

			api.log_file("Table '%s' found at: [0x%.8X]", pRecvTable->GetName(), (DWORD)pRecvTable);
			for (int i = 0; i < pRecvTable->GetNumProps(); i++)
			{
				RecvProp * pRecvProp = pRecvTable->GetProp(i);
				if (!pRecvProp)
					continue;

				int x = pRecvProp->get_offset();
				if (x == 0)
				{
					api.log_file("\tProperty '%s' found at: [0x%.8X]", pRecvProp->GetName(), (DWORD)pRecvProp);
					if (strcmp(pRecvProp->GetName(), "baseclass"))
					{
						int tableOffset;
						RecvProp * p = pFindProp(pRecvTable, pRecvProp->GetName(), &tableOffset);
						RecvTable * subTable = p->get_data_table();
						if (!subTable)
							continue;
						for (int i = 0; i < subTable->GetNumProps(); i++)
						{
							RecvProp * subProp = subTable->GetProp(i);
							if (!subProp)
								continue;
							api.log_file("\t\tProperty '%s' offset: [0x%.2X]", subProp->GetName(),
							             subProp->get_offset());
						}
					}
				}
				else
					api.log_file("\tProperty '%s' offset: [0x%.2X]", pRecvProp->GetName(), pRecvProp->get_offset());
			}
			pClass = pClass->nextclass;
		}
		while (pClass);
	}

	void c_offsets::Initialize()
	{
		RecvTable * tbl{NULL};
		RecvProp * p{NULL};

		tbl = pFindTable("DT_BasePlayer");
		offset_DT_BasePlayer__m_lifeState = iFindOffset(tbl, "m_lifeState");
		offset_DT_BasePlayer__m_fFlags = iFindOffset(tbl, "m_fFlags");
		offset_DT_BasePlayer__m_iHealth = iFindOffset(tbl, "m_iHealth");
		tbl = pFindTable("DT_BaseEntity");
		offset_DT_BaseEntity__m_iTeamNum = iFindOffset(tbl, "m_iTeamNum");
		offset_DT_BaseEntity__m_vecOrigin = iFindOffset(tbl, "m_vecOrigin");
		tbl = pFindTable("DT_BaseAnimating");
		offset_DT_BaseAnimating__m_nSequence = iFindOffset(tbl, "m_nSequence");
		tbl = pFindTable("DT_BaseCombatCharacter");
		offset_DT_BaseCombatCharacter__m_hActiveWeapon = iFindOffset(tbl, "m_hActiveWeapon");
		tbl = pFindTable("DT_Witch");
		offset_DT_Witch__m_wanderRage = iFindOffset(tbl, "m_wanderRage");
		tbl = pFindTable("DT_Infected");
		offset_DT_Infected__m_vecOrigin = iFindOffset(tbl, "m_vecOrigin");
	}

	int c_offsets::get_health(baseentity * pEntity) { return *make_ptr(int, pEntity, offset_DT_BasePlayer__m_iHealth); }

	bool c_offsets::is_alive(baseentity * pBaseEntity)
	{
		return handle_structs::life_states::LIFE_ALIVE == *make_ptr(BYTE, pBaseEntity,
		                                                            offset_DT_BasePlayer__m_lifeState);
	}

	int c_offsets::get_flags(baseentity * pBaseEntity)
	{
		return *make_ptr(int, pBaseEntity, offset_DT_BasePlayer__m_fFlags);
	}

	int c_offsets::get_sequence(baseentity * ent) { return *make_ptr(int, ent, offset_DT_BaseAnimating__m_nSequence); }

	const HANDLE c_offsets::get_active_weapon(baseentity * ent)
	{
		return *make_ptr(HANDLE, ent, offset_DT_BaseCombatCharacter__m_hActiveWeapon);
	}

	vector c_offsets::get_infected_origin(baseentity * ent)
	{
		return *make_ptr(vector, ent, offset_DT_Infected__m_vecOrigin);
	}

	vector c_offsets::get_entity_origin(baseentity * ent)
	{
		return *make_ptr(vector, ent, offset_DT_BaseEntity__m_vecOrigin);
	}

	bool c_offsets::get_witch_state(baseentity * ent) { return *make_ptr(bool, ent, offset_DT_Witch__m_wanderRage); }
	int c_offsets::get_team_num(baseentity * ent) { return *make_ptr(int, ent, offset_DT_BaseEntity__m_iTeamNum); }
};

//===================================================================================
extern c_offsets g_offsets;
