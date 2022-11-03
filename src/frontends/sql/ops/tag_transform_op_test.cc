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
#include "src/frontends/sql/ops/tag_transform_op.h"

#include <algorithm>

#include "src/common/testing/gtest.h"
#include "src/frontends/sql/ops/example_value_test_helper.h"
#include "src/frontends/sql/ops/sql_op.h"
#include "src/ir/attributes/attribute.h"
#include "src/ir/attributes/int_attribute.h"
#include "src/ir/attributes/string_attribute.h"
#include "src/ir/ir_context.h"
#include "src/ir/value.h"

namespace raksha::frontends::sql {
namespace {

using ::testing::Combine;
using ::testing::ValuesIn;

class TagTransformOpTest
    : public ::testing::TestWithParam<
          std::tuple<const ir::Block*, absl::string_view, ir::Value,
                     std::vector<std::pair<std::string, ir::Value>>>> {
 public:
  TagTransformOpTest() { SqlOp::RegisterOperator<TagTransformOp>(context); }

 protected:
  ir::IRContext context;
};

TEST_P(TagTransformOpTest, TestSomething) {
  const auto& [parent_block, policy_rule_name, xformed_value, preconditions] =
      GetParam();
  std::unique_ptr<ir::Operation> tag_transform_op = TagTransformOp::Create(
      parent_block, context, policy_rule_name, xformed_value, preconditions);

  EXPECT_EQ(tag_transform_op->op().name(), OpTraits<TagTransformOp>::kName);
  EXPECT_EQ(tag_transform_op->parent(), parent_block);

  const ir::ValueList& inputs = tag_transform_op->inputs();
  EXPECT_EQ(inputs.size(), preconditions.size() + 1);
  ASSERT_GT(inputs.size(), 0);
  EXPECT_EQ(inputs.front(), xformed_value)
      << "First operand is not the transformed value.";
  ir::ValueList precondition_values;
  ir::NamedAttributeMap attributes;
  attributes.insert(
      {std::string(TagTransformOp::kRuleNameAttribute),
       ir::Attribute::Create<ir::StringAttribute>(policy_rule_name)});
  absl::c_for_each(
      preconditions, [&attributes, &precondition_values,
                      index = 1](const auto& precondition) mutable {
        attributes.insert({precondition.first,
                           ir::Attribute::Create<ir::Int64Attribute>(index++)});
        precondition_values.push_back(precondition.second);
      });
  EXPECT_THAT(utils::make_range(inputs.begin() + 1, inputs.end()),
              ::testing::ElementsAreArray(precondition_values));
  EXPECT_EQ(tag_transform_op->attributes(), attributes);
}

TEST_P(TagTransformOpTest, AccessorsReturnTheCorrectValue) {
  const auto& [parent_block, policy_rule_name, xformed_value, preconditions] =
      GetParam();
  std::unique_ptr<TagTransformOp> tag_transform_op = TagTransformOp::Create(
      parent_block, context, policy_rule_name, xformed_value, preconditions);
  EXPECT_EQ(tag_transform_op->GetRuleName(), policy_rule_name);
  EXPECT_EQ(tag_transform_op->GetTransformedValue(), xformed_value);
  EXPECT_THAT(tag_transform_op->GetPreconditions(),
              ::testing::UnorderedElementsAreArray(preconditions));
}

TEST(TagTransformOpDeathTest, DuplicatePreconditionNamesAreDetected) {
  ir::IRContext context;
  SqlOp::RegisterOperator<TagTransformOp>(context);
  auto any_value = ir::Value(ir::value::Any());
  EXPECT_DEATH(
      {
        TagTransformOp::Create(
            nullptr, context, "test", any_value,
            {{"some", any_value}, {"another", any_value}, {"some", any_value}});
      },
      "Duplicate precondition name 'some'.");
}

static testing::ExampleValueTestHelper example_helper;

static const absl::string_view kSampleRuleNames[] = {
    "clear_milliseconds",
    "SSN",
    "ApprovedWindow",
};

static const ir::Block* kSampleBlocks[] = {nullptr, &example_helper.GetBlock()};
static const ir::Value kSampleValues[] = {
    example_helper.GetAny(),
    example_helper.GetOperationResult("out0"),
    example_helper.GetOperationResult("out1"),
    example_helper.GetBlockArgument("arg0"),
    example_helper.GetBlockArgument("arg1"),
};

static const std::vector<std::pair<std::string, ir::Value>>
    kSamplePreconditions[] = {
        {},
        {{"a", example_helper.GetAny()},
         {"b", example_helper.GetBlockArgument("b")},
         {"c", example_helper.GetOperationResult("c")},
         {"d", example_helper.GetBlockArgument("d")}},
        {{"one", example_helper.GetAny()},
         {"two", example_helper.GetBlockArgument("two")},
         {"three", example_helper.GetOperationResult("three")}}};

INSTANTIATE_TEST_SUITE_P(TagTransformOpTest, TagTransformOpTest,
                         Combine(ValuesIn(kSampleBlocks),
                                 ValuesIn(kSampleRuleNames),
                                 ValuesIn(kSampleValues),
                                 ValuesIn(kSamplePreconditions)));
}  // namespace
}  // namespace raksha::frontends::sql
