/* vibra-backend.c
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
 * variant, fbd-droid-vibra-backend.c @ 8e1245ce25d48cd4107754d13c6c16375f550622
 *
 * Copyright 2022 Eugenio "g7" Paolantonio
 *
 * The authors gave written permission for the license change for libdroid.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define G_LOG_DOMAIN "droid-vibra-backend"

#include "vibra-backend.h"

G_DEFINE_INTERFACE (DroidVibraBackend, droid_vibra_backend, G_TYPE_OBJECT)

static void
droid_vibra_backend_default_init (DroidVibraBackendInterface *iface)
{
    /* Nothing yet */
}


gboolean
droid_vibra_backend_on (DroidVibraBackend *self,
                        int32_t            duration)
{
  DroidVibraBackendInterface *iface;

  g_return_val_if_fail (DROID_IS_VIBRA_BACKEND (self), FALSE);

  iface = DROID_VIBRA_BACKEND_GET_IFACE (self);
  g_return_val_if_fail (iface->on != NULL, FALSE);
  return iface->on (self, duration);
}


gboolean
droid_vibra_backend_off (DroidVibraBackend *self)
{
  DroidVibraBackendInterface *iface;

  g_return_val_if_fail (DROID_IS_VIBRA_BACKEND (self), FALSE);

  iface = DROID_VIBRA_BACKEND_GET_IFACE (self);
  g_return_val_if_fail (iface->off != NULL, FALSE);
  return iface->off (self);
}
