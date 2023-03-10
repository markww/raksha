//-------------------------------------------------------------------------------
// Copyright 2022 Google LLC
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
//
//-------------------------------------------------------------------------------

#include <algorithm>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>

#include "src/common/testing/gtest.h"
#include "src/common/utils/map_iter.h"
#include "src/ir/auth_logic/ast.h"
#include "src/ir/auth_logic/ast_construction.h"
#include "src/ir/auth_logic/ast_string.h"

namespace raksha::ir::auth_logic {

struct SaysAssertionTestData {
  std::string input_text;
  std::string principal;
  std::vector<std::string> assertions;
};

class AstConstructionTest
    : public testing::TestWithParam<SaysAssertionTestData> {};

class SaysAssertionAstConstructionTest
    : public testing::TestWithParam<SaysAssertionTestData> {};

TEST_P(SaysAssertionAstConstructionTest, SimpleTestWithSaysAssertionProgram) {
  const auto& [input_text, principal, assertions] = GetParam();
  Program prog_out = ParseProgram(input_text);
  ASSERT_EQ(prog_out.says_assertions().size(), 1);
  const SaysAssertion& says_assertion = prog_out.says_assertions().front();
  EXPECT_EQ(says_assertion.principal().name(), principal);
  EXPECT_EQ(says_assertion.assertions().size(), assertions.size());
  std::vector<std::string> assertion_string = utils::MapIter<std::string>(
      says_assertion.assertions(),
      [](Assertion assertion) { return ToString(assertion); });
  EXPECT_THAT(assertion_string,
              ::testing::UnorderedElementsAreArray(assertions));
}

INSTANTIATE_TEST_SUITE_P(
    SaysAssertionAstConstructionTest, SaysAssertionAstConstructionTest,
    testing::Values(
        SaysAssertionTestData({R"("UserA" says { currTimeIs(2022).
                               mayA(NotifyA) :- currTimeIs > 2021. })",
                               R"("UserA")",
                               {"currTimeIs(2022).",
                                "mayA(NotifyA) :- currTimeIs > 2021."}}),
        SaysAssertionTestData({R"("UserB" says { currTimeIs(2020).
                               mayB(NotifyB) :- currTimeIs < 2021. })",
                               R"("UserB")",
                               {"currTimeIs(2020).",
                                "mayB(NotifyB) :- currTimeIs < 2021."}}),
        SaysAssertionTestData({R"("UserC" says { currTimeIs(2021).
                               mayC(NotifyC) :- currTimeIs = 2021). })",
                               R"("UserC")",
                               {"currTimeIs(2021).",
                                "mayC(NotifyC) :- currTimeIs = 2021."}}),
        SaysAssertionTestData({R"("UserC" says { currTimeIs(2021).
                               mayC(NotifyC) :- currTimeIs >= 2021). })",
                               R"("UserC")",
                               {"currTimeIs(2021).",
                                "mayC(NotifyC) :- currTimeIs >= 2021."}}),
        SaysAssertionTestData({R"("UserC" says { currTimeIs("""A20
21""").
                               mayC(NotifyC) :- """currTime
Is""" <= 2021). })",
                               R"("UserC")",
                               {R"(currTimeIs("A2021").)",
                                R"(mayC(NotifyC) :- "currTimeIs" <= 2021.)"}}),
        SaysAssertionTestData({
            R"("OakTeam" says { ourFavoritePublicKeyIs("""-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAErnyr7XY6gxXPnqRGYEpv5lDsbBeX
izVFLckaF/cbgBuzx7kfmf1qi2j+3DDOQYgISQOj9/LYt4c1cxbt+XtG4g==
-----END PUBLIC KEY-----""").
            ourFavoriteTrustedBuilderIs("https://github.com/project-oak/transparent-release"). })",
            R"("OakTeam")",
            {R"(ourFavoritePublicKeyIs("-----BEGIN PUBLIC KEY-----MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAErnyr7XY6gxXPnqRGYEpv5lDsbBeXizVFLckaF/cbgBuzx7kfmf1qi2j+3DDOQYgISQOj9/LYt4c1cxbt+XtG4g==-----END PUBLIC KEY-----").)",
             R"(ourFavoriteTrustedBuilderIs("https://github.com/project-oak/transparent-release").)"}}),
        SaysAssertionTestData({
            R"("OakTeam" says { ourFavoritePublicKeyIs("""-----BEGIN PUBLIC KEY-----
MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAErnyr7XY6gxXPnqRGYEpv5lDsbBeX
izVFLckaF/cbgBuzx7kfmf1qi2j+3DDOQYgISQOj9/LYt4c1cxbt+XtG4g==
-----END PUBLIC KEY-----""").
            ourFavoriteTrustedBuilderIs("https://github.com/Attestations/GitHubHostedActions@v1").})",
            R"("OakTeam")",
            {R"(ourFavoritePublicKeyIs("-----BEGIN PUBLIC KEY-----MFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAErnyr7XY6gxXPnqRGYEpv5lDsbBeXizVFLckaF/cbgBuzx7kfmf1qi2j+3DDOQYgISQOj9/LYt4c1cxbt+XtG4g==-----END PUBLIC KEY-----").)",
             R"(ourFavoriteTrustedBuilderIs("https://github.com/Attestations/GitHubHostedActions@v1").)"}})));

}  // namespace raksha::ir::auth_logic
