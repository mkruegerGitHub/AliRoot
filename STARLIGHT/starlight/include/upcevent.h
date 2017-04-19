///////////////////////////////////////////////////////////////////////////
//
//    Copyright 2010
//
//    This file is part of starlight.
//
//    starlight is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    starlight is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with starlight. If not, see <http://www.gnu.org/licenses/>.
//
///////////////////////////////////////////////////////////////////////////
//
// File and Version Information:
// $Rev:: 263                         $: revision of last commit
// $Author:: butter                   $: author of last commit
// $Date:: 2016-06-05 00:03:58 +0200 #$: date of last commit
//
// Description:
//
//
//
///////////////////////////////////////////////////////////////////////////


#ifndef UPCEVENT_H
#define UPCEVENT_H


#include <vector>

#include "starlightconstants.h"
#include "starlightparticle.h"

class Gammaavectormeson;

class upcEvent
{
   public:

      upcEvent();
      upcEvent(starlightConstants::event &ev);
      ~upcEvent();

      void addParticle(starlightParticle &part) { _particles.push_back(part); }
      void addVertex(vector3 &vertex) { _vertices.push_back(vertex); }
      void addGamma(float egamma) { _gammaEnergies.push_back(egamma); }

      const std::vector<starlightParticle> * getParticles() const { return &_particles; }
      const std::vector<vector3> * getVertices() const { return &_vertices; }
      const std::vector<float> * getGammaEnergies() const { return &_gammaEnergies; }

      bool   isGammaavm() const { return _isGammaavm; }
      double getBslope() const { return _bSlope; }
      double gett() const { return _t; }
      double getEgam() const { return _Egam; }
      double getEpom() const { return _Epom; }
      const double* getPtGam() const { return _ptGam; }
      const double* getPtPom() const { return _ptPom; }

      upcEvent & operator=(const upcEvent&);
      upcEvent & operator+(const upcEvent&);

      void boost(double rapidity);
   private:

      friend class Gammaavectormeson; // this class sets _bSlope, ..., _Epom;

      std::vector<starlightParticle> _particles;
      std::vector<vector3> _vertices;
      std::vector<float> _gammaEnergies;
      bool   _isGammaavm; // if true the member variables below are set
      double _bSlope;     // gammaavm b-slope
      double _t;          // gammaavm t
      double _ptGam[2];   // gammaavm photon pt
      double _Egam;       // gammaavm photon energy
      double _ptPom[2];   // gammaavm pomeron pt
      double _Epom;       // gammaavm pomeron energy
};


#endif  // UPCEVENT_H
