/****************************************************************************
 *
 * Copyright 2016 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 ****************************************************************************/
//***************************************************************************
// libxx/libxx_eabi_atexit.cxx
//
//   Copyright (C) 2012 Gregory Nutt. All rights reserved.
//   Author: Gregory Nutt <gnutt@nuttx.org>
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materials provided with the
//    distribution.
// 3. Neither the name NuttX nor the names of its contributors may be
//    used to endorse or promote products derived from this software
//    without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
//***************************************************************************

//***************************************************************************
// Included Files
//***************************************************************************

#include <tinyara/config.h>

#include <cassert>

#include "libxx_internal.hxx"
#include "libxx_cxa_guard.hxx"

//***************************************************************************
// Pre-processor Definitions
//***************************************************************************

//***************************************************************************
// Private Types
//***************************************************************************

struct __cxa_atexit_s
{
  __cxa_exitfunc_t func;
  FAR void *arg;
};

//***************************************************************************
// Private Data
//***************************************************************************

extern "C"
{
  //*************************************************************************
  // Public Data
  //*************************************************************************

  FAR void *__dso_handle = NULL;

  //*************************************************************************
  // Private Functions
  //*************************************************************************

  //*************************************************************************
  // Name: __cxa_guard_reset
  // Description : This function is to support resetting the static object
  // 	after destruction. So that Static object can be instantiated again
  //*************************************************************************

  static void __cxa_guard_reset(void *__pstatic)
  {
    // Get to the guard pointer:
    // if __ARM_EABI__, then -32 bytes from the object otherwise -64 bytes
    __guard *g = (__guard *)__pstatic - 1;
#ifdef __ARM_EABI__
    *g = 0;
#else
    *(char *)g = 0;
#endif
  }

  //*************************************************************************
  // Name: __cxa_callback
  //
  // Description:
  //   This is really just an "adaptor" function that matches the form of
  //   the __cxa_exitfunc_t to an onexitfunc_t using an allocated structure
  //   to marshall the call parameters.
  //
  //*************************************************************************

#ifdef CONFIG_SCHED_ONEXIT
  static void __cxa_callback(int exitcode, FAR void *arg)
  {
    FAR struct __cxa_atexit_s *alloc = (FAR struct __cxa_atexit_s *)arg;
    DEBUGASSERT(alloc && alloc->func);

    alloc->func(alloc->arg);
    __cxa_guard_reset(alloc->arg);
    lib_free(alloc);
  }
#endif

  //*************************************************************************
  // Public Functions
  //*************************************************************************

  //*************************************************************************
  // Name: __cxa_atexit
  //
  // Description:
  //   __cxa_atexit() registers a destructor function to be called by exit().
  //   On a call to exit(), the registered functions should be called with
  //   the single argument 'arg'. Destructor functions shall always be
  //   called in the reverse order to their registration (i.e. the most
  //   recently registered function shall be called first),
  //
  //   If shared libraries were supported, the callbacks should be invoked
  //   when the shared library is unloaded as well.
  //
  // Reference:
  //   Linux base
  //
  //*************************************************************************

  int __cxa_atexit(__cxa_exitfunc_t func, FAR void *arg, FAR void *dso_handle)
  {
#ifdef CONFIG_SCHED_ONEXIT
    // Allocate memory to hold the marshaled __cxa_exitfunc_t call
    // information.

    FAR struct __cxa_atexit_s *alloc =
      (FAR struct __cxa_atexit_s *)lib_malloc(sizeof(struct __cxa_atexit_s));

    if (alloc)
    {
      // Register the function to be called when the task/thread exists.

      alloc->func = func;
      alloc->arg  = arg;

      return on_exit(__cxa_callback, alloc);
    }
    else
#endif
    {
      // What else can we do?

      return 0;
    }
  }
}
