#ifndef WORLD_H
#define WORLD_H

#include <stdlib.h>
#include <iostream>
#include <vector>
#include <string>
#include <gl/gl.h>
#include "definitions.h"

using namespace std;

int char_to_int(char c);

class world
{
    public:
        world();

        bool init(int tex_tile);
        bool update(void);
        bool draw(void);
        int  collision_test(pos2d test_point);

        vector< vector<tile> > m_vecvec_tile;
        vector<fish_area>      m_vec_fish_area;
        vector<pos2d>          m_vec_water_tile;
        pos2d                  m_sale_area_pos;
        pos2d                  m_buy_area_pos;
        float                  m_sale_area_size;

    private:

        int m_tex_tile;

        float m_fish_spawn_time;
        float m_fish_spawn_timer;
        float m_fish_spawn_variation;
        float m_fish_lifetime;
        float m_fish_lifetime_variation;

        /*float m_sale_area_spawn_time;
        float m_sale_area_spawn_timer;
        float m_sale_area_spawn_variation;
        float m_sale_area_lifetime;
        float m_sale_area_lifetime_variation;*/


};

#endif // WORLD_H
