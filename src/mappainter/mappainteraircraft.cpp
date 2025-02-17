/*****************************************************************************
* Copyright 2015-2020 Alexander Barthel alex@littlenavmap.org
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "mappainter/mappainteraircraft.h"

#include "mapgui/mapwidget.h"
#include "navapp.h"
#include "online/onlinedatacontroller.h"
#include "mapgui/mapfunctions.h"
#include "util/paintercontextsaver.h"
#include "geo/calculations.h"

#include <marble/GeoPainter.h>
#include <marble/ViewportParams.h>

using atools::fs::sc::SimConnectAircraft;

const int NUM_CLOSEST_AI_LABELS = 5;
const float DIST_METER_CLOSEST_AI_LABELS = atools::geo::nmToMeter(20);
const float DIST_FT_CLOSEST_AI_LABELS = 5000;

MapPainterAircraft::MapPainterAircraft(MapPaintWidget *mapWidget, MapScale *mapScale, PaintContext *paintContext)
  : MapPainterVehicle(mapWidget, mapScale, paintContext)
{

}

MapPainterAircraft::~MapPainterAircraft()
{

}

void MapPainterAircraft::render()
{
  atools::util::PainterContextSaver saver(context->painter);
  const atools::fs::sc::SimConnectUserAircraft& userAircraft = mapPaintWidget->getUserAircraft();

  if(context->objectTypes & map::AIRCRAFT_ALL)
  {
    const atools::geo::Pos& pos = userAircraft.getPosition();

    // Draw AI and online aircraft - not boats ====================================================================
    bool onlineEnabled = context->objectTypes.testFlag(map::AIRCRAFT_ONLINE) && NavApp::isOnlineNetworkActive();
    bool aiEnabled = context->objectTypes.testFlag(map::AIRCRAFT_AI) && NavApp::isConnected();
    if(aiEnabled || onlineEnabled)
    {
      bool overflow = false;

      // Merge simulator aircraft and online aircraft
      QVector<const atools::fs::sc::SimConnectAircraft *> allAircraft;

      // Get all pure (slowly updated) online aircraft ======================================
      if(onlineEnabled)
      {
        // Filters duplicates from simulator and user aircraft out - remove shadow aircraft
        const QList<atools::fs::sc::SimConnectAircraft> *onlineAircraft =
          NavApp::getOnlinedataController()->getAircraft(context->viewport->viewLatLonAltBox(),
                                                         context->mapLayer, context->lazyUpdate, overflow);

        context->setQueryOverflow(overflow);

        for(const atools::fs::sc::SimConnectAircraft& ac : *onlineAircraft)
          allAircraft.append(&ac);
      }

      // Get all AI and online shadow aircraft ======================================
      for(const SimConnectAircraft& ac : mapPaintWidget->getAiAircraft())
      {
        // Skip boats
        if(ac.isAnyBoat())
          continue;

        // Skip shadow aircraft if online is disabled
        if(!onlineEnabled && ac.isOnlineShadow())
          continue;

        // Skip AI aircraft (means not shadow) if AI is disabled
        if(!aiEnabled && !ac.isOnlineShadow())
          continue;

        allAircraft.append(&ac);
      }

      // Sort by distance to user aircraft
      struct AiDistType
      {
        const SimConnectAircraft *aircraft;
        float distanceLateralMeter, distanceVerticalFt;
      };

      QVector<AiDistType> aiSorted;

      for(const SimConnectAircraft *ac : allAircraft)
        aiSorted.append({ac,
                         pos.distanceMeterTo(ac->getPosition()),
                         std::abs(pos.getAltitude() - ac->getActualAltitudeFt())});

      std::sort(aiSorted.begin(), aiSorted.end(), [](const AiDistType& ai1,
                                                     const AiDistType& ai2) -> bool
      {
        // returns ​true if the first argument is less than (i.e. is ordered before) the second.
        return ai1.distanceLateralMeter > ai2.distanceLateralMeter;
      });

      int num = aiSorted.size();
      for(const AiDistType& adt : aiSorted)
      {
        const SimConnectAircraft& ac = *adt.aircraft;
        if(mapfunc::aircraftVisible(ac, context->mapLayer))
        {
          paintAiVehicle(ac, --num < NUM_CLOSEST_AI_LABELS &&
                         adt.distanceLateralMeter < DIST_METER_CLOSEST_AI_LABELS &&
                         adt.distanceVerticalFt < DIST_FT_CLOSEST_AI_LABELS);
        }
      }
    }

    // Draw user aircraft ====================================================================
    if(context->objectTypes.testFlag(map::AIRCRAFT))
    {
      if(pos.isValid())
      {
        bool hidden = false;
        float x, y;
        if(wToS(pos, x, y, DEFAULT_WTOS_SIZE, &hidden))
        {
          if(!hidden)
            paintUserAircraft(userAircraft, x, y);
        }
      }
    }
  } // if(context->objectTypes & map::AIRCRAFT_ALL)

  // Wind display depends only on option
  if(context->dOptUserAc(optsac::ITEM_USER_AIRCRAFT_WIND_POINTER) && userAircraft.isValid())
    paintWindPointer(userAircraft, context->painter->device()->width() / 2, 0);
}
