/*=========================================================================

  Program:   ParaView
  Module:    vtkSMFixedTypeDomain.cxx

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkSMFixedTypeDomain.h"

#include "vtkObjectFactory.h"
#include "vtkPVDataInformation.h"
#include "vtkPVXMLElement.h"
#include "vtkSMPart.h"
#include "vtkSMProxyProperty.h"
#include "vtkSMSourceProxy.h"

vtkStandardNewMacro(vtkSMFixedTypeDomain);
vtkCxxRevisionMacro(vtkSMFixedTypeDomain, "1.1");

//---------------------------------------------------------------------------
vtkSMFixedTypeDomain::vtkSMFixedTypeDomain()
{
}

//---------------------------------------------------------------------------
vtkSMFixedTypeDomain::~vtkSMFixedTypeDomain()
{
}

//---------------------------------------------------------------------------
int vtkSMFixedTypeDomain::IsInDomain(vtkSMProperty* property)
{
  if (!property)
    {
    return 0;
    }

  vtkSMProxyProperty* pp = vtkSMProxyProperty::SafeDownCast(property);
  if (pp)
    {
    unsigned int numOldProxs = pp->GetNumberOfProxies();
    if (numOldProxs == 0)
      {
      return 1;
      }
    unsigned int numNewProxs = pp->GetNumberOfUncheckedProxies();
    if (numOldProxs != numNewProxs)
      {
      return 0;
      }
    for (unsigned int i=0; i<numOldProxs; i++)
      {
      if (!this->IsInDomain( 
            vtkSMSourceProxy::SafeDownCast(pp->GetProxy(i)), 
            vtkSMSourceProxy::SafeDownCast(pp->GetUncheckedProxy(i)) ) )
        {
        return 0;
        }
      }
    return 1;
    }

  return 0;
}

//---------------------------------------------------------------------------
int vtkSMFixedTypeDomain::IsInDomain(vtkSMSourceProxy* oldProxy,
                                     vtkSMSourceProxy* newProxy)
{
  if (!oldProxy || !newProxy)
    {
    return 0;
    }

  // Make sure the outputs are created.
  newProxy->CreateParts();

  if (oldProxy->GetNumberOfParts() != newProxy->GetNumberOfParts())
    {
    return 0;
    }

  unsigned int oNumParts = oldProxy->GetNumberOfParts();
  unsigned int nNumParts = oldProxy->GetNumberOfParts();
  if (oNumParts != nNumParts)
    {
    return 0;
    }

  for (unsigned int i=0; i<oNumParts; i++)
    {
    vtkPVDataInformation* oDI = oldProxy->GetPart(i)->GetDataInformation();
    vtkPVDataInformation* nDI = newProxy->GetPart(i)->GetDataInformation();
  
    if (!oDI || !nDI)
      {
      return 0;
      }
    
    if (oDI->GetDataSetType() != nDI->GetDataSetType())
      {
      return 0;
      }
    }

  return 1;
}

//---------------------------------------------------------------------------
void vtkSMFixedTypeDomain::SaveState(
  const char* name, ofstream* file, vtkIndent indent)
{
  *file << indent 
        << "<Domain name=\"" << this->XMLName << "\" id=\"" << name << "\">"
        << endl;
  *file << indent << "</Domain>" << endl;
}

//---------------------------------------------------------------------------
void vtkSMFixedTypeDomain::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
