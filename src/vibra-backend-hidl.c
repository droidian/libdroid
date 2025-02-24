/* vibra-backend-hidl.c
 *
 * Copyright 2025 Eugenio "g7" Paolantonio <me@medesimo.eu>
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
 * variant, fbd-droid-vibra-backend-hidl.c @ 831b36b5932180c4145d6d3bf0c70f16dfce76eb
 *
 * Copyright 2021 Erfan Abdi
 * Copyright 2022 Eugenio "g7" Paolantonio
 *
 * The authors gave written permission for the license change for libdroid.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#define G_LOG_DOMAIN "fbd-droid-vibra-backend-hidl"

#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>
#include <gbinder.h>

#include "binder.h"
#include "vibra-backend.h"
#include "vibra-backend-hidl.h"

#define BINDER_VIBRATOR_DEFAULT_HIDL_DEVICE "/dev/hwbinder"

#define BINDER_VIBRATOR_HIDL_IFACE(v) "android.hardware.vibrator@" v "::IVibrator"
#define BINDER_VIBRATOR_HIDL_SLOT "default"

#define BINDER_VIBRATOR_HIDL_1_0_IFACE BINDER_VIBRATOR_HIDL_IFACE("1.0")

/* Methods */
enum
{
  /* on(uint32_t timeoutMs) generates (Status vibratorOnRet); */
  BINDER_VIBRATOR_HIDL_1_0_ON = 1,
  /* off() generates (Status vibratorOffRet); */
  BINDER_VIBRATOR_HIDL_1_0_OFF = 2,
};

struct _DroidVibraBackendHidl
{
  GObject parent_instance;

  GBinderServiceManager *service_manager;
  GBinderRemoteObject   *remote;
  GBinderClient         *client;
};

static void initable_interface_init (GInitableIface *iface);
static void droid_vibra_backend_interface_init (DroidVibraBackendInterface *iface);

G_DEFINE_TYPE_WITH_CODE (DroidVibraBackendHidl, droid_vibra_backend_hidl, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, initable_interface_init)
                         G_IMPLEMENT_INTERFACE (DROID_TYPE_VIBRA_BACKEND,
                                                droid_vibra_backend_interface_init))

static gboolean
droid_vibra_backend_hidl_on (DroidVibraBackend *backend,
                             int32_t            duration)
{
  DroidVibraBackendHidl *self = DROID_VIBRA_BACKEND_HIDL (backend);
  GBinderLocalRequest *req = gbinder_client_new_request (self->client);
  GBinderRemoteReply *reply;
  int32_t status;

  gbinder_local_request_append_int32 (req, duration); /* duration */

  reply = gbinder_client_transact_sync_reply (self->client,
                                              BINDER_VIBRATOR_HIDL_1_0_ON,
                                              req, &status);
  gbinder_local_request_unref (req);

  if (status == GBINDER_STATUS_OK && binder_reply_status_is_ok (reply)) {
    return TRUE;
  } else {
    g_warning ("Unable to turn the vibrator on");
    return FALSE;
  }
}


static gboolean
droid_vibra_backend_hidl_off (DroidVibraBackend *backend)
{
  DroidVibraBackendHidl *self = DROID_VIBRA_BACKEND_HIDL (backend);
  GBinderLocalRequest *req = gbinder_client_new_request (self->client);
  GBinderRemoteReply *reply;
  int32_t status;

  reply = gbinder_client_transact_sync_reply (self->client,
                                              BINDER_VIBRATOR_HIDL_1_0_OFF,
                                              req, &status);
  gbinder_local_request_unref (req);

  if (status == GBINDER_STATUS_OK && binder_reply_status_is_ok (reply)) {
    return TRUE;
  } else {
    g_warning ("Unable to turn the vibrator off");
    return FALSE;
  }
}

static gboolean
initable_init (GInitable     *initable,
               GCancellable  *cancellable,
               GError       **error)
{
  DroidVibraBackendHidl *self = DROID_VIBRA_BACKEND_HIDL (initable);
  gboolean success;

  g_debug ("Initializing droid vibra hidl");

  success = binder_init (BINDER_VIBRATOR_DEFAULT_HIDL_DEVICE,
                         BINDER_VIBRATOR_HIDL_1_0_IFACE,
                         (BINDER_VIBRATOR_HIDL_1_0_IFACE "/" BINDER_VIBRATOR_HIDL_SLOT),
                         &self->service_manager,
                         &self->remote,
                         &self->client);

  if (!success) {
    g_set_error (error,
                 G_IO_ERROR, G_IO_ERROR_FAILED,
                 "Failed to obtain suitable vibrator hal");
    return FALSE;
  }

  return TRUE;
}


static void
droid_vibra_backend_hidl_constructed (GObject *obj)
{
  DroidVibraBackendHidl *self = DROID_VIBRA_BACKEND_HIDL (obj);

  G_OBJECT_CLASS (droid_vibra_backend_hidl_parent_class)->constructed (obj);

  self->service_manager = NULL;
  self->remote = NULL;
  self->client = NULL;
}


static void
droid_vibra_backend_hidl_dispose (GObject *obj)
{
  DroidVibraBackendHidl *self = DROID_VIBRA_BACKEND_HIDL (obj);

  g_debug ("Disposing droid vibra hidl");

  if (self->client) {
    gbinder_client_unref (self->client);
  }

  if (self->remote) {
    gbinder_remote_object_unref (self->remote);
  }

  if (self->service_manager) {
    gbinder_servicemanager_unref (self->service_manager);
  }

  G_OBJECT_CLASS (droid_vibra_backend_hidl_parent_class)->dispose (obj);
}


static void
droid_vibra_backend_hidl_class_init (DroidVibraBackendHidlClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = droid_vibra_backend_hidl_constructed;
  object_class->dispose      = droid_vibra_backend_hidl_dispose;
}


static void
initable_interface_init (GInitableIface *iface)
{
  iface->init = initable_init;
}


static void
droid_vibra_backend_interface_init (DroidVibraBackendInterface *iface)
{
  iface->on  = droid_vibra_backend_hidl_on;
  iface->off = droid_vibra_backend_hidl_off;
}


static void
droid_vibra_backend_hidl_init (DroidVibraBackendHidl *self)
{
}


DroidVibraBackendHidl *
droid_vibra_backend_hidl_new (GError **error)
{
  return DROID_VIBRA_BACKEND_HIDL (
    g_initable_new (DROID_TYPE_VIBRA_BACKEND_HIDL,
                    NULL,
                    error,
                    NULL));
}
