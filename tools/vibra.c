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
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <libdroid/libdroid.h>

#include <glib.h>
#include <stdio.h>

int
main (int argc, char** argv)
{
  gint duration = 250;
  g_autoptr (GError) err = NULL;
  g_autoptr (GOptionContext) context = NULL;
  g_autoptr (DroidVibra) vibra = NULL;
  const GOptionEntry entries[] = {
    {
      "duration", 'd', G_OPTION_FLAG_NONE, G_OPTION_ARG_INT, &duration,
      "The duration of the vibration, in ms (defaults to 250)", NULL,
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

  vibra = droid_vibra_new ();

  g_message ("Vibrating for %d ms...", duration);

  if (!droid_vibra_on (vibra, duration))
    g_error ("Unable to vibrate!");

  return EXIT_SUCCESS;
}
