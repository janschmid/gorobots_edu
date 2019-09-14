/***************************************************************************
 *   Copyright (C) 2005-2011 LpzRobots development team                    *
 *    Georg Martius  <georg dot martius at web dot de>                     *
 *    Frank Guettler <guettler at informatik dot uni-leipzig dot de        *
 *    Frank Hesse    <frank at nld dot ds dot mpg dot de>                  *
 *    Ralf Der       <ralfder at mis dot mpg dot de>                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                         *
 ***************************************************************************/

#include <assert.h>

#include <ode_robots/primitive.h>
#include "relativepositionsensor.h"
#include <ode_robots/mathutils.h>

namespace lpzrobots {

  RelativePositionSensor::RelativePositionSensor(double maxDistance, double exponent,
                                                 short dimensions /* = X | Y | Z */ , bool local_coordinates /*= false*/)
    : maxDistance(maxDistance), exponent(exponent), dimensions (dimensions), local_coords(local_coordinates){
    own=0;
    ref=0;
  //  setBaseInfo(SensorMotorInfo("RelPos-Old-").changequantity(SensorMotorInfo::Distance).changemin(0));
 //   setNamingFunc([dimensions](int index) {return dimensions2String(dimensions).substr(index,1);});
  }

  void RelativePositionSensor::init(Primitive* own, Joint* joint){
    this->own = own;
  }

  void RelativePositionSensor::setReference(Primitive* ref){
    this->ref = ref;
  }

  int RelativePositionSensor::getSensorNumber() const{
    return (dimensions & X) + ((dimensions & Y) >> 1)  + ((dimensions & Z) >> 2);
  }

  std::list<sensor> RelativePositionSensor::getList() const {
    assert(own);
    std::list<sensor> s;
    osg::Vec3 v;
    osg::Vec3 refpos = ref ? ref->getPosition() : osg::Vec3(0,0,0);
    if (local_coords){
      v = own->toLocal(refpos);
    }else{
      v = refpos - own->getPosition();
    }
    double scale = pow(v.length() / maxDistance, exponent);
    v *= (1/maxDistance)*scale;
/// in order to use the new sensor interface right away we swap then sensor values z and x here (it was done in the robot before)
//    if (dimensions & Z) s.push_back(v.z());
//    if (dimensions & Y) s.push_back(v.y());
//    if (dimensions & X) s.push_back(v.x());
    if (dimensions & X) s.push_back(v.x());
    if (dimensions & Y) s.push_back(v.y());
    if (dimensions & Z) s.push_back(v.z());
    return s;
  }

  bool RelativePositionSensor::sense(const GlobalData& globaldata){
    return true;
  }

}
