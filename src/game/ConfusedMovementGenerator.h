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

#ifndef MANGOS_CONFUSEDMOVEMENTGENERATOR_H
#define MANGOS_CONFUSEDMOVEMENTGENERATOR_H

#include "MovementGenerator.h"
#include "Timer.h"

#define MAX_CONF_WAYPOINTS 24

template<class T>
class MANGOS_DLL_SPEC ConfusedMovementGenerator
        : public MovementGeneratorMedium< T, ConfusedMovementGenerator<T> >
{
    public:
        explicit ConfusedMovementGenerator() : NextMoveTime(0) {}

        void Initialize( T & );
        void Finalize( T & );
        void Interrupt( T & );
        void Reset( T & );
        bool Update( T &, const uint32 & );

        MovementGeneratorType GetMovementGeneratorType() const { return CONFUSED_MOTION_TYPE; }
    private:
        TimeTracker NextMoveTime;
        float i_waypoints[MAX_CONF_WAYPOINTS+1][3];
        uint32 i_nextMove;

        //Новые переменные и ф-ии для реализации ConfusedMovement
        void _setTargetLocation( T &unit );
        bool _getPoint( T &unit, float &x, float &y, float &z );
        bool _setMoveData( T &unit );
        void _Init( T & );

        bool is_water_ok    :   1;
        bool is_land_ok     :   1;
        bool only_forward   :   1;

        float unit_pos_x;
        float unit_pos_y;
        float unit_pos_z;
        float current_angle;

        //ObjectGuid i_frightGuid; //пока уберу
        //TimeTracker i_nextCheckTime; allreay in class
        //Для чего эти две переменные?
        float i_last_distance_from_caster;
        float i_to_distance_from_caster;

};

/*class MANGOS_DLL_SPEC TimedConfusedMovementGenerator
        : public ConfusedMovementGenerator<Creature>
{
    public:
        TimedConfusedMovementGenerator(ObjectGuid fright, uint32 time) :
            ConfusedMovementGenerator<Creature>(fright),
            i_totalFleeTime(time) {}

        MovementGeneratorType GetMovementGeneratorType() const { return TIMED_FLEEING_MOTION_TYPE; }
        bool Update(Unit &, const uint32 &);
        void Finalize(Unit &);

    private:
        TimeTracker i_totalFleeTime;
};*/
#endif
