/* leds.h
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

#pragma once

#include <glib-object.h>
#include <stdint.h>

G_BEGIN_DECLS

#define DROID_TYPE_LEDS droid_leds_get_type ()
G_DECLARE_FINAL_TYPE (DroidLeds, droid_leds, DROID, LEDS, GObject)

typedef enum _DroidLedsKind {
  DROID_LEDS_KIND_BACKLIGHT = 0,
  DROID_LEDS_KIND_NOTIFICATION,
} DroidLedsKind;

DroidLeds *droid_leds_new                (void);
gboolean   droid_leds_set_backlight      (DroidLeds *self,
                                          guint      level,
                                          gboolean   save);
guint      droid_leds_get_backlight      (DroidLeds *self);
gboolean   droid_leds_set_notification   (DroidLeds *self,
                                          uint32_t   color,
                                          int32_t    flash_on_ms,
                                          int32_t    flash_off_ms);
gboolean   droid_leds_clear_notification (DroidLeds *self);
gboolean   droid_leds_is_kind_supported  (DroidLeds *self,
                                          DroidLedsKind kind);

G_END_DECLS
