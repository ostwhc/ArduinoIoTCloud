/*
   This file is part of ArduinoIoTCloud.

   Copyright 2020 ARDUINO SA (http://www.arduino.cc/)

   This software is released under the GNU General Public License version 3,
   which covers the main part of arduino-cli.
   The terms of this license can be found at:
   https://www.gnu.org/licenses/gpl-3.0.en.html

   You can be released from the requirements of the above licenses by purchasing
   a commercial license. Buying such a license is mandatory if you want to modify or
   otherwise use the software for commercial activities involving the Arduino
   software without disclosing the source code of your own applications. To purchase
   a commercial license, send an email to license@arduino.cc.
*/

/******************************************************************************
   INCLUDE
 ******************************************************************************/

#include "PropertyContainer.h"

#include <algorithm>

#include "types/CloudWrapperBase.h"

/******************************************************************************
   PUBLIC MEMBER FUNCTIONS
 ******************************************************************************/

Property & addPropertyToContainer(PropertyContainer & prop_cont, Property & property, String const & name, Permission const permission, int propertyIdentifier, GetTimeCallbackFunc func)
{
  /* Check whether or not the property already has been added to the container */
  Property * p = getProperty(prop_cont, name);
  if(p != nullptr) return (*p);

  /* Initialize property and add it to the container */
  property.init(name, permission, func);

  addProperty(prop_cont, &property, propertyIdentifier);
  return property;
}


Property * getProperty(PropertyContainer & prop_cont, String const & name)
{
  std::list<Property *>::iterator iter;

  iter = std::find_if(prop_cont.begin(),
                      prop_cont.end(),
                      [name](Property * p) -> bool
                      {
                        return (p->name() == name);
                      });

  if (iter == prop_cont.end())
    return nullptr;
  else
    return (*iter);
}

Property * getProperty(PropertyContainer & prop_cont, int const identifier)
{
  std::list<Property *>::iterator iter;

  iter = std::find_if(prop_cont.begin(),
                      prop_cont.end(),
                      [identifier](Property * p) -> bool
                      {
                        return (p->identifier() == identifier);
                      });

  if (iter == prop_cont.end())
    return nullptr;
  else
    return (*iter);
}

int appendChangedProperties(PropertyContainer & prop_cont, CborEncoder * arrayEncoder, bool lightPayload)
{
  int appendedProperties = 0;
  std::for_each(prop_cont.begin(),
                prop_cont.end(),
                [arrayEncoder, lightPayload, &appendedProperties](Property * p)
                {
                  if (p->shouldBeUpdated() && p->isReadableByCloud())
                  {
                    p->append(arrayEncoder, lightPayload);
                    appendedProperties++;
                  }
                });
  return appendedProperties;
}

void requestUpdateForAllProperties(PropertyContainer & prop_cont)
{
  std::for_each(prop_cont.begin(),
                prop_cont.end(),
                [](Property * p)
                {
                  p->requestUpdate();
                });
}

void updateTimestampOnLocallyChangedProperties(PropertyContainer & prop_cont)
{
  /* This function updates the timestamps on the primitive properties 
   * that have been modified locally since last cloud synchronization
   */
  std::for_each(prop_cont.begin(),
                prop_cont.end(),
                [](Property * p)
                {
                  CloudWrapperBase * pbase = reinterpret_cast<CloudWrapperBase *>(p);
                  if (pbase->isPrimitive() && pbase->isChangedLocally() && pbase->isReadableByCloud())
                  {
                    p->updateLocalTimestamp();
                  }
                });
}

/******************************************************************************
   PRIVATE MEMBER FUNCTIONS
 ******************************************************************************/

void addProperty(PropertyContainer & prop_cont, Property * property_obj, int propertyIdentifier)
{
  if (propertyIdentifier != -1)
  {
    property_obj->setIdentifier(propertyIdentifier);
  }
  /* If property identifier is -1, an incremental value will be assigned as identifier. */
  else
  {
    property_obj->setIdentifier(prop_cont.size() + 1); /* This is in order to stay compatible to the old system of first increasing _numProperties and then assigning it here. */
  }
  prop_cont.push_back(property_obj);
}