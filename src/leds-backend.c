/* leds-backend.c
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
 * variant, fbd-droid-leds-backend.c @ 7a6043d4794ae4d5df67b95a951f770a49191cdc
 *
 * Copyright 2022 Eugenio "g7" Paolantonio
 * Copyright 2021 Erfan Abdi
 *
 * The authors gave written permission for the license change for libdroid.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define G_LOG_DOMAIN "droid-leds-backend"

#include "leds-backend.h"

G_DEFINE_INTERFACE (DroidLedsBackend, droid_leds_backend, G_TYPE_OBJECT)

static void
droid_leds_backend_default_init (DroidLedsBackendInterface *iface)
{
    /* Nothing yet */
}


gboolean
droid_leds_backend_is_supported (DroidLedsBackend *self,
                                 LightType         light_type)
{
  DroidLedsBackendInterface *iface;

  g_return_val_if_fail (DROID_IS_LEDS_BACKEND (self), FALSE);

  iface = DROID_LEDS_BACKEND_GET_IFACE (self);
  g_return_val_if_fail (iface->is_supported != NULL, FALSE);
  return iface->is_supported (self, light_type);
}


gboolean
droid_leds_backend_set (DroidLedsBackend *self,
                        uint32_t           color,
                        LightType         light_type,
                        FlashType         flash_type,
                        BrightnessType    brightness_type,
                        int32_t           flash_on_ms,
                        int32_t           flash_off_ms)
{
  DroidLedsBackendInterface *iface;

  g_return_val_if_fail (DROID_IS_LEDS_BACKEND (self), FALSE);

  iface = DROID_LEDS_BACKEND_GET_IFACE (self);
  g_return_val_if_fail (iface->set != NULL, FALSE);
  return iface->set (self, color, light_type, flash_type, brightness_type,
    flash_on_ms, flash_off_ms);
}

