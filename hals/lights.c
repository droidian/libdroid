/* hal-lights.c
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
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <gudev/gudev.h>

#include <libdroid-shared/leds-objects.h>

#include "common/hal-service.h"
#include "common/hal-implementation.h"
#include "common/utils.h"

#define DROID_TYPE_HAL_LIGHTS droid_hal_lights_get_type ()
G_DECLARE_FINAL_TYPE (DroidHalLights, droid_hal_lights, DROID, HAL_LIGHTS, GObject)

#define BINDER_LIGHT_DEVICE "/dev/hwbinder"

#define BINDER_LIGHT_HIDL_IFACE(v) "android.hardware.light@" v "::ILight"
#define BINDER_LIGHT_HIDL_SLOT_LIBDROID "libdroid"

#define BINDER_LIGHT_HIDL_2_0_IFACE BINDER_LIGHT_HIDL_IFACE("2.0")

#define FALLBACK_RED_PATH       "/sys/class/leds/red"
#define FALLBACK_GREEN_PATH     "/sys/class/leds/green"
#define FALLBACK_BLUE_PATH      "/sys/class/leds/blue"

#define BRIGHTNESS_PATH(p)      p "/brightness"

#define TO_SYSFS_VALUE(n, max)  n * max / 255

typedef enum
{
  LIGHT_DEVICE_BLINK_TYPE_NONE = 0,
  LIGHT_DEVICE_BLINK_TYPE_BLINK,
  LIGHT_DEVICE_BLINK_TYPE_BREATH
} LightDeviceBlinkType;

typedef struct
{
  GUdevDevice          *device;
  guint                 max;
  LightDeviceBlinkType  blink_type;
} LightDevice;

struct _DroidHalLights
{
  GObject parent_instance;

  GUdevClient *udev;
  LightDevice *backlight_device;
  LightDevice *red_device;
  LightDevice *green_device;
  LightDevice *blue_device;
};

/* Methods */
enum
{
  /* setLight(Type type, LightState state) generates (Status status); */
  BINDER_LIGHT_HIDL_2_0_SET_LIGHT = 1,
  /* getSupportedTypes() generates (vec<Type> types); */
  BINDER_LIGHT_HIDL_2_0_GET_SUPPORTED_TYPES = 2,
};

static void droid_hal_lights_interface_init (DroidHalImplementationInterface *iface);

G_DEFINE_TYPE_WITH_CODE (DroidHalLights, droid_hal_lights, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (DROID_TYPE_HAL_IMPLEMENTATION,
                                                droid_hal_lights_interface_init))

static gboolean
udev_write_int (LightDevice *device,
                const gchar *component,
                gint         value)
{
  g_autofree gchar *path = NULL;
  g_autofree gchar *content = NULL;
  g_return_val_if_fail (device != NULL, FALSE);

  path = g_build_filename (g_udev_device_get_sysfs_path (device->device), component, NULL);
  content = g_strdup_printf ("%i", value);

  return droid_utils_write_to_file (path, content);
}

static gboolean
udev_blink (LightDevice *device,
            gboolean     enable)
{
  switch (device->blink_type)
    {
    case LIGHT_DEVICE_BLINK_TYPE_BREATH:
      return udev_write_int (device, "breath", enable);
    case LIGHT_DEVICE_BLINK_TYPE_BLINK:
      return udev_write_int (device, "blink", enable);
    case LIGHT_DEVICE_BLINK_TYPE_NONE:
    default:
      /* Unsupported */
      return TRUE;
    }
}

static LightDevice *
droid_leds_udev_new_device (GUdevDevice *device)
{
  LightDevice *light = g_new0 (LightDevice, 1);

  light->device = device;
  light->max    = g_udev_device_get_sysfs_attr_as_int (device, "max_brightness");

  if (g_udev_device_has_sysfs_attr (device, "breath"))
      light->blink_type = LIGHT_DEVICE_BLINK_TYPE_BREATH;
  else if (g_udev_device_has_sysfs_attr (device, "blink"))
      light->blink_type = LIGHT_DEVICE_BLINK_TYPE_BLINK;

  return light;
}

static void
droid_hal_lights_probe (DroidHalLights *self)
{
  static const gchar *known_backlight_paths[] = {
    "/sys/class/leds/lcd-backlight",
    "/sys/class/backlight/panel0-backlight",
  };
  g_autolist(GUdevDevice) backlight_list = NULL;
  GList *item;
  const gchar *backlight_type;
  gchar *path_to_check = NULL;

  for (int i=0; i < 2; i++)
    {
      path_to_check = g_build_filename (known_backlight_paths[i], "brightness", NULL);
      if (droid_utils_file_exists (path_to_check))
          self->backlight_device = droid_leds_udev_new_device (g_udev_client_query_by_sysfs_path (self->udev,
            known_backlight_paths[i]));
      g_free (path_to_check);
      if (self->backlight_device)
          break;
    }

  if (!self->backlight_device)
    {
      /* Try using udev */
      backlight_list = g_udev_client_query_by_subsystem (self->udev, "backlight");

      for (item = backlight_list; item != NULL; item = item->next)
        {
          backlight_type = g_udev_device_get_sysfs_attr (item->data, "type");
          g_debug ("Fallback to %s type %s (name %s)", g_udev_device_get_sysfs_path (item->data),
            backlight_type, g_udev_device_get_name (item->data));
          if (g_strcmp0 (backlight_type, "firmware") == 0 ||
              g_strcmp0 (backlight_type, "platform") == 0 ||
              g_strcmp0 (backlight_type, "raw") == 0)
            {
              path_to_check = g_build_filename (g_udev_device_get_sysfs_path (item->data),
                "brightness", NULL);
              if (droid_utils_file_exists (path_to_check))
                  self->backlight_device = droid_leds_udev_new_device (G_UDEV_DEVICE (g_object_ref (item->data)));
              g_free (path_to_check);
              if (self->backlight_device)
                  break;
            }
        }
    }


  if (droid_utils_file_exists (BRIGHTNESS_PATH (FALLBACK_RED_PATH)))
      self->red_device = droid_leds_udev_new_device (g_udev_client_query_by_sysfs_path (self->udev,
        FALLBACK_RED_PATH));

  if (droid_utils_file_exists (BRIGHTNESS_PATH (FALLBACK_GREEN_PATH)))
      self->green_device = droid_leds_udev_new_device (g_udev_client_query_by_sysfs_path (self->udev,
        FALLBACK_GREEN_PATH));

  if (droid_utils_file_exists (BRIGHTNESS_PATH (FALLBACK_BLUE_PATH)))
      self->blue_device = droid_leds_udev_new_device (g_udev_client_query_by_sysfs_path (self->udev,
        FALLBACK_BLUE_PATH));
}

static gboolean
droid_hal_lights_set (DroidHalLights *backend,
                      uint32_t        color,
                      LightType       light_type,
                      FlashType       flash_type,
                      BrightnessType  backlight_type,
                      int32_t         flash_on_ms,
                      int32_t         flash_off_ms)
{
  DroidHalLights *self = DROID_HAL_LIGHTS (backend);
  gint value, red, green, blue;
  gboolean result = FALSE;

  g_return_val_if_fail (DROID_IS_HAL_LIGHTS (self), FALSE);

  if (light_type == LIGHT_TYPE_BACKLIGHT && self->backlight_device != NULL)
    {
      value = ((77 * ((color >> 16) & 0x00ff)) +
        (150 * ((color >> 8) & 0x00ff)) + (29 * (color & 0x00ff))) >> 8;

      g_debug ("backlight: got backlight change request: %d", value);

      result = udev_write_int (self->backlight_device, "brightness",
        TO_SYSFS_VALUE (value, self->backlight_device->max));
    }
  else if (light_type == LIGHT_TYPE_NOTIFICATIONS)
    {
      red = (color >> 16) & 0xff;
      green = (color >> 8) & 0xff;
      blue = color & 0xff;

      g_debug ("notification: got change request: r %d g %d b %d", red, green, blue);

      if (self->red_device != NULL &&
          udev_write_int (self->red_device, "brightness", TO_SYSFS_VALUE (red, self->red_device->max)) &&
          udev_blink (self->red_device, (red > 0)))
        {
          result = TRUE;
        }

      if (self->green_device != NULL &&
          udev_write_int (self->green_device, "brightness", TO_SYSFS_VALUE (green, self->green_device->max)) &&
          udev_blink (self->green_device, (green > 0)))
        {
          result = TRUE;
        }

      if (self->blue_device != NULL &&
          udev_write_int (self->blue_device, "brightness", TO_SYSFS_VALUE (blue, self->blue_device->max)) &&
          udev_blink (self->blue_device, (blue > 0)))
        {
          result = TRUE;
        }
    }

  return result;
}


static GBinderLocalReply *
droid_hal_lights_reply (DroidHalImplementation *implementation,
                        GBinderLocalObject     *object,
                        GBinderRemoteRequest   *request,
                        guint                   code)
{
  DroidHalLights *self = DROID_HAL_LIGHTS (implementation);
  GBinderReader reader;
  GBinderWriter writer;
  GBinderLocalReply *reply = NULL;

  g_return_val_if_fail (DROID_IS_HAL_LIGHTS (self), NULL);

  switch (code)
    {
    case BINDER_LIGHT_HIDL_2_0_SET_LIGHT:
      GBinderBuffer *buf;
      LightState *notification_state;
      gint32 light_type;

      reply = gbinder_local_object_new_reply (object);
      gbinder_remote_request_init_reader (request, &reader);
      gbinder_local_reply_init_writer (reply, &writer);

      if (gbinder_reader_read_int32 (&reader, &light_type) &&
            (buf = gbinder_reader_read_buffer (&reader)) != NULL)
        {
          notification_state = buf->data;
          gbinder_writer_append_int32(&writer, GBINDER_STATUS_OK);
          droid_hal_lights_set (self, notification_state->color, (LightType) light_type,
            notification_state->flashMode, notification_state->brightnessMode,
            notification_state->flashOnMs, notification_state->flashOffMs);
        }
      else
        {
          gbinder_writer_append_int32(&writer, GBINDER_STATUS_FAILED);
        }
      break;

    case BINDER_LIGHT_HIDL_2_0_GET_SUPPORTED_TYPES:
      LightType supported[LIGHT_TYPE_COUNT];
      gint count = 0;

      reply = gbinder_local_object_new_reply (object);
      gbinder_local_reply_init_writer (reply, &writer);

      if (self->backlight_device != NULL)
          supported[count++] = LIGHT_TYPE_BACKLIGHT;

      if (self->red_device != NULL || self->green_device != NULL ||
        self->blue_device != NULL)
          supported[count++] = LIGHT_TYPE_NOTIFICATIONS;

      gbinder_writer_append_int32(&writer, GBINDER_STATUS_OK);
      gbinder_writer_append_hidl_vec (&writer, supported, count, sizeof(LightType));
      break;

    default:
      g_warning ("Unknown code %d", code);
      break;

    }

  return reply;
}

static void
droid_hal_lights_constructed (GObject *obj)
{
  DroidHalLights *self = DROID_HAL_LIGHTS (obj);

  G_OBJECT_CLASS (droid_hal_lights_parent_class)->constructed (obj);

  self->udev = g_udev_client_new (NULL);
  self->backlight_device = NULL;
  self->red_device = NULL;
  self->green_device = NULL;
  self->blue_device = NULL;

  droid_hal_lights_probe (self);
}

static void
droid_hal_lights_dispose (GObject *obj)
{
  DroidHalLights *self = DROID_HAL_LIGHTS (obj);

  G_OBJECT_CLASS (droid_hal_lights_parent_class)->dispose (obj);

  g_clear_object (&self->udev);

  if (self->backlight_device != NULL)
      g_free (self->backlight_device);

  if (self->red_device != NULL)
      g_free (self->red_device);

  if (self->green_device != NULL)
      g_free (self->green_device);

  if (self->blue_device != NULL)
      g_free (self->blue_device);
}

static void
droid_hal_lights_class_init (DroidHalLightsClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = droid_hal_lights_constructed;
  object_class->dispose      = droid_hal_lights_dispose;
}


static void
droid_hal_lights_interface_init (DroidHalImplementationInterface *iface)
{
  iface->reply = droid_hal_lights_reply;
}

static void
droid_hal_lights_init (DroidHalLights *self)
{
}

static DroidHalLights *
droid_hal_lights_new (void)
{
  return DROID_HAL_LIGHTS (
    g_object_new (DROID_TYPE_HAL_LIGHTS, NULL));
}


int
main (int argc, char** argv)
{
  g_autoptr (GError) err = NULL;
  g_autoptr (GOptionContext) context = NULL;
  g_autoptr (DroidHalService) service = NULL;
  g_autoptr (DroidHalLights) lights = NULL;

  g_autofree gchar *device = NULL;
  g_autofree gchar *iface = NULL;
  g_autofree gchar *name = NULL;
  const GOptionEntry entries[] = {
    {
      "device", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &device,
      "The device name, defaults to " BINDER_LIGHT_DEVICE, NULL,
    },
    {
      "iface", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &iface,
      "The interface name to use, defaults to " BINDER_LIGHT_HIDL_2_0_IFACE, NULL,
    },
    {
      "name", 'n', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &name,
      "The slot name to use, defaults to " BINDER_LIGHT_HIDL_SLOT_LIBDROID, NULL,
    },
    {NULL},
  };

  context = g_option_context_new (NULL);
  g_option_context_add_main_entries(context, entries, NULL);
  g_option_context_parse(context, &argc, &argv, &err);

  if (err != NULL)
    {
      g_error ("Unable to parse arguments: %s", err->message);
      return EXIT_FAILURE;
    }

  if (device == NULL)
      device = g_strdup (BINDER_LIGHT_DEVICE);

  if (iface == NULL)
      iface = g_strdup (BINDER_LIGHT_HIDL_2_0_IFACE);

  if (name == NULL)
      name = g_strdup (BINDER_LIGHT_HIDL_SLOT_LIBDROID);

  lights = droid_hal_lights_new ();
  service = droid_hal_service_new ((DroidHalImplementation *)lights,
    device, iface, name);

  return droid_hal_service_run (service);
}
