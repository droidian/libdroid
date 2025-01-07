/* utils.c
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

#define G_LOG_DOMAIN "droid-leds-utils"

#include "utils.h"

gboolean
droid_utils_file_exists (const gchar *path)
{
  gboolean result =  g_file_test (path, G_FILE_TEST_EXISTS);

  g_debug ("file_exists: %s: %i", path, result);

  return result;
}

gboolean
droid_utils_can_write_to (const gchar *path)
{
  g_autoptr (GFile) file = g_file_new_for_path (path);
  g_autoptr (GFileInfo) info = g_file_query_info (file, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE,
    G_FILE_QUERY_INFO_NONE, NULL, NULL);
  gboolean result = FALSE;

  result = (info != NULL &&
            g_file_info_has_attribute (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE) &&
            g_file_info_get_attribute_boolean (info, G_FILE_ATTRIBUTE_ACCESS_CAN_WRITE));

  g_debug ("can_write_to: %s: %i", path, result);

  return result;
}

gboolean
droid_utils_write_to_file (const gchar *path,
                           gchar       *content)
{
  g_autoptr (GError) error = NULL;

  if (g_file_set_contents_full (path, content, strlen (content),
        G_FILE_SET_CONTENTS_NONE, 0666, &error))
    {
      g_debug ("Content set correctly to %s", path);
      return TRUE;
    }
  else
    {
      g_warning ("Unable to write to %s: %s", path, error->message);
      return FALSE;
    }
}
