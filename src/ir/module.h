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
#ifndef SRC_IR_MODULE_H_
#define SRC_IR_MODULE_H_

#include <cstdint>
#include <vector>

#include "src/ir/attributes/attribute.h"
#include "src/ir/data_decl.h"
#include "src/ir/ir_visitor.h"
#include "src/ir/operator.h"
#include "src/ir/types/type.h"
#include "src/ir/value.h"

namespace raksha::ir {

class Module;
class Block;

// An Operation represents a unit of execution.
class Operation {
 public:
  Operation(const Block* parent, const Operator& op,
            NamedAttributeMap attributes, ValueList inputs)
      : parent_(parent),
        op_(std::addressof(op)),
        attributes_(std::move(attributes)),
        inputs_(std::move(inputs)) {}

  Operation(const Operator& op, NamedAttributeMap attributes, ValueList inputs)
      : Operation(nullptr, op, attributes, inputs) {}

  Operation(const Operator& op) : Operation(nullptr, op, {}, {}) {}

  // Disable copy (and move) semantics.
  Operation(const Operation&) = delete;
  Operation& operator=(const Operation&) = delete;

  virtual ~Operation() {}

  const Operator& op() const { return *op_; }
  const Block* parent() const { return parent_; }
  const ValueList& inputs() const { return inputs_; }
  const NamedAttributeMap& attributes() const { return attributes_; }

  const Value& GetInputValue(uint64_t index) const {
    CHECK(index < inputs_.size())
        << "Operation:GetInputValue fails because index is out of bounds";
    return inputs_[index];
  }
  uint64_t NumberOfOutputs() const { return op_->number_of_return_values(); }

  Value GetOutputValue(uint64_t index) const {
    CHECK(index < op_->number_of_return_values())
        << "Operation:GetOutputValue fails because index is out of bounds";
    return Value::MakeOperationResultValue(*this, index);
  }

  template <typename Derived, typename Result>
  Result Accept(IRVisitor<Derived, Result, false>& visitor) {
    return visitor.Visit(*this);
  }

  template <typename Derived, typename Result>
  Result Accept(IRVisitor<Derived, Result, true>& visitor) const {
    return visitor.Visit(*this);
  }
  void AddInput(const Value& value) { inputs_.push_back(value); }
  void AddAttribute(const std::string name, const Attribute& value) {
    attributes_.emplace(name, value);
  }
  void set_parent(const Block* parent) { parent_ = parent; }

 private:
  // The parent block if any to which this operation belongs to.
  const Block* parent_;
  // The operator being executed.
  const Operator* op_;
  // The attributes of the operation.
  NamedAttributeMap attributes_;
  // The inputs of the operation.
  ValueList inputs_;
};

// A collection of operations.
class Block {
 public:
  // The type for a collection of `Operation` instances.
  using OperationList = std::vector<std::unique_ptr<Operation>>;

  Block() : parent_module_(nullptr) {}

  // Disable copy (and move) semantics.
  Block(const Block&) = delete;
  Block& operator=(const Block&) = delete;

  const OperationList& operations() const { return operations_; }
  const DataDeclCollection& inputs() const { return inputs_; }
  const DataDeclCollection& outputs() const { return outputs_; }
  const IndexedValueMap& results() const { return results_; }
  const Module* parent_module() const { return parent_module_; }

  template <typename Derived, typename Result>
  Result Accept(IRVisitor<Derived, Result, false>& visitor) {
    return visitor.Visit(*this);
  }

  template <typename Derived, typename Result>
  Result Accept(IRVisitor<Derived, Result, true>& visitor) const {
    return visitor.Visit(*this);
  }

  friend class BlockBuilder;
  friend class Module;

 private:
  // Set the module to which this block belongs. This is private so that only
  // the Module can set it via its `AddBlock` function.
  void set_parent_module(const Module& module) { parent_module_ = &module; }

  // Module to which this belongs to.
  const Module* parent_module_;
  // The inputs to this block.
  DataDeclCollection inputs_;
  // Outputs from this block.
  DataDeclCollection outputs_;
  // The list of operations that belong to this block. This can be empty.
  OperationList operations_;
  // Maps the outputs of the operations in the list of operations in this
  // block to the corresponding name. Note that a result can have more than
  // one value which is used to represent non-determinism.
  IndexedValueMap results_;
};

// A class that contains a collection of blocks.
class Module {
 public:
  using BlockListType = std::vector<std::unique_ptr<Block>>;
  Module() {}

  Module(const Module&) = delete;
  Module& operator=(const Module&) = delete;
  Module(Module&&) = default;
  Module& operator=(Module&&) = default;

  // Adds a block to the module and returns a pointer to it.
  const Block& AddBlock(std::unique_ptr<Block> block) {
    // Note: this check should be impossible due to this function taking a
    // `unique_ptr` to the `Block`. But it's a cheap thing to check, so might
    // as well just do it.
    CHECK(block->parent_module() == nullptr) << "Attempt to add a Block to two "
                                                "different Modules!";
    block->set_parent_module(*this);
    blocks_.push_back(std::move(block));
    return *blocks_.back();
  }

  const BlockListType& blocks() const { return blocks_; }

  template <typename Derived, typename Result>
  Result Accept(IRVisitor<Derived, Result, false>& visitor) {
    return visitor.Visit(*this);
  }

  template <typename Derived, typename Result>
  Result Accept(IRVisitor<Derived, Result, true>& visitor) const {
    return visitor.Visit(*this);
  }

 private:
  BlockListType blocks_;
};

}  // namespace raksha::ir

#endif  // SRC_IR_MODULE_H_
