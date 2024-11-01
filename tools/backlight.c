/* backlight.c
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

#include <libdroid/libdroid.h>

#include <glib.h>
#include <stdio.h>

int
main (int argc, char** argv)
{
  gint level = 255;
  gboolean save = FALSE;
  gboolean restore = FALSE;
  gboolean info = FALSE;
  g_autoptr (GError) err = NULL;
  g_autoptr (GOptionContext) context = NULL;
  g_autoptr (DroidLeds) leds = NULL;
  const GOptionEntry entries[] = {
    {
      "level", 'l', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &level,
      "The backlight level to set", NULL,
    },
    {
      "save", 's', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &save,
      "Whether to save the backlight level", NULL,
    },
    {
      "restore", 'r', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &restore,
      "Whether to restore the backlight level", NULL,
    },
    {
      "info", 'i', G_OPTION_FLAG_NONE, G_OPTION_ARG_NONE, &info,
      "Show info on supported lights, and exit", NULL,
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

  leds = droid_leds_new ();

  if (info)
    {
      printf ("Backlight supported: %d\n"
              "Backlight stored level: %d\n"
              "Notification light supported: %d\n",
              droid_leds_is_kind_supported (leds, DROID_LEDS_KIND_BACKLIGHT),
              droid_leds_get_backlight (leds),
              droid_leds_is_kind_supported (leds, DROID_LEDS_KIND_NOTIFICATION));

      return EXIT_SUCCESS;
    }
  else if (restore)
    {
      save = FALSE;
      level = droid_leds_get_backlight (leds);
    }

  if (!droid_leds_set_backlight (leds, level, save))
    g_error ("Unable to set backlight!");

  return EXIT_SUCCESS;
}
