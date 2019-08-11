#ifndef DEF_H
#define DEF_H

#include <math.h>

const float _version=0.7;
const float _pi=3.14159265359;
const float _game_update_step=10;
const int   _world_width=40;
const int   _world_height=30;
const int   _tile_size=128;
const float _water_speed=0.2;

const float _enemy_wave_time=30;
const float _enemy_wave_time_variation=0.3;
const int   _enemy_base_spawn_inc=1;
const float _enemy_spawn_variation=0.4;
const float _enemy_move_dist=80;
const float _enemy_think_time=3.0;
const float _enemy_think_time_variation=0.5;
const float _enemy_detection_dist=350;
const float _enemy_attack_cooldown=1.0;
const float _enemy_attack_count=10;
const float _enemy_attack_range=60;

const float _player_think_time=3.0;
const float _player_think_time_variation=0.5;
const float _player_detection_dist=200;
const float _player_move_dist=30;
const float _player_walk_speed=4.0;

enum gamestates
{
    gs_menu=0,
    gs_controls,
    gs_running,
    gs_gameover
};

enum main_unit_type
{
    type_player=0,
    type_enemy
};

enum enemy_state
{
    mode_idle=0,
    mode_move,
    mode_attack,
    mode_flee
};

enum directions_8
{
    dir_none=0,
    dir_up,
    dir_up_right,
    dir_right,
    dir_down_right,
    dir_down,
    dir_down_left,
    dir_left,
    dir_up_left
};

enum directions_4
{
    dir4_up=0,
    dir4_right,
    dir4_down,
    dir4_left
};

enum collision_type
{
    col_none=0,
    col_wall,
    col_water
};

enum tile_types
{
    tile_none=0,
    tile_grass,
    tile_water,
    tile_stone,
    tile_tree,
    tile_water_grass_right,
    tile_water_grass_down,
    tile_water_grass_left,
    tile_water_grass_up,
    tile_computer,
    tile_computer_buy
};

enum weapon_types
{
    wt_none=0,
    wt_pike,
    wt_salmon,
    wt_mackerel,
    wt_eel,
    wt_enemy
};

enum fishing_state
{
    fs_off=0,
    fs_waiting,
    fs_landing,
};

struct pos2d
{
    pos2d()
    {
        x=y=0.0;
    }
    pos2d(float _x,float _y)
    {
        x=_x;
        y=_y;
    }
    pos2d operator+(pos2d b)
    {
        pos2d sum;
        sum.x=x+b.x;
        sum.y=y+b.y;

        return sum;
    }
    pos2d operator-(pos2d b)
    {
        pos2d dif;
        dif.x=x-b.x;
        dif.y=y-b.y;

        return dif;
    }
    pos2d operator+=(pos2d b)
    {
        x+=b.x;
        y+=b.y;

        return *this;
    }
    pos2d operator=(pos2d b)
    {
        x=b.x;
        y=b.y;

        return *this;
    }
    bool operator==(pos2d b)
    {
        return ( x==b.x && y==b.y );
    }
    bool operator!=(pos2d b)
    {
        return ( x!=b.x || y!=b.y );
    }
    pos2d normalize()
    {
        float length=sqrt( (x*x)+(y*y) );
        if(length==0) return *this;
        x/=length;
        y/=length;

        return *this;
    }
    float length()
    {
        return sqrt( (x*x)+(y*y) );
    }

    float x,y;
};

struct tile
{
    tile()
    {
        type=0;
        dir=0;
        var=0;
    }
    tile(int _var)
    {
        type=0;
        dir=0;
        var=_var;
    }
    int type;
    int dir;
    int var;
    float frame_time,frame_timer;
    int   frame;
    bool frame_inc;
};

struct weapon
{
    weapon(int _type)
    {
        type=_type;
        switch(type)
        {
            case wt_none:
            {
                damage_count=0;
                range_width=0;
                range_height=0;
                range_dist=0;
                cooldown=0;
                durability=0;
            }break;

            case wt_pike:
            {
                damage_count=30;
                range_width=40;
                range_height=40;
                range_dist=60;
                cooldown=0.2;
                durability=100;
            }break;

            case wt_salmon:
            {
                damage_count=30;
                range_width=50;
                range_height=20;
                range_dist=60;
                cooldown=0.2;
                durability=100;
            }break;

            case wt_mackerel:
            {
                damage_count=30;
                range_width=20;
                range_height=20;
                range_dist=60;
                cooldown=0.2;
                durability=100;
            }break;

            case wt_eel:
            {
                damage_count=30;
                range_width=20;
                range_height=20;
                range_dist=100;
                cooldown=0.2;
                durability=100;
            }break;

            case wt_enemy:
            {
                damage_count=2;
                range_width=30;
                range_height=30;
                range_dist=60;
                cooldown=1.0;
                durability=100;
            }
        }
    }
    int type;
    float damage_count;
    float range_width;
    float range_height;
    float range_dist;
    float cooldown;
    float durability;
};

struct damage_area
{
    damage_area(int from,weapon* pWeap,pos2d dgm_pos,pos2d dmg_size)
    {
        dealt_from=from;
        pWeapon=pWeap;
        damage_pos=dgm_pos;
        damage_size=dmg_size;
        life_time=0.1;
    }
    int dealt_from;
    weapon* pWeapon;
    pos2d damage_pos;
    pos2d damage_size;
    float life_time;
};

struct fish_area
{
    fish_area(int tile_x,int tile_y,float _lifetime)
    {
        tile_pos[0]=tile_x;
        tile_pos[1]=tile_y;

        frame_ind=0;
        frame_timer=0;

        lifetime=_lifetime;
        in_use=false;
    }
    int tile_pos[2];
    float lifetime;
    bool in_use;
    int frame_ind;
    float frame_timer;
};

#endif
