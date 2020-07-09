#include "sdk.h"

const handle_structs::class_groups::groups ssdk::c_base_entity::get_entity_group(c_base_entity * ent)
{
	constexpr int boss[] = {handle_structs::class_ids::Tank, handle_structs::class_ids::Witch};
	constexpr int special[] = {
		handle_structs::class_ids::Boomer, handle_structs::class_ids::Charger, handle_structs::class_ids::Smoker,
		handle_structs::class_ids::Hunter, handle_structs::class_ids::Jockey, handle_structs::class_ids::Spitter
	};
	constexpr int infected[] = {handle_structs::class_ids::Infected, handle_structs::class_ids::CInsectSwarm};
	constexpr int player[] = {handle_structs::class_ids::CTerrorPlayer, handle_structs::class_ids::SurvivorBot};

	const int class_id{ent->get_client_class()->classid};

	if (std::find(std::begin(infected), std::end(infected), class_id) != std::end(infected))
		return handle_structs::class_groups::groups::INFECTED;
	if (std::find(std::begin(special), std::end(special), class_id) != std::end(special))
		return handle_structs::class_groups::groups::SPECIAL;
	if (std::find(std::begin(player), std::end(player), class_id) != std::end(player))
		return handle_structs::class_groups::groups::PLAYER;
	if (std::find(std::begin(boss), std::end(boss), class_id) != std::end(boss))
		return handle_structs::class_groups::groups::STRONK;
	return handle_structs::class_groups::groups::NONE;
}

const bool ssdk::c_base_entity::is_entity_valid()
{
	const auto team{g_offsets.get_team_num(this)};
	if (team != 2 && team != 3)
		return false;

	auto group{get_entity_group(this)};
	auto sequence{g_offsets.get_sequence(this)}; // ida 0x000008A4 from cbaseanimating
	auto flags{this->get_collideable()->get_solid_flags()};
	auto witch_rage{g_offsets.get_witch_state(this)};

	auto realize_validity = [&](void)
	{
		if (group == handle_structs::class_groups::NONE) return false;
		if (group == handle_structs::class_groups::STRONK)
		{
			if (flags & 4) return false;
			if (sequence > 70) return false;
			if (witch_rage == true) return false;
		}
		else if (group == handle_structs::class_groups::SPECIAL)
		{
			if (flags & 4) return false;
			if (sequence == 8) return true;
		}
		else if (group == handle_structs::class_groups::INFECTED)
		{
			if (flags & 4) return false;
			if (sequence > 305) return false;
		}
	};

	realize_validity();
	
	return true;
}
