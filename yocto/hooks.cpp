#include "sdk.h"
#include "hooks.h"
#include "baseapi.h"

#define player(x) handle_structs::player_related::x

bool __fastcall hooks::createmove(PVOID client_mode, int edx, float input_sample_frametime,
                                  handle_structs::player_related::user_cmd * cmd)
{
	try
	{
		vmt_manager & hook = vmt_manager::get_hooken(client_mode);
		bool b_return{
			hook.get_method<bool(__thiscall*)(PVOID, float, handle_structs::player_related::user_cmd *)>(27)(
				client_mode, input_sample_frametime, cmd)
		};

		ssdk::c_base_entity * local = get_base_entity(ints.engine->get_local_player());
		if (local == NULL)
			return b_return;

		auto flags = g_offsets.get_flags(local);

		cmd->buttons &= cmd->buttons & player(IN_JUMP) && !(flags & player(FL_ONGROUND)) ? ~player(IN_JUMP) : UINT_MAX;

		auto do_autopistol = [&](void)
		{
			static bool shootshoot{false};
			if (cmd->buttons & player(IN_ATTACK))
			{
				if (shootshoot)
					cmd->buttons &= ~player(IN_ATTACK);
			}
			shootshoot = cmd->buttons & player(IN_ATTACK) ? true : false;
		};

		do_autopistol();

		return b_return;
	}
	catch (const std::exception & e) { api.log_file("failed createmove"); }
}

void __fastcall hooks::painttraverse(PVOID pPanels, int edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce)
{
	try
	{
		// hooked in the first place because I wanted to draw on painttraverse but then I just switched to paint

		vmt_manager & hook = vmt_manager::get_hooken(pPanels);
		hook.get_method<void(__thiscall*)(PVOID, unsigned int, bool, bool)>(41)(
			pPanels, vguiPanel, forceRepaint, allowForce);

		static unsigned int panel;

		if (panel == NULL)
		{
			const char * n = ints.panels->get_name(vguiPanel);
			if (n[0] == 'M' && n[3] == 'S' && n[4] == 'y' && n[5] == 's' && n[6] == 't')
			{
				panel = vguiPanel;
				pt::intro();
			}
		}

		if (!(panel == vguiPanel))
			return;
	}
	catch (...) { api.log_file("failed painttraverse"); }
}

void __fastcall hooks::paint(PVOID engine, int edx, int mode)
{
	try
	{
		vmt_manager & hook = vmt_manager::get_hooken(engine);
		hook.get_method<void(__thiscall*)(PVOID, int)>(14)(engine, mode);

		auto const start_drawing{
			reinterpret_cast<void(__thiscall*)(void *)>(api.get_sig("vguimatsurface.dll",
			                                                        "33 C5 50 8D 45 F4 64 A3 ? ? ? ? 8B F9 80 3D") -
				0x1b)
		};
		auto const finish_drawing{
			reinterpret_cast<void(__thiscall*)(void *)>(api.get_sig("vguimatsurface.dll",
			                                                        "51 56 A1 ? ? ? ? 33 C5 50 8D 45 F4 64 A3 ? ? ? ? 6A")
				- 0x11)
		};

		auto draw_entities = [&](void)
		{
			ssdk::c_base_entity * local = get_base_entity(ints.engine->get_local_player());

			if (local == NULL)
				return;

			vector screen, worldpos;
			for (auto i = 1; i <= ints.ent_list->get_highest_entity_index(); i++)
			{
				ssdk::c_base_entity * entity = ints.ent_list->get_client_entity(i);

				if (entity == NULL)
					continue;

				worldpos = g_offsets.get_infected_origin(entity);
				if (!draw.w2s(worldpos, screen))
					continue;

				auto class_id{entity->get_client_class()->classid};
				char * name{entity->get_client_class()->name};

				if (entity->is_entity_valid())
				{
					if (((class_id == handle_structs::class_ids::Infected) || (class_id == handle_structs::class_ids::
						CInsectSwarm)))
						draw.draw_string(screen.x, screen.y, COLORCODE(0, 255, 0, 255), "infected");
					screen.y += ESP_HEIGHT;
				}
			}
		};

		if (mode & 2)
		{
			start_drawing(ints.isurface);
			vector s(5, 10, 0);
			draw.draw_string(s.x, s.y, COLORCODE(255, 255, 255, 255), "yocto-base");
			draw_entities();
			finish_drawing(ints.isurface);
		}
	}
	catch (...) { api.error_box("fug, enginevgui"); }
}

void __fastcall hooks::scene_end(PVOID renderview, int edx)
{
	try
	{
		vmt_manager & hook = vmt_manager::get_hooken(renderview);
		hook.get_method<void(__thiscall*)(PVOID)>(9)(renderview);

		ssdk::c_base_entity * local = get_base_entity(ints.engine->get_local_player());
		if (local == NULL)
			return;

		auto remove_boomer_vomit = [&](void)
		{
			static ssdk::material * mat{nullptr}; //nullptr > null owned

			if (!mat)
			{
				mat = ints.i_material_system->find_mat("particle/screenspaceboomervomit", "Particle textures");
				mat->set_mat_var_flag(handle_structs::mat::mat_var_flags::MATERIAL_VAR_NO_DRAW, true);
			}

			ints.i_model_renderer->forced_mat_override(mat);
			ints.i_model_renderer->forced_mat_override(nullptr);
		};

		remove_boomer_vomit();
	}
	catch (...) { api.log_file("dead"); }
}

vector backup_punch;

bool __fastcall hooks::in_prediction(PVOID iprediction, int edx)
{
	try
	{
		vmt_manager & hook = vmt_manager::get_hooken(iprediction);
		static auto ret = hook.get_method<bool(__thiscall*)(PVOID)>(14)(iprediction);

		void * vesi;
		__asm mov vesi, esi;

		ssdk::c_base_entity * local = get_base_entity(ints.engine->get_local_player());
		if (local == NULL)
			return ret;

		if (!ints.ent_list->get_client_entity(ints.engine->get_local_player()) || ints
		                                                                          .ent_list->get_client_entity(
			                                                                          ints.engine->get_local_player())
			!= vesi)
			return ret;

		auto * c_return = _ReturnAddress();

		return ret;
	}
	catch (...) { api.log_file("fok"); }
}

void hooks::cl_move::client_move(float a, bool b)
{
	o_client_move(a, b);
	if (GetAsyncKeyState(VK_XBUTTON1) & 0x8000)
	{
		for (auto var = 0; var < 2; var++)
			o_client_move(a, b);
	}
}

void hooks::pt::intro()
{
	auto do_init = [&](void)
	{
		try
		{
			g_offsets.Initialize();
			draw.initialize_font();
		}
		catch (const std::exception & e) { api.log_file("couldn't init"); }
	};
	const bool dump_vars{false};
	dump_vars == true ? g_offsets.dump_netvars() : do_init();
}
