#ifndef ENEMY_H
#define ENEMY_H

#include <math.h>
#include <stdlib.h>
#include <gl/gl.h>
#include "definitions.h"

class enemy
{
    public:
        enemy();

        bool init(int texture);
        bool update(void);
        bool draw(void);

        bool  move_enemy(float x_shift,float y_shift);
        float get_enemy_size(void);
        pos2d get_enemy_pos(void);
        bool  set_enemy_pos(pos2d);
        int   get_enemy_dir(void);
        bool  set_enemy_dir(int new_dir);
        bool  change_hp(float value);
        bool  set_attack_target(pos2d target_pos);

        int   m_state;
        float m_attack_cooldown;
        float m_color[3];

    private:

        pos2d m_pos;
        pos2d m_move_target;
        float m_size,m_draw_size;
        float m_hp_curr,m_hp_max;
        int   m_dir;//from 1-8 (0=off) start top going cw
        float m_think_timer;
        float m_follow_timer;
        weapon* m_pWeapon;

        int   m_texture;
        int   m_tex_walk_ind;
        float m_tex_walk_timer;
        float m_tex_walk_time;
        float m_hurt_timer,m_hurt_time;
};

#endif // ENEMY_H
