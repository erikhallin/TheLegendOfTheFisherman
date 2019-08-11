#include "world.h"

world::world()
{
    m_fish_spawn_timer=1;
    m_fish_spawn_variation=0.5;
    m_fish_lifetime=10;
    m_fish_lifetime_variation=0.5;
}

bool world::init(int tex_tile)
{
    m_tex_tile=tex_tile;

    //spawn tiles
    for(int tile_x=0;tile_x<_world_width;tile_x++)
    {
        m_vecvec_tile.push_back( vector<tile>() );
        for(int tile_y=0;tile_y<_world_height;tile_y++)
        {
            m_vecvec_tile[tile_x].push_back( tile(rand()%4) );
        }
    }

    m_fish_spawn_time=m_fish_spawn_timer+float(rand()%2000-1000)/1000.0*m_fish_spawn_timer*m_fish_spawn_variation;


    //tiles id
    string tiles_id(
"4444444444444444444444444444444444444444"
"4444444444444444444444444444444444444444"
"4444444111111441111111441111441111144422"
"4444111111111141111111411111111111172222"
"4444111b11111111111111311111111111172222"
"4444111111311111111111111111111111172222"
"4441111111111111111111111114411111172222"
"4444111111111111111111111111111111172222"
"4444411111111441111111111111111111172222"
"4441111111111444411111311111111111172222"
"4444441111134411111111111111111141172222"
"4444111111111111111111111111111111172222"
"4441111111111111111111111111111111172222"
"4444411131111111111111111111411111172222"
"4441111111111111111111111111111111172222"
"4444111111111131111111111111111131172222"
"4444111111111444441111111111111111172222"
"4444111111114441111111111131111111172222"
"4444411111311111111114111111111111172222"
"4444111111111111111444441111133111172222"
"4444411111141111111114411111111111172222"
"4441111111444111311111111111111111172222"
"4444111111144441111111111111111111172222"
"4444111113111111111111131111111111172222"
"4444111911111111111111111111111111172222"
"4444111111111113111111111111111131172222"
"4444441111111114111111111111111111172222"
"4444444441114111114444111114111111172222"
"4444444444444444444444444444444444444222"
"4444444444444444444444444444444444444422"
);

    //assign tile type
    for(int tile_x=0;tile_x<_world_width;tile_x++)
    for(int tile_y=0;tile_y<_world_height;tile_y++)
    {
        m_vecvec_tile[tile_x][tile_y].type=char_to_int(tiles_id[tile_x+tile_y*_world_width]);

        if(m_vecvec_tile[tile_x][tile_y].type==tile_water) m_vec_water_tile.push_back(pos2d(tile_x,tile_y));

        //give variation tile id
        switch(m_vecvec_tile[tile_x][tile_y].type)
        {
            case tile_none:
            {
                m_vecvec_tile[tile_x][tile_y].var=rand()%1;
            }break;

            case tile_grass:
            {
                m_vecvec_tile[tile_x][tile_y].var=rand()%1;
            }break;

            case tile_stone:
            {
                m_vecvec_tile[tile_x][tile_y].var=rand()%2;
            }break;

            case tile_tree:
            {
                m_vecvec_tile[tile_x][tile_y].var=rand()%1;
            }break;

            case tile_water:
            {
                m_vecvec_tile[tile_x][tile_y].frame=rand()%2;
                m_vecvec_tile[tile_x][tile_y].frame_time=0.1+0.3*float(rand()%100)/100.0;
                m_vecvec_tile[tile_x][tile_y].frame_timer=0.3*float(rand()%100)/100.0;
            }break;

            case tile_computer:
            {
                m_sale_area_pos.x=_tile_size*tile_x+_tile_size*0.5;
                m_sale_area_pos.y=_tile_size*tile_y+_tile_size*0.5;
                //place sale area
                m_sale_area_size=_tile_size;
            }break;

            case tile_computer_buy:
            {
                m_buy_area_pos.x=_tile_size*tile_x+_tile_size*0.5;
                m_buy_area_pos.y=_tile_size*tile_y+_tile_size*0.5;
            }break;
        }
    }

    //place start fish area
    int numof_fish=(int)m_vec_water_tile.size()*0.3;//rand()%(int)m_vec_water_tile.size();
    for(int fish_i=0;fish_i<numof_fish;fish_i++)
    {
        //select position
        int tile_ind=rand()%(int)m_vec_water_tile.size();
        //test if occupied
        bool abort_spawn=false;
        for(int fish_i=0;fish_i<(int)m_vec_fish_area.size();fish_i++)
        {
            if(m_vec_fish_area[fish_i].tile_pos[0]==m_vec_water_tile[tile_ind].x &&
               m_vec_fish_area[fish_i].tile_pos[1]==m_vec_water_tile[tile_ind].y)
            {
                abort_spawn=true;
                break;
            }
        }
        if(!abort_spawn)
        {
            float lifetime=m_fish_lifetime+float(rand()%2000-1000)/1000.0*m_fish_lifetime*m_fish_lifetime_variation;
            m_vec_fish_area.push_back( fish_area( m_vec_water_tile[tile_ind].x,m_vec_water_tile[tile_ind].y,lifetime ) );
        }
    }



    return true;
}

bool world::update(void)
{
    //spawn fish area
    if(m_fish_spawn_time>0.0) m_fish_spawn_time-=_game_update_step*0.001;
    if(m_fish_spawn_time<=0.0)
    {
        //next spawn time
        m_fish_spawn_time=m_fish_spawn_timer+float(rand()%2000-1000)/1000.0*m_fish_spawn_timer*m_fish_spawn_variation;

        //select position
        int tile_ind=rand()%(int)m_vec_water_tile.size();
        //test if occupied
        bool abort_spawn=false;
        for(int fish_i=0;fish_i<(int)m_vec_fish_area.size();fish_i++)
        {
            if(m_vec_fish_area[fish_i].tile_pos[0]==m_vec_water_tile[tile_ind].x &&
               m_vec_fish_area[fish_i].tile_pos[1]==m_vec_water_tile[tile_ind].y)
            {
                abort_spawn=true;
                break;
            }
        }
        if(!abort_spawn)
        {
            float lifetime=m_fish_lifetime+float(rand()%2000-1000)/1000.0*m_fish_lifetime*m_fish_lifetime_variation;
            m_vec_fish_area.push_back( fish_area( m_vec_water_tile[tile_ind].x,m_vec_water_tile[tile_ind].y,lifetime ) );
        }
    }

    //age fish areas
    for(int fish_i=0;fish_i<(int)m_vec_fish_area.size();fish_i++)
    {
        m_vec_fish_area[fish_i].lifetime-=_game_update_step*0.001;
        if(m_vec_fish_area[fish_i].lifetime<=0 && !m_vec_fish_area[fish_i].in_use)
        {
            //remove
            m_vec_fish_area.erase(m_vec_fish_area.begin()+fish_i);
            fish_i--;
        }
    }

    //update tile anim
    for(int tile_x=0;tile_x<_world_width;tile_x++)
    for(int tile_y=0;tile_y<_world_height;tile_y++)
    {
        if(m_vecvec_tile[tile_x][tile_y].type==tile_water)
        {
            m_vecvec_tile[tile_x][tile_y].frame_timer-=_game_update_step*0.001;
            if(m_vecvec_tile[tile_x][tile_y].frame_timer<=0)
            {
                m_vecvec_tile[tile_x][tile_y].frame_timer=m_vecvec_tile[tile_x][tile_y].frame_time;

                if(m_vecvec_tile[tile_x][tile_y].frame_inc) m_vecvec_tile[tile_x][tile_y].frame++;
                else                                        m_vecvec_tile[tile_x][tile_y].frame--;
                if(m_vecvec_tile[tile_x][tile_y].frame>2)
                {
                    m_vecvec_tile[tile_x][tile_y].frame_inc=false;
                    m_vecvec_tile[tile_x][tile_y].frame=2;
                }
                if(m_vecvec_tile[tile_x][tile_y].frame<0)
                {
                    m_vecvec_tile[tile_x][tile_y].frame_inc=true;
                    m_vecvec_tile[tile_x][tile_y].frame=0;
                }
            }
        }
    }


    return true;
}

bool world::draw(void)
{

    //draw tiles
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D,m_tex_tile);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    float tex_tile_pos[2]={0,0};
    float tex_tile_size=(_tile_size*0.5)/1024.0;
    glColor3f(1,1,1);
    glBegin(GL_QUADS);
    for(int tile_x=0;tile_x<_world_width;tile_x++)
    for(int tile_y=0;tile_y<_world_height;tile_y++)
    {
        switch(m_vecvec_tile[tile_x][tile_y].type)
        {
            case tile_none:
            {
                //glColor3f(0.5,0.4,0.2);
                tex_tile_pos[0]=64.0/1024.0;
                tex_tile_pos[1]=0.0/1024.0+tex_tile_size*m_vecvec_tile[tile_x][tile_y].var;
            }break;
            case tile_grass:
            {
                //glColor3f(0.3,0.5,0.3);
                tex_tile_pos[0]=0.0/1024.0;
                tex_tile_pos[1]=0.0/1024.0+tex_tile_size*m_vecvec_tile[tile_x][tile_y].var;
            }break;
            case tile_water:
            {
                //glColor3f(0.3,0.3,0.9);
                tex_tile_pos[0]=256.0/1024.0;
                tex_tile_pos[1]=0.0/1024.0+tex_tile_size*m_vecvec_tile[tile_x][tile_y].frame;
            }break;
            case tile_stone:
            {
                //glColor3f(0.6,0.6,0.6);
                tex_tile_pos[0]=128.0/1024.0;
                tex_tile_pos[1]=0.0/1024.0+tex_tile_size*m_vecvec_tile[tile_x][tile_y].var;
            }break;
            case tile_tree:
            {
                //glColor3f(0.1,0.4,0.1);
                tex_tile_pos[0]=192.0/1024.0;
                tex_tile_pos[1]=0.0/1024.0+tex_tile_size*m_vecvec_tile[tile_x][tile_y].var;
            }break;

            case tile_computer:
            {
                //glColor3f(0.1,0.4,0.1);
                tex_tile_pos[0]=576.0/1024.0;
                tex_tile_pos[1]=0.0/1024.0;
            }break;

            case tile_computer_buy:
            {
                //glColor3f(0.1,0.4,0.1);
                tex_tile_pos[0]=640.0/1024.0;
                tex_tile_pos[1]=0.0/1024.0;
            }break;

            default://grass/water rotations 5-8
            {
                tex_tile_pos[0]=m_vecvec_tile[tile_x][tile_y].type*tex_tile_size;
                tex_tile_pos[1]=0.0/1024.0;
            }break;
        }

        /*//direction
        switch(m_vecvec_tile[tile_x][tile_y].dir)
        {
            case dir4_up:
            {
                 glTexCoord2f(tex_tile_pos[0],tex_tile_pos[1]);
                glVertex2f(tile_x*_tile_size,tile_y*_tile_size);
                 glTexCoord2f(tex_tile_pos[0],tex_tile_pos[1]+tex_tile_size);
                glVertex2f(tile_x*_tile_size,tile_y*_tile_size+_tile_size);
                 glTexCoord2f(tex_tile_pos[0]+tex_tile_size,tex_tile_pos[1]+tex_tile_size);
                glVertex2f(tile_x*_tile_size+_tile_size,tile_y*_tile_size+_tile_size);
                 glTexCoord2f(tex_tile_pos[0]+tex_tile_size,tex_tile_pos[1]);
                glVertex2f(tile_x*_tile_size+_tile_size,tile_y*_tile_size);
            }break;

            case dir4_right:
            {
                 glTexCoord2f(tex_tile_pos[0],tex_tile_pos[1]+tex_tile_size);
                glVertex2f(tile_x*_tile_size,tile_y*_tile_size);
                 glTexCoord2f(tex_tile_pos[0]+tex_tile_size,tex_tile_pos[1]+tex_tile_size);
                glVertex2f(tile_x*_tile_size,tile_y*_tile_size+_tile_size);
                 glTexCoord2f(tex_tile_pos[0]+tex_tile_size,tex_tile_pos[1]);
                glVertex2f(tile_x*_tile_size+_tile_size,tile_y*_tile_size+_tile_size);
                 glTexCoord2f(tex_tile_pos[0],tex_tile_pos[1]);
                glVertex2f(tile_x*_tile_size+_tile_size,tile_y*_tile_size);
            }break;

            case dir4_down:
            {
                 glTexCoord2f(tex_tile_pos[0]+tex_tile_size,tex_tile_pos[1]+tex_tile_size);
                glVertex2f(tile_x*_tile_size,tile_y*_tile_size);
                 glTexCoord2f(tex_tile_pos[0]+tex_tile_size,tex_tile_pos[1]);
                glVertex2f(tile_x*_tile_size,tile_y*_tile_size+_tile_size);
                 glTexCoord2f(tex_tile_pos[0],tex_tile_pos[1]);
                glVertex2f(tile_x*_tile_size+_tile_size,tile_y*_tile_size+_tile_size);
                 glTexCoord2f(tex_tile_pos[0],tex_tile_pos[1]+tex_tile_size);
                glVertex2f(tile_x*_tile_size+_tile_size,tile_y*_tile_size);
            }break;

            case dir4_left:
            {
                 glTexCoord2f(tex_tile_pos[0],tex_tile_pos[1]+tex_tile_size);
                glVertex2f(tile_x*_tile_size,tile_y*_tile_size);
                 glTexCoord2f(tex_tile_pos[0]+tex_tile_size,tex_tile_pos[1]+tex_tile_size);
                glVertex2f(tile_x*_tile_size,tile_y*_tile_size+_tile_size);
                 glTexCoord2f(tex_tile_pos[0]+tex_tile_size,tex_tile_pos[1]);
                glVertex2f(tile_x*_tile_size+_tile_size,tile_y*_tile_size+_tile_size);
                 glTexCoord2f(tex_tile_pos[0],tex_tile_pos[1]);
                glVertex2f(tile_x*_tile_size+_tile_size,tile_y*_tile_size);
            }break;
        }*/

         glTexCoord2f(tex_tile_pos[0],tex_tile_pos[1]);
        glVertex2f(tile_x*_tile_size,tile_y*_tile_size);
         glTexCoord2f(tex_tile_pos[0],tex_tile_pos[1]+tex_tile_size);
        glVertex2f(tile_x*_tile_size,tile_y*_tile_size+_tile_size);
         glTexCoord2f(tex_tile_pos[0]+tex_tile_size,tex_tile_pos[1]+tex_tile_size);
        glVertex2f(tile_x*_tile_size+_tile_size,tile_y*_tile_size+_tile_size);
         glTexCoord2f(tex_tile_pos[0]+tex_tile_size,tex_tile_pos[1]);
        glVertex2f(tile_x*_tile_size+_tile_size,tile_y*_tile_size);
    }
    glEnd();
    glDisable(GL_TEXTURE_2D);

    /*//draw fish areas
    float tile_gap=0.4;
    for(int fish_i=0;fish_i<(int)m_vec_fish_area.size();fish_i++)
    {
        glColor3f(0.2,0.2,0.6);
        glBegin(GL_QUADS);
        glVertex2f(m_vec_fish_area[fish_i].tile_pos[0]*_tile_size+_tile_size*0.5-_tile_size*tile_gap,
                   m_vec_fish_area[fish_i].tile_pos[1]*_tile_size+_tile_size*0.5-_tile_size*tile_gap);
        glVertex2f(m_vec_fish_area[fish_i].tile_pos[0]*_tile_size+_tile_size*0.5-_tile_size*tile_gap,
                   m_vec_fish_area[fish_i].tile_pos[1]*_tile_size+_tile_size*0.5+_tile_size*tile_gap);
        glVertex2f(m_vec_fish_area[fish_i].tile_pos[0]*_tile_size+_tile_size*0.5+_tile_size*tile_gap,
                   m_vec_fish_area[fish_i].tile_pos[1]*_tile_size+_tile_size*0.5+_tile_size*tile_gap);
        glVertex2f(m_vec_fish_area[fish_i].tile_pos[0]*_tile_size+_tile_size*0.5+_tile_size*tile_gap,
                   m_vec_fish_area[fish_i].tile_pos[1]*_tile_size+_tile_size*0.5-_tile_size*tile_gap);
        glEnd();
    }*/

    /*//draw sale area
    glColor3f(0.8,0.8,0.4);
    glBegin(GL_QUADS);
    glVertex2f(m_sale_area_pos.x-m_sale_area_size,m_sale_area_pos.y-m_sale_area_size);
    glVertex2f(m_sale_area_pos.x-m_sale_area_size,m_sale_area_pos.y+m_sale_area_size);
    glVertex2f(m_sale_area_pos.x+m_sale_area_size,m_sale_area_pos.y+m_sale_area_size);
    glVertex2f(m_sale_area_pos.x+m_sale_area_size,m_sale_area_pos.y-m_sale_area_size);
    glEnd();*/

    return true;
}

int world::collision_test(pos2d test_point)
{
    //ignore negative values
    if(test_point.x<0 || test_point.y<0) return -1;

    //test tile type
    int tile_pos_x=int( test_point.x/(float)_tile_size );
    int tile_pos_y=int( test_point.y/(float)_tile_size );

    /*//specisal rule
    if(m_vecvec_tile[tile_pos_x][tile_pos_y].type==tile_water_grass_down||
       m_vecvec_tile[tile_pos_x][tile_pos_y].type==tile_water_grass_up||
       m_vecvec_tile[tile_pos_x][tile_pos_y].type==tile_water_grass_right||
       m_vecvec_tile[tile_pos_x][tile_pos_y].type==tile_water_grass_left) return tile_water;*/

    if(m_vecvec_tile[tile_pos_x][tile_pos_y].type==tile_computer||
       m_vecvec_tile[tile_pos_x][tile_pos_y].type==tile_computer_buy) return tile_stone;

    return m_vecvec_tile[tile_pos_x][tile_pos_y].type;
}

inline int char_to_int(char c)
{
    switch(c)
    {
        case '0': return tile_none;
        case '1': return tile_grass;
        case '2': return tile_water;
        case '3': return tile_stone;
        case '4': return tile_tree;
        case '5': return tile_water_grass_right;
        case '6': return tile_water_grass_down;
        case '7': return tile_water_grass_left;
        case '8': return tile_water_grass_up;
        case '9': return tile_computer;
        case 'b': return tile_computer_buy;
    }

    return 0;
}

