#ifndef PLAYER_H
#define PLAYER_H

#include <math.h>
#include <stdlib.h>
#include <gl/gl.h>
#include "definitions.h"
#include "sound.h"
#include "sound_list.h"
#include "particle_engine.h"

class player
{
    public:
        player();

        bool init(int texture,int texture_item,int texture_fishing,int* tex_player_attack,sound* pSound,particle_engine* pPart);
        int  update(void);
        bool draw(int index);

        bool  move_player(float x_shift,float y_shift);
        float get_player_size(void);
        pos2d get_player_pos(void);
        bool  set_player_pos(pos2d,bool forced=true);
        int   get_player_dir(void);
        bool  set_player_dir(int new_dir);
        bool  attack(void);
        weapon* get_weapon_ptr(void);
        bool    set_weapon_ptr(weapon* new_weapon);
        bool  change_hp(float value);
        bool  set_flee_target(pos2d target_pos);
        bool  use_default_weapon(void);


        bool  m_key_trigger_attack,m_key_trigger_fish,m_key_trigger_abort;
        bool  m_key_trigger_invent_left,m_key_trigger_invent_right,m_key_trigger_y;
        int   m_fishing_state;
        pos2d m_fishing_line_pos_start;
        pos2d m_fishing_line_pos_end;
        float m_fish_landing_progress;
        float m_hp_curr,m_hp_max;
        bool  m_human_control;
        int   m_state;
        int   m_inventory_selection_ind;
        int   m_fish_on_hook;
        bool  m_have_boat,m_have_boat_ready;
        float m_fishing_rod_force,m_fishing_rod_force_multiplyer;
        int   m_coins_curr,m_coins_max;
        float m_hurt_timer,m_hurt_time;
        bool  m_hurt_anim_one;

    private:

        pos2d m_pos;
        float m_size,m_draw_size;

        sound* m_pSound;
        particle_engine* m_pPart;

        int   m_dir;//from 1-8 (0=off) start top going cw
        float m_attack_cooldown;
        float m_fishing_waiting_time;
        float m_fishing_waiting_time_variation;
        float m_fishing_waiting_timer;

        pos2d m_move_target;
        float m_think_timer;
        float m_follow_timer;

        int   m_texture,m_texture_item,m_texture_fishing;
        int*  m_pTex_attack;
        int   m_tex_walk_ind;
        float m_tex_walk_timer;
        float m_tex_walk_time;

        weapon* m_pWeapon_curr;
        weapon* m_pWeapon_default;
};

#endif // PLAYER_H
