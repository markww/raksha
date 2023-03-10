/* Copyright 2022 Google LLC.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// This file contains the datalog IR (DLIR) which makes the translation from
// this authorization logic into datalog simpler.

#ifndef SRC_IR_DATALOG_PROGRAM_H_
#define SRC_IR_DATALOG_PROGRAM_H_

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/string_view.h"

namespace raksha::ir::datalog {

// Used to represent whether a predicate is negated or not
enum Sign { kNegated, kPositive };

// Predicate corresponds to a predicate of the form
// <pred_name>(arg_1, ..., arg_n), and it may or may not be negated
class Predicate {
 public:
  Predicate(std::string name, std::vector<std::string> args, Sign sign)
      : name_(std::move(name)),
        args_(std::move(args)),
        // TODO move surpresses an unused private field error about sign_
        // for now, get rid of this eventually
        sign_(std::move(sign)) {}

  const std::string& name() const { return name_; }
  const std::vector<std::string>& args() const { return args_; }
  Sign sign() const { return sign_; }

  template <typename H>
  friend H AbslHashValue(H h, const Predicate& p) {
    return H::combine(std::move(h), p.name(), p.args(), p.sign());
  }
  // Equality is also needed to use a Predicate in a flat_hash_set
  bool operator==(const Predicate& otherPredicate) const {
    return this->name() == otherPredicate.name() &&
           this->sign() == otherPredicate.sign() &&
           this->args() == otherPredicate.args();
  }

  // < operator is needed for btree_set, which is only used for declarations.
  // Since declarations are uniquely defined by the name of the predicate,
  // this implementation that just uses < on the predicate names should be
  // sufficent in the context where it is used.
  bool operator<(const Predicate& otherPredicate) const {
    return this->name() < otherPredicate.name();
  }

  // (TODO: #672) once an AST node for numbers as RVALUEs in numeric
  // comparisons is added, this visitor should also visit
  // the numeric RVALUEs and add them to the type environment
  // with type ArgumentType(kNumber, "Number")
  // This is a workaround that makes use of the fact that
  // numeric comparisons are represented as predicates
  // with a name that matches the operator:
  // (https://github.com/google-research/raksha/blob/be6ef8e1e1a20735a06637c12db9ed0b87e3d2a2/src/ir/auth_logic/ast_construction.cc#L92)
  bool IsNumericOperator() const {
    static const auto* const numeric_operators = 
      new absl::flat_hash_set<std::string>({
        "<", ">", "=", "!=", "<=", ">="});
    return numeric_operators->contains(name_);
  }

 private:
  std::string name_;
  std::vector<std::string> args_;
  Sign sign_;
};

// Relation declaration types are of 3 forms
// 1. NumberType
// 2. PrincipalType
// 3. User defined CustomType (string to store name of the type)
class ArgumentType {
 public:
  enum class Kind { kNumber, kPrincipal, kCustom };
  Kind kind() const { return kind_; }
  absl::string_view name() const { return name_; }
  static ArgumentType MakePrincipalType() {
    return ArgumentType(Kind::kPrincipal, "Principal");
  }
  static ArgumentType MakeNumberType() {
    return ArgumentType(Kind::kNumber, "Number");
  }
  static ArgumentType MakeCustomType(absl::string_view name) {
    return ArgumentType(Kind::kCustom, name);
  }

  // This is needed because Argument has private members
  // of type ArgumentType, RelationDeclaration has private
  // members of type Argument, and RelationDeclaration
  // appears in an absl::flat_hash_map in the implementation of
  // DeclarationEnvironment.
  template <typename H>
  friend H AbslHashValue(H h, const ArgumentType& typ) {
    return H::combine(std::move(h), typ.name_, typ.kind_);
  }
  // Equality is needed to use a RelationDeclaration in a flat_hash_set
  bool operator==(const ArgumentType& otherType) const {
    return this->kind_ == otherType.kind_ && this->name_ == otherType.name_;
  }
  
  bool operator!=(const ArgumentType& otherType) const {
    return !(*this == otherType);
  }

 private:
  explicit ArgumentType(Kind kind, absl::string_view name)
      : kind_(kind), name_(name) {}
  Kind kind_;
  std::string name_;
};

class Argument {
 public:
  explicit Argument(std::string_view argument_name, ArgumentType argument_type)
      : argument_name_(argument_name),
        argument_type_(std::move(argument_type)) {}
  absl::string_view argument_name() const { return argument_name_; }
  ArgumentType argument_type() const { return argument_type_; }

  bool operator==(const Argument& otherArgument) const {
    return this->argument_name_ == otherArgument.argument_name_ &&
           this->argument_type_ == otherArgument.argument_type_;
  }

  // This is needed because RelationDeclaration has private
  // members of type Argument, and RelationDeclaration
  // appears in an absl::flat_hash_map in the implementation of
  // DeclarationEnvironment.
  template <typename H>
  friend H AbslHashValue(H h, const Argument& arg) {
    return H::combine(std::move(h), arg.argument_name(), arg.argument_type());
  }

 private:
  std::string argument_name_;
  ArgumentType argument_type_;
};

class RelationDeclaration {
 public:
  explicit RelationDeclaration(absl::string_view relation_name,
                               bool is_attribute,
                               std::vector<Argument> arguments)
      : relation_name_(relation_name),
        is_attribute_(is_attribute),
        arguments_(std::move(arguments)) {}
  absl::string_view relation_name() const { return relation_name_; }
  bool is_attribute() const { return is_attribute_; }
  const std::vector<Argument>& arguments() const { return arguments_; }

  bool operator==(const RelationDeclaration& otherDeclaration) const {
    return this->relation_name_ == otherDeclaration.relation_name_ &&
           this->is_attribute_ == otherDeclaration.is_attribute_ &&
           this->arguments_ == otherDeclaration.arguments_;
  }

  // This is needed because RelationDeclaration appears in
  // an absl::flat_hash_map as a part of the implementation
  // of DeclarationEnvironment
  template <typename H>
  friend H AbslHashValue(H h, const RelationDeclaration& rd) {
    return H::combine(std::move(h), rd.relation_name(), rd.is_attribute(),
                      rd.arguments());
  }

 private:
  std::string relation_name_;
  bool is_attribute_;
  std::vector<Argument> arguments_;
};

// A conditional datalog assertion with a left hand side and/or a right hand
// side. A Rule is either:
//    - an unconditional fact which is a predicate
//    - a conditional assertion
class Rule {
 public:
  explicit Rule(Predicate lhs, std::vector<Predicate> rhs)
      : lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}
  explicit Rule(Predicate lhs) : lhs_(std::move(lhs)) {}
  const Predicate& lhs() const { return lhs_; }
  const std::vector<Predicate>& rhs() const { return rhs_; }

  bool operator==(const Rule& otherRule) const {
    return this->lhs_ == otherRule.lhs_ && this->rhs_ == otherRule.rhs_;
  }

 private:
  Predicate lhs_;
  std::vector<Predicate> rhs_;
};

class Program {
 public:
  Program(std::vector<RelationDeclaration> relation_declarations,
          std::vector<Rule> rules, std::vector<std::string> outputs)
      : relation_declarations_(std::move(relation_declarations)),
        rules_(std::move(rules)),
        outputs_(std::move(outputs)) {}
  const std::vector<RelationDeclaration>& relation_declarations() const {
    return relation_declarations_;
  }
  const std::vector<Rule>& rules() const { return rules_; }
  const std::vector<std::string>& outputs() const { return outputs_; }

  bool operator==(const Program& otherProgram) const {
    return this->relation_declarations_ ==
               otherProgram.relation_declarations_ &&
           this->rules_ == otherProgram.rules_ &&
           this->outputs_ == otherProgram.outputs_;
  }

 private:
  std::vector<RelationDeclaration> relation_declarations_;
  std::vector<Rule> rules_;
  std::vector<std::string> outputs_;
};

}  // namespace raksha::ir::datalog

#endif  // SRC_IR_DATALOG_PROGRAM_H_
