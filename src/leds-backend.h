/* leds-backend.h
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
 * variant, fbd-droid-leds-backend.h @ 7a6043d4794ae4d5df67b95a951f770a49191cdc
 *
 * Copyright 2022 Eugenio "g7" Paolantonio
 *
 * The authors gave written permission for the license change for libdroid.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <stdint.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define ALIGNED(x) __attribute__ ((aligned(x)))

#define DROID_TYPE_LEDS_BACKEND droid_leds_backend_get_type()
G_DECLARE_INTERFACE (DroidLedsBackend, droid_leds_backend, DROID, LEDS_BACKEND, GObject)

/* Light types */
typedef enum light_type {
  LIGHT_TYPE_BACKLIGHT = 0,
  LIGHT_TYPE_KEYBOARD = 1,
  LIGHT_TYPE_BUTTONS = 2,
  LIGHT_TYPE_BATTERY = 3,
  LIGHT_TYPE_NOTIFICATIONS = 4,
  LIGHT_TYPE_ATTENTION = 5,
  LIGHT_TYPE_BLUETOOTH = 6,
  LIGHT_TYPE_WIFI = 7,
  LIGHT_TYPE_COUNT = 8,
} LightType;

/* Flash types */
typedef enum flash_type {
  FLASH_TYPE_NONE = 0,
  FLASH_TYPE_TIMED = 1,
  FLASH_TYPE_HARDWARE = 2,
} FlashType;

/* Brightness types */
typedef enum brightness_type {
  BRIGHTNESS_MODE_USER = 0,
  BRIGHTNESS_MODE_SENSOR = 1,
  BRIGHTNESS_MODE_LOW_PERSISTENCE = 2,
} BrightnessType;

/* The light state */
typedef struct light_state {
  uint32_t color;
  FlashType flashMode ALIGNED(4);
  int32_t flashOnMs;
  int32_t flashOffMs;
  BrightnessType brightnessMode ALIGNED(4);
} LightState;
G_STATIC_ASSERT(sizeof(LightState) == 20);

struct _DroidLedsBackendInterface
{
  GTypeInterface parent_iface;

  gboolean (*is_supported) (DroidLedsBackend *self,
                            LightType         light_type);
  gboolean (*set)          (DroidLedsBackend *self,
                            uint32_t           color,
                            LightType         light_type,
                            FlashType         flash_type,
                            BrightnessType    brightness_type,
                            int32_t           flash_on_ms,
                            int32_t           flash_off_ms);
};

gboolean droid_leds_backend_is_supported (DroidLedsBackend *self,
                                          LightType         light_type);
gboolean droid_leds_backend_set          (DroidLedsBackend *self,
                                          uint32_t           color,
                                          LightType         light_type,
                                          FlashType         flash_type,
                                          BrightnessType    brightness_type,
                                          int32_t           flash_on_ms,
                                          int32_t           flash_off_ms);

G_END_DECLS

