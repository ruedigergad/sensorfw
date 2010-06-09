/**
   @file topedgeinterpreter.cpp
   @brief TopEdgeInterpreterFilter

   <p>
   Copyright (C) 2009-2010 Nokia Corporation

   @author Timo Rongas <ext-timo.2.rongas@nokia.com>
   @author Üstün Ergenoglu <ext-ustun.ergenoglu@nokia.com>

   This file is part of Sensord.

   Sensord is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License
   version 2.1 as published by the Free Software Foundation.

   Sensord is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with Sensord.  If not, see <http://www.gnu.org/licenses/>.
   </p>
 */
#include "topedgeinterpreter.h"
#include "sensord/logging.h"

#define DEFAULT_THRESHOLD 250

TopEdgeInterpreter::TopEdgeInterpreter() :
        Filter<TimedXyzData, TopEdgeInterpreter, PoseData>(this, &TopEdgeInterpreter::interpret),
        threshold_(*this),
        pose(PoseData::Undefined)
{
    qRegisterMetaType<PoseData>("PoseData");
    threshold_(DEFAULT_THRESHOLD);
}

void TopEdgeInterpreter::interpret(unsigned, const TimedXyzData* data)
{
    int x = data->x_;
    int y = data->y_;
    int z = data->z_;
    sensordLogT() << "orientation vector:" << x << "," << y << "," << z;
    int t = threshold_();

    int gVector = ((x*x + y*y + z*z)/1000);
    bool goodVector = ((gVector >= 800) && (gVector <=1250)) ? true : false;
    if (!goodVector)
        sensordLogT() << "vector is not good...";

    if ( goodVector && (abs(x) >= 200 || abs(y) >=200) ) {
        if (pose.orientation_ == PoseData::Undefined) {
            /*Change topedge away from unknown when x or y
             * goes above t.*/
            if ( abs(y) > t &&
                    (abs(y) > abs(x)) )
                newPose.orientation_ = (y>0?PoseData::BottomUp : PoseData::BottomDown);
            else if ( abs(x) > t &&
                    (abs(x) > abs (y)) )
                newPose.orientation_ = (x>0?PoseData::LeftUp : PoseData::RightUp);
        } else if (pose.orientation_ == PoseData::LeftUp || pose.orientation_ == PoseData::RightUp) {
            if (abs(y) > abs(x) + t) {
                newPose.orientation_ = (y > 0?PoseData::BottomUp:PoseData::BottomDown);
            } else {
                newPose.orientation_ = (x > 0?PoseData::LeftUp:PoseData::RightUp);
            }
        } else if (pose.orientation_ == PoseData::BottomUp || pose.orientation_ == PoseData::BottomDown) {
            if (abs(x) > abs(y) + t) {
                newPose.orientation_ = (x > 0?PoseData::LeftUp:PoseData::RightUp);
            } else {
                newPose.orientation_ = (y > 0?PoseData::BottomUp:PoseData::BottomDown);
            }
        }
    } else {
        newPose.orientation_ = PoseData::Undefined;
    }

    if (pose.orientation_ != newPose.orientation_) {
        // Axis direction changed directly or difference larger than threshold
        pose.orientation_ = newPose.orientation_;
        sensordLogT() << "TopEdge is:" << pose.orientation_;
        pose.timestamp_ = data->timestamp_;
        source_.propagate(1, &pose);
    }
}