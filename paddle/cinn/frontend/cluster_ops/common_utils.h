// Copyright (c) 2024 PaddlePaddle Authors. All Rights Reserved.
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

#pragma once

#include <algorithm>
#include <atomic>
#include <optional>
#include <typeinfo>
#include <unordered_map>
#include <variant>
#include <vector>

#include "glog/logging.h"

#include "paddle/cinn/common/bfs_walker.h"
#include "paddle/cinn/common/topo_walker.h"

#include "paddle/cinn/hlir/dialect/operator/ir/cinn_op.h"
#include "paddle/cinn/hlir/dialect/operator/ir/manual_op.h"
#include "paddle/cinn/hlir/framework/op.h"
#include "paddle/pir/include/dialect/control_flow/ir/cf_op.h"

namespace cinn::frontend::cluster_ops {

using OpSet = std::unordered_set<const pir::Operation*>;
using OpSetPtr = std::shared_ptr<OpSet>;
using OpVisitor = std::function<void(const pir::Operation*)>;
using OpPatternKind = cinn::hlir::framework::OpPatternKind;

struct OpTopo {
  OpSetPtr ops;

  static OpTopo Make(const std::vector<const pir::Operation*>& ops) {
    auto ops_set = std::make_shared<OpSet>(ops.begin(), ops.end());
    return OpTopo{
        .ops = ops_set,
    };
  }

  template <typename OpVisitorT>
  void VisitInputOp(const pir::Operation* op, const OpVisitorT& DoEach) const {
    if (this->ops->count(op) == 0) return;
    for (int i = 0; i < op->num_operands(); ++i) {
      const auto* input_op = op->operand_source(i).defining_op();
      if (this->ops->count(input_op) == 0) continue;
      DoEach(input_op);
    }
  }

  template <typename OpVisitorT>
  void VisitOutputOp(const pir::Operation* op, const OpVisitorT& DoEach) const {
    for (int i = 0; i < op->num_results(); ++i) {
      pir::Value output = op->result(i);
      for (auto consumer_it = output.use_begin();
           consumer_it != output.use_end();
           ++consumer_it) {
        const auto* consumer_op = consumer_it->owner();
        if (consumer_op->isa<pir::YieldOp>()) continue;
        if (this->ops->count(consumer_op) == 0) continue;
        DoEach(consumer_op);
      }
    }
  }
};

OpPatternKind GetOpPatternKind(const ::pir::Operation* node);

bool IsGeneralInjective(const pir::Operation* op);

size_t GetRank(pir::Value value);

std::list<const pir::Operation*> GetSinks(const OpSet& ops);

const pir::Operation* GetSoleSink(const OpSet& ops);

common::TopoWalker<const pir::Operation*> GetOpsReversedTopoWalker(
    const OpTopo& op_topo);

std::vector<int64_t> GetReduceAxes(const pir::Operation* reduce_op);

bool GetReduceOpKeepDims(const pir::Operation* reduce_op);

std::function<size_t(const pir::Operation*)> MakeTopoOrderFinderOfOp(
    const std::vector<const pir::Operation*>& ops);

std::function<bool(const pir::Operation*)> MakePredicatorIsInThisFusionOp(
    const std::vector<const pir::Operation*>& ops);

std::string OpsDebugStr(std::vector<const pir::Operation*> ops);

}  // namespace cinn::frontend::cluster_ops