#include "enemy.h"

enemy::enemy()
{
    m_tex_walk_time=0.1;
    m_hurt_time=0;
}

bool enemy::init(int texture)
{
    //place at random height at the left of the map
    m_pos.x=float(rand()%10+1)*_tile_size;
    m_pos.y=float(rand()%(_world_height-20)+10)*_tile_size;


    m_size=50.0;
    m_draw_size=64.0;
    m_dir=dir_down;
    m_hp_max=m_hp_curr=100;
    m_attack_cooldown=0;
    m_state=mode_move;
    m_follow_timer=0;
    m_texture=texture;
    m_tex_walk_ind=0;
    m_tex_walk_timer=0;
    m_hurt_timer=0;

    switch(rand()%6)
    {
        case 0: m_color[0]=1.0;m_color[1]=0.5;m_color[2]=0.5; break;
        case 1: m_color[0]=0.5;m_color[1]=1.0;m_color[2]=0.5; break;
        case 2: m_color[0]=0.5;m_color[1]=0.5;m_color[2]=1.0; break;
        case 3: m_color[0]=1.0;m_color[1]=1.0;m_color[2]=0.5; break;
        case 4: m_color[0]=0.5;m_color[1]=1.0;m_color[2]=1.0; break;
        case 5: m_color[0]=1.0;m_color[1]=0.5;m_color[2]=1.0; break;
    }

    m_pWeapon=new weapon(wt_enemy);

    return true;
}

bool enemy::update(void)
{
    if(m_attack_cooldown>0.0) m_attack_cooldown-=_game_update_step/1000.0;

    if(m_tex_walk_timer>0.0) m_tex_walk_timer-=_game_update_step*0.0001;//stop slow walk

    if(m_think_timer>0.0) m_think_timer-=_game_update_step*0.001;
    if(m_think_timer<=0.0)
    {
        m_think_timer=_enemy_think_time+float(rand()%2000-1000)/1000.0*m_think_timer*_enemy_think_time_variation;

        if(m_state==mode_idle)
        {
            //start to move
            if(rand()%2==0)
            {
                m_state=mode_move;
                m_move_target.x=(rand()%2000-1000)/1000.0;
                m_move_target.y=(rand()%2000-1000)/1000.0;
                m_move_target.normalize();
                m_move_target.x=m_move_target.x*_player_move_dist;
                m_move_target.y=m_move_target.y*_player_move_dist;
                //do not move outside world
                if(m_pos.x+m_move_target.x<1) m_move_target.x=0;
                if(m_pos.x+m_move_target.x>=_world_width*_tile_size) m_move_target.x=0;
                if(m_pos.y+m_move_target.y<1) m_move_target.y=0;
                if(m_pos.y+m_move_target.y>=_world_height*_tile_size) m_move_target.y=0;
            }
        }
        else if(m_state==mode_move)
        {
            //chance of going idle
            if(rand()%2==0) m_state=mode_idle;
            else//find a new move target
            {
                m_move_target.x=(rand()%2000-1000)/1000.0;
                m_move_target.y=(rand()%2000-1000)/1000.0;
                m_move_target.normalize();
                m_move_target.x=m_move_target.x*_player_move_dist;
                m_move_target.y=m_move_target.y*_player_move_dist;
                //do not move outside world
                if(m_pos.x+m_move_target.x<1) m_move_target.x=0;
                if(m_pos.x+m_move_target.x>=_world_width*_tile_size) m_move_target.x=0;
                if(m_pos.y+m_move_target.y<1) m_move_target.y=0;
                if(m_pos.y+m_move_target.y>=_world_height*_tile_size) m_move_target.y=0;
            }
        }
    }

    switch(m_state)
    {
        case mode_idle:
        {
            ;
        }break;

        case mode_move://move towards the target
        {
            //set dir
            if(m_move_target.x>0.0) m_dir=dir_right;
            if(m_move_target.x<0.0) m_dir=dir_left;
            if(m_move_target.y>0.0 && fabs(m_move_target.y)>fabs(m_move_target.x) ) m_dir=dir_down;
            if(m_move_target.y<0.0 && fabs(m_move_target.y)>fabs(m_move_target.x) ) m_dir=dir_up;
            m_move_target.normalize();

            //move pos
            float move_sens=1.0;
            m_pos.x+=m_move_target.x*move_sens;
            m_pos.y+=m_move_target.y*move_sens;

            //walk frame
            m_tex_walk_timer-=_game_update_step*0.001;
            if(m_tex_walk_timer<=0.0)
            {
                m_tex_walk_timer=m_tex_walk_time;
                m_tex_walk_ind++;
                if(m_tex_walk_ind>3) m_tex_walk_ind=0;
            }

        }break;

        case mode_attack:
        {
            pos2d rel_pos=m_move_target-m_pos;
            rel_pos.normalize();

            //set dir
            if(rel_pos.x>0.0) m_dir=dir_right;
            if(rel_pos.x<0.0) m_dir=dir_left;
            if(rel_pos.y>0.0 && fabs(rel_pos.y)>fabs(rel_pos.x) ) m_dir=dir_down;
            if(rel_pos.y<0.0 && fabs(rel_pos.y)>fabs(rel_pos.x) ) m_dir=dir_up;

            //move pos
            float move_sens=1.0;
            m_pos.x+=rel_pos.x*move_sens;
            m_pos.y+=rel_pos.y*move_sens;
            //float walk_speed=rel_pos.length();

            if(m_follow_timer>0.0) m_follow_timer-=_game_update_step*0.001;
            if(m_follow_timer<=0.0) m_state=mode_idle;

            //walk frame
            m_tex_walk_timer-=_game_update_step*0.001;
            if(m_tex_walk_timer<=0.0)
            {
                m_tex_walk_timer=m_tex_walk_time;
                m_tex_walk_ind++;
                if(m_tex_walk_ind>3) m_tex_walk_ind=0;
            }
        }break;
    }

    return true;
}

bool enemy::draw(void)
{
    glPushMatrix();

    glTranslatef(m_pos.x,m_pos.y,0.0);

    /*glColor3f(1,0,0);
    glBegin(GL_QUADS);
    glVertex2f(-m_size,-m_size);
    glVertex2f(-m_size,m_size);
    glVertex2f(m_size,m_size);
    glVertex2f(m_size,-m_size);
    glEnd();*/

    glColor3fv(m_color);

    if(m_attack_cooldown>0 && m_attack_cooldown>m_pWeapon->cooldown*0.75)
    {
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D,m_texture);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        float tex_tile_size=128.0/1024.0;
        float tex_pos[2]={0,0};
        tex_pos[0]=tex_tile_size;

        float attack_prog=(m_attack_cooldown-m_pWeapon->cooldown*0.75)/m_pWeapon->cooldown*20.0;
        //float attack_prog=(1.0-(speeded_attack/m_pWeapon->cooldown))*10.0;
        int attack_frame=(int)attack_prog;
        if(attack_frame>4) attack_frame=4;

        tex_pos[1]=attack_frame*tex_tile_size;

        //if hurt
        if(m_hurt_timer>0) glColor3f(0.6,0.1,0.1);

        glBegin(GL_QUADS);
        glTexCoord2f(tex_pos[0],tex_pos[1]);
        glVertex2f(-m_draw_size*1.0,-m_draw_size*1.0);
        glTexCoord2f(tex_pos[0],tex_pos[1]+tex_tile_size);
        glVertex2f(-m_draw_size*1.0,m_draw_size*1.0);
        glTexCoord2f(tex_pos[0]+tex_tile_size,tex_pos[1]+tex_tile_size);
        glVertex2f(m_draw_size*1.0,m_draw_size*1.0);
        glTexCoord2f(tex_pos[0]+tex_tile_size,tex_pos[1]);
        glVertex2f(m_draw_size*1.0,-m_draw_size*1.0);
        glEnd();
    }
    else
    {
        glEnable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glBindTexture(GL_TEXTURE_2D,m_texture);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        float tex_tile_size=128.0/1024.0;
        float tex_pos[2]={0,0};
        tex_pos[0]=0.0;
        tex_pos[1]=m_tex_walk_ind*tex_tile_size;
        /*switch(m_dir)
        {
            case dir_up:    tex_pos[0]=tex_tile_size*1.0; break;
            case dir_right: tex_pos[0]=tex_tile_size*3.0; break;
            case dir_down:  tex_pos[0]=tex_tile_size*0.0; break;
            case dir_left:  tex_pos[0]=tex_tile_size*2.0; break;
        }*/

        /*//use color filter
        switch(index)
        {
            case 0: glColor3f(1.0,1.0,1.0); break;
            case 1: glColor3f(1.0,0.8,0.8); break;
            case 2: glColor3f(0.8,1.0,0.8); break;
            case 3: glColor3f(0.8,0.8,1.0); break;
        }*/

        //if hurt
        if(m_hurt_timer>0) glColor3f(0.6,0.1,0.1);

        glBegin(GL_QUADS);
        glTexCoord2f(tex_pos[0],tex_pos[1]);
        glVertex2f(-m_draw_size,-m_draw_size);
        glTexCoord2f(tex_pos[0],tex_pos[1]+tex_tile_size);
        glVertex2f(-m_draw_size,m_draw_size);
        glTexCoord2f(tex_pos[0]+tex_tile_size,tex_pos[1]+tex_tile_size);
        glVertex2f(m_draw_size,m_draw_size);
        glTexCoord2f(tex_pos[0]+tex_tile_size,tex_pos[1]);
        glVertex2f(m_draw_size,-m_draw_size);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);
    }



    /*//draw direction marker, TEMP
    glColor3f(0,0,0);
    glBegin(GL_LINES);
    switch(m_dir)
    {
        case dir_up:         glVertex2f(0,-m_size); glVertex2f(0,0); break;
        case dir_up_right:   glVertex2f(m_size,-m_size); glVertex2f(0,0); break;
        case dir_right:      glVertex2f(m_size,0); glVertex2f(0,0); break;
        case dir_down_right: glVertex2f(m_size,m_size); glVertex2f(0,0); break;
        case dir_down:       glVertex2f(0,m_size); glVertex2f(0,0); break;
        case dir_down_left:  glVertex2f(-m_size,m_size); glVertex2f(0,0); break;
        case dir_left:       glVertex2f(-m_size,0); glVertex2f(0,0); break;
        case dir_up_left:    glVertex2f(-m_size,-m_size); glVertex2f(0,0); break;
    }
    glEnd();*/


    glPopMatrix();

    return true;
}

bool enemy::move_enemy(float x_shift,float y_shift)
{
    m_pos.x+=x_shift;
    m_pos.y+=y_shift;

    return true;
}

float enemy::get_enemy_size(void)
{
    return m_size;
}

pos2d enemy::get_enemy_pos(void)
{
    return m_pos;
}

bool enemy::set_enemy_pos(pos2d new_pos)
{
    m_pos=new_pos;

    return true;
}

int enemy::get_enemy_dir(void)
{
    return m_dir;
}

bool enemy::set_enemy_dir(int new_dir)
{
    m_dir=new_dir;

    return true;
}

bool enemy::change_hp(float value)
{
    m_hp_curr+=value;

    if(m_hp_curr<=0) return false;
    return true;
}

bool enemy::set_attack_target(pos2d target_pos)
{
    m_state=mode_attack;
    m_move_target=target_pos;
    m_follow_timer=10+10*float(rand()%2000-1000)/1000.0*0.5;
}
