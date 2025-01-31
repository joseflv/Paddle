//   Copyright (c) 2020 PaddlePaddle Authors. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "paddle/fluid/platform/gloo_context.h"

namespace paddle::platform {
#if defined(PADDLE_WITH_GLOO)
void GlooParallelContext::Init() {
  auto gloo_ptr = paddle::framework::GlooWrapper::GetInstance();
  gloo_ptr->SetRank(strategy_.rank);
  gloo_ptr->SetSize(strategy_.rank_num);
  gloo_ptr->SetIface(strategy_.iface);
  gloo_ptr->SetTimeoutSeconds(strategy_.init_seconds, strategy_.run_seconds);
  gloo_ptr->SetHttpStore(
      strategy_.ip_address, strategy_.ip_port, strategy_.scope);
  gloo_ptr->Init();
}

void GlooParallelContext::Barrier() {
  auto gloo_ptr = paddle::framework::GlooWrapper::GetInstance();
  PADDLE_ENFORCE_EQ(
      gloo_ptr->IsInitialized(),
      true,
      common::errors::Unavailable("Gloo context is not initialized."));
  gloo_ptr->Barrier();
}

void GlooParallelContext::ReleaseContext() {
  auto gloo_ptr = paddle::framework::GlooWrapper::GetInstance();
  if (gloo_ptr->IsInitialized() == true) {
    gloo_ptr.reset();
  }
}
#endif

}  // namespace paddle::platform
