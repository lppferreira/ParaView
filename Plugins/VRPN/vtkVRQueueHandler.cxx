/*=========================================================================

   Program: ParaView
   Module:  vtkVRQueueHandler.cxx

   Copyright (c) 2005,2006 Sandia Corporation, Kitware Inc.
   All rights reserved.

   ParaView is a free software; you can redistribute it and/or modify it
   under the terms of the ParaView license version 1.2.

   See License_v1.2.txt for the full ParaView license.
   A copy of this license can be obtained by contacting
   Kitware Inc.
   28 Corporate Drive
   Clifton Park, NY 12065
   USA

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "vtkVRQueueHandler.h"

#include "vtkObjectFactory.h"
#include "vtkPVXMLElement.h"
#include "vtkVRGenericStyle.h"
#include "vtkVRInteractorStyle.h"
#include "vtkVRQueue.h"
#include "vtkVRVectorPropertyStyle.h"

#include <QList>
#include <QPointer>
#include <QtDebug>
#include <QTimer>

class vtkVRQueueHandler::pqInternals
{
public:
  QList<QPointer<vtkVRInteractorStyle> > Styles;
  QPointer<vtkVRQueue> Queue;
  QTimer Timer;
};

//----------------------------------------------------------------------------
vtkVRQueueHandler::vtkVRQueueHandler(
  vtkVRQueue* queue, QObject* parentObject)
  : Superclass(parentObject)
{
  this->Internals = new pqInternals();
  this->Internals->Queue = queue;
  this->Internals->Timer.setInterval(100);
  this->Internals->Timer.setSingleShot(true);
  QObject::connect(&this->Internals->Timer, SIGNAL(timeout()),
    this, SLOT(processEvents()));
}

//----------------------------------------------------------------------------
vtkVRQueueHandler::~vtkVRQueueHandler()
{
  delete this->Internals;
}

//----------------------------------------------------------------------------
void vtkVRQueueHandler::add(vtkVRInteractorStyle* style)
{
  this->Internals->Styles.push_front(style);
}

//----------------------------------------------------------------------------
void vtkVRQueueHandler::remove(vtkVRInteractorStyle* style)
{
  this->Internals->Styles.removeAll(style);
}

//----------------------------------------------------------------------------
void vtkVRQueueHandler::start()
{
  this->Internals->Timer.start();
}

//----------------------------------------------------------------------------
void vtkVRQueueHandler::stop()
{
  this->Internals->Timer.stop();
}

//----------------------------------------------------------------------------
void vtkVRQueueHandler::processEvents()
{
  Q_ASSERT(this->Internals->Queue != NULL);
  QQueue<vtkVREventData> events;
  this->Internals->Queue->tryDequeue(events);
  while (!events.isEmpty())
    {
    vtkVREventData data = events.dequeue();
    foreach (vtkVRInteractorStyle* style, this->Internals->Styles)
      {
      if (style && style->handleEvent(data))
        {
        break;
        }
      }
    }

  // since timer is single-shot we start it again.
  this->Internals->Timer.start();
}

//----------------------------------------------------------------------------
/* Sample configuration:
  <VRInteractorStyles>
    <Style class="vtkVRVectorPropertyStyle"
           proxy="12"
           propertyname="Normal"
           mode="direction">
      <Event device="wand" button="1" />
    </Style>
  
    <Style class="vtkVRVectorPropertyStyle"
           proxy="12"
           propertyname="Origin"
           mode="displacement">
      <Event device="wand" button="2" />
    </Style>
  </VRInteractorStyles>
*/
void vtkVRQueueHandler::configureStyles(vtkPVXMLElement* xml,
  vtkSMProxyLocator* locator)
{
  if (!xml)
    {
    return;
    }

  if (xml->GetName() && strcmp(xml->GetName(), "VRInteractorStyles"))
    {
    this->Internals->Styles.clear();
    for (unsigned cc=0; cc < xml->GetNumberOfNestedElements(); cc++)
      {
      vtkPVXMLElement* child = xml->GetNestedElement(cc);
      if (child && child->GetName() && strcmp(child->GetName(), "Style")==0)
        {
        const char* class_name = child->GetAttributeOrEmpty("class");
        if (strcmp(class_name, "vtkVRVectorPropertyStyle")==0)
          {
          vtkVRVectorPropertyStyle* style = new vtkVRVectorPropertyStyle(this);
          style->configure(child, locator);
          this->add(style);
          }
        else if (strcmp(class_name, "vtkVRGenericStyle")==0)
          {
          vtkVRGenericStyle* style = new vtkVRGenericStyle(this);
          style->configure(child, locator);
          this->add(style);
          }
        else
          {
          qWarning() << "Unknown interactor style: \"" << class_name << "\"";
          }
        }
      }
    }
  else
    {
    this->configureStyles(xml->FindNestedElementByName("VRInteractorStyles"),
      locator); 
    }
}
