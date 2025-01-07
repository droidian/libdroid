/* hal-service.c
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

#define G_LOG_DOMAIN "droid-hal-service"

#include <stdlib.h>
#include <glib.h>
#include <glib-object.h>
#include <glib-unix.h>
#include <gio/gio.h>

#include "hal-service.h"

#define DEFAULT_BINDER_DEVICE "/dev/hwbinder"

typedef enum
{
  PROP_IMPLEMENTATION = 1,
  PROP_BINDER_DEVICE,
  PROP_BINDER_IFACE,
  PROP_BINDER_NAME,
  N_PROPERTIES
} DroidHalServiceProperty;

static GParamSpec *properties[N_PROPERTIES] = { NULL, };

struct _DroidHalService
{
  GObject                 parent_instance;

  GBinderServiceManager  *service_manager;
  GBinderLocalObject     *local_object;
  GMainLoop              *main_loop;

  DroidHalImplementation *implementation;
  const gchar            *binder_device;
  const gchar            *binder_iface;
  const gchar            *binder_name;

  guint                   exit_code;
};

G_DEFINE_FINAL_TYPE (DroidHalService, droid_hal_service, G_TYPE_OBJECT)

static void
droid_hal_service_constructed (GObject *obj)
{
  DroidHalService *self = DROID_HAL_SERVICE (obj);

  G_OBJECT_CLASS (droid_hal_service_parent_class)->constructed (obj);

  self->service_manager = gbinder_servicemanager_new (self->binder_device);
  self->main_loop       = g_main_loop_new (NULL, TRUE);
}


static void
droid_hal_service_dispose (GObject *obj)
{
  DroidHalService *self = DROID_HAL_SERVICE (obj);

  G_OBJECT_CLASS (droid_hal_service_parent_class)->dispose (obj);

  g_clear_object (&self->implementation);
  g_clear_object (&self->service_manager);
  g_clear_object (&self->local_object);

  g_main_loop_unref (self->main_loop);
}

static void
droid_hal_service_set_property (GObject      *object,
                                guint         property_id,
                                const GValue *value,
                                GParamSpec   *pspec)
{
  DroidHalService *self = DROID_HAL_SERVICE (object);

  switch ((DroidHalServiceProperty) property_id)
    {
    case PROP_IMPLEMENTATION:
      /* This is construct only, so we don't need to handle existing value */
      self->implementation = g_value_get_object (value);
      g_object_ref (self->implementation);
      break;

    case PROP_BINDER_DEVICE:
      /* This is construct only, so we don't need to handle existing value */
      self->binder_device = g_value_get_string (value);
      break;

    case PROP_BINDER_IFACE:
      /* This is construct only, so we don't need to handle existing value */
      self->binder_iface = g_value_get_string (value);
      break;

    case PROP_BINDER_NAME:
      /* This is construct only, so we don't need to handle existing value */
      self->binder_name = g_value_get_string (value);
      break;

   case N_PROPERTIES:
   default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
droid_hal_service_get_property (GObject    *object,
                                guint       property_id,
                                GValue     *value,
                                GParamSpec *pspec)
{
  DroidHalService *self = DROID_HAL_SERVICE (object);

  switch ((DroidHalServiceProperty) property_id)
    {
    case PROP_IMPLEMENTATION:
      g_value_set_object (value, self->implementation);
      break;

    case PROP_BINDER_DEVICE:
      g_value_set_string (value, self->binder_device);
      break;

    case PROP_BINDER_IFACE:
      g_value_set_string (value, self->binder_iface);
      break;

    case PROP_BINDER_NAME:
      g_value_set_string (value, self->binder_name);
      break;

    case N_PROPERTIES:
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
droid_hal_service_class_init (DroidHalServiceClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  object_class->constructed  = droid_hal_service_constructed;
  object_class->dispose      = droid_hal_service_dispose;
  object_class->set_property = droid_hal_service_set_property;
  object_class->get_property = droid_hal_service_get_property;

  properties[PROP_IMPLEMENTATION] =
    g_param_spec_object ("implementation",
                         "Implementation",
                         "An instance of the implementation to use",
                         DROID_TYPE_HAL_IMPLEMENTATION,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_BINDER_DEVICE] =
    g_param_spec_string ("binder-device",
                         "Binder device",
                         "The binder device to use",
                         DEFAULT_BINDER_DEVICE,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_BINDER_IFACE] =
    g_param_spec_string ("binder-iface",
                         "Binder interface",
                         "The binder interface to use",
                         NULL,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  properties[PROP_BINDER_NAME] =
    g_param_spec_string ("binder-name",
                         "Binder name",
                         "The binder service name to use",
                         NULL,
                         G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY);

  g_object_class_install_properties (object_class, N_PROPERTIES, properties);
}


static void
droid_hal_service_init (DroidHalService *self)
{
}


DroidHalService *
droid_hal_service_new (DroidHalImplementation *implementation,
                       gchar            *binder_device,
                       gchar            *binder_iface,
                       gchar            *binder_name)
{
  return DROID_HAL_SERVICE (
    g_object_new (DROID_TYPE_HAL_SERVICE,
      "implementation", implementation,
      "binder-device",  binder_device,
      "binder-iface",   binder_iface,
      "binder-name",    binder_name,
      NULL));
}

static GBinderLocalReply *
droid_hal_service_reply (GBinderLocalObject   *object,
                         GBinderRemoteRequest *request,
                         guint                 code,
                         guint                 flags,
                         int                  *status,
                         void                 *user_data)
{
  DroidHalService *self = DROID_HAL_SERVICE (user_data);
  GBinderLocalReply *result = NULL;
  const char *binder_iface = gbinder_remote_request_interface (request);

  g_return_val_if_fail (DROID_IS_HAL_SERVICE (self), NULL);

  g_debug ("Called interface %s, code %d", binder_iface, code);
  if (g_strcmp0 (binder_iface, self->binder_iface) == 0)
    {
      *status = 0; /* FIXME? */
      result = droid_hal_implementation_reply ((DroidHalImplementation *) self->implementation,
        object, request, code);
    }
  else
    {
      *status = -1;
    }

  return result;
}

static void
droid_hal_service_added(GBinderServiceManager *service_manager,
                        int                    status,
                        void                  *user_data)
{
  DroidHalService *self = DROID_HAL_SERVICE (user_data);

  if (status == GBINDER_STATUS_OK)
    {
      g_message ("Service '%s' added", self->binder_name);
      self->exit_code = EXIT_SUCCESS;
    }
  else
    {
      g_message ("Unable to add '%s': %d", self->binder_name, status);
      g_main_loop_quit (self->main_loop);
    }
}

static void
droid_hal_service_presence_handler (GBinderServiceManager *service_manager,
                                    void                  *user_data)
{
  DroidHalService *self = DROID_HAL_SERVICE (user_data);

  if (gbinder_servicemanager_is_present (self->service_manager))
    {
      gbinder_servicemanager_add_service(self->service_manager, self->binder_name,
        self->local_object, droid_hal_service_added, self);
    }
  else
    {
      g_warning ("Service manager disappeared");
    }
}

static gboolean
droid_hal_service_signal (gpointer user_data)
{
  DroidHalService *self = DROID_HAL_SERVICE (user_data);

  g_debug ("Caught signal");
  g_return_val_if_fail (DROID_IS_HAL_SERVICE (self), G_SOURCE_REMOVE);

  g_main_loop_quit (self->main_loop);
  return G_SOURCE_CONTINUE; /* Cleaned up at exit */
}

int
droid_hal_service_run (DroidHalService *self)
{
  gulong presence_id;
  guint sigterm = g_unix_signal_add (SIGTERM, droid_hal_service_signal, self);
  guint sigint = g_unix_signal_add (SIGINT, droid_hal_service_signal, self);

  self->exit_code = EXIT_FAILURE;

  g_debug ("Waiting for service manager...");
  if (gbinder_servicemanager_wait (self->service_manager, -1))
    {
      g_debug ("Creating local object");
      self->local_object = gbinder_servicemanager_new_local_object (self->service_manager,
        self->binder_iface, droid_hal_service_reply, self);

      presence_id = gbinder_servicemanager_add_presence_handler (self->service_manager,
        droid_hal_service_presence_handler, self);

      droid_hal_service_presence_handler (self->service_manager, (void *)self);
      g_debug ("Added service %s/%s on device %s", self->binder_iface,
        self->binder_name, self->binder_device);

      g_main_loop_run (self->main_loop);

      gbinder_servicemanager_remove_handler (self->service_manager, presence_id);
    }

  if (sigterm)
      g_source_remove (sigterm);

  if (sigint)
      g_source_remove (sigint);

  return self->exit_code;
}
