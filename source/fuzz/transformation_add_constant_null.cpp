// Copyright (c) 2020 Google LLC
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

#include "source/fuzz/transformation_add_constant_null.h"

#include "source/fuzz/fuzzer_util.h"

namespace spvtools {
namespace fuzz {

TransformationAddConstantNull::TransformationAddConstantNull(
    const spvtools::fuzz::protobufs::TransformationAddConstantNull& message)
    : message_(message) {}

TransformationAddConstantNull::TransformationAddConstantNull(uint32_t fresh_id,
                                                             uint32_t type_id) {
  message_.set_fresh_id(fresh_id);
  message_.set_type_id(type_id);
}

bool TransformationAddConstantNull::IsApplicable(
    opt::IRContext* context, const TransformationContext& /*unused*/) const {
  // A fresh id is required.
  if (!fuzzerutil::IsFreshId(context, message_.fresh_id())) {
    return false;
  }
  auto type = context->get_type_mgr()->GetType(message_.type_id());
  // The type must exist.
  if (!type) {
    return false;
  }
  // The type must be one of the types for which null constants are allowed,
  // according to the SPIR-V spec.
  return fuzzerutil::IsNullConstantSupported(*type);
}

void TransformationAddConstantNull::Apply(
    opt::IRContext* context, TransformationContext* /*unused*/) const {
  context->module()->AddGlobalValue(MakeUnique<opt::Instruction>(
      context, SpvOpConstantNull, message_.type_id(), message_.fresh_id(),
      opt::Instruction::OperandList()));
  fuzzerutil::UpdateModuleIdBound(context, message_.fresh_id());
  // We have added an instruction to the module, so need to be careful about the
  // validity of existing analyses.
  context->InvalidateAnalysesExceptFor(opt::IRContext::Analysis::kAnalysisNone);
}

protobufs::Transformation TransformationAddConstantNull::ToMessage() const {
  protobufs::Transformation result;
  *result.mutable_add_constant_null() = message_;
  return result;
}

}  // namespace fuzz
}  // namespace spvtools