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
#ifndef SRC_IR_IR_PRINTER_H_
#define SRC_IR_IR_PRINTER_H_

#include <cstdint>
#include <memory>

#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "src/ir/ir_traversing_visitor.h"
#include "src/ir/module.h"
#include "src/ir/ssa_names.h"
#include "src/ir/value.h"
#include "src/ir/value_string_converter.h"

namespace raksha::ir {

// A visitor that prints the IR.
class IRPrinter : public IRTraversingVisitor<IRPrinter> {
 public:
  template <typename T>
  static void ToString(std::ostream& out, const T& entity,
                       SsaNames& ssa_names) {
    IRPrinter printer(out, ssa_names);
    entity.Accept(printer);
  }

  template <typename T>
  static void ToString(std::ostream& out, const T& entity) {
    SsaNames ssa_names;
    IRPrinter printer(out, ssa_names);
    entity.Accept(printer);
  }

  template <typename T>
  static std::string ToString(const T& entity, SsaNames& ssa_names) {
    std::ostringstream out;
    ToString(out, entity, ssa_names);
    return out.str();
  }

  template <typename T>
  static std::string ToString(const T& entity) {
    std::ostringstream out;
    ToString(out, entity);
    return out.str();
  }

  Unit PreVisit(const Module& module) override {
    out_ << Indent()
         << absl::StreamFormat("module %s {\n",
                               ssa_names_.GetOrCreateID(module));
    IncreaseIndent();
    return Unit();
  }

  Unit PostVisit(const Module& module, Unit result) override {
    DecreaseIndent();
    out_ << Indent()
         << absl::StreamFormat("}  // module %s\n",
                               ssa_names_.GetOrCreateID(module));
    return result;
  }

  Unit PreVisit(const Block& block) override {
    out_ << Indent()
         << absl::StreamFormat("block %s {\n", ssa_names_.GetOrCreateID(block));
    IncreaseIndent();
    return Unit();
  }

  Unit PostVisit(const Block& block, Unit result) override {
    DecreaseIndent();
    out_ << Indent()
         << absl::StreamFormat("}  // block %s\n",
                               ssa_names_.GetOrCreateID(block));
    return result;
  }

  Unit PreVisit(const Operation& operation) override {
    constexpr absl::string_view kOperationFormat = "%s = %s [%s](%s)";

    // We want the attribute names to print in a stable order. This means that
    // we cannot just print from the attribute map directly. Gather the names
    // into a vector and sort that, then use the order in that vector to print
    // the attributes.
    std::string attributes_string = PrintNamedMapInNameOrder(
        operation.attributes(),
        [](const Attribute& attr) { return attr.ToString(); });

    std::string inputs_string = absl::StrJoin(
        operation.inputs(), ", ", [this](std::string* out, const Value& value) {
          absl::StrAppend(out, ValueToString(value, ssa_names_));
        });

    const ir::Operator& op = operation.op();
    uint64_t number_of_op_return_values = op.number_of_return_values();
    // Produce the result value's SSA name after the inputs. It is technically
    // correct both ways, and in real programs shouldn't matter, but for unit
    // tests, producing the result first can produce the surprising result of
    // the result being numbered before the inputs.
    std::vector<std::string> op_return_values;
    for (uint64_t i = 0; i < number_of_op_return_values; ++i) {
      op_return_values.push_back(ssa_names_.GetOrCreateID(
          ir::Value::MakeOperationResultValue(operation, i)));
    }
    SsaNames::ID result_name = absl::StrJoin(op_return_values, ", ");

    out_ << Indent()
         << absl::StreamFormat(kOperationFormat, result_name,
                               operation.op().name(), attributes_string,
                               inputs_string)
         << "\n";
    return Unit();
  }

  Unit PostVisit(const Operation& operation, Unit result) override {
    return result;
  }

  // Returns a pretty-printed map where entries are sorted by the key.
  template <class T, class F>
  static std::string PrintNamedMapInNameOrder(
      const absl::flat_hash_map<std::string, T>& map_to_print,
      F value_pretty_printer) {
    std::vector<absl::string_view> names;
    names.reserve(map_to_print.size());
    for (auto& key_value_pair : map_to_print) {
      names.push_back(key_value_pair.first);
    }
    std::sort(names.begin(), names.end());
    return absl::StrJoin(
        names, ", ", [&](std::string* out, const absl::string_view name) {
          absl::StrAppend(out, name, ": ",
                          value_pretty_printer(map_to_print.at(name)));
        });
  }

 private:
  std::string Indent() { return std::string(indent_ * 2, ' '); }
  void IncreaseIndent() { ++indent_; }
  void DecreaseIndent() { --indent_; }

  IRPrinter(std::ostream& out, SsaNames& ssa_names)
      : out_(out), indent_(0), ssa_names_(ssa_names) {}

  std::ostream& out_;
  int indent_;
  // TODO(#615): Make ssa_names_ const.
  SsaNames& ssa_names_;
};

inline std::ostream& operator<<(std::ostream& out, const Operation& operation) {
  IRPrinter::ToString(out, operation);
  return out;
}

inline std::ostream& operator<<(std::ostream& out, const Block& block) {
  IRPrinter::ToString(out, block);
  return out;
}
inline std::ostream& operator<<(std::ostream& out, const Module& module) {
  IRPrinter::ToString(out, module);
  return out;
}
}  // namespace raksha::ir

#endif  // SRC_IR_IR_PRINTER_H_
