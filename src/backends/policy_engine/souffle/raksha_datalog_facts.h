//-----------------------------------------------------------------------------
// Copyright 2022 Google LLC
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//----------------------------------------------------------------------------
#ifndef SRC_BACKENDS_POLICY_ENGINE_SOUFFLE_RAKSHA_DATALOG_FACTS_H_
#define SRC_BACKENDS_POLICY_ENGINE_SOUFFLE_RAKSHA_DATALOG_FACTS_H_

#include <filesystem>
#include <string>
#include <vector>

#include "absl/status/statusor.h"
#include "src/ir/datalog/operation.h"

namespace raksha::backends::policy_engine::souffle {

// A class containing the Datalog facts produced by the IR translator in
// structured (ie, C++ objects, not strings) form.
class RakshaDatalogFacts {
 public:
  void AddIsOperationFact(ir::datalog::IsOperationFact fact) {
    is_operation_facts_.push_back(std::move(fact));
  }

  std::string ToDatalogString() const {
    return absl::StrJoin(
        is_operation_facts_, "\n",
        [](std::string *out, const ir::datalog::IsOperationFact &arg) {
          absl::StrAppend(out, arg.ToDatalogString());
        });
  }

  // Dumps facts as individual files in the provided directory so that they can
  // be used as inputs to running a souffle program using the souffle C++
  // interface. Returns the directory where the files where created. The method
  // also an optional parameter called `empty_relations` that creates empty
  // files corresponding to relations in `empty_relations`.
  absl::Status DumpFactsToDirectory(
      const std::filesystem::path &directory,
      const std::vector<std::string> &empty_relations = {}) const;

  const std::vector<ir::datalog::IsOperationFact> &is_operation_facts() const {
    return is_operation_facts_;
  }

 private:
  std::vector<ir::datalog::IsOperationFact> is_operation_facts_;
};

}  // namespace raksha::backends::policy_engine::souffle

#endif  // SRC_BACKENDS_POLICY_ENGINE_SOUFFLE_RAKSHA_DATALOG_FACTS_H_
