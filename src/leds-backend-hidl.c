/* leds-backend-hidl.c
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
 * variant, fbd-droid-leds-backend-hidl.c @ 7a6043d4794ae4d5df67b95a951f770a49191cdc
 *
 * Copyright 2021 Erfan Abdi
 * Copyright 2022 Eugenio "g7" Paolantonio
 *
 * The authors gave written permission for the license change for libdroid.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#define G_LOG_DOMAIN "fbd-droid-leds-backend-hidl"

#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gbinder.h>

#include "binder.h"
#include "leds-backend.h"
#include "leds-backend-hidl.h"

#define BINDER_LIGHT_DEFAULT_HIDL_DEVICE "/dev/hwbinder"

#define BINDER_LIGHT_HIDL_IFACE(v) "android.hardware.light@" v "::ILight"
#define BINDER_LIGHT_HIDL_SLOT_DEFAULT "default"
#define BINDER_LIGHT_HIDL_SLOT_LIBDROID "libdroid"

#define BINDER_LIGHT_HIDL_2_0_IFACE BINDER_LIGHT_HIDL_IFACE("2.0")

/* Methods */
enum
{
  /* setLight(Type type, LightState state) generates (Status status); */
  BINDER_LIGHT_HIDL_2_0_SET_LIGHT = 1,
  /* getSupportedTypes() generates (vec<Type> types); */
  BINDER_LIGHT_HIDL_2_0_GET_SUPPORTED_TYPES = 2,
};

struct _DroidLedsBackendHidl
{
  GObject parent_instance;

  GBinderServiceManager *service_manager;
  GBinderRemoteObject   *remote;
  GBinderClient         *client;
};

static void initable_interface_init (GInitableIface *iface);
static void droid_leds_backend_interface_init (DroidLedsBackendInterface *iface);

G_DEFINE_TYPE_WITH_CODE (DroidLedsBackendHidl, droid_leds_backend_hidl, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, initable_interface_init)
                         G_IMPLEMENT_INTERFACE (DROID_TYPE_LEDS_BACKEND,
                                                droid_leds_backend_interface_init))

static gboolean
droid_leds_backend_hidl_is_supported (DroidLedsBackend *backend,
                                      LightType         light_type)
{
  DroidLedsBackendHidl *self = DROID_LEDS_BACKEND_HIDL (backend);
  GBinderLocalRequest *req = gbinder_client_new_request (self->client);
  GBinderRemoteReply *reply;
  GBinderReader reader;
  int status;
  gsize count = 0, vecSize = 0;
  const int32_t *types;

  reply = gbinder_client_transact_sync_reply (self->client,
                                              BINDER_LIGHT_HIDL_2_0_GET_SUPPORTED_TYPES,
                                              req, &status);
  gbinder_local_request_unref (req);

  gbinder_remote_reply_init_reader (reply, &reader);

  if (status == GBINDER_STATUS_OK && binder_status_is_ok (&reader)) {
    types = gbinder_reader_read_hidl_vec(&reader, &count, &vecSize);
    for (int i = 0; i < count; i++) {
        if (types[i] == light_type) {
            g_debug ("Type %d usable", light_type);
            return TRUE;
        }
    }
    g_warning ("No suitable Light for type %d found", light_type);
  } else {
    g_warning ("Failed to get supported LED types");
  }

  return FALSE;
}

static gboolean
droid_leds_backend_hidl_set (DroidLedsBackend *backend,
                             uint32_t           color,
                             LightType         light_type,
                             FlashType         flash_type,
                             BrightnessType    brightness_type,
                             int32_t           flash_on_ms,
                             int32_t           flash_off_ms)
{
  DroidLedsBackendHidl *self = DROID_LEDS_BACKEND_HIDL (backend);
  GBinderLocalRequest *req = gbinder_client_new_request (self->client);
  GBinderRemoteReply *reply;
  GBinderWriter writer;
  LightState* notification_state;
  int32_t status;

  gbinder_local_request_init_writer (req, &writer);
  notification_state = gbinder_writer_new0 (&writer, LightState);
  notification_state->color = color;
  notification_state->flashMode = flash_type;
  notification_state->flashOnMs = flash_on_ms;
  notification_state->flashOffMs = flash_off_ms;
  notification_state->brightnessMode = brightness_type;

  gbinder_writer_append_int32 (&writer, light_type);
  gbinder_writer_append_buffer_object (&writer, notification_state,
    sizeof(*notification_state));

  reply = gbinder_client_transact_sync_reply (self->client,
                                              BINDER_LIGHT_HIDL_2_0_SET_LIGHT,
                                              req, &status);
  gbinder_local_request_unref (req);

  if (status == GBINDER_STATUS_OK && binder_reply_status_is_ok (reply)) {
    return TRUE;
  } else {
    g_warning ("Unable to turn to set notification LED");
    return FALSE;
  }
}


static gboolean
initable_init (GInitable     *initable,
               GCancellable  *cancellable,
               GError       **error)
{
  static const gchar *slots[] = {
    BINDER_LIGHT_HIDL_2_0_IFACE "/" BINDER_LIGHT_HIDL_SLOT_LIBDROID,
    BINDER_LIGHT_HIDL_2_0_IFACE "/" BINDER_LIGHT_HIDL_SLOT_DEFAULT,
  };
  DroidLedsBackendHidl *self = DROID_LEDS_BACKEND_HIDL (initable);
  gboolean success;

  g_debug ("Initializing droid leds hidl");

  for (int i=0; i < 2; i++)
    {
      success = binder_init (BINDER_LIGHT_DEFAULT_HIDL_DEVICE,
                             BINDER_LIGHT_HIDL_2_0_IFACE,
                             slots[i],
                             &self->service_manager,
                             &self->remote,
                             &self->client);

      if (success)
        break;
    }

  if (!success) {
    g_set_error (error,
                 G_IO_ERROR, G_IO_ERROR_FAILED,
                 "Failed to obtain suitable light hal");
    return FALSE;
  }

  return TRUE;
}


static void
droid_leds_backend_hidl_constructed (GObject *obj)
{
  DroidLedsBackendHidl *self = DROID_LEDS_BACKEND_HIDL (obj);

  G_OBJECT_CLASS (droid_leds_backend_hidl_parent_class)->constructed (obj);

  self->service_manager = NULL;
  self->remote = NULL;
  self->client = NULL;
}


static void
droid_leds_backend_hidl_dispose (GObject *obj)
{
  DroidLedsBackendHidl *self = DROID_LEDS_BACKEND_HIDL (obj);

  g_debug ("Disposing droid leds hidl");

  if (self->client) {
    gbinder_client_unref (self->client);
  }

  if (self->remote) {
    gbinder_remote_object_unref (self->remote);
  }

  if (self->service_manager) {
    gbinder_servicemanager_unref (self->service_manager);
  }

  G_OBJECT_CLASS (droid_leds_backend_hidl_parent_class)->dispose (obj);
}


static void
droid_leds_backend_hidl_class_init (DroidLedsBackendHidlClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = droid_leds_backend_hidl_constructed;
  object_class->dispose      = droid_leds_backend_hidl_dispose;
}


static void
initable_interface_init (GInitableIface *iface)
{
  iface->init = initable_init;
}


static void
droid_leds_backend_interface_init (DroidLedsBackendInterface *iface)
{
  iface->is_supported    = droid_leds_backend_hidl_is_supported;
  iface->set             = droid_leds_backend_hidl_set;
}


static void
droid_leds_backend_hidl_init (DroidLedsBackendHidl *self)
{
}


DroidLedsBackendHidl *
droid_leds_backend_hidl_new (GError **error)
{
  return DROID_LEDS_BACKEND_HIDL (
    g_initable_new (DROID_TYPE_LEDS_BACKEND_HIDL,
                    NULL,
                    error,
                    NULL));
}
