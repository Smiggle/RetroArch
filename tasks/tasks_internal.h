/*  RetroArch - A frontend for libretro.
 *  Copyright (C) 2011-2017 - Higor Euripedes
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
#ifndef TASKS_HANDLER_INTERNAL_H
#define TASKS_HANDLER_INTERNAL_H

#include <stdint.h>

#include <boolean.h>
#include <retro_common_api.h>
#include <retro_miscellaneous.h>

#include <queues/task_queue.h>

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

RETRO_BEGIN_DECLS

#ifdef HAVE_NETWORKING
typedef struct
{
   char *data;
   size_t len;
} http_transfer_data_t;

void *task_push_http_transfer(const char *url, bool mute, const char *type,
      retro_task_callback_t cb, void *userdata);

void *task_push_http_post_transfer(const char *url, const char *post_data, bool mute, const char *type,
      retro_task_callback_t cb, void *userdata);

task_retriever_info_t *http_task_get_transfer_list(void);

bool task_push_wifi_scan(retro_task_callback_t cb);

bool task_push_netplay_lan_scan(retro_task_callback_t cb);

bool task_push_netplay_crc_scan(uint32_t crc, char* name,
      const char *hostname, const char *corename, const char* subsystem);

bool task_push_netplay_lan_scan_rooms(retro_task_callback_t cb);

bool task_push_netplay_nat_traversal(void *nat_traversal_state, uint16_t port);

#endif

bool task_push_image_load(const char *fullpath,
      retro_task_callback_t cb, void *userdata);

#ifdef HAVE_LIBRETRODB
bool task_push_dbscan(
      const char *playlist_directory,
      const char *content_database,
      const char *fullpath,
      bool directory, bool show_hidden_files,
      retro_task_callback_t cb);
#endif

#ifdef HAVE_OVERLAY
bool task_push_overlay_load_default(
        retro_task_callback_t cb, void *user_data);
#endif

bool task_check_decompress(const char *source_file);

bool task_push_decompress(
      const char *source_file,
      const char *target_dir,
      const char *target_file,
      const char *subdir,
      const char *valid_ext,
      retro_task_callback_t cb,
      void *user_data,
      void *frontend_userdata);

void task_file_load_handler(retro_task_t *task);

bool take_screenshot(const char *path, bool silence,
      bool has_valid_framebuffer, bool fullpath, bool use_thread);

bool event_load_save_files(void);

bool event_save_files(void);

void path_init_savefile_rtc(const char *savefile_path);

void *savefile_ptr_get(void);

void path_init_savefile_new(void);

bool input_is_autoconfigured(unsigned i);

unsigned input_autoconfigure_get_device_name_index(unsigned i);

void input_autoconfigure_reset(void);

bool input_autoconfigure_connect(
      const char *name,
      const char *display_name,
      const char *driver,
      unsigned idx,
      unsigned vid,
      unsigned pid);

bool input_autoconfigure_disconnect(unsigned i, const char *ident);

bool input_autoconfigure_get_swap_override(void);

void input_autoconfigure_joypad_reindex_devices(void);

void set_save_state_in_background(bool state);

extern const char* const input_builtin_autoconfs[];

RETRO_END_DECLS

#endif
