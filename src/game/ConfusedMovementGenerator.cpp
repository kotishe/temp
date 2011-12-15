/*
 * Copyright (C) 2005-2011 MaNGOS <http://getmangos.com/>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ConfusedMovementGenerator.h"
#include "MapManager.h"
#include "Creature.h"
#include "Player.h"
#include "movement/MoveSplineInit.h"
#include "movement/MoveSpline.h"

#define MIN_CONFUSED_DISTANCE 3.0f
#define MAX_CONFUSED_DISTANCE 15.0f

/*template<class T> void ConfusedMovementGenerator<T>::Initialize(T &unit)
{
    const float wander_distance=11;
    float x,y,z;
    x = unit.GetPositionX();
    y = unit.GetPositionY();
    z = unit.GetPositionZ();

    TerrainInfo const* map = unit.GetTerrain();

    i_nextMove = 1;

    bool is_water_ok, is_land_ok;
    _InitSpecific(unit, is_water_ok, is_land_ok);

    for(unsigned int idx=0; idx < MAX_CONF_WAYPOINTS+1; ++idx)
    {
        const float wanderX=wander_distance*rand_norm_f() - wander_distance/2;
        const float wanderY=wander_distance*rand_norm_f() - wander_distance/2;

        i_waypoints[idx][0] = x + wanderX;
        i_waypoints[idx][1] = y + wanderY;

        // prevent invalid coordinates generation
        MaNGOS::NormalizeMapCoord(i_waypoints[idx][0]);
        MaNGOS::NormalizeMapCoord(i_waypoints[idx][1]);

        bool is_water = map->IsInWater(i_waypoints[idx][0],i_waypoints[idx][1],z);
        // if generated wrong path just ignore
        if ((is_water && !is_water_ok) || (!is_water && !is_land_ok))
        {
            i_waypoints[idx][0] = idx > 0 ? i_waypoints[idx-1][0] : x;
            i_waypoints[idx][1] = idx > 0 ? i_waypoints[idx-1][1] : y;
        }

        unit.UpdateAllowedPositionZ(i_waypoints[idx][0],i_waypoints[idx][1],z);
        i_waypoints[idx][2] =  z;
    }

    unit.StopMoving();
    unit.addUnitState(UNIT_STAT_CONFUSED|UNIT_STAT_CONFUSED_MOVE);
}
*/

template<class T> void ConfusedMovementGenerator<T>::Initialize( T &unit )
{
    unit.addUnitState( UNIT_STAT_CONFUSED|UNIT_STAT_CONFUSED_MOVE );

    _Init( unit );

    unit_pos_x = unit.GetPositionX();
    unit_pos_y = unit.GetPositionY();
    unit_pos_z = unit.GetPositionZ();


    only_forward = true;
    current_angle = 0.0f;
    i_last_distance_from_caster = 0.0f;
    i_to_distance_from_caster = 0.0f;
    _setTargetLocation( unit );
}

template<> void ConfusedMovementGenerator<Creature>::_Init( Creature &creature )
{
    is_water_ok = creature.CanSwim();
    is_land_ok  = creature.CanWalk();
}

template<> void ConfusedMovementGenerator<Player>::_Init( Player & )
{
    is_water_ok = true;
    is_land_ok  = true;
}

template<class T> void ConfusedMovementGenerator<T>::_setTargetLocation( T &unit )
{
    if( !&unit )
        return;

    // ignore in case other no reaction state
    if ( unit.hasUnitState( UNIT_STAT_CAN_NOT_REACT & ~UNIT_STAT_CONFUSED ))
        return;

    if( !_setMoveData( unit ))
        return;

    float x, y, z;
    if( !_getPoint( unit, x, y, z ))
        return;

    unit.addUnitState( UNIT_STAT_CONFUSED_MOVE );

    Movement::MoveSplineInit init( unit );
    init.MoveTo( x, y, z );
    init.SetWalk( true );
    init.Launch();
}

template<class T> bool ConfusedMovementGenerator<T>::_setMoveData( T &unit )
{
    float cur_dist_xyz = unit.GetDistance( unit_pos_x, unit_pos_y, unit_pos_z );

    if ( i_to_distance_from_caster > 0.0f )
    {
        if ( ( i_last_distance_from_caster > i_to_distance_from_caster
               && cur_dist_xyz < i_to_distance_from_caster) || // if we reach lower distance
             ( i_last_distance_from_caster > i_to_distance_from_caster
               && cur_dist_xyz > i_last_distance_from_caster) || // if we can't be close
             ( i_last_distance_from_caster < i_to_distance_from_caster
               && cur_dist_xyz > i_to_distance_from_caster) || // if we reach bigger distance
             ( cur_dist_xyz > MAX_CONFUSED_DISTANCE ) || // if we are too far
             ( i_last_distance_from_caster > MIN_CONFUSED_DISTANCE
               && cur_dist_xyz < MIN_CONFUSED_DISTANCE ) )
                                                            // if we leave 'quiet zone'
        {
            // we are very far or too close, stopping
            i_to_distance_from_caster = 0.0f;
            NextMoveTime.Reset( urand(500,1000) );
            return false;
        }
        else
        {
            // now we are running, continue
            i_last_distance_from_caster = cur_dist_xyz;
            return true;
        }
    }

    float current_distance;
    float unit_angle;

    /*if (Unit* fright = owner.GetMap()->GetUnit(i_frightGuid))
    {
        cur_dist = fright->GetDistance(&owner);
        if (cur_dist < cur_dist_xyz)
        {
            i_caster_x = fright->GetPositionX();
            i_caster_y = fright->GetPositionY();
            i_caster_z = fright->GetPositionZ();
            angle_to_caster = fright->GetAngle(&owner);
        }
        else
        {
            cur_dist = cur_dist_xyz;
            angle_to_caster = owner.GetAngle(i_caster_x, i_caster_y) + M_PI_F;
        }
    }*/
    //else
    //{
        current_distance = cur_dist_xyz;
        unit_angle = unit.GetAngle( unit_pos_x, unit_pos_y ) + M_PI_F;
    //}

    // if we too close may use 'path-finding' else just stop
    only_forward = current_distance >= MIN_CONFUSED_DISTANCE / 3;

    //get angle and 'distance from caster' to walk
    float angle;

    if( current_angle == 0.0f && i_last_distance_from_caster == 0.0f) //just started, first time
    {
        angle = rand_norm_f() * ( 1.0f - current_distance / MIN_CONFUSED_DISTANCE )
                * M_PI_F / 3 + rand_norm_f() * M_PI_F * 2/3;
        i_to_distance_from_caster = MIN_CONFUSED_DISTANCE;
        only_forward = true;
    }
    else if( current_distance < MIN_CONFUSED_DISTANCE )
    {
        angle = M_PI_F / 6 + rand_norm_f() * M_PI_F * 2 / 3;
        i_to_distance_from_caster = current_distance * 2 / 3 + rand_norm_f()
                * ( MIN_CONFUSED_DISTANCE - current_distance * 2 / 3 );
    }
    else if( current_distance > MAX_CONFUSED_DISTANCE )
    {
        angle = rand_norm_f() * M_PI_F / 3 + M_PI_F * 2 / 3;
        i_to_distance_from_caster = MIN_CONFUSED_DISTANCE + 2.5f + rand_norm_f()
                * ( MAX_CONFUSED_DISTANCE - MIN_CONFUSED_DISTANCE - 2.5f );
    }
    else
    {
        angle = rand_norm_f() * M_PI_F;
        i_to_distance_from_caster = MIN_CONFUSED_DISTANCE + 2.5f + rand_norm_f()
                * ( MAX_CONFUSED_DISTANCE - MIN_CONFUSED_DISTANCE - 2.5f );
    }

    int8 sign = rand_norm_f() > 0.5f ? 1 : -1;
    current_angle = sign * angle + unit_angle;

    // current distance
    i_last_distance_from_caster = current_distance;

    return true;
}

template<class T> bool ConfusedMovementGenerator<T>::_getPoint(T &unit, float &x, float &y, float &z)
{
    if( !&unit )
        return false;

    x = unit.GetPositionX();
    y = unit.GetPositionY();
    z = unit.GetPositionZ();

    float temp_x, temp_y, angle;
    const TerrainInfo * _map = unit.GetTerrain();
    //primitive path-finding
    for( uint8 i = 0; i < 18; ++i )
    {
        if( only_forward && i > 2)
            break;

        float distance = 5.0f;

        switch(i)
        {
            case 0:
                angle = current_angle;
                break;
            case 1:
                angle = current_angle;
                distance /= 2;
                break;
            case 2:
                angle = current_angle;
                distance /= 4;
                break;
            case 3:
                angle = current_angle + M_PI_F / 4.0f;
                break;
            case 4:
                angle = current_angle - M_PI_F / 4.0f;
                break;
            case 5:
                angle = current_angle + M_PI_F / 4.0f;
                distance /= 2;
                break;
            case 6:
                angle = current_angle - M_PI_F / 4.0f;
                distance /= 2;
                break;
            case 7:
                angle = current_angle + M_PI_F / 2.0f;
                break;
            case 8:
                angle = current_angle - M_PI_F / 2.0f;
                break;
            case 9:
                angle = current_angle + M_PI_F / 2.0f;
                distance /= 2;
                break;
            case 10:
                angle = current_angle - M_PI_F / 2.0f;
                distance /= 2;
                break;
            case 11:
                angle = current_angle + M_PI_F / 4.0f;
                distance /= 4;
                break;
            case 12:
                angle = current_angle - M_PI_F / 4.0f;
                distance /= 4;
                break;
            case 13:
                angle = current_angle + M_PI_F / 2.0f;
                distance /= 4;
                break;
            case 14:
                angle = current_angle - M_PI_F / 2.0f;
                distance /= 4;
                break;
            case 15:
                angle = current_angle + M_PI_F * 3 / 4.0f;
                distance /= 2;
                break;
            case 16:
                angle = current_angle - M_PI_F * 3 / 4.0f;
                distance /= 2;
                break;
            case 17:
                angle = current_angle + M_PI_F;
                distance /= 2;
                break;
        }
        temp_x = x + distance * cos( angle );
        temp_y = y + distance * sin( angle );
        MaNGOS::NormalizeMapCoord( temp_x );
        MaNGOS::NormalizeMapCoord( temp_y );
        if( unit.IsWithinLOS( temp_x, temp_y, z ) )
        {
            bool is_water_now = _map->IsInWater( x, y, z );

            if( is_water_now && _map->IsInWater( temp_x, temp_y, z ) )
            {
                x = temp_x;
                y = temp_y;
                return true;
            }
            float new_z = _map->GetHeight( temp_x, temp_y, z, true);

            if( new_z <= INVALID_HEIGHT )
                continue;

            bool is_water_next = _map->IsInWater( temp_x, temp_y, new_z );

            if( ( is_water_now && !is_water_next && !is_land_ok )
                    || ( !is_water_now && is_water_next && !is_water_ok ) )
                continue;

            if( !(new_z - z) || distance / fabs( new_z - z ) > 1.0f)
            {
                float new_z_left = _map->GetHeight( temp_x + 1.0f * cos( angle + M_PI_F / 2 ),
                                                    temp_y + 1.0f * sin( angle + M_PI_F / 2 ), z, true);
                float new_z_right = _map->GetHeight( temp_x + 1.0f * cos( angle - M_PI_F /2 ),
                                                     temp_y + 1.0f * sin( angle - M_PI_F / 2 ), z, true);
                if( fabs( new_z_left - new_z ) < 1.2f && fabs( new_z_right - new_z ) < 1.2f )
                {
                    x = temp_x;
                    y = temp_y;
                    z = new_z;
                    return true;
                }
            }
        }
    }
    i_to_distance_from_caster = 0.0f;
    NextMoveTime.Reset( urand(500,1000) );
    return false;
}

template<class T> void ConfusedMovementGenerator<T>::Interrupt(T &unit)
{
    // confused state still applied while movegen disabled
    unit.clearUnitState( UNIT_STAT_CONFUSED_MOVE );
}

template<class T> void ConfusedMovementGenerator<T>::Reset(T &unit)
{
    Initialize( unit );
}

template<class T> bool ConfusedMovementGenerator<T>::Update( T &unit, const uint32 &diff )
{
    // ignore in case other no reaction state
    /*if (unit.hasUnitState(UNIT_STAT_CAN_NOT_REACT & ~UNIT_STAT_CONFUSED))
        return true;

    if (i_nextMoveTime.Passed())
    {
        // currently moving, update location
        unit.addUnitState(UNIT_STAT_CONFUSED_MOVE);

        if (unit.movespline->Finalized())
        {
            i_nextMove = urand(1,MAX_CONF_WAYPOINTS);
            i_nextMoveTime.Reset(urand(0, 1500-1));     // TODO: check the minimum reset time, should be probably higher
        }
    }
    else
    {
        // waiting for next move
        i_nextMoveTime.Update(diff);
        if(i_nextMoveTime.Passed() )
        {
            // start moving
            unit.addUnitState(UNIT_STAT_CONFUSED_MOVE);

            MANGOS_ASSERT( i_nextMove <= MAX_CONF_WAYPOINTS );
            float x = i_waypoints[i_nextMove][0];
            float y = i_waypoints[i_nextMove][1];
            float z = i_waypoints[i_nextMove][2];
            Movement::MoveSplineInit init(unit);
            init.MoveTo(x, y, z);
            init.SetWalk(true);
            init.Launch();
        }
    }
    return true;*/
    if( !&unit || !unit.isAlive() )
        return false;
    // ignore in case other no reaction state
    if ( unit.hasUnitState( UNIT_STAT_CAN_NOT_REACT & ~UNIT_STAT_CONFUSED ) )
    {
        unit.clearUnitState( UNIT_STAT_CONFUSED_MOVE );
        return true;
    }
    NextMoveTime.Update( diff );
    if ( NextMoveTime.Passed() && unit.movespline->Finalized() )
        _setTargetLocation( unit );

    return true;
}

template<> void ConfusedMovementGenerator<Player>::Finalize(Player &unit)
{
    unit.clearUnitState( UNIT_STAT_CONFUSED|UNIT_STAT_CONFUSED_MOVE );
}

template<> void ConfusedMovementGenerator<Creature>::Finalize(Creature &unit)
{
    unit.clearUnitState( UNIT_STAT_CONFUSED|UNIT_STAT_CONFUSED_MOVE );
}

template void ConfusedMovementGenerator<Player>::Initialize(Player &player);
template void ConfusedMovementGenerator<Creature>::Initialize(Creature &creature);
template void ConfusedMovementGenerator<Player>::Interrupt(Player &player);
template void ConfusedMovementGenerator<Creature>::Interrupt(Creature &creature);
template void ConfusedMovementGenerator<Player>::Reset(Player &player);
template void ConfusedMovementGenerator<Creature>::Reset(Creature &creature);
template bool ConfusedMovementGenerator<Player>::Update(Player &player, const uint32 &diff);
template bool ConfusedMovementGenerator<Creature>::Update(Creature &creature, const uint32 &diff);


/*void TimedConfusedMovementGenerator::Finalize(Unit &unit)
{
    unit.clearUnitState( UNIT_STAT_CONFUSED|UNIT_STAT_CONFUSED_MOVE );
    if ( Unit * victim = unit.getVictim() )
    {
        if ( unit.isAlive() )
        {
            unit.AttackStop( true );
            ((Creature*)&unit)->AI()->AttackStart( victim );
        }
    }
}

bool TimedFleeingMovementGenerator::Update( Unit & unit, const uint32 & diff )
{
    if( !unit.isAlive() )
        return false;

    // ignore in case other no reaction state
    if ( unit.hasUnitState( UNIT_STAT_CAN_NOT_REACT & ~UNIT_STAT_CONFUSED ) )
    {
        unit.clearUnitState( UNIT_STAT_CONFUSED_MOVE );
        return true;
    }

    i_totalFleeTime.Update( diff );
    if ( i_totalFleeTime.Passed() )
        return false;

    // This calls grant-parent Update method hiden by FleeingMovementGenerator::Update(Creature &, const uint32 &) version
    // This is done instead of casting Unit& to Creature& and call parent method, then we can use Unit directly
    return MovementGeneratorMedium< Creature, FleeingMovementGenerator<Creature> >::Update( unit , diff );
}*/
