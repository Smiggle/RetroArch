/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2010-2014 - Hans-Kristian Arntzen
 *  Copyright (C) 2011-2017 - Daniel De Matteis
 *
 *  RetroArch is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  RetroArch is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with RetroArch.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "location_driver.h"

#include "../configuration.h"
#include "../core.h"
#include "../driver.h"
#include "../retroarch.h"
#include "../list_special.h"
#include "../verbosity.h"

static const location_driver_t *location_drivers[] = {
#ifdef ANDROID
   &location_android,
#endif
   &location_null,
   NULL,
};

static const location_driver_t *location_driver;
static void *location_data;

/**
 * location_driver_find_handle:
 * @idx                : index of driver to get handle to.
 *
 * Returns: handle to location driver at index. Can be NULL
 * if nothing found.
 **/
const void *location_driver_find_handle(int idx)
{
   const void *drv = location_drivers[idx];
   if (!drv)
      return NULL;
   return drv;
}

/**
 * location_driver_find_ident:
 * @idx                : index of driver to get handle to.
 *
 * Returns: Human-readable identifier of location driver at index. Can be NULL
 * if nothing found.
 **/
const char *location_driver_find_ident(int idx)
{
   const location_driver_t *drv = location_drivers[idx];
   if (!drv)
      return NULL;
   return drv->ident;
}

/**
 * config_get_location_driver_options:
 *
 * Get an enumerated list of all location driver names,
 * separated by '|'.
 *
 * Returns: string listing of all location driver names,
 * separated by '|'.
 **/
const char* config_get_location_driver_options(void)
{
   return char_list_new_special(STRING_LIST_LOCATION_DRIVERS, NULL);
}

void find_location_driver(void)
{
   int i;
   driver_ctx_info_t drv;
   settings_t *settings = config_get_ptr();

   drv.label = "location_driver";
   drv.s     = settings->arrays.location_driver;

   driver_ctl(RARCH_DRIVER_CTL_FIND_INDEX, &drv);

   i         = (int)drv.len;

   if (i >= 0)
      location_driver = (const location_driver_t*)location_driver_find_handle(i);
   else
   {

      if (verbosity_is_enabled())
      {
         unsigned d;
         RARCH_ERR("Couldn't find any location driver named \"%s\"\n",
               settings->arrays.location_driver);
         RARCH_LOG_OUTPUT("Available location drivers are:\n");
         for (d = 0; location_driver_find_handle(d); d++)
            RARCH_LOG_OUTPUT("\t%s\n", location_driver_find_ident(d));

         RARCH_WARN("Going to default to first location driver...\n");
      }

      location_driver = (const location_driver_t*)location_driver_find_handle(0);

      if (!location_driver)
         retroarch_fail(1, "find_location_driver()");
   }
}

/**
 * driver_location_start:
 *
 * Starts location driver interface..
 * Used by RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE.
 *
 * Returns: true (1) if successful, otherwise false (0).
 **/
bool driver_location_start(void)
{
   settings_t *settings = config_get_ptr();

   if (location_driver && location_data && location_driver->start)
   {
      if (settings->bools.location_allow)
         return location_driver->start(location_data);

      runloop_msg_queue_push("Location is explicitly disabled.\n", 1, 180, true, NULL, MESSAGE_QUEUE_ICON_DEFAULT, MESSAGE_QUEUE_CATEGORY_INFO);
   }
   return false;
}

/**
 * driver_location_stop:
 *
 * Stops location driver interface..
 * Used by RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE.
 *
 * Returns: true (1) if successful, otherwise false (0).
 **/
void driver_location_stop(void)
{
   if (location_driver && location_driver->stop && location_data)
      location_driver->stop(location_data);
}

/**
 * driver_location_set_interval:
 * @interval_msecs     : Interval time in milliseconds.
 * @interval_distance  : Distance at which to update.
 *
 * Sets interval update time for location driver interface.
 * Used by RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE.
 **/
void driver_location_set_interval(unsigned interval_msecs,
      unsigned interval_distance)
{
   if (location_driver && location_driver->set_interval
         && location_data)
      location_driver->set_interval(location_data,
            interval_msecs, interval_distance);
}

/**
 * driver_location_get_position:
 * @lat                : Latitude of current position.
 * @lon                : Longitude of current position.
 * @horiz_accuracy     : Horizontal accuracy.
 * @vert_accuracy      : Vertical accuracy.
 *
 * Gets current positioning information from
 * location driver interface.
 * Used by RETRO_ENVIRONMENT_GET_LOCATION_INTERFACE.
 *
 * Returns: bool (1) if successful, otherwise false (0).
 **/
bool driver_location_get_position(double *lat, double *lon,
      double *horiz_accuracy, double *vert_accuracy)
{
   if (location_driver && location_driver->get_position
         && location_data)
      return location_driver->get_position(location_data,
            lat, lon, horiz_accuracy, vert_accuracy);

   *lat = 0.0;
   *lon = 0.0;
   *horiz_accuracy = 0.0;
   *vert_accuracy = 0.0;
   return false;
}

void init_location(void)
{
   rarch_system_info_t *system = runloop_get_system_info();

   /* Resource leaks will follow if location interface is initialized twice. */
   if (location_data)
      return;

   find_location_driver();

   location_data = location_driver->init();

   if (!location_data)
   {
      RARCH_ERR("Failed to initialize location driver. Will continue without location.\n");
      location_driver_ctl(RARCH_LOCATION_CTL_UNSET_ACTIVE, NULL);
   }

   if (system->location_cb.initialized)
      system->location_cb.initialized();
}

static void uninit_location(void)
{
   rarch_system_info_t *system = runloop_get_system_info();

   if (location_data && location_driver)
   {
      if (system->location_cb.deinitialized)
         system->location_cb.deinitialized();

      if (location_driver->free)
         location_driver->free(location_data);
   }

   location_data = NULL;
}

bool location_driver_ctl(enum rarch_location_ctl_state state, void *data)
{
   static bool location_driver_active              = false;
   static bool location_driver_data_own            = false;

   switch (state)
   {
      case RARCH_LOCATION_CTL_DESTROY:
         location_driver_active    = false;
         location_driver_data_own  = false;
         location_driver           = NULL;
         break;
      case RARCH_LOCATION_CTL_DEINIT:
         uninit_location();
         break;
      case RARCH_LOCATION_CTL_SET_OWN_DRIVER:
         location_driver_data_own = true;
         break;
      case RARCH_LOCATION_CTL_UNSET_OWN_DRIVER:
         location_driver_data_own = false;
         break;
      case RARCH_LOCATION_CTL_OWNS_DRIVER:
         return location_driver_data_own;
      case RARCH_LOCATION_CTL_SET_ACTIVE:
         location_driver_active = true;
         break;
      case RARCH_LOCATION_CTL_UNSET_ACTIVE:
         location_driver_active = false;
         break;
      case RARCH_LOCATION_CTL_IS_ACTIVE:
        return location_driver_active;
      default:
         break;
   }

   return true;
}
