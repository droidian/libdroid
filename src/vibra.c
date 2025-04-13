/* vibra.c
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
 * variant, fbd-droid-vibra.c @ 6a38c0555d07d5da7ad64c16c663dc416191b3c5
 *
 * Copyright 2021 Giuseppe Corti
 * Copyright 2021 Erfan Abdi
 * Copyright 2022 Eugenio "g7" Paolantonio
 *
 * The authors gave written permission for the license change for libdroid.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define G_LOG_DOMAIN "droid-vibra"

#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <gio/gio.h>

#include <libdroid/vibra.h>

#include "vibra-backend.h"
#include "vibra-backend-aidl.h"
#include "vibra-backend-hidl.h"


struct _DroidVibra
{
  GObject           parent_instance;

  DroidVibraBackend *backend;
};

static void initable_interface_init (GInitableIface *iface);

G_DEFINE_TYPE_WITH_CODE (DroidVibra, droid_vibra, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (G_TYPE_INITABLE, initable_interface_init))


gboolean
droid_vibra_on (DroidVibra *self,
                int32_t     duration)
{
  if (!DROID_IS_VIBRA (self) || !self->backend)
    return FALSE;

  return droid_vibra_backend_on (self->backend, duration);
}

gboolean
droid_vibra_off (DroidVibra *self)
{
  if (!DROID_IS_VIBRA (self) || !self->backend)
    return FALSE;

  return droid_vibra_backend_off (self->backend);
}


static DroidVibraBackend *
droid_vibra_create_backend (void)
{
  g_autoptr (GError) error = NULL;
  DroidVibraBackend *backend;

  backend = (DroidVibraBackend *) droid_vibra_backend_hidl_new (&error);

  if (!backend)
    {
      backend = (DroidVibraBackend *) droid_vibra_backend_aidl_new (&error);

      if (!backend)
        return NULL;
    }

  return backend;
}


static gboolean
initable_init (GInitable     *initable,
               GCancellable  *cancellable,
               GError       **error)
{
  DroidVibra *self = DROID_VIBRA (initable);

  g_debug ("Initializing libdroid vibra");

  self->backend = droid_vibra_create_backend ();

  if (!self->backend)
    {
      g_set_error (error,
                   G_IO_ERROR, G_IO_ERROR_FAILED,
                   "No vibra available");
      return FALSE;
    }

  return TRUE;
}


static void
droid_vibra_constructed (GObject *obj)
{
  G_OBJECT_CLASS (droid_vibra_parent_class)->constructed (obj);
}


static void
droid_vibra_dispose (GObject *obj)
{
  DroidVibra *self = DROID_VIBRA (obj);

  g_debug ("Disposing droid vibra");

  g_clear_object (&self->backend);

  G_OBJECT_CLASS (droid_vibra_parent_class)->dispose (obj);
}


static void
initable_interface_init (GInitableIface *iface)
{
  iface->init = initable_init;
}


static void
droid_vibra_class_init (DroidVibraClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = droid_vibra_constructed;
  object_class->dispose      = droid_vibra_dispose;
}


static void
droid_vibra_init (DroidVibra *self)
{
}


DroidVibra *
droid_vibra_new (GError **error)
{
  return DROID_VIBRA (
    g_initable_new (DROID_TYPE_VIBRA,
                    NULL,
                    error,
                    NULL));
}
