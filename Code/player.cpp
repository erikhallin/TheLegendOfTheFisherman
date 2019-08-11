#include "player.h"

player::player()
{
    m_fishing_waiting_time=3.0;
    m_fishing_waiting_time_variation=1.0;
    m_tex_walk_time=0.1;
    m_hurt_time=0.1;
}

bool player::init(int texture,int texture_item,int texture_fishing,int* tex_player_attack,sound* pSound,particle_engine* pPart)
{
    m_pSound=pSound;
    m_pPart=pPart;
    m_pos.x=_world_width*0.5*_tile_size;
    m_pos.y=_world_height*0.5*_tile_size;
    //m_pos.x=15*_tile_size;
    //m_pos.y=10*_tile_size;
    m_size=50.0;
    m_draw_size=64.0;
    m_hp_max=m_hp_curr=100;
    m_dir=dir_down;
    m_attack_cooldown=0;
    m_pWeapon_default=m_pWeapon_curr=new weapon(wt_none);
    m_key_trigger_attack=m_key_trigger_fish=m_key_trigger_abort=false;
    m_key_trigger_invent_left=m_key_trigger_invent_right=m_key_trigger_y=false;
    m_fishing_state=fs_off;
    m_fishing_waiting_timer=m_fishing_waiting_time+float(rand()%2000-1000)/1000.0*m_fishing_waiting_time*m_fishing_waiting_time_variation;
    m_fish_landing_progress=0.0;
    m_human_control=false;
    m_think_timer=0;
    m_state=mode_idle;
    m_inventory_selection_ind=-1;
    m_fish_on_hook=wt_none;
    m_fishing_rod_force=1;
    m_fishing_rod_force_multiplyer=1.0;
    m_have_boat=false;
    m_have_boat_ready=false;
    m_coins_curr=0;
    m_coins_max=100;
    m_texture=texture;
    m_texture_item=texture_item;
    m_texture_fishing=texture_fishing;
    m_pTex_attack=tex_player_attack;
    m_tex_walk_ind=0;
    m_tex_walk_timer=0;
    m_hurt_timer=0;
    m_hurt_anim_one=false;

    return true;
}

int player::update(void)
{
    if(m_attack_cooldown>0.0) m_attack_cooldown-=_game_update_step*0.001;

    if(m_hurt_timer>0)
    {
        m_hurt_timer-=_game_update_step*0.001;
        if(m_hurt_timer<=0) m_hurt_timer=0;
    }

    if(m_tex_walk_timer>0.0) m_tex_walk_timer-=_game_update_step*0.00001;//stop slow walk

    /*//walk frame
    m_tex_walk_timer-=_game_update_step*0.001;
    if(m_tex_walk_timer<=0.0)
    {
        m_tex_walk_timer=m_tex_walk_time;
        m_tex_walk_ind++;
        if(m_tex_walk_ind>7) m_tex_walk_ind=0;
    }*/

    if(!m_human_control)
    {
        if(m_think_timer>0.0) m_think_timer-=_game_update_step*0.001;
        if(m_think_timer<=0.0)
        {
            m_think_timer=_player_think_time+float(rand()%2000-1000)/1000.0*m_think_timer*_player_think_time_variation;

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
                m_tex_walk_ind=0;
            }break;

            case mode_move://move towards the target
            {
                pos2d rel_pos=m_move_target-m_pos;
                rel_pos.normalize();
                float walk_speed=rel_pos.length();

                //set dir
                if(m_move_target.x>0.0) m_dir=dir_right;
                if(m_move_target.x<0.0) m_dir=dir_left;
                if(m_move_target.y>0.0 && fabs(m_move_target.y)>fabs(m_move_target.x) ) m_dir=dir_down;
                if(m_move_target.y<0.0 && fabs(m_move_target.y)>fabs(m_move_target.x) ) m_dir=dir_up;

                //move pos
                float move_sens=0.008;
                m_pos.x+=rel_pos.x*move_sens;
                m_pos.y+=rel_pos.y*move_sens;

                //walk frame
                m_tex_walk_timer-=_game_update_step*0.001*walk_speed*_player_walk_speed*0.5;
                if(m_tex_walk_timer<=0.0)
                {
                    m_tex_walk_timer=m_tex_walk_time;
                    m_tex_walk_ind++;
                    if(m_tex_walk_ind>7) m_tex_walk_ind=0;
                    //play sound
                    if((m_tex_walk_ind==0||m_tex_walk_ind==4)&&m_human_control&&!m_have_boat)
                    {
                        if(rand()%2==0) m_pSound->playSimpleSound(wav_footstep1,1);
                        else m_pSound->playSimpleSound(wav_footstep2,1);

                        //particles
                        float pos[2]={m_pos.x,m_pos.y+44};
                        float color[3]={0,1,0};
                        m_pPart->add_explosion(color,pos,10,300,0.20);
                    }
                }

            }break;

            case mode_flee:
            {
                pos2d rel_pos=m_pos-m_move_target;
                rel_pos.normalize();
                float walk_speed=rel_pos.length();

                //set dir
                if(rel_pos.x>0.0) m_dir=dir_right;
                if(rel_pos.x<0.0) m_dir=dir_left;
                if(rel_pos.y>0.0 && fabs(rel_pos.y)>fabs(rel_pos.x) ) m_dir=dir_down;
                if(rel_pos.y<0.0 && fabs(rel_pos.y)>fabs(rel_pos.x) ) m_dir=dir_up;

                //move pos
                float move_sens=0.13;
                m_pos.x+=rel_pos.x*move_sens;
                m_pos.y+=rel_pos.y*move_sens;

                if(m_follow_timer>0.0) m_follow_timer-=_game_update_step*0.001;
                if(m_follow_timer<=0.0) m_state=mode_idle;

                //walk frame
                m_tex_walk_timer-=_game_update_step*0.001*walk_speed*_player_walk_speed*0.5;
                if(m_tex_walk_timer<=0.0)
                {
                    m_tex_walk_timer=m_tex_walk_time;
                    m_tex_walk_ind++;
                    if(m_tex_walk_ind>7) m_tex_walk_ind=0;
                    //play sound
                    if((m_tex_walk_ind==0||m_tex_walk_ind==4)&&m_human_control&&!m_have_boat)
                    {
                        if(rand()%2==0) m_pSound->playSimpleSound(wav_footstep1,1);
                        else m_pSound->playSimpleSound(wav_footstep2,1);

                        //particles
                        float pos[2]={m_pos.x,m_pos.y+44};
                        float color[3]={0,1,0};
                        m_pPart->add_explosion(color,pos,10,300,0.20);
                    }
                }
            }break;
        }
    }

    if(m_fishing_state==fs_landing)
    {
        //reduce progress
        float sens=0.001;
        m_fish_landing_progress-=sens;

        //abort if zero
        if(m_fish_landing_progress<=0.0)
        {
            m_fish_landing_progress=0;
            m_fishing_state=fs_off;
            return 2;
        }
    }

    if(m_fishing_state==fs_waiting)
    {
        if(m_fishing_waiting_timer>0.0) m_fishing_waiting_timer-=_game_update_step*0.001;
        else//start landing
        {
            m_fishing_waiting_timer=m_fishing_waiting_time+float(rand()%2000-1000)/1000.0*m_fishing_waiting_time*m_fishing_waiting_time_variation;
            m_fishing_state=fs_landing;
            m_fish_landing_progress=0.2;//start at 20%
        }
    }

    return 0;
}

bool player::draw(int index)
{
    glPushMatrix();

    //draw fishing line
    if(m_fishing_state!=fs_off)
    {
        glLineWidth(1);
        glColor3f(0.9,0.9,0.9);
        glBegin(GL_LINES);
        glVertex2f(m_fishing_line_pos_start.x,m_fishing_line_pos_start.y);
        //glVertex2f(m_fishing_line_pos_end.x,m_fishing_line_pos_end.y);
        switch(m_dir)
        {
            case dir_up:    glVertex2f(m_pos.x+13,m_pos.y-100); break;
            case dir_right: glVertex2f(m_pos.x+121,m_pos.y-64); break;
            case dir_down:  glVertex2f(m_pos.x-4,m_pos.y-93); break;
            case dir_left:  glVertex2f(m_pos.x-122,m_pos.y-63); break;
        }

        glEnd();
    }

    glTranslatef(m_pos.x,m_pos.y,0.0);

    /*glColor3f(0.4,0.8,0.4);
    if(m_hp_curr<=0.0) glColor3f(0.8,0.4,0.4);//dead
    glBegin(GL_QUADS);
    glVertex2f(-m_size,-m_size);
    glVertex2f(-m_size,m_size);
    glVertex2f(m_size,m_size);
    glVertex2f(m_size,-m_size);
    glEnd();*/

    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    if(m_have_boat)//draw boat
    {
        glColor3f(1,1,1);
        glBindTexture(GL_TEXTURE_2D,m_texture_item);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        float tex_tile_size=64.0/1024.0;
        float tex_pos[2]={0,0};
        tex_pos[0]=704.0/1024.0;
        tex_pos[1]=0.0/1024.0;

        switch(m_dir)
        {
            case dir_up:
            {
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
            }break;

            case dir_right:
            {
                glBegin(GL_QUADS);
                glTexCoord2f(tex_pos[0],tex_pos[1]+tex_tile_size);
                glVertex2f(-m_draw_size,-m_draw_size);
                glTexCoord2f(tex_pos[0]+tex_tile_size,tex_pos[1]+tex_tile_size);
                glVertex2f(-m_draw_size,m_draw_size);
                glTexCoord2f(tex_pos[0]+tex_tile_size,tex_pos[1]);
                glVertex2f(m_draw_size,m_draw_size);
                glTexCoord2f(tex_pos[0],tex_pos[1]);
                glVertex2f(m_draw_size,-m_draw_size);
                glEnd();
            }break;

            case dir_down:
            {
                glBegin(GL_QUADS);
                glTexCoord2f(tex_pos[0]+tex_tile_size,tex_pos[1]+tex_tile_size);
                glVertex2f(-m_draw_size,-m_draw_size);
                glTexCoord2f(tex_pos[0]+tex_tile_size,tex_pos[1]);
                glVertex2f(-m_draw_size,m_draw_size);
                glTexCoord2f(tex_pos[0],tex_pos[1]);
                glVertex2f(m_draw_size,m_draw_size);
                glTexCoord2f(tex_pos[0],tex_pos[1]+tex_tile_size);
                glVertex2f(m_draw_size,-m_draw_size);
                glEnd();
            }break;

            case dir_left:
            {
                glBegin(GL_QUADS);
                glTexCoord2f(tex_pos[0]+tex_tile_size,tex_pos[1]);
                glVertex2f(-m_draw_size,-m_draw_size);
                glTexCoord2f(tex_pos[0],tex_pos[1]);
                glVertex2f(-m_draw_size,m_draw_size);
                glTexCoord2f(tex_pos[0],tex_pos[1]+tex_tile_size);
                glVertex2f(m_draw_size,m_draw_size);
                glTexCoord2f(tex_pos[0]+tex_tile_size,tex_pos[1]+tex_tile_size);
                glVertex2f(m_draw_size,-m_draw_size);
                glEnd();
            }break;
        }
    }

    //if dead
    if(m_hp_curr<=0)
    {
        glBindTexture(GL_TEXTURE_2D,m_texture);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        float tex_tile_size=128.0/1024.0;
        float tex_pos[2]={0,0};
        tex_pos[0]=512.0/1024.0;
        tex_pos[1]=0.0;

        glColor3f(1.0,0.3,0.3);
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

        glPopMatrix();

        return true;//stop here
    }



    //draw attack sprite?
    if(m_attack_cooldown>0)
    {
        glBindTexture(GL_TEXTURE_2D,m_pTex_attack[0]);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        float tex_tile_size_x=256.0/1024.0;
        float tex_tile_size_y=256.0/1280.0;
        float tex_pos[2]={0,0};
        tex_pos[0]=0.0;

        float attack_prog=(1.0-(m_attack_cooldown/m_pWeapon_curr->cooldown))*5.0;
        int attack_frame=(int)attack_prog;
        if(attack_frame>4) attack_frame=4;

        tex_pos[1]=attack_frame*tex_tile_size_y;

        switch(m_dir)
        {
            case dir_up:    tex_pos[0]=tex_tile_size_x*1.0; break;
            case dir_right: tex_pos[0]=tex_tile_size_x*3.0; break;
            case dir_down:  tex_pos[0]=tex_tile_size_x*0.0; break;
            case dir_left:  tex_pos[0]=tex_tile_size_x*2.0; break;
        }

        //use color filter
        switch(index)
        {
            case 0: glColor3f(1.0,1.0,1.0); break;
            case 1: glColor3f(1.0,0.8,0.8); break;
            case 2: glColor3f(0.8,1.0,0.8); break;
            case 3: glColor3f(0.8,0.8,1.0); break;
        }

        //if hurt
        if(m_hurt_timer>0) glColor3f(1,0.3,0.3);

        glBegin(GL_QUADS);
        glTexCoord2f(tex_pos[0],tex_pos[1]);
        glVertex2f(-m_draw_size*2.0,-m_draw_size*2.0);
        glTexCoord2f(tex_pos[0],tex_pos[1]+tex_tile_size_y);
        glVertex2f(-m_draw_size*2.0,m_draw_size*2.0);
        glTexCoord2f(tex_pos[0]+tex_tile_size_x,tex_pos[1]+tex_tile_size_y);
        glVertex2f(m_draw_size*2.0,m_draw_size*2.0);
        glTexCoord2f(tex_pos[0]+tex_tile_size_x,tex_pos[1]);
        glVertex2f(m_draw_size*2.0,-m_draw_size*2.0);
        glEnd();
    }
    else if(m_fishing_state==fs_off)//draw normal, not fishing
    {
        //if hurt
        if(m_hurt_timer>0)
        {
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            float tex_tile_size=128.0/1024.0;
            float tex_pos[2]={0,0};
            tex_pos[0]=640.0/1024.0;
            if(m_hurt_anim_one) tex_pos[1]=0.0;
            else tex_pos[1]=tex_tile_size;

            glColor3f(1.0,0.3,0.3);
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

        }
        //not hurt
        else
        {
            glBindTexture(GL_TEXTURE_2D,m_texture);
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
            float tex_tile_size=128.0/1024.0;
            float tex_pos[2]={0,0};
            tex_pos[0]=0.0;
            tex_pos[1]=m_tex_walk_ind*tex_tile_size;
            //no walk if in a boat
            if(m_have_boat) tex_pos[1]=0;

            switch(m_dir)
            {
                case dir_up:    tex_pos[0]=tex_tile_size*1.0; break;
                case dir_right: tex_pos[0]=tex_tile_size*3.0; break;
                case dir_down:  tex_pos[0]=tex_tile_size*0.0; break;
                case dir_left:  tex_pos[0]=tex_tile_size*2.0; break;
            }

            //use color filter
            switch(index)
            {
                case 0: glColor3f(1.0,1.0,1.0); break;
                case 1: glColor3f(1.0,0.8,0.8); break;
                case 2: glColor3f(0.8,1.0,0.8); break;
                case 3: glColor3f(0.8,0.8,1.0); break;
            }

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

        }

    }
    //draw fishing stand
    else
    {
        glBindTexture(GL_TEXTURE_2D,m_texture_fishing);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        float tex_tile_size_x=256.0/1024.0;
        float tex_tile_size_y=256.0/256.0;
        float tex_pos[2]={0,0};
        tex_pos[0]=0.0;

        switch(m_dir)
        {
            case dir_up:    tex_pos[0]=tex_tile_size_x*1.0; break;
            case dir_right: tex_pos[0]=tex_tile_size_x*3.0; break;
            case dir_down:  tex_pos[0]=tex_tile_size_x*0.0; break;
            case dir_left:  tex_pos[0]=tex_tile_size_x*2.0; break;
        }

        //use color filter
        switch(index)
        {
            case 0: glColor3f(1.0,1.0,1.0); break;
            case 1: glColor3f(1.0,0.8,0.8); break;
            case 2: glColor3f(0.8,1.0,0.8); break;
            case 3: glColor3f(0.8,0.8,1.0); break;
        }

        //if hurt
        if(m_hurt_timer>0) glColor3f(1,0.3,0.3);

        glBegin(GL_QUADS);
        glTexCoord2f(tex_pos[0],tex_pos[1]);
        glVertex2f(-m_draw_size*2.0,-m_draw_size*2.0);
        glTexCoord2f(tex_pos[0],tex_pos[1]+tex_tile_size_y);
        glVertex2f(-m_draw_size*2.0,m_draw_size*2.0);
        glTexCoord2f(tex_pos[0]+tex_tile_size_x,tex_pos[1]+tex_tile_size_y);
        glVertex2f(m_draw_size*2.0,m_draw_size*2.0);
        glTexCoord2f(tex_pos[0]+tex_tile_size_x,tex_pos[1]);
        glVertex2f(m_draw_size*2.0,-m_draw_size*2.0);
        glEnd();
    }

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    glPopMatrix();

    //draw fishing line again for front
    if(m_fishing_state!=fs_off && m_dir==dir_down)
    {
        glLineWidth(1);
        glColor3f(0.9,0.9,0.9);
        glBegin(GL_LINES);
        glVertex2f(m_fishing_line_pos_start.x,m_fishing_line_pos_start.y);
        //glVertex2f(m_fishing_line_pos_end.x,m_fishing_line_pos_end.y);
        switch(m_dir)
        {
            case dir_up:    glVertex2f(m_pos.x+13,m_pos.y-100); break;
            case dir_right: glVertex2f(m_pos.x+121,m_pos.y-64); break;
            case dir_down:  glVertex2f(m_pos.x-4,m_pos.y-93); break;
            case dir_left:  glVertex2f(m_pos.x-122,m_pos.y-63); break;
        }

        glEnd();
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



    return true;
}

bool player::move_player(float x_shift,float y_shift)
{
    m_pos.x+=x_shift;
    m_pos.y+=y_shift;

    /*if(x_shift!=0 && y_shift!=0)
    {
        //walk frame
        m_tex_walk_timer-=_game_update_step*0.001;
        if(m_tex_walk_timer<=0.0)
        {
            m_tex_walk_timer=m_tex_walk_time;
            m_tex_walk_ind++;
            if(m_tex_walk_ind>7) m_tex_walk_ind=0;
        }
    }*/

    return true;
}

float player::get_player_size(void)
{
    return m_size;
}

pos2d player::get_player_pos(void)
{
    return m_pos;
}

bool player::set_player_pos(pos2d new_pos,bool forced)
{
    if(!forced && m_attack_cooldown>0.0)
    {
        //ignore movement
        return false;
    }

    //if moved
    if(m_pos!=new_pos)
    {
        //walk frame
        m_tex_walk_timer-=_game_update_step*0.001*_player_walk_speed*0.5;
        if(m_tex_walk_timer<=0.0)
        {
            m_tex_walk_timer=m_tex_walk_time;
            m_tex_walk_ind++;
            if(m_tex_walk_ind>7) m_tex_walk_ind=0;
            //play sound
            if((m_tex_walk_ind==0||m_tex_walk_ind==4)&&m_human_control&&!m_have_boat)
            {
                if(rand()%2==0) m_pSound->playSimpleSound(wav_footstep1,1);
                else m_pSound->playSimpleSound(wav_footstep2,1);

                //particles
                float pos[2]={m_pos.x,m_pos.y+44};
                float color[3]={0,1,0};
                m_pPart->add_explosion(color,pos,10,300,0.20);
            }

        }
    }
    else//reset walk frame
    {
        m_tex_walk_ind=0;
    }

    m_pos=new_pos;



    return true;
}

int player::get_player_dir(void)
{
    return m_dir;
}

bool player::set_player_dir(int new_dir)
{
    m_dir=new_dir;

    return true;
}

bool player::attack(void)
{
    if(m_attack_cooldown<=0 && m_pWeapon_curr->durability>0.0)
    {
        m_attack_cooldown=m_pWeapon_curr->cooldown;
        return true;
    }

    return false;
}

weapon* player::get_weapon_ptr(void)
{
    return m_pWeapon_curr;
}

bool player::set_weapon_ptr(weapon* new_weapon)
{
    m_pWeapon_curr=new_weapon;

    return true;
}

bool player::change_hp(float value)
{
    m_hp_curr+=value;

    if(m_hp_curr>m_hp_max) m_hp_curr=m_hp_max;

    //set hurt anim
    if(value<0)
    {
        m_hurt_timer=m_hurt_time;
        if(rand()%2==0) m_hurt_anim_one=true;
        else m_hurt_anim_one=false;
    }

    if(m_hp_curr<=0) return false;
    return true;
}

bool player::set_flee_target(pos2d target_pos)
{
    m_state=mode_flee;
    m_move_target=target_pos;
    m_follow_timer=10+10*float(rand()%2000-1000)/1000.0*0.5;
}

bool player::use_default_weapon(void)
{
    m_pWeapon_curr=m_pWeapon_default;

    return true;
}

