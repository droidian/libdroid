/* leds.c
 *
 * Copyright 2024 Eugenio "g7" Paolantonio <me@medesimo.eu>
 *
 * You may use this file under the terms of the BSD license as follows:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *   1. Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *   3. Neither the names of the copyright holders nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * any official policies, either expressed or implied.
 *
 * This file is based on the original integration in Droidian's feedbackd
 * variant, fbd-droid-leds.c @ 7a6043d4794ae4d5df67b95a951f770a49191cdc
 *
 * Copyright 2021 Giuseppe Corti
 * Copyright 2021 Erfan Abdi
 * Copyright 2022 Eugenio "g7" Paolantonio
 *
 * The authors gave written permission for the license change for libdroid.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define G_LOG_DOMAIN "droid-leds"

#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <libdroid/leds.h>

#include "settings.h"

#include "leds-backend.h"
#include "leds-backend-aidl.h"
#include "leds-backend-hidl.h"

#define BACKLIGHT_MAX                     255
#define LIBDROID_LEDS_BACKLIGHT_LEVEL_KEY "backlight-level"

struct _DroidLeds
{
  GObject           parent_instance;

  DroidLedsBackend *backend;
  GSettings        *settings;
  gboolean          backlight_supported;
  gboolean          notifications_supported;
};

G_DEFINE_FINAL_TYPE (DroidLeds, droid_leds, G_TYPE_OBJECT)


gboolean
droid_leds_set_backlight (DroidLeds *self,
                          guint      level,
                          gboolean   save)
{
  uint32_t brightness;

  if (!DROID_IS_LEDS (self) || !self->backlight_supported)
    return FALSE;

  level = MIN(level, BACKLIGHT_MAX);
  brightness = (0xff << 24) + (level << 16) + (level << 8) + level;

  if (!droid_leds_backend_set (self->backend, brightness, LIGHT_TYPE_BACKLIGHT,
    FLASH_TYPE_HARDWARE, BRIGHTNESS_MODE_USER, 0, 0))
    return FALSE;

  if (save)
    g_settings_set_uint (self->settings, LIBDROID_LEDS_BACKLIGHT_LEVEL_KEY,
      level);

  return TRUE;
}


guint
droid_leds_get_backlight (DroidLeds *self)
{
  g_return_val_if_fail (DROID_IS_LEDS (self), BACKLIGHT_MAX);

  return g_settings_get_uint (self->settings, LIBDROID_LEDS_BACKLIGHT_LEVEL_KEY);
}


gboolean
droid_leds_set_notification (DroidLeds *self,
                             uint32_t color,
                             int32_t flash_on_ms,
                             int32_t flash_off_ms)
{
  if (!DROID_IS_LEDS (self) || !self->notifications_supported)
    return FALSE;

  return droid_leds_backend_set (self->backend, color, LIGHT_TYPE_NOTIFICATIONS,
    FLASH_TYPE_NONE, BRIGHTNESS_MODE_USER, flash_on_ms, flash_off_ms);
}


gboolean
droid_leds_clear_notification (DroidLeds *self)
{
  if (!DROID_IS_LEDS (self) || !self->notifications_supported)
    return FALSE;

  return droid_leds_backend_set (self->backend, 0, LIGHT_TYPE_NOTIFICATIONS,
    FLASH_TYPE_NONE, BRIGHTNESS_MODE_USER, 0, 0);
}


gboolean
droid_leds_is_kind_supported (DroidLeds *self,
                              DroidLedsKind kind)
{
  g_return_val_if_fail (DROID_IS_LEDS (self), FALSE);

  switch (kind)
    {
    case DROID_LEDS_KIND_BACKLIGHT:
      return self->backlight_supported;
    case DROID_LEDS_KIND_NOTIFICATION:
      return self->notifications_supported;
    default:
      return FALSE;
    }
}

static DroidLedsBackend *
droid_leds_create_backend (void)
{
  g_autoptr (GError) error = NULL;
  DroidLedsBackend *backend;

  backend = (DroidLedsBackend *) droid_leds_backend_aidl_new (&error);

  if (!backend)
    {
      backend = (DroidLedsBackend *) droid_leds_backend_hidl_new (&error);

      if (!backend)
        return NULL;
    }

  return backend;
}

static void
droid_leds_constructed (GObject *obj)
{
  DroidLeds *self = DROID_LEDS (obj);

  G_OBJECT_CLASS (droid_leds_parent_class)->constructed (obj);

  self->settings = droid_settings_get_default ();

  self->backend = droid_leds_create_backend ();

  if (self->backend)
    {
      self->backlight_supported = droid_leds_backend_is_supported (self->backend,
        LIGHT_TYPE_BACKLIGHT);
      self->notifications_supported = droid_leds_backend_is_supported (self->backend,
        LIGHT_TYPE_NOTIFICATIONS);
    }
  else
    {
      self->backlight_supported = FALSE;
      self->notifications_supported = FALSE;
    }
}


static void
droid_leds_dispose (GObject *obj)
{
  DroidLeds *self = DROID_LEDS (obj);

  g_debug ("Disposing droid leds");

  g_clear_object (&self->backend);
  g_clear_object (&self->settings);

  G_OBJECT_CLASS (droid_leds_parent_class)->dispose (obj);
}


static void
droid_leds_class_init (DroidLedsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = droid_leds_constructed;
  object_class->dispose      = droid_leds_dispose;
}


static void
droid_leds_init (DroidLeds *self)
{
}


DroidLeds *
droid_leds_new (void)
{
  return DROID_LEDS (
    g_object_new (DROID_TYPE_LEDS, NULL));
}
