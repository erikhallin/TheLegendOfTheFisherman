#include "game.h"

game::game()
{
    //ctor
}

bool game::init(int screen_size[2],bool* pKeys,bool* pMouse_but,int* pMouse_pos)
{
    cout<<"Game: Initialization start\n";

    m_game_state=gs_menu;
    m_key_trigger_any_timer=1.5;

    m_screen_width=screen_size[0];
    m_screen_height=screen_size[1];
    m_pKeys=pKeys;
    m_pMouse_but=pMouse_but;
    m_pMouse_pos=pMouse_pos;
    m_time_this_cycle=m_time_last_cycle=(float)clock()/(float)CLOCKS_PER_SEC;
    srand(time(0));
    m_enemy_wave_timer=_enemy_wave_time;
    m_numof_enemies_to_spawn=1;
    m_pWeapon_enemy=new weapon(wt_enemy);
    m_cam_pos.y=_world_height*0.5*_tile_size;
    m_cam_pos.x=_world_width*0.5*_tile_size;

    m_pPart=new particle_engine();
    m_pPart->init();

    //load texture
    if(!load_textures())
    {
        return false;
    }

    //load sound
    if(!load_sounds())
    {
        return false;
    }

    //spawn world
    m_pWorld=new world();
    m_pWorld->init(m_tex_tile);

    //spawn players
    m_vec_player.push_back( player() );
    m_vec_player.back().init(m_tex_player,m_tex_tile,m_tex_fishing,m_tex_player_attack,m_pSound,m_pPart);
    m_vec_player.back().m_human_control=true;//for keyboard players
    //spawn bots
    m_vec_player.push_back( player() );
    m_vec_player.back().init(m_tex_player,m_tex_tile,m_tex_fishing,m_tex_player_attack,m_pSound,m_pPart);
    m_vec_player.push_back( player() );
    m_vec_player.back().init(m_tex_player,m_tex_tile,m_tex_fishing,m_tex_player_attack,m_pSound,m_pPart);
    m_vec_player.push_back( player() );
    m_vec_player.back().init(m_tex_player,m_tex_tile,m_tex_fishing,m_tex_player_attack,m_pSound,m_pPart);

    //spawn enemies
    m_vec_enemy.push_back( enemy() );
    m_vec_enemy.back().init(m_tex_enemy);
    m_vec_enemy.push_back( enemy() );
    m_vec_enemy.back().init(m_tex_enemy);
    m_vec_enemy.push_back( enemy() );
    m_vec_enemy.back().init(m_tex_enemy);

    /*//temp inventory
    m_vec_pWeapon[0].push_back( new weapon(wt_salmon) );
    m_vec_pWeapon[0].push_back( new weapon(wt_mackerel) );
    m_vec_pWeapon[0].push_back( new weapon(wt_pike) );
    m_vec_pWeapon[0].push_back( new weapon(wt_eel) );
    m_vec_pWeapon[0].push_back( new weapon(wt_salmon) );
    m_vec_pWeapon[0].push_back( new weapon(wt_salmon) );
    m_vec_pWeapon[0].push_back( new weapon(wt_mackerel) );
    m_vec_pWeapon[0].push_back( new weapon(wt_pike) );
    m_vec_pWeapon[0].push_back( new weapon(wt_salmon) );
    m_vec_pWeapon[1].push_back( new weapon(wt_salmon) );
    m_vec_pWeapon[1].push_back( new weapon(wt_mackerel) );
    m_vec_pWeapon[1].push_back( new weapon(wt_pike) );
    m_vec_pWeapon[1].push_back( new weapon(wt_salmon) );
    m_vec_pWeapon[2].push_back( new weapon(wt_salmon) );
    m_vec_pWeapon[2].push_back( new weapon(wt_mackerel) );
    m_vec_pWeapon[3].push_back( new weapon(wt_pike) );
    m_vec_pWeapon[3].push_back( new weapon(wt_salmon) );*/

    //init gamepads
    m_pGamepads=new gamepad[4];
    for(int gamepad_i=0;gamepad_i<4;gamepad_i++)
    {
        m_pGamepads[gamepad_i]=gamepad(gamepad_i);
        if( m_pGamepads[gamepad_i].IsConnected() )
         m_gamepad_connected[gamepad_i]=true;
        else
         m_gamepad_connected[gamepad_i]=false;
    }



    //start background noise
    m_pSound->playSimpleSound(wav_noise,1.0,20,true);

    return true;
}

bool game::update(void)
{
    //time
    m_time_last_cycle=m_time_this_cycle;
    m_time_this_cycle=(float)clock()/(float)CLOCKS_PER_SEC;

    m_pPart->update();

    //get gamepad data
    st_gamepad_data gamepad_data[4];
    for(int gamepad_i=0;gamepad_i<4;gamepad_i++)
    {
        if( m_pGamepads[gamepad_i].IsConnected() )
        {
            //new gamepad?
            if(!m_gamepad_connected[gamepad_i])
            {
                cout<<"New connection to controller: "<<gamepad_i+1<<endl;
            }

            m_gamepad_connected[gamepad_i]=true;
            gamepad_data[gamepad_i]=m_pGamepads[gamepad_i].GetState();

            //activate if AI controlled
            if(!m_vec_player[gamepad_i].m_human_control)
            {
                //test for any input
                if(gamepad_data[gamepad_i].button_A ||
                   gamepad_data[gamepad_i].button_B ||
                   gamepad_data[gamepad_i].button_X ||
                   gamepad_data[gamepad_i].button_Y ||
                   gamepad_data[gamepad_i].button_start ||
                   gamepad_data[gamepad_i].button_back ||
                   gamepad_data[gamepad_i].button_RB ||
                   gamepad_data[gamepad_i].button_LB)
                {
                    m_vec_player[gamepad_i].m_human_control=true;
                }
            }
        }
        else//lost controller
        {
            if( m_gamepad_connected[gamepad_i] )//had connection and lost it
            {
                m_gamepad_connected[gamepad_i]=false;
                cout<<"Lost connection to controller: "<<gamepad_i+1<<endl;

                //turn AI on
                m_vec_player[gamepad_i].m_human_control=false;
            }
        }
    }

    switch(m_game_state)
    {
        case gs_menu:
        {
            //gamepad input
            for(int gamepad_i=0;gamepad_i<4;gamepad_i++)
            {
                if(gamepad_data[gamepad_i].button_start||
                   gamepad_data[gamepad_i].button_A)
                {
                    m_key_trigger_any_timer=1.0;
                    m_game_state=gs_controls;
                    break;
                }
            }

            for(int i=0;i<256;i++)
            {
                if(m_pKeys[i])
                {
                    m_key_trigger_any_timer=1.0;
                    m_game_state=gs_controls;
                    break;
                }
            }
            //clear keys
            for(int i=0;i<256;i++) m_pKeys[i]=false;
        }break;

        case gs_controls:
        {
            if(m_key_trigger_any_timer<=0)
            {
                //gamepad input
                for(int gamepad_i=0;gamepad_i<4;gamepad_i++)
                {
                    if(gamepad_data[gamepad_i].button_start||
                       gamepad_data[gamepad_i].button_A)
                    {
                        m_game_state=gs_running;
                        break;
                    }
                }

                for(int i=0;i<256;i++)
                {
                    if(m_pKeys[i])
                    {
                        m_game_state=gs_running;
                        break;
                    }
                }
                //clear keys
                for(int i=0;i<256;i++) m_pKeys[i]=false;
            }
            else m_key_trigger_any_timer-=_game_update_step*0.001;

        }break;

        case gs_running:
        {
            //birdsound
            if(rand()%5000==0) m_pSound->playSimpleSound(wav_seagulls,1.0);

            //enemy spawn
            if(m_enemy_wave_timer>0.0) m_enemy_wave_timer-=_game_update_step*0.001;
            if(m_enemy_wave_timer<=0.0)
            {
                m_enemy_wave_timer=_enemy_wave_time+float(rand()%2000-1000)/1000.0*_enemy_wave_time*_enemy_wave_time_variation;

                int numof_enemies=m_numof_enemies_to_spawn+m_numof_enemies_to_spawn*float(rand()%2000-1000)/1000.0*_enemy_spawn_variation;
                m_numof_enemies_to_spawn+=_enemy_base_spawn_inc;

                //place enemies
                for(int new_enemy_i=0;new_enemy_i<numof_enemies;new_enemy_i++)
                {
                    m_vec_enemy.push_back( enemy() );
                    m_vec_enemy.back().init(m_tex_enemy);
                }
            }


            //manual move cam
            float move_sens=_player_walk_speed;
            /*if(m_pKeys[98])
            {
                m_cam_pos.y+=move_sens;
            }
            if(m_pKeys[100])
            {
                m_cam_pos.x-=move_sens;
            }
            if(m_pKeys[102])
            {
                m_cam_pos.x+=move_sens;
            }
            if(m_pKeys[104])
            {
                m_cam_pos.y-=move_sens;
            }*/

            //move players
            for(int player_i=0;player_i<(int)m_vec_player.size();player_i++)
            {
                if(m_vec_player[player_i].m_fishing_state!=fs_off) continue;//can not move while fishing
                if(m_vec_player[player_i].m_hp_curr<=0.0) continue;//dead

                pos2d player_pos_curr=m_vec_player[player_i].get_player_pos();
                pos2d player_pos_change;

                //keyboard input
                if(player_i==0)
                {
                    if(m_pKeys[87])//w
                    {
                        player_pos_change.y-=move_sens;
                    }
                    if(m_pKeys[65])//a
                    {
                        player_pos_change.x-=move_sens;
                    }
                    if(m_pKeys[83])//s
                    {
                        player_pos_change.y+=move_sens;
                    }
                    if(m_pKeys[68])//d
                    {
                        player_pos_change.x+=move_sens;
                    }

                    //limit if 2 dir
                    if(player_pos_change.x!=0 && player_pos_change.y!=0)
                    {
                        player_pos_change.x*=0.7;
                        player_pos_change.y*=0.7;
                    }

                    //set player direction (keyboard)
                    if(m_pKeys[68]) m_vec_player[player_i].set_player_dir(dir_right);
                    if(m_pKeys[65]) m_vec_player[player_i].set_player_dir(dir_left);
                    if(m_pKeys[87]) m_vec_player[player_i].set_player_dir(dir_up);
                    if(m_pKeys[83]) m_vec_player[player_i].set_player_dir(dir_down);
                }
                if(player_i==1)
                {
                    if(m_pKeys[38])//w
                    {
                        player_pos_change.y-=move_sens;
                    }
                    if(m_pKeys[37])//a
                    {
                        player_pos_change.x-=move_sens;
                    }
                    if(m_pKeys[40])//s
                    {
                        player_pos_change.y+=move_sens;
                    }
                    if(m_pKeys[39])//d
                    {
                        player_pos_change.x+=move_sens;
                    }

                    //limit if 2 dir
                    if(player_pos_change.x!=0 && player_pos_change.y!=0)
                    {
                        player_pos_change.x*=0.7;
                        player_pos_change.y*=0.7;
                    }

                    //turn off AI
                    if(player_pos_change.x!=0 || player_pos_change.y!=0)
                    {
                        m_vec_player[player_i].m_human_control=true;
                    }

                    //set player direction (keyboard)
                    if(m_pKeys[39]) m_vec_player[player_i].set_player_dir(dir_right);
                    if(m_pKeys[37]) m_vec_player[player_i].set_player_dir(dir_left);
                    if(m_pKeys[38]) m_vec_player[player_i].set_player_dir(dir_up);
                    if(m_pKeys[40]) m_vec_player[player_i].set_player_dir(dir_down);
                }

                //gamepad input (grab first active, ignore rest)
                if(m_gamepad_connected[player_i])
                {
                    //movement
                    float gamepad_move_sens=_player_walk_speed;
                    if(player_pos_change.x!=0 || player_pos_change.y!=0) gamepad_move_sens=0;//do not allow keyboard+gamepad
                    int thumbstick_trigger_limit=10000;

                    if(gamepad_data[player_i].thumbstick_left_x > thumbstick_trigger_limit ||
                       gamepad_data[player_i].thumbstick_left_x < -thumbstick_trigger_limit)
                    {
                        //move side
                        player_pos_change.x+=gamepad_move_sens*(float)gamepad_data[player_i].thumbstick_left_x/32767.0;
                        //change dir
                        if(gamepad_data[player_i].thumbstick_left_x>0) m_vec_player[player_i].set_player_dir(dir_right);
                        else m_vec_player[player_i].set_player_dir(dir_left);
                    }
                    if(gamepad_data[player_i].thumbstick_left_y > thumbstick_trigger_limit ||
                       gamepad_data[player_i].thumbstick_left_y < -thumbstick_trigger_limit)
                    {
                        //move vertical
                        player_pos_change.y-=gamepad_move_sens*(float)gamepad_data[player_i].thumbstick_left_y/32767.0;
                        //change dir
                        if(gamepad_data[player_i].thumbstick_left_y>0) m_vec_player[player_i].set_player_dir(dir_up);
                        else m_vec_player[player_i].set_player_dir(dir_down);
                    }

                    //dpad
                    if(gamepad_data[player_i].dpad_left)
                    {
                        //move left
                        player_pos_change.x-=gamepad_move_sens;
                        //change dir
                        m_vec_player[player_i].set_player_dir(dir_left);
                    }
                    if(gamepad_data[player_i].dpad_right)
                    {
                        //move right
                        player_pos_change.x+=gamepad_move_sens;
                        //change dir
                        m_vec_player[player_i].set_player_dir(dir_right);
                    }
                    if(gamepad_data[player_i].dpad_up)
                    {
                        //move up
                        player_pos_change.y-=gamepad_move_sens;
                        //change dir
                        m_vec_player[player_i].set_player_dir(dir_up);
                    }
                    if(gamepad_data[player_i].dpad_down)
                    {
                        //move down
                        player_pos_change.y+=gamepad_move_sens;
                        //change dir
                        m_vec_player[player_i].set_player_dir(dir_down);
                    }
                }
                else if(player_i!=0)//if not player 1 and not gamepad, stop here
                {
                    continue;
                }


                //test new pos against world
                int ret_val=m_pWorld->collision_test(player_pos_curr+player_pos_change);
                if(ret_val==-1) ;//stop to avoid min and max values
                if( (ret_val==tile_water&&!m_vec_player[player_i].m_have_boat_ready) || ret_val==tile_stone || ret_val==tile_tree)
                {
                    //try altering move vector
                    pos2d temp_pos_change;
                    temp_pos_change.x=player_pos_change.x;
                    ret_val=m_pWorld->collision_test(player_pos_curr+temp_pos_change);
                    if( (ret_val==tile_water&&!m_vec_player[player_i].m_have_boat_ready) || ret_val==tile_stone || ret_val==tile_tree)
                    {
                        //try other direction
                        temp_pos_change.x=0;
                        temp_pos_change.y=player_pos_change.y;
                        ret_val=m_pWorld->collision_test(player_pos_curr+temp_pos_change);
                        if( (ret_val==tile_water&&!m_vec_player[player_i].m_have_boat_ready) || ret_val==tile_stone || ret_val==tile_tree)
                        {
                            ;//stop movement
                        }
                        else player_pos_curr=player_pos_curr+temp_pos_change;
                    }
                    else player_pos_curr=player_pos_curr+temp_pos_change;
                }
                else player_pos_curr=player_pos_curr+player_pos_change;


                //set new pos
                m_vec_player[player_i].set_player_pos(player_pos_curr,false);


                /*if(m_pKeys[87]&&m_pKeys[68]) m_vec_player[player_i].set_player_dir(dir_up_right);
                if(m_pKeys[83]&&m_pKeys[68]) m_vec_player[player_i].set_player_dir(dir_down_right);
                if(m_pKeys[83]&&m_pKeys[65]) m_vec_player[player_i].set_player_dir(dir_down_left);
                if(m_pKeys[87]&&m_pKeys[65]) m_vec_player[player_i].set_player_dir(dir_up_left);
                //override directions with arrows
                if(m_pKeys[38]) m_vec_player[player_i].set_player_dir(dir_up);
                if(m_pKeys[39]) m_vec_player[player_i].set_player_dir(dir_right);
                if(m_pKeys[40]) m_vec_player[player_i].set_player_dir(dir_down);
                if(m_pKeys[37]) m_vec_player[player_i].set_player_dir(dir_left);
                if(m_pKeys[38]&&m_pKeys[39]) m_vec_player[player_i].set_player_dir(dir_up_right);
                if(m_pKeys[40]&&m_pKeys[39]) m_vec_player[player_i].set_player_dir(dir_down_right);
                if(m_pKeys[40]&&m_pKeys[37]) m_vec_player[player_i].set_player_dir(dir_down_left);
                if(m_pKeys[38]&&m_pKeys[37]) m_vec_player[player_i].set_player_dir(dir_up_left);*/

            }


            //player player/enemy/world col
            for(int player_i1=0;player_i1<(int)m_vec_player.size();player_i1++)
            {
                pos2d player_pos_prim=m_vec_player[player_i1].get_player_pos();
                float player_size_prim=m_vec_player[player_i1].get_player_size();

                //player col
                for(int player_i2=0;player_i2<(int)m_vec_player.size();player_i2++)
                {
                    if(player_i1==player_i2) continue;

                    pos2d player_pos_sec=m_vec_player[player_i2].get_player_pos();
                    float player_size_sec=m_vec_player[player_i2].get_player_size();

                    if( !( player_pos_sec.x-player_size_sec > player_pos_prim.x+player_size_prim ||
                           player_pos_sec.x+player_size_sec < player_pos_prim.x-player_size_prim ||
                           player_pos_sec.y+player_size_sec < player_pos_prim.y-player_size_prim ||
                           player_pos_sec.y-player_size_sec > player_pos_prim.y+player_size_prim) )
                    {
                        //cout<<"Player-Player collision\n";
                        //collision, move prim player
                        float force_move_sens=0.5;

                        //calc dir
                        if(player_pos_prim.x>player_pos_sec.x) m_vec_player[player_i1].move_player(force_move_sens,0.0);
                        if(player_pos_prim.x<player_pos_sec.x) m_vec_player[player_i1].move_player(-force_move_sens,0.0);
                        if(player_pos_prim.y>player_pos_sec.y) m_vec_player[player_i1].move_player(0.0,force_move_sens);
                        if(player_pos_prim.y<player_pos_sec.y) m_vec_player[player_i1].move_player(0.0,-force_move_sens);
                    }
                }

                //enemy col
                player_pos_prim=m_vec_player[player_i1].get_player_pos();
                for(int enemy_i=0;enemy_i<(int)m_vec_enemy.size();enemy_i++)
                {
                    pos2d enemy_pos=m_vec_enemy[enemy_i].get_enemy_pos();
                    float enemy_size=m_vec_enemy[enemy_i].get_enemy_size();

                    if( !( enemy_pos.x-enemy_size > player_pos_prim.x+player_size_prim ||
                           enemy_pos.x+enemy_size < player_pos_prim.x-player_size_prim ||
                           enemy_pos.y+enemy_size < player_pos_prim.y-player_size_prim ||
                           enemy_pos.y-enemy_size > player_pos_prim.y+player_size_prim) )
                    {
                        //cout<<"Player-Enemy collision\n";
                        //collision, move prim player
                        float force_move_sens=1.5;

                        //calc dir
                        if(player_pos_prim.x>enemy_pos.x) m_vec_player[player_i1].move_player(force_move_sens,0.0);
                        if(player_pos_prim.x<enemy_pos.x) m_vec_player[player_i1].move_player(-force_move_sens,0.0);
                        if(player_pos_prim.y>enemy_pos.y) m_vec_player[player_i1].move_player(0.0,force_move_sens);
                        if(player_pos_prim.y<enemy_pos.y) m_vec_player[player_i1].move_player(0.0,-force_move_sens);
                    }
                }

                //world col
                player_pos_prim=m_vec_player[player_i1].get_player_pos();
                int ret_val=m_pWorld->collision_test(player_pos_prim);
                if(ret_val==-1)
                {
                    //set value to within world
                    if(player_pos_prim.x<1) player_pos_prim.x=1;
                    if(player_pos_prim.x>=_world_width*_tile_size) player_pos_prim.x=_world_width*_tile_size-1;
                    if(player_pos_prim.y<1) player_pos_prim.y=1;
                    if(player_pos_prim.y>=_world_height*_tile_size) player_pos_prim.y=_world_height*_tile_size-1;
                    m_vec_player[player_i1].set_player_pos(player_pos_prim);
                }
                if( (ret_val==tile_water&&!m_vec_player[player_i1].m_have_boat_ready) || ret_val==tile_stone || ret_val==tile_tree )
                {
                    //need to find nearest free tile for push direction
                    int dist_to_side[4]={0,0,0,0};
                    dist_to_side[dir4_up]=player_pos_prim.y;
                    while(dist_to_side[dir4_up]>0) dist_to_side[dir4_up]-=_tile_size;
                    dist_to_side[dir4_down]=_tile_size-dist_to_side[dir4_up];
                    dist_to_side[dir4_left]=player_pos_prim.x;
                    while(dist_to_side[dir4_left]>0) dist_to_side[dir4_left]-=_tile_size;
                    dist_to_side[dir4_right]=_tile_size-dist_to_side[dir4_left];
                    int nearest_side=dir4_up;
                    int nearest_dist=dist_to_side[dir4_up];
                    for(int dir=dir4_right;dir<=dir_left;dir++)
                    {
                        if(dist_to_side[dir]<nearest_dist)
                        {
                            nearest_side=dir;
                            nearest_dist=dist_to_side[dir];
                        }
                    }
                    //is nearest tile free
                    pos2d test_pos=player_pos_prim;
                    switch(nearest_side)
                    {
                        case dir4_up:     test_pos.y-=_tile_size; break;
                        case dir4_right:  test_pos.x+=_tile_size; break;
                        case dir4_down:   test_pos.y+=_tile_size; break;
                        case dir4_left:   test_pos.x-=_tile_size; break;
                    }
                    ret_val=m_pWorld->collision_test(test_pos);
                    if( (ret_val==tile_water&&!m_vec_player[player_i1].m_have_boat_ready) && ret_val!=tile_stone && ret_val!=tile_tree )
                    {
                        float force_move_sens=1.0;
                        //move player towards this tile
                        switch(nearest_side)
                        {
                            case dir4_up:     m_vec_player[player_i1].move_player(0.0,-force_move_sens); break;
                            case dir4_right:  m_vec_player[player_i1].move_player(force_move_sens,0.0); break;
                            case dir4_down:   m_vec_player[player_i1].move_player(0.0,force_move_sens); break;
                            case dir4_left:   m_vec_player[player_i1].move_player(-force_move_sens,0.0); break;
                        }

                    }
                    else//that tile can not be stepped on, try second closest
                    {
                        int nearest_side_old=nearest_side;
                        nearest_dist=dist_to_side[dir4_up];
                        if(nearest_side_old==dir4_up) nearest_dist=dist_to_side[dir4_right];
                        for(int dir=dir4_right;dir<=dir_left;dir++)
                        {
                            if(dir==nearest_side_old) continue;

                            if(dist_to_side[dir]<nearest_dist)
                            {
                                nearest_side=dir;
                                nearest_dist=dist_to_side[dir];
                            }
                        }
                        //is nearest tile free
                        test_pos=player_pos_prim;
                        switch(nearest_side)
                        {
                            case dir4_up:     test_pos.y-=_tile_size; break;
                            case dir4_right:  test_pos.x+=_tile_size; break;
                            case dir4_down:   test_pos.y+=_tile_size; break;
                            case dir4_left:   test_pos.x-=_tile_size; break;
                        }
                        ret_val=m_pWorld->collision_test(test_pos);
                        if( (ret_val==tile_water&&!m_vec_player[player_i1].m_have_boat_ready) && ret_val!=tile_stone && ret_val!=tile_tree )
                        {
                            float force_move_sens=1.0;
                            //move player towards this tile
                            switch(nearest_side)
                            {
                                case dir4_up:     m_vec_player[player_i1].move_player(0.0,-force_move_sens); break;
                                case dir4_right:  m_vec_player[player_i1].move_player(force_move_sens,0.0); break;
                                case dir4_down:   m_vec_player[player_i1].move_player(0.0,force_move_sens); break;
                                case dir4_left:   m_vec_player[player_i1].move_player(-force_move_sens,0.0); break;
                            }

                        }
                        else//not near a free tile on that side either, find nearest free tile
                        {
                            //step around in circles to find free tile
                            pos2d tile_pos_curr=player_pos_prim;
                            int row_numof_steps_max=1;
                            int row_numof_steps_curr=1;
                            bool second_row_run=false;
                            int dir=dir4_up;
                            //start loop
                            //cout<<"loop start\n";
                            while(true)
                            {
                                //cout<<"Finding free tile: Curr pos: "<<tile_pos_curr.x<<", "<<tile_pos_curr.y<<"\t"<<ret_val<<endl;
                                //string asd;
                                //cin>>asd;
                                //move to next tile
                                switch(dir)
                                {
                                    case dir4_up:     tile_pos_curr+=pos2d(0.0,-_tile_size); break;
                                    case dir4_right:  tile_pos_curr+=pos2d(_tile_size,0.0); break;
                                    case dir4_down:   tile_pos_curr+=pos2d(0.0,_tile_size); break;
                                    case dir4_left:   tile_pos_curr+=pos2d(-_tile_size,0.0); break;
                                }
                                row_numof_steps_curr++;
                                if(row_numof_steps_curr>row_numof_steps_max)
                                {
                                    //new dir
                                    dir++;
                                    if(dir>dir4_left) dir=dir4_up;
                                    if(second_row_run)
                                    {
                                        second_row_run=false;
                                        row_numof_steps_max++;
                                        row_numof_steps_curr=1;
                                    }
                                    else
                                    {
                                        second_row_run=true;
                                        row_numof_steps_curr=1;
                                    }
                                }

                                //test new tile pos
                                ret_val=m_pWorld->collision_test(tile_pos_curr);
                                if( ret_val!=tile_water && ret_val!=tile_stone && ret_val!=tile_tree && ret_val!=-1 )
                                {
                                    //found free tile

                                    //shift pos
                                    pos2d move_dir=tile_pos_curr-player_pos_prim;
                                    float sens=0.1;
                                    m_vec_player[player_i1].move_player(move_dir.x*sens,move_dir.y*sens);


                                    //place player there
                                    //m_vec_player[player_i1].set_player_pos(tile_pos_curr);

                                    break;//leave the loop
                                }
                            }
                        }
                    }
                }
            }

            //player input
            for(int player_i=0;player_i<(int)m_vec_player.size();player_i++)
            {
                bool key_attack=false;
                bool key_interact=false;
                bool key_cancel=false;
                bool key_inv_left=false;
                bool key_inv_right=false;
                bool key_eat_fish=false;
                if(player_i==0)
                {
                    key_attack=m_pKeys[72];
                    key_interact=m_pKeys[74];
                    key_cancel=m_pKeys[76];
                    key_inv_left=m_pKeys[78];
                    key_inv_right=m_pKeys[77];
                    key_eat_fish=m_pKeys[75];
                }
                if(player_i==1)
                {
                    key_attack=m_pKeys[97];
                    key_interact=m_pKeys[98];
                    key_cancel=m_pKeys[110];
                    key_inv_left=m_pKeys[109];
                    key_inv_right=m_pKeys[107];
                    key_eat_fish=m_pKeys[99];
                }
                if(m_gamepad_connected[player_i])
                {
                    if(gamepad_data[player_i].button_X) key_attack=true;
                    if(gamepad_data[player_i].button_A) key_interact=true;
                    if(gamepad_data[player_i].button_B) key_cancel=true;
                    if(gamepad_data[player_i].button_Y) key_eat_fish=true;
                    if(gamepad_data[player_i].button_RB) key_inv_left=true;
                    if(gamepad_data[player_i].button_LB) key_inv_right=true;
                }

                if(m_vec_player[player_i].m_hp_curr<=0.0) continue;//dead

                //eat fish (heal)
                if(key_eat_fish)
                {
                    if(!m_vec_player[player_i].m_key_trigger_y)
                    {
                        m_vec_player[player_i].m_key_trigger_y=true;

                        //test if fish selected
                        if(m_vec_player[player_i].m_inventory_selection_ind!=-1)
                        {
                            //get hp
                            float amount_of_hp=0;
                            switch( m_vec_pWeapon[player_i][m_vec_player[player_i].m_inventory_selection_ind]->type )
                            {
                                case wt_none:       amount_of_hp=0; break;
                                case wt_mackerel:   amount_of_hp=20; break;
                                case wt_salmon:     amount_of_hp=40; break;
                                case wt_eel:        amount_of_hp=20; break;
                                case wt_pike:       amount_of_hp=10; break;
                            }

                            if(amount_of_hp!=0)
                            {
                                //fill hp
                                m_vec_player[player_i].change_hp(amount_of_hp);

                                //remove fish from inventory
                                delete m_vec_pWeapon[player_i][m_vec_player[player_i].m_inventory_selection_ind];
                                m_vec_pWeapon[player_i].erase(m_vec_pWeapon[player_i].begin()+m_vec_player[player_i].m_inventory_selection_ind);

                                //move selection marker
                                m_vec_player[player_i].m_inventory_selection_ind=-1;

                                //use default weapon for player
                                m_vec_player[player_i].use_default_weapon();

                                //play sound
                                m_pSound->playSimpleSound(wav_eat,1);
                            }
                        }
                    }
                }
                else m_vec_player[player_i].m_key_trigger_y=false;

                //inventory select
                if(key_inv_left)//q
                {
                    if(!m_vec_player[player_i].m_key_trigger_invent_left)
                    {
                        m_vec_player[player_i].m_key_trigger_invent_left=true;

                        m_vec_player[player_i].m_inventory_selection_ind++;
                        if(m_vec_player[player_i].m_inventory_selection_ind>=(int)m_vec_pWeapon[player_i].size())
                         m_vec_player[player_i].m_inventory_selection_ind=0;

                        //make weapon active
                        if(!m_vec_pWeapon[player_i].empty())
                         m_vec_player[player_i].set_weapon_ptr( m_vec_pWeapon[player_i][m_vec_player[player_i].m_inventory_selection_ind] );
                        else m_vec_player[player_i].m_inventory_selection_ind=-1;
                    }
                }
                else m_vec_player[player_i].m_key_trigger_invent_left=false;

                if(key_inv_right)//e
                {
                    if(!m_vec_player[player_i].m_key_trigger_invent_right)
                    {
                        m_vec_player[player_i].m_key_trigger_invent_right=true;

                        m_vec_player[player_i].m_inventory_selection_ind--;
                        if(m_vec_player[player_i].m_inventory_selection_ind<0)
                         m_vec_player[player_i].m_inventory_selection_ind=(int)m_vec_pWeapon[player_i].size()-1;

                        //make weapon active
                        if(!m_vec_pWeapon[player_i].empty())
                         m_vec_player[player_i].set_weapon_ptr( m_vec_pWeapon[player_i][m_vec_player[player_i].m_inventory_selection_ind] );
                        else m_vec_player[player_i].m_inventory_selection_ind=-1;
                    }
                }
                else m_vec_player[player_i].m_key_trigger_invent_right=false;

                //attack
                if(key_attack)//v
                {
                    if(!m_vec_player[player_i].m_key_trigger_attack)
                    if( m_vec_player[player_i].attack() && m_vec_player[player_i].m_fishing_state==fs_off )
                    {
                        pos2d player_pos=m_vec_player[player_i].get_player_pos();

                        //calc attack area
                        weapon* pWeapon=m_vec_player[player_i].get_weapon_ptr();
                        int attack_dir=m_vec_player[player_i].get_player_dir();
                        if(attack_dir==dir_up_left)
                        {
                            int coin=rand()%2;
                            if(coin==0) attack_dir=dir_up;
                            else attack_dir=dir_left;
                        }
                        if(attack_dir==dir_down_left)
                        {
                            int coin=rand()%2;
                            if(coin==0) attack_dir=dir_down;
                            else attack_dir=dir_left;
                        }
                        if(attack_dir==dir_up_right)
                        {
                            int coin=rand()%2;
                            if(coin==0) attack_dir=dir_up;
                            else attack_dir=dir_right;
                        }
                        if(attack_dir==dir_down_right)
                        {
                            int coin=rand()%2;
                            if(coin==0) attack_dir=dir_down;
                            else attack_dir=dir_right;
                        }
                        if(attack_dir!=dir_up&&attack_dir!=dir_right&&attack_dir!=dir_down&&attack_dir!=dir_left) attack_dir=dir_up;

                        pos2d attack_pos=player_pos;
                        switch(attack_dir)
                        {
                            case dir_up:    attack_pos.y-=pWeapon->range_dist; break;
                            case dir_right: attack_pos.x+=pWeapon->range_dist; break;
                            case dir_down:  attack_pos.y+=pWeapon->range_dist; break;
                            case dir_left:  attack_pos.x-=pWeapon->range_dist; break;
                        }
                        pos2d attack_size(pWeapon->range_width,pWeapon->range_height);
                        if(attack_dir==dir_right||attack_dir==dir_left) attack_size=pos2d(pWeapon->range_height,pWeapon->range_width);

                        //store for drawing
                        //m_vec_damage_area.push_back( damage_area(type_player,pWeapon,attack_pos,attack_size) );

                        //particles
                        float pos[2]={attack_pos.x,attack_pos.y};
                        float color[3]={1,1,1};
                        m_pPart->add_explosion(color,pos,10,1000,0.2);


                        //test if enemies nearby
                        bool a_hit=false;
                        for(int enemy_i=0;enemy_i<(int)m_vec_enemy.size();enemy_i++)
                        {
                            pos2d enemy_pos=m_vec_enemy[enemy_i].get_enemy_pos();
                            float enemy_size=m_vec_enemy[enemy_i].get_enemy_size();

                            if( !( enemy_pos.x-enemy_size > attack_pos.x+attack_size.x ||
                                   enemy_pos.x+enemy_size < attack_pos.x-attack_size.x ||
                                   enemy_pos.y+enemy_size < attack_pos.y-attack_size.y ||
                                   enemy_pos.y-enemy_size > attack_pos.y+attack_size.y) )
                            {

                                //deal damage
                                if( !m_vec_enemy[enemy_i].change_hp(-pWeapon->damage_count) )
                                {
                                    //remove enemy
                                    m_vec_enemy.erase(m_vec_enemy.begin()+enemy_i);
                                    enemy_i--;

                                    //particles
                                    float pos[2]={enemy_pos.x,enemy_pos.y};
                                    float color[3]={0.3,0.3,0.3};
                                    m_pPart->add_explosion(color,pos,100,1000,0.4);
                                }

                                //play sound
                                if(!a_hit) m_pSound->playSimpleSound(wav_smack1,1);

                                //particles
                                float pos[2]={attack_pos.x,attack_pos.y};
                                float color[3]={1,0,0};
                                m_pPart->add_explosion(color,pos,20,1000,0.2);

                                //weapon dur
                                pWeapon->durability-=2;
                                if(pWeapon->durability<=0)
                                {
                                    //find in inventory
                                    int inv_ind=-1;
                                    for(int inv_i=0;inv_i<(int)m_vec_pWeapon[player_i].size();inv_i++)
                                    {
                                        if(pWeapon==m_vec_pWeapon[player_i][inv_i])
                                        {
                                            inv_ind=inv_i;
                                            break;
                                        }
                                    }
                                    if(inv_ind!=-1)
                                    {
                                        //remove fish from inventory
                                        delete m_vec_pWeapon[player_i][inv_ind];
                                        m_vec_pWeapon[player_i].erase(m_vec_pWeapon[player_i].begin()+inv_ind);

                                        //move selection marker
                                        m_vec_player[player_i].m_inventory_selection_ind=-1;

                                        //use default weapon for player
                                        m_vec_player[player_i].use_default_weapon();

                                        //playe sound
                                        m_pSound->playSimpleSound(wav_damaged,1);
                                    }
                                }

                                a_hit=true;
                            }
                        }

                        //miss? play sound
                        if(!a_hit) m_pSound->playSimpleSound(wav_miss,1);
                    }

                    m_vec_player[player_i].m_key_trigger_attack=true;//no extra attack until key release
                }
                else m_vec_player[player_i].m_key_trigger_attack=false;//reset attack

                //abort (fishing)
                if(key_cancel)//n
                {
                    //stop fishing
                    if(!m_vec_player[player_i].m_key_trigger_abort &&
                       (m_vec_player[player_i].m_fishing_state==fs_waiting||m_vec_player[player_i].m_fishing_state==fs_landing) )
                    {
                        m_vec_player[player_i].m_fishing_state=fs_off;
                        m_vec_player[player_i].m_key_trigger_abort=true;

                        //disable fish area in use
                        int fish_ind=-1;
                        for(int fish_i=0;fish_i<(int)m_pWorld->m_vec_fish_area.size();fish_i++)
                        {
                            if( !( m_pWorld->m_vec_fish_area[fish_i].tile_pos[0]*_tile_size >            m_vec_player[player_i].m_fishing_line_pos_start.x ||
                                   m_pWorld->m_vec_fish_area[fish_i].tile_pos[0]*_tile_size+_tile_size < m_vec_player[player_i].m_fishing_line_pos_start.x ||
                                   m_pWorld->m_vec_fish_area[fish_i].tile_pos[1]*_tile_size+_tile_size < m_vec_player[player_i].m_fishing_line_pos_start.y ||
                                   m_pWorld->m_vec_fish_area[fish_i].tile_pos[1]*_tile_size >            m_vec_player[player_i].m_fishing_line_pos_start.y ) )
                            {
                                fish_ind=fish_i;
                                break;
                            }
                        }
                        if(fish_ind!=-1)
                        {
                            //set to not in use
                            m_pWorld->m_vec_fish_area[fish_ind].in_use=false;
                        }
                    }

                }
                else m_vec_player[player_i].m_key_trigger_abort=false;

                //interact (fish/sell fish)
                if(key_interact)//b
                {
                    //land fish
                    if(!m_vec_player[player_i].m_key_trigger_fish && m_vec_player[player_i].m_fishing_state==fs_landing)
                    {
                        m_vec_player[player_i].m_key_trigger_fish=true;

                        m_vec_player[player_i].m_fish_landing_progress+=m_vec_player[player_i].m_fishing_rod_force;

                        if(m_vec_player[player_i].m_fish_landing_progress>=1.0)
                        {
                            //cout<<"Fish landed\n";
                            //fish landed
                            m_vec_player[player_i].m_fishing_state=fs_off;

                            //put fish in inventory
                            if(m_vec_player[player_i].m_fish_on_hook!=wt_none)
                             m_vec_pWeapon[player_i].push_back( new weapon(m_vec_player[player_i].m_fish_on_hook) );

                            //play sound
                            if(m_vec_player[player_i].m_fish_on_hook!=wt_none)
                             m_pSound->playSimpleSound(wav_catchfish,0.5);

                            //disable fish area in use
                            int fish_ind=-1;
                            for(int fish_i=0;fish_i<(int)m_pWorld->m_vec_fish_area.size();fish_i++)
                            {
                                if( !( m_pWorld->m_vec_fish_area[fish_i].tile_pos[0]*_tile_size >            m_vec_player[player_i].m_fishing_line_pos_start.x ||
                                       m_pWorld->m_vec_fish_area[fish_i].tile_pos[0]*_tile_size+_tile_size < m_vec_player[player_i].m_fishing_line_pos_start.x ||
                                       m_pWorld->m_vec_fish_area[fish_i].tile_pos[1]*_tile_size+_tile_size < m_vec_player[player_i].m_fishing_line_pos_start.y ||
                                       m_pWorld->m_vec_fish_area[fish_i].tile_pos[1]*_tile_size >            m_vec_player[player_i].m_fishing_line_pos_start.y ) )
                                {
                                    fish_ind=fish_i;
                                    break;
                                }
                            }
                            if(fish_ind!=-1)
                            {
                                //set to not in use
                                m_pWorld->m_vec_fish_area[fish_ind].in_use=false;
                                m_pWorld->m_vec_fish_area.erase(m_pWorld->m_vec_fish_area.begin()+fish_ind);
                            }
                        }
                        else//play sound
                         m_pSound->playSimpleSound(wav_hit,0.7);
                    }

                    //sell/buy fish
                    if(!m_vec_player[player_i].m_key_trigger_fish && m_vec_player[player_i].m_fishing_state==fs_off)
                    {
                        //test if in sale area
                        pos2d player_pos=m_vec_player[player_i].get_player_pos();
                        if( !( m_pWorld->m_sale_area_pos.x-m_pWorld->m_sale_area_size > player_pos.x ||
                               m_pWorld->m_sale_area_pos.x+m_pWorld->m_sale_area_size < player_pos.x ||
                               m_pWorld->m_sale_area_pos.y+m_pWorld->m_sale_area_size < player_pos.y ||
                               m_pWorld->m_sale_area_pos.y-m_pWorld->m_sale_area_size > player_pos.y ) )
                        {
                            m_vec_player[player_i].m_key_trigger_fish=true;

                            //test if anything selected in inventory
                            if(m_vec_player[player_i].m_inventory_selection_ind!=-1)
                            {
                                //get value
                                float value=0;
                                switch( m_vec_pWeapon[player_i][m_vec_player[player_i].m_inventory_selection_ind]->type )
                                {
                                    case wt_none:       value=0; break;
                                    case wt_mackerel:   value=2; break;
                                    case wt_salmon:     value=2; break;
                                    case wt_eel:        value=3; break;
                                    case wt_pike:       value=1; break;
                                }

                                if(value!=0)
                                {
                                    //fill wallet
                                    m_vec_player[player_i].m_coins_curr+=value;

                                    //remove fish from inventory
                                    delete m_vec_pWeapon[player_i][m_vec_player[player_i].m_inventory_selection_ind];
                                    m_vec_pWeapon[player_i].erase(m_vec_pWeapon[player_i].begin()+m_vec_player[player_i].m_inventory_selection_ind);

                                    //move selection marker
                                    m_vec_player[player_i].m_inventory_selection_ind=-1;

                                    //use default weapon for player
                                    m_vec_player[player_i].use_default_weapon();

                                    //play sound
                                    m_pSound->playSimpleSound(wav_sell,1.0);
                                }
                            }

                        }
                    }

                    //buy upgrade
                    if(!m_vec_player[player_i].m_key_trigger_fish && m_vec_player[player_i].m_fishing_state==fs_off)
                    {
                        //test if in sale area
                        pos2d player_pos=m_vec_player[player_i].get_player_pos();
                        if( !( m_pWorld->m_buy_area_pos.x-m_pWorld->m_sale_area_size > player_pos.x ||
                               m_pWorld->m_buy_area_pos.x+m_pWorld->m_sale_area_size < player_pos.x ||
                               m_pWorld->m_buy_area_pos.y+m_pWorld->m_sale_area_size < player_pos.y ||
                               m_pWorld->m_buy_area_pos.y-m_pWorld->m_sale_area_size > player_pos.y ) )
                        {
                            m_vec_player[player_i].m_key_trigger_fish=true;

                            //test if have any coins
                            if(m_vec_player[player_i].m_coins_curr>0)
                            {
                                //buy boat
                                if(m_vec_player[player_i].m_coins_curr>=10 && !m_vec_player[player_i].m_have_boat_ready)
                                {
                                    m_vec_player[player_i].m_have_boat_ready=true;
                                    m_vec_player[player_i].m_coins_curr-=10;
                                    //play sound
                                    m_pSound->playSimpleSound(wav_buyboat,1.0);
                                }
                                else//buy upgrade
                                {
                                    m_vec_player[player_i].m_coins_curr--;
                                    //sell one coin, improve rod
                                    m_vec_player[player_i].m_fishing_rod_force_multiplyer+=0.05;
                                    //play sound
                                    m_pSound->playSimpleSound(wav_buyupgrade,1.0);
                                }
                            }

                        }
                    }

                    //start fishing
                    if(!m_vec_player[player_i].m_key_trigger_fish && m_vec_player[player_i].m_fishing_state==fs_off)
                    {
                        m_vec_player[player_i].m_key_trigger_fish=true;

                        pos2d player_pos=m_vec_player[player_i].get_player_pos();
                        float player_size=m_vec_player[player_i].get_player_size();

                        //calc attack area
                        int attack_dir=m_vec_player[player_i].get_player_dir();
                        if(attack_dir==dir_up_left)
                        {
                            int coin=rand()%2;
                            if(coin==0) attack_dir=dir_up;
                            else attack_dir=dir_left;
                        }
                        if(attack_dir==dir_down_left)
                        {
                            int coin=rand()%2;
                            if(coin==0) attack_dir=dir_down;
                            else attack_dir=dir_left;
                        }
                        if(attack_dir==dir_up_right)
                        {
                            int coin=rand()%2;
                            if(coin==0) attack_dir=dir_up;
                            else attack_dir=dir_right;
                        }
                        if(attack_dir==dir_down_right)
                        {
                            int coin=rand()%2;
                            if(coin==0) attack_dir=dir_down;
                            else attack_dir=dir_right;
                        }
                        if(attack_dir!=dir_up&&attack_dir!=dir_right&&attack_dir!=dir_down&&attack_dir!=dir_left) attack_dir=dir_up;

                        pos2d attack_pos=player_pos;
                        switch(attack_dir)
                        {
                            case dir_up:    attack_pos.y-=player_size*2; break;
                            case dir_right: attack_pos.x+=player_size*2; break;
                            case dir_down:  attack_pos.y+=player_size*2; break;
                            case dir_left:  attack_pos.x-=player_size*2; break;
                        }

                        //any fish tile there
                        int fish_ind=-1;
                        for(int fish_i=0;fish_i<(int)m_pWorld->m_vec_fish_area.size();fish_i++)
                        {
                            if( !( m_pWorld->m_vec_fish_area[fish_i].tile_pos[0]*_tile_size > attack_pos.x ||
                                   m_pWorld->m_vec_fish_area[fish_i].tile_pos[0]*_tile_size+_tile_size < attack_pos.x ||
                                   m_pWorld->m_vec_fish_area[fish_i].tile_pos[1]*_tile_size+_tile_size < attack_pos.y ||
                                   m_pWorld->m_vec_fish_area[fish_i].tile_pos[1]*_tile_size > attack_pos.y ) )
                            {
                                fish_ind=fish_i;
                                break;
                            }
                        }
                        if(fish_ind!=-1)
                        {
                            //test if in use
                            if(!m_pWorld->m_vec_fish_area[fish_ind].in_use)
                            {
                                //pick fish type
                                int rand_val=rand()%100;
                                if(rand_val>70)
                                {
                                    //wt_none
                                    m_vec_player[player_i].m_fish_on_hook=wt_none;
                                    m_vec_player[player_i].m_fishing_rod_force=0.2*m_vec_player[player_i].m_fishing_rod_force_multiplyer;
                                }
                                else if(rand_val>40)
                                {
                                    //wt_mackerel
                                    m_vec_player[player_i].m_fish_on_hook=wt_mackerel;
                                    m_vec_player[player_i].m_fishing_rod_force=0.1*m_vec_player[player_i].m_fishing_rod_force_multiplyer;
                                }
                                else if(rand_val>30)
                                {
                                    //wt_eel
                                    m_vec_player[player_i].m_fish_on_hook=wt_eel;
                                    m_vec_player[player_i].m_fishing_rod_force=0.05*m_vec_player[player_i].m_fishing_rod_force_multiplyer;
                                }
                                else if(rand_val>20)
                                {
                                    //wt_salmon
                                    m_vec_player[player_i].m_fish_on_hook=wt_salmon;
                                    m_vec_player[player_i].m_fishing_rod_force=0.05*m_vec_player[player_i].m_fishing_rod_force_multiplyer;
                                }
                                else if(rand_val>10)
                                {
                                    //wt_pike
                                    m_vec_player[player_i].m_fish_on_hook=wt_pike;
                                    m_vec_player[player_i].m_fishing_rod_force=0.02*m_vec_player[player_i].m_fishing_rod_force_multiplyer;
                                }

                                //start fishing
                                m_vec_player[player_i].m_fishing_line_pos_start.x=m_pWorld->m_vec_fish_area[fish_ind].tile_pos[0]*_tile_size+_tile_size*0.5;
                                m_vec_player[player_i].m_fishing_line_pos_start.y=m_pWorld->m_vec_fish_area[fish_ind].tile_pos[1]*_tile_size+_tile_size*0.5;
                                m_vec_player[player_i].m_fishing_line_pos_end=player_pos;

                                m_vec_player[player_i].m_fishing_state=fs_waiting;
                                m_pWorld->m_vec_fish_area[fish_ind].in_use=true;
                            }
                        }
                    }
                }
                else m_vec_player[player_i].m_key_trigger_fish=false;//reset attack
            }

            //enemy-enemy push
            for(int enemy_i1=0;enemy_i1<<(int)m_vec_enemy.size();enemy_i1++)
            {
                pos2d enemy_pos_prim=m_vec_enemy[enemy_i1].get_enemy_pos();
                float enemy_size_prim=m_vec_enemy[enemy_i1].get_enemy_size();

                //enemy col
                for(int enemy_i2=0;enemy_i2<(int)m_vec_enemy.size();enemy_i2++)
                {
                    if(enemy_i1==enemy_i2) continue;

                    pos2d enemy_pos_sec=m_vec_enemy[enemy_i2].get_enemy_pos();
                    float enemy_size_sec=m_vec_enemy[enemy_i2].get_enemy_size();

                    if( !( enemy_pos_sec.x-enemy_size_sec > enemy_pos_prim.x+enemy_size_prim ||
                           enemy_pos_sec.x+enemy_size_sec < enemy_pos_prim.x-enemy_size_prim ||
                           enemy_pos_sec.y+enemy_size_sec < enemy_pos_prim.y-enemy_size_prim ||
                           enemy_pos_sec.y-enemy_size_sec > enemy_pos_prim.y+enemy_size_prim) )
                    {
                        //collision, move prim enemy
                        float force_move_sens=1.0;

                        //calc dir
                        if(enemy_pos_prim.x>enemy_pos_sec.x) m_vec_enemy[enemy_i1].move_enemy(force_move_sens,0.0);
                        if(enemy_pos_prim.x<enemy_pos_sec.x) m_vec_enemy[enemy_i1].move_enemy(-force_move_sens,0.0);
                        if(enemy_pos_prim.y>enemy_pos_sec.y) m_vec_enemy[enemy_i1].move_enemy(0.0,force_move_sens);
                        if(enemy_pos_prim.y<enemy_pos_sec.y) m_vec_enemy[enemy_i1].move_enemy(0.0,-force_move_sens);
                    }
                }
            }

            //update world
            m_pWorld->update();

            //update players
            for(int player_i=0;player_i<(int)m_vec_player.size();player_i++)
            {
                pos2d player_pos=m_vec_player[player_i].get_player_pos();
                float player_size=m_vec_player[player_i].get_player_size();
                //test if seen by any enemies
                for(int enemy_i=0;enemy_i<(int)m_vec_enemy.size();enemy_i++)
                {
                    pos2d enemy_pos=m_vec_enemy[enemy_i].get_enemy_pos();
                    float enemy_size=m_vec_enemy[enemy_i].get_enemy_size();

                    if( !( enemy_pos.x-enemy_size > player_pos.x+_enemy_detection_dist ||
                           enemy_pos.x+enemy_size < player_pos.x-_enemy_detection_dist ||
                           enemy_pos.y+enemy_size < player_pos.y-_enemy_detection_dist ||
                           enemy_pos.y-enemy_size > player_pos.y+_enemy_detection_dist) )
                    {
                        //alert that enemy
                        m_vec_enemy[enemy_i].set_attack_target(player_pos);

                        //attack test
                        if(m_vec_enemy[enemy_i].m_attack_cooldown<=0.0)
                        {
                            if( !( enemy_pos.x-enemy_size > player_pos.x+_enemy_attack_range ||
                                   enemy_pos.x+enemy_size < player_pos.x-_enemy_attack_range ||
                                   enemy_pos.y+enemy_size < player_pos.y-_enemy_attack_range ||
                                   enemy_pos.y-enemy_size > player_pos.y+_enemy_attack_range) )
                            {
                                m_vec_enemy[enemy_i].m_attack_cooldown=_enemy_attack_cooldown;
                                //damage player
                                m_vec_player[player_i].change_hp(-_enemy_attack_count);
                                //store for drawing
                                //m_vec_damage_area.push_back( damage_area(type_enemy,m_pWeapon_enemy,player_pos,pos2d(player_size,player_size)) );
                                //play sound
                                m_pSound->playSimpleSound(wav_damaged,1);

                                //particles
                                float pos[2]={player_pos.x,player_pos.y};
                                float color[3]={1,0,0};
                                m_pPart->add_explosion(color,pos,20,1000,0.2);
                            }
                        }

                        //test if player AI should flee
                        if(!m_vec_player[player_i].m_human_control)
                        if( !( enemy_pos.x-enemy_size > player_pos.x+_player_detection_dist ||
                               enemy_pos.x+enemy_size < player_pos.x-_player_detection_dist ||
                               enemy_pos.y+enemy_size < player_pos.y-_player_detection_dist ||
                               enemy_pos.y-enemy_size > player_pos.y+_player_detection_dist) )
                        {
                            //alert that player
                            m_vec_player[player_i].set_flee_target(enemy_pos);
                        }
                    }
                }

                int ret_val=m_vec_player[player_i].update();
                switch(ret_val)
                {
                    case 0: break;//nothing

                    case 1: //death
                    {
                        //XXXXXXXX
                    }break;

                    case 2: //lost fish
                    {
                        //disable fish area in use
                        int fish_ind=-1;
                        for(int fish_i=0;fish_i<(int)m_pWorld->m_vec_fish_area.size();fish_i++)
                        {
                            if( !( m_pWorld->m_vec_fish_area[fish_i].tile_pos[0]*_tile_size >            m_vec_player[player_i].m_fishing_line_pos_start.x ||
                                   m_pWorld->m_vec_fish_area[fish_i].tile_pos[0]*_tile_size+_tile_size < m_vec_player[player_i].m_fishing_line_pos_start.x ||
                                   m_pWorld->m_vec_fish_area[fish_i].tile_pos[1]*_tile_size+_tile_size < m_vec_player[player_i].m_fishing_line_pos_start.y ||
                                   m_pWorld->m_vec_fish_area[fish_i].tile_pos[1]*_tile_size >            m_vec_player[player_i].m_fishing_line_pos_start.y ) )
                            {
                                fish_ind=fish_i;
                                break;
                            }
                        }
                        if(fish_ind!=-1)
                        {
                            //set to not in use
                            m_pWorld->m_vec_fish_area[fish_ind].in_use=false;
                        }
                    }break;
                }

                //if in water and have boat ready, show boat
                if( m_pWorld->collision_test(player_pos)==tile_water )
                {
                    if(m_vec_player[player_i].m_have_boat_ready)
                    {
                        m_vec_player[player_i].m_have_boat=true;
                    }
                }
                //if not, disable boat
                else if(m_vec_player[player_i].m_have_boat)
                {
                    m_vec_player[player_i].m_have_boat=false;
                }
            }

            //update enemies
            for(int enemy_i=0;enemy_i<(int)m_vec_enemy.size();enemy_i++)
            {
                m_vec_enemy[enemy_i].update();
            }

            //update water anim
            for(int area_i=0;area_i<(int)m_pWorld->m_vec_fish_area.size();area_i++)
            {
                m_pWorld->m_vec_fish_area[area_i].frame_timer-=_game_update_step*0.003;
                if(m_pWorld->m_vec_fish_area[area_i].frame_timer<0)
                {
                    m_pWorld->m_vec_fish_area[area_i].frame_timer=_water_speed;
                    m_pWorld->m_vec_fish_area[area_i].frame_ind++;
                    if(m_pWorld->m_vec_fish_area[area_i].frame_ind>8) m_pWorld->m_vec_fish_area[area_i].frame_ind=0;
                }
            }

            //update cam pos
            pos2d player_pos_sum;
            int player_count=0;
            for(int player_i=0;player_i<(int)m_vec_player.size();player_i++)
            {
                if(m_vec_player[player_i].m_human_control || player_i==0)
                {
                    if(m_vec_player[player_i].m_hp_curr<=0) continue;

                    player_count++;
                    pos2d temp_pos=m_vec_player[player_i].get_player_pos();
                    player_pos_sum.x+=temp_pos.x;
                    player_pos_sum.y+=temp_pos.y;
                }
            }
            player_pos_sum.x/=(float)player_count;
            player_pos_sum.y/=(float)player_count;
            //only one player TEMP
            //player_pos_sum=m_vec_player[0].get_player_pos();

            float box_dead_zone=80;
            float box_dead_zone_max=500;
            float box_dead_zone_ymax=300;
            float cam_move_sens=1.5;
            if(m_cam_pos.x<player_pos_sum.x-box_dead_zone)
            {
                m_cam_pos.x+=cam_move_sens;
            }
            if(m_cam_pos.x>player_pos_sum.x+box_dead_zone)
            {
                m_cam_pos.x-=cam_move_sens;
            }
            if(m_cam_pos.y<player_pos_sum.y-box_dead_zone)
            {
                m_cam_pos.y+=cam_move_sens*1;
            }
            if(m_cam_pos.y>player_pos_sum.y+box_dead_zone)
            {
                m_cam_pos.y-=cam_move_sens*1;
            }
            //layer2
            if(m_cam_pos.x<player_pos_sum.x-box_dead_zone_max)
            {
                m_cam_pos.x+=cam_move_sens*2;
            }
            if(m_cam_pos.x>player_pos_sum.x+box_dead_zone_max)
            {
                m_cam_pos.x-=cam_move_sens*2;
            }
            if(m_cam_pos.y<player_pos_sum.y-box_dead_zone_ymax)
            {
                m_cam_pos.y+=cam_move_sens*2;
            }
            if(m_cam_pos.y>player_pos_sum.y+box_dead_zone_ymax)
            {
                m_cam_pos.y-=cam_move_sens*2;
            }

            //cap cam pos
            if(m_cam_pos.x<m_screen_width*0.5) m_cam_pos.x=m_screen_width*0.5;
            if(m_cam_pos.x>_world_width*_tile_size-m_screen_width*0.5) m_cam_pos.x=_world_width*_tile_size-m_screen_width*0.5;
            if(m_cam_pos.y<m_screen_height*0.5) m_cam_pos.y=m_screen_height*0.5;
            if(m_cam_pos.y>_world_height*_tile_size-m_screen_height*0.5) m_cam_pos.y=_world_height*_tile_size-m_screen_height*0.5;


            //update damage areas
            for(int damage_i=0;damage_i<(int)m_vec_damage_area.size();damage_i++)
            {
                m_vec_damage_area[damage_i].life_time-=_game_update_step*0.001;
                if(m_vec_damage_area[damage_i].life_time<=0.0)
                {
                    m_vec_damage_area.erase(m_vec_damage_area.begin()+damage_i);
                    damage_i--;
                }
            }

            //gameover test, if all are dead
            bool any_alive=false;
            for(int player_i=0;player_i<m_vec_player.size();player_i++)
            {
                if(m_vec_player[player_i].m_hp_curr>0 && m_vec_player[player_i].m_human_control)
                {
                    any_alive=true;
                    break;
                }
            }
            if(!any_alive)
            {
                m_game_state=gs_gameover;
            }



        }break;

        case gs_gameover:
        {
            //nothing
        }break;
    }




    return true;
}

bool game::draw(void)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    switch(m_game_state)
    {
        case gs_menu:
        {
            //menu texture draw
            glColor3f(1,1,1);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_tex_menu);
            glBegin(GL_QUADS);
            glTexCoord2f(0,1);
            glVertex2f(0,0);
            glTexCoord2f(0,0);
            glVertex2f(0,m_screen_height);
            glTexCoord2f(1,0);
            glVertex2f(m_screen_width,m_screen_height);
            glTexCoord2f(1,1);
            glVertex2f(m_screen_width,0);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }break;

        case gs_controls:
        {
            //next menu texture draw
            glColor3f(1,1,1);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_tex_controls);
            glBegin(GL_QUADS);
            glTexCoord2f(0,1);
            glVertex2f(0,0);
            glTexCoord2f(0,0);
            glVertex2f(0,m_screen_height);
            glTexCoord2f(1,0);
            glVertex2f(m_screen_width,m_screen_height);
            glTexCoord2f(1,1);
            glVertex2f(m_screen_width,0);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }break;

        case gs_running:
        {
            glPushMatrix();
            //move to cam pos
            glTranslatef(int(-m_cam_pos.x+m_screen_width*0.5),int(-m_cam_pos.y+m_screen_height*0.5),0);

            //draw world
            m_pWorld->draw();

            //draw water ripple
            for(int area_i=0;area_i<(int)m_pWorld->m_vec_fish_area.size();area_i++)
            {
                glEnable(GL_TEXTURE_2D);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                glEnable(GL_BLEND);
                glBindTexture(GL_TEXTURE_2D,m_tex_water);
                glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
                float tex_tile_size=64.0/576.0;//for y
                float tex_pos[2]={0,0};
                tex_pos[0]=0.0;
                tex_pos[1]=m_pWorld->m_vec_fish_area[area_i].frame_ind*tex_tile_size;

                glBegin(GL_QUADS);
                glTexCoord2f(tex_pos[0],tex_pos[1]);
                glVertex2f(_tile_size*m_pWorld->m_vec_fish_area[area_i].tile_pos[0],_tile_size*m_pWorld->m_vec_fish_area[area_i].tile_pos[1]);
                glTexCoord2f(tex_pos[0],tex_pos[1]+tex_tile_size);
                glVertex2f(_tile_size*m_pWorld->m_vec_fish_area[area_i].tile_pos[0],_tile_size*m_pWorld->m_vec_fish_area[area_i].tile_pos[1]+_tile_size);
                glTexCoord2f(tex_pos[0]+1.0,tex_pos[1]+tex_tile_size);
                glVertex2f(_tile_size*m_pWorld->m_vec_fish_area[area_i].tile_pos[0]+_tile_size,_tile_size*m_pWorld->m_vec_fish_area[area_i].tile_pos[1]+_tile_size);
                glTexCoord2f(tex_pos[0]+1.0,tex_pos[1]);
                glVertex2f(_tile_size*m_pWorld->m_vec_fish_area[area_i].tile_pos[0]+_tile_size,_tile_size*m_pWorld->m_vec_fish_area[area_i].tile_pos[1]);
                glEnd();

                glDisable(GL_TEXTURE_2D);
                glDisable(GL_BLEND);
            }

            /*//temp world
            glColor3f(0.5,0.5,0.5);
            int lines=20;
            glBegin(GL_LINES);
            for(int i=0;i<lines;i++)
            {
                glVertex2f( m_screen_width/lines*2*i, 0 );
                glVertex2f( 0, m_screen_height/lines*2*i );
            }
            glEnd();*/

            //draw enemies
            for(int enemy_i=0;enemy_i<(int)m_vec_enemy.size();enemy_i++)
            {
                m_vec_enemy[enemy_i].draw();
            }

            //draw players in order
            int draw_order[4]={0,1,2,3};
            float dist[4]={0,0,0,0};
            for(int player_i=0;player_i<(int)m_vec_player.size();player_i++)
            {
                dist[player_i]=m_vec_player[player_i].get_player_pos().y;
            }
            while(true)
            {
                bool update=false;

                for(int i=0;i<3;i++)
                {
                    if(dist[i]>dist[i+1])
                    {
                        float ftemp=dist[i+1];
                        dist[i+1]=dist[i];
                        dist[i]=ftemp;

                        int itemp=draw_order[i+1];
                        draw_order[i+1]=draw_order[i];
                        draw_order[i]=itemp;

                        update=true;
                    }
                }

                if(!update) break;
            }
            for(int player_i=0;player_i<(int)m_vec_player.size();player_i++)
            {
                m_vec_player[draw_order[player_i]].draw(draw_order[player_i]);
            }

            //draw damage areas
            for(int damage_i=0;damage_i<(int)m_vec_damage_area.size();damage_i++)
            {
                glColor3f(1,1,1);
                glBegin(GL_QUADS);
                glVertex2f(m_vec_damage_area[damage_i].damage_pos.x-m_vec_damage_area[damage_i].damage_size.x,
                           m_vec_damage_area[damage_i].damage_pos.y-m_vec_damage_area[damage_i].damage_size.y);
                glVertex2f(m_vec_damage_area[damage_i].damage_pos.x-m_vec_damage_area[damage_i].damage_size.x,
                           m_vec_damage_area[damage_i].damage_pos.y+m_vec_damage_area[damage_i].damage_size.y);
                glVertex2f(m_vec_damage_area[damage_i].damage_pos.x+m_vec_damage_area[damage_i].damage_size.x,
                           m_vec_damage_area[damage_i].damage_pos.y+m_vec_damage_area[damage_i].damage_size.y);
                glVertex2f(m_vec_damage_area[damage_i].damage_pos.x+m_vec_damage_area[damage_i].damage_size.x,
                           m_vec_damage_area[damage_i].damage_pos.y-m_vec_damage_area[damage_i].damage_size.y);
                glEnd();
            }

            //draw particles
            m_pPart->draw();

            glPopMatrix();



            //HUD


            //hp bar
            glPushMatrix();
            for(int player_i=0;player_i<(int)m_vec_player.size();player_i++)
            {
                //background
                glColor3f(0.1,0.1,0.1);
                glBegin(GL_QUADS);
                glVertex2f(m_screen_width*0.005,m_screen_height*0.01);
                glVertex2f(m_screen_width*0.005,m_screen_height*0.04);
                glVertex2f(m_screen_width*0.245,m_screen_height*0.04);
                glVertex2f(m_screen_width*0.245,m_screen_height*0.01);
                glEnd();

                //draw bar
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,m_tex_tile);
                float tex_tile_size=64.0/1024.0;
                float tex_tile_sizey=30.0/1024.0;
                float progress=m_vec_player[player_i].m_hp_curr/m_vec_player[player_i].m_hp_max;
                if(progress<0) progress=0;
                glColor3f(0.6,0.3,0.3);
                glBegin(GL_QUADS);
                glTexCoord2f(0,tex_tile_size);
                glVertex2f(m_screen_width*0.009,m_screen_height*0.014);
                glTexCoord2f(0,tex_tile_size+tex_tile_sizey);
                glVertex2f(m_screen_width*0.009,m_screen_height*0.037);
                glTexCoord2f(tex_tile_size*progress*1.0,tex_tile_size+tex_tile_sizey);
                glVertex2f(m_screen_width*0.009+m_screen_width*0.232*progress,m_screen_height*0.037);
                glTexCoord2f(tex_tile_size*progress*1.0,tex_tile_size);
                glVertex2f(m_screen_width*0.009+m_screen_width*0.232*progress,m_screen_height*0.014);
                glEnd();
                glDisable(GL_TEXTURE_2D);

                //glTranslatef(0,m_screen_height*0.06,0);
                glTranslatef(m_screen_width*0.25,0,0);
            }
            glPopMatrix();

            //coin bar
            glPushMatrix();
            glTranslatef(0,m_screen_height*0.10,0);
            for(int player_i=0;player_i<(int)m_vec_player.size();player_i++)
            {
                //background
                glColor3f(0.1,0.1,0.1);
                glBegin(GL_QUADS);
                glVertex2f(m_screen_width*0.005,m_screen_height*0.01);
                glVertex2f(m_screen_width*0.005,m_screen_height*0.04);
                glVertex2f(m_screen_width*0.245,m_screen_height*0.04);
                glVertex2f(m_screen_width*0.245,m_screen_height*0.01);
                glEnd();

                /*//draw bar
                float progress=m_vec_player[player_i].m_hp_curr/m_vec_player[player_i].m_hp_max;
                if(progress<0) progress=0;
                glColor3f(0.6,0.3,0.3);
                glBegin(GL_QUADS);
                glVertex2f(m_screen_width*0.009,m_screen_height*0.014);
                glVertex2f(m_screen_width*0.009,m_screen_height*0.037);
                glVertex2f(m_screen_width*0.009+m_screen_width*0.232*progress,m_screen_height*0.037);
                glVertex2f(m_screen_width*0.009+m_screen_width*0.232*progress,m_screen_height*0.014);
                glEnd();*/

                //draw lines
                glPushMatrix();
                glLineWidth(2);
                glColor3f(1.0,0.7,0.0);
                int max_coins=47;
                for(int coin=0; coin<m_vec_player[player_i].m_coins_curr && coin<max_coins ;coin++)
                {
                    glBegin(GL_LINES);
                    glVertex2f(m_screen_width*0.009,m_screen_height*0.014);
                    glVertex2f(m_screen_width*0.009,m_screen_height*0.037);
                    glEnd();
                    glTranslatef(m_screen_width*0.005,0,0);
                }
                glPopMatrix();

                //glTranslatef(0,m_screen_height*0.06,0);
                glTranslatef(m_screen_width*0.25,0,0);
            }
            glPopMatrix();

            //draw inventory
            glPushMatrix();
            glTranslatef(-m_screen_width*0.96,m_screen_height*0.93,0);//go to left of the screen
            for(int player_i=0;player_i<(int)m_vec_player.size();player_i++)
            {
                glPushMatrix();
                int slot_counter=0;
                glPushMatrix();//row push;
                for(int slot_i=0;slot_i<(int)m_vec_pWeapon[player_i].size();slot_i++)
                {
                    slot_counter++;
                    if(slot_counter>8)
                    {
                        slot_counter=0;
                        //jump up one row
                        glPopMatrix();
                        glTranslatef(0,-m_screen_height*0.05,0);
                        glPushMatrix();
                    }

                    glColor3f(0.1,0.1,0.1);
                    glBegin(GL_QUADS);
                    glVertex2f(m_screen_width*0.99,m_screen_height*0.015);
                    glVertex2f(m_screen_width*0.99,m_screen_height*0.055);
                    glVertex2f(m_screen_width*0.9675,m_screen_height*0.055);
                    glVertex2f(m_screen_width*0.9675,m_screen_height*0.015);
                    glEnd();

                    /*//only color
                    switch(m_vec_pWeapon[player_i][slot_i]->type)
                    {
                        case wt_pike:     glColor3f(0.8,0.3,0.3); break;
                        case wt_salmon:   glColor3f(0.3,0.8,0.3); break;
                        case wt_mackerel: glColor3f(0.3,0.3,0.8); break;
                        case wt_eel:      glColor3f(0.3,0.8,0.8); break;
                    }
                    glBegin(GL_QUADS);
                    glVertex2f(m_screen_width*0.988,m_screen_height*0.018);
                    glVertex2f(m_screen_width*0.988,m_screen_height*0.052);
                    glVertex2f(m_screen_width*0.9695,m_screen_height*0.052);
                    glVertex2f(m_screen_width*0.9695,m_screen_height*0.018);
                    glEnd();*/

                    //texture
                    glColor3f(1,1,1);
                    glEnable(GL_TEXTURE_2D);
                    glEnable(GL_BLEND);
                    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                    glBindTexture(GL_TEXTURE_2D,m_tex_tile);
                    float tex_tile_size=36.0/1024.0;
                    float tex_pos[2]={0,0};
                    tex_pos[0]=tex_tile_size*(m_vec_pWeapon[player_i][slot_i]->type-1.0);
                    tex_pos[1]=988.0/1024.0;
                    /*switch(m_vec_pWeapon[player_i][slot_i]->type)
                    {
                        case wt_pike:     tex_pos[0]=.0; break;
                        case wt_salmon:   tex_pos[0]=.0; break;
                        case wt_mackerel: tex_pos[0]=.0; break;
                        case wt_eel:      tex_pos[0]=.0; break;
                    }*/
                    glBegin(GL_QUADS);
                    glTexCoord2f(tex_pos[0],tex_pos[1]);
                    glVertex2f(m_screen_width*0.988,m_screen_height*0.018);
                    glTexCoord2f(tex_pos[0],tex_pos[1]+tex_tile_size);
                    glVertex2f(m_screen_width*0.988,m_screen_height*0.052);
                    glTexCoord2f(tex_pos[0]+tex_tile_size,tex_pos[1]+tex_tile_size);
                    glVertex2f(m_screen_width*0.9695,m_screen_height*0.052);
                    glTexCoord2f(tex_pos[0]+tex_tile_size,tex_pos[1]);
                    glVertex2f(m_screen_width*0.9695,m_screen_height*0.018);
                    glEnd();

                    glDisable(GL_TEXTURE_2D);
                    glDisable(GL_BLEND);

                    //draw marker
                    if(slot_i==m_vec_player[player_i].m_inventory_selection_ind)
                    {
                        glLineWidth(2);
                        glColor3f(0.9,0.9,0.9);
                        glBegin(GL_LINE_STRIP);
                        glVertex2f(m_screen_width*0.99,m_screen_height*0.015);
                        glVertex2f(m_screen_width*0.99,m_screen_height*0.055);
                        glVertex2f(m_screen_width*0.9675,m_screen_height*0.055);
                        glVertex2f(m_screen_width*0.9675,m_screen_height*0.015);
                        glVertex2f(m_screen_width*0.99,m_screen_height*0.015);
                        glEnd();
                    }

                    glTranslatef(m_screen_width*0.03,0,0);
                }
                glPopMatrix();
                glPopMatrix();

                glTranslatef(m_screen_width*0.25,0,0);
            }
            glPopMatrix();

            //draw durability bar
            glPushMatrix();
            glTranslatef(0,m_screen_height*0.05,0);
            for(int player_i=0;player_i<(int)m_vec_player.size();player_i++)
            {
                //background
                glColor3f(0.1,0.1,0.1);
                glBegin(GL_QUADS);
                glVertex2f(m_screen_width*0.005,m_screen_height*0.01);
                glVertex2f(m_screen_width*0.005,m_screen_height*0.04);
                glVertex2f(m_screen_width*0.245,m_screen_height*0.04);
                glVertex2f(m_screen_width*0.245,m_screen_height*0.01);
                glEnd();

                //draw bar
                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D,m_tex_tile);
                float tex_tile_size=64.0/1024.0;
                float tex_tile_sizey=30.0/1024.0;
                float progress=m_vec_player[player_i].get_weapon_ptr()->durability*0.01;
                if(progress<0) progress=0;
                glColor3f(0.6,0.5,0.3);
                glBegin(GL_QUADS);
                glTexCoord2f(0,tex_tile_size);
                glVertex2f(m_screen_width*0.009,m_screen_height*0.014);
                glTexCoord2f(0,tex_tile_size+tex_tile_sizey);
                glVertex2f(m_screen_width*0.009,m_screen_height*0.037);
                glTexCoord2f(tex_tile_size*progress*1.0,tex_tile_size+tex_tile_sizey);
                glVertex2f(m_screen_width*0.009+m_screen_width*0.232*progress,m_screen_height*0.037);
                glTexCoord2f(tex_tile_size*progress*1.0,tex_tile_size);
                glVertex2f(m_screen_width*0.009+m_screen_width*0.232*progress,m_screen_height*0.014);
                glEnd();
                glDisable(GL_TEXTURE_2D);

                glTranslatef(m_screen_width*0.25,0,0);
            }
            glPopMatrix();

            //draw fish landing bar
            glPushMatrix();
            glTranslatef(0,m_screen_height*0.05,0);
            for(int player_i=0;player_i<(int)m_vec_player.size();player_i++)
            {
                if(m_vec_player[player_i].m_fishing_state==fs_landing)
                {
                    glPushMatrix();
                    //rand vibration
                    float rand_x=float(rand()%1000)/1000.0*5.0;
                    float rand_y=float(rand()%1000)/1000.0*5.0;
                    glTranslatef(rand_x,rand_y,0);

                    glEnable(GL_TEXTURE_2D);
                    glEnable(GL_BLEND);
                    glBindTexture(GL_TEXTURE_2D,m_tex_tile);

                    float tex_tile_size=64.0/1024.0;
                    float tex_tile_sizey=30.0/1024.0;
                    //background

                    glColor4f(0.8,0.8,0.8,0.8);
                    glBegin(GL_QUADS);
                    glTexCoord2f(0,tex_tile_size);
                    glVertex2f(m_screen_width*0.5-m_screen_width*0.3,m_screen_height*0.1);
                    glTexCoord2f(0,tex_tile_size+tex_tile_sizey);
                    glVertex2f(m_screen_width*0.5-m_screen_width*0.3,m_screen_height*0.2);
                    glTexCoord2f(tex_tile_size,tex_tile_size+tex_tile_sizey);
                    glVertex2f(m_screen_width*0.5+m_screen_width*0.3,m_screen_height*0.2);
                    glTexCoord2f(tex_tile_size,tex_tile_size);
                    glVertex2f(m_screen_width*0.5+m_screen_width*0.3,m_screen_height*0.1);

                    /*

                    glTexCoord2f(0,870.0/1024.0);
                    glVertex2f(m_screen_width*0.5-m_screen_width*0.3,m_screen_height*0.1);
                    glTexCoord2f(0,870.0/1024.0);
                    glVertex2f(m_screen_width*0.5-m_screen_width*0.3,m_screen_height*0.2);
                    glTexCoord2f(1,975.0/1024.0);
                    glVertex2f(m_screen_width*0.5+m_screen_width*0.3,m_screen_height*0.2);
                    glTexCoord2f(1,870.0/1024.0);
                    glVertex2f(m_screen_width*0.5+m_screen_width*0.3,m_screen_height*0.1);

                    */
                    glEnd();
                    glDisable(GL_BLEND);

                    //draw bar
                    glColor3f(1,1,1);
                    float progress=m_vec_player[player_i].m_fish_landing_progress;
                    glBegin(GL_QUADS);
                    glTexCoord2f(0.5-progress*0.5,870.0/1024.0);
                    glVertex2f(m_screen_width*0.5-m_screen_width*0.28*progress,m_screen_height*0.12);
                    glTexCoord2f(0.5-progress*0.5,975.0/1024.0);
                    glVertex2f(m_screen_width*0.5-m_screen_width*0.28*progress,m_screen_height*0.18);
                    glTexCoord2f(0.5+progress*0.5,975.0/1024.0);
                    glVertex2f(m_screen_width*0.5+m_screen_width*0.28*progress,m_screen_height*0.18);
                    glTexCoord2f(0.5+progress*0.5,870.0/1024.0);
                    glVertex2f(m_screen_width*0.5+m_screen_width*0.28*progress,m_screen_height*0.12);

                    /*
                    glTexCoord2f(0,tex_tile_size);
                    glVertex2f(m_screen_width*0.5-m_screen_width*0.28*progress,m_screen_height*0.12);
                    glTexCoord2f(0,tex_tile_size+tex_tile_sizey);
                    glVertex2f(m_screen_width*0.5-m_screen_width*0.28*progress,m_screen_height*0.18);
                    glTexCoord2f(tex_tile_size*progress*1.0,tex_tile_size+tex_tile_sizey);
                    glVertex2f(m_screen_width*0.5+m_screen_width*0.28*progress,m_screen_height*0.18);
                    glTexCoord2f(tex_tile_size*progress*1.0,tex_tile_size);
                    glVertex2f(m_screen_width*0.5+m_screen_width*0.28*progress,m_screen_height*0.12);
                    */
                    glEnd();


                    glDisable(GL_TEXTURE_2D);

                    glPopMatrix();

                    glTranslatef(0,m_screen_height*0.15,0);
                }
            }
            glPopMatrix();



        }break;

        case gs_gameover:
        {
            //last menu texture draw
            glColor3f(1,1,1);
            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D,m_tex_gameover);
            glBegin(GL_QUADS);
            glTexCoord2f(0,1);
            glVertex2f(0,0);
            glTexCoord2f(0,0);
            glVertex2f(0,m_screen_height);
            glTexCoord2f(1,0);
            glVertex2f(m_screen_width,m_screen_height);
            glTexCoord2f(1,1);
            glVertex2f(m_screen_width,0);
            glEnd();
            glDisable(GL_TEXTURE_2D);
        }break;
    }





    return true;
}



//PRIVATE

bool game::load_textures(void)
{
    cout<<"Loading textures\n";

    m_tex_menu=SOIL_load_OGL_texture
	(
		"data\\texture\\menu.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT //SOIL_FLAG_MIPMAPS
	);

	m_tex_controls=SOIL_load_OGL_texture
	(
		"data\\texture\\controls.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT //SOIL_FLAG_MIPMAPS
	);

	m_tex_gameover=SOIL_load_OGL_texture
	(
		"data\\texture\\gameover.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_INVERT_Y | SOIL_FLAG_COMPRESS_TO_DXT //SOIL_FLAG_MIPMAPS
	);

	m_tex_tile=SOIL_load_OGL_texture
	(
		"data\\texture\\tiles.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_COMPRESS_TO_DXT
	);


	m_tex_player=SOIL_load_OGL_texture
	(
		"data\\texture\\FishermanSpriteSheet.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_COMPRESS_TO_DXT
	);

	m_tex_enemy=SOIL_load_OGL_texture
	(
		"data\\texture\\EnemySpriteSheet.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_COMPRESS_TO_DXT
	);

	m_tex_fishing=SOIL_load_OGL_texture
	(
		"data\\texture\\FishingSpriteSheet.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_COMPRESS_TO_DXT
	);

	m_tex_player_attack[0]=SOIL_load_OGL_texture
	(
		"data\\texture\\FishSpriteSheet.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_COMPRESS_TO_DXT
	);

	m_tex_water=SOIL_load_OGL_texture
	(
		"data\\texture\\rippleAnim.png",
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_COMPRESS_TO_DXT
	);

	if(m_tex_menu==0 || m_tex_controls==0 || m_tex_gameover==0 || m_tex_tile==0 ||
       m_tex_player==0 || m_tex_enemy==0 || m_tex_water==0 )
	{
	    return false;
	}

    return true;
}

bool game::load_sounds(void)
{
    cout<<"Loading sounds\n";

    m_pSound=new sound();

    bool error_flag=false;

    if( !m_pSound->load_WAVE_from_file( wav_beep1,"data\\sound\\beep1.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_noise,"data\\sound\\noise.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_smack1,"data\\sound\\smack1.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_eat,"data\\sound\\eat.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_hit,"data\\sound\\hit.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_damaged,"data\\sound\\damaged.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_footstep1,"data\\sound\\footstep1.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_footstep2,"data\\sound\\footstep2.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_miss,"data\\sound\\miss.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_seagulls,"data\\sound\\seagulls.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_sell,"data\\sound\\sell.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_buyboat,"data\\sound\\buyboat.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_buyupgrade,"data\\sound\\buyupgrade.wav" ) ) error_flag=true;
    if( !m_pSound->load_WAVE_from_file( wav_catchfish,"data\\sound\\catchfish.wav" ) ) error_flag=true;

    if(error_flag)
    {
        cout<<"ERROR: Problem loading sound files\n";
        return false;
    }

    return true;
}

