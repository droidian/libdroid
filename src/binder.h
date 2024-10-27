/* binder.h
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
 * variant, fbd-binder.h @ 7a6043d4794ae4d5df67b95a951f770a49191cdc
 *
 * Copyright 2022 Eugenio "g7" Paolantonio
 *
 * The authors gave written permission for the license change for libdroid.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#pragma once

#include <glib.h>
#include <gbinder.h>

G_BEGIN_DECLS

/* Source: https://android.googlesource.com/platform/frameworks/native/+/master/libs/binder/include/binder/Stability.h */
enum BinderStability {
  BINDER_STABILITY_UNDECLARED = 0,

  BINDER_STABILITY_VENDOR = 0b000011,
  BINDER_STABILITY_SYSTEM = 0b001100,
  BINDER_STABILITY_VINTF  = 0b111111,
};

gboolean binder_init (const char             *device,
                      const char             *iface,
                      const char             *fqname,
                      GBinderServiceManager **service_manager,
                      GBinderRemoteObject   **remote,
                      GBinderClient         **client);
gboolean binder_status_is_ok (GBinderReader *reader);
gboolean binder_reply_status_is_ok (GBinderRemoteReply *reply);

G_END_DECLS
