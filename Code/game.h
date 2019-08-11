#ifndef GAME_H
#define GAME_H

#include <SOIL/SOIL.h>
#include <iostream>
#include <gl/gl.h>
#include <ctime>
#include <vector>
#include <stdlib.h>

#include "player.h"
#include "enemy.h"
#include "world.h"
#include "gamepad.h"
#include "sound.h"
#include "particle_engine.h"

using namespace std;

class game
{
    public:
        game();

        bool init(int screen_size[2],bool* pKeys,bool* pMouse_but,int* pMouse_pos);
        bool update(void);
        bool draw(void);

    private:

        int   m_game_state;
        int   m_screen_width,m_screen_height;
        bool* m_pKeys;
        bool* m_pMouse_but;
        int*  m_pMouse_pos;
        float m_time_last_cycle,m_time_this_cycle;
        pos2d m_cam_pos;
        float m_key_trigger_any_timer;

        float m_enemy_wave_timer;
        int   m_numof_enemies_to_spawn;

        weapon* m_pWeapon_enemy;
        world* m_pWorld;
        vector<player> m_vec_player;
        vector<enemy>  m_vec_enemy;
        vector<damage_area> m_vec_damage_area;
        vector<weapon*> m_vec_pWeapon[4];

        bool  m_gamepad_connected[4];
        gamepad* m_pGamepads;

        //texture
        int    m_tex_menu,m_tex_controls,m_tex_gameover,m_tex_tile,m_tex_player,m_tex_enemy,m_tex_fishing,m_tex_water;
        int    m_tex_player_attack[5];

        //sound
        sound* m_pSound;

        particle_engine* m_pPart;

        bool load_textures(void);
        bool load_sounds(void);


};

#endif // GAME_H
