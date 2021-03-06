/**************************************************************************
 * Copyright(c) 1998-2016, ALICE Experiment at CERN, All rights reserved. *
 *                                                                        *
 * Author: The ALICE Off-line Project.                                    *
 * Contributors are mentioned in the code where appropriate.              *
 *                                                                        *
 * Permission to use, copy, modify and distribute this software and its   *
 * documentation strictly for non-commercial purposes is hereby granted   *
 * without fee, provided that the above copyright notice appears in all   *
 * copies and that both the copyright notice and this permission notice   *
 * appear in the supporting documentation. The authors make no claims     *
 * about the suitability of this software for any purpose. It is          *
 * provided "as is" without express or implied warranty.                  *
 **************************************************************************/

#include "AliGenToyEventHeader.h"
#include <map>
#include <string>

AliGenToyEventHeader::AliGenToyEventHeader(const char* name) :
  AliGenEventHeader(name),
  fCentrality(-1.),
  fParameters()
{

}

AliGenToyEventHeader::AliGenToyEventHeader(const AliGenToyEventHeader &rhs) :
  AliGenEventHeader(rhs),
  fCentrality(rhs.fCentrality),
  fParameters(rhs.fParameters)
{

}

AliGenToyEventHeader::~AliGenToyEventHeader()
{

}

AliGenToyEventHeader& AliGenToyEventHeader::operator=(const  AliGenToyEventHeader& rhs)
{
  if (this != &rhs) {
    fCentrality = rhs.fCentrality;
    fParameters = rhs.fParameters;
  }

  return *this;
}

