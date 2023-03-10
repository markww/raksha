#-------------------------------------------------------------------------------
# Copyright 2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
#-------------------------------------------------------------------------------
load("//build_defs:package_groups.oss.bzl", "frontend_packages")

package(
    default_visibility = [
        "//intelligence/federated_perception:__subpackages__",
        "//src:__subpackages__",
    ],
    features = ["layering_check"],
    licenses = ["notice"],
)

cc_library(
    name = "block_builder",
    hdrs = ["block_builder.h"],
    visibility = ["//src:__subpackages__"] + frontend_packages,
    deps = [
        ":module",
        ":value",
        "//src/ir/types",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_test(
    name = "block_builder_test",
    srcs = [
        "block_builder_test.cc",
    ],
    deps = [
        ":block_builder",
        ":ir_context",
        ":ssa_names",
        ":value_string_converter",
        "//src/common/testing:gtest",
        "//src/ir/attributes:attribute",
        "//src/ir/types",
    ],
)

cc_library(
    name = "data_decl",
    hdrs = ["data_decl.h"],
    deps = [
        "//src/ir/types",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_test(
    name = "data_decl_test",
    srcs = ["data_decl_test.cc"],
    deps = [
        ":data_decl",
        "//src/common/testing:gtest",
        "//src/ir/types",
    ],
)

cc_library(
    name = "ir_context",
    hdrs = ["ir_context.h"],
    visibility = ["//:__subpackages__"] + frontend_packages,
    deps = [
        ":operator",
        ":storage",
        "//src/ir/types",
        "@com_google_absl//absl/container:flat_hash_map",
    ],
)

cc_test(
    name = "ir_context_test",
    srcs = ["ir_context_test.cc"],
    deps = [
        ":ir_context",
        "//src/common/testing:gtest",
        "//src/ir/types",
    ],
)

cc_library(
    name = "ir_printer",
    hdrs = ["ir_printer.h"],
    deps = [
        ":ir_traversing_visitor",
        ":module",
        ":ssa_names",
        ":value",
        ":value_string_converter",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "ir_printer_test",
    srcs = ["ir_printer_test.cc"],
    copts = ["-fexceptions"],
    features = ["-use_header_modules"],
    deps = [
        ":block_builder",
        ":ir_printer",
        ":module",
        ":value",
        "//src/common/testing:gtest",
        "//src/ir/attributes:string_attribute",
        "//src/parser/ir:ir_parser",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_googlesource_code_re2//:re2",
    ],
)

cc_library(
    name = "ir_traversing_visitor",
    hdrs = ["ir_traversing_visitor.h"],
    deps = [
        ":ir_visitor",
        ":module",
        "//src/common/utils:fold",
        "//src/common/utils:types",
    ],
)

cc_test(
    name = "ir_traversing_visitor_test",
    srcs = ["ir_traversing_visitor_test.cc"],
    deps = [
        ":block_builder",
        ":ir_traversing_visitor",
        ":module",
        "//src/common/testing:gtest",
    ],
)

cc_library(
    name = "operator",
    hdrs = ["operator.h"],
    visibility = ["//src:__subpackages__"] + frontend_packages,
    deps = ["@com_google_absl//absl/strings"],
)

cc_test(
    name = "operator_test",
    srcs = ["operator_test.cc"],
    deps = [
        ":operator",
        "//src/common/testing:gtest",
    ],
)

cc_test(
    name = "operation_test",
    srcs = ["operation_test.cc"],
    deps = [
        ":ir_printer",
        ":module",
        ":ssa_names",
        ":value",
        "//src/common/testing:gtest",
        "//src/ir/attributes:float_attribute",
        "//src/ir/attributes:int_attribute",
        "//src/ir/attributes:string_attribute",
        "@com_google_absl//absl/container:flat_hash_set",
    ],
)

cc_library(
    name = "ir_visitor",
    hdrs = ["ir_visitor.h"],
    deps = ["//src/common/utils:types"],
)

cc_library(
    name = "module",
    hdrs = ["module.h"],
    visibility = ["//src:__subpackages__"] + frontend_packages,
    deps = [
        ":data_decl",
        ":ir_visitor",
        ":operator",
        ":storage",
        ":value",
        "//src/common/utils:types",
        "//src/ir/attributes:attribute",
        "//src/ir/types",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "module_test",
    srcs = ["module_test.cc"],
    deps = [
        ":module",
        ":storage",
        ":value",
        "//src/common/testing:gtest",
        "//src/common/utils:map_iter",
        "//src/ir/types",
        "@fuzztest//fuzztest",
    ],
)

cc_library(
    name = "ssa_names",
    hdrs = ["ssa_names.h"],
    visibility = ["//src:__subpackages__"] + frontend_packages,
    deps = [
        ":value",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/log:check",
    ],
)

cc_test(
    name = "ssa_names_test",
    srcs = ["ssa_names_test.cc"],
    deps = [
        ":module",
        ":operator",
        ":ssa_names",
        "//src/common/testing:gtest",
    ],
)

cc_library(
    name = "storage",
    hdrs = ["storage.h"],
    deps = [
        ":value",
        "//src/ir/types",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "storage_test",
    srcs = ["storage_test.cc"],
    deps = [
        ":module",
        ":storage",
        ":value",
        "//src/common/testing:gtest",
        "//src/ir/types",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "value",
    hdrs = ["value.h"],
    visibility = ["//src:__subpackages__"] + frontend_packages,
    deps = [
        "//src/common/utils:iterator_range",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "value_test",
    srcs = ["value_test.cc"],
    deps = [
        ":ir_printer",
        ":module",
        ":ssa_names",
        ":storage",
        ":value",
        "//src/common/testing:gtest",
        "//src/ir/types",
        "@com_google_absl//absl/hash:hash_testing",
    ],
)

cc_library(
    name = "value_string_converter",
    srcs = ["value_string_converter.cc"],
    hdrs = ["value_string_converter.h"],
    deps = [
        ":ssa_names",
        ":storage",
        ":value",
        "//src/common/utils:overloaded",
    ],
)

cc_library(
    name = "access_path_selectors",
    hdrs = [
        "access_path_selectors.h",
        "access_path_selectors_set.h",
        "field_selector.h",
        "selector.h",
    ],
    deps = [
        "//src/common/logging",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/hash",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "access_path_selectors_test",
    srcs = ["access_path_selectors_test.cc"],
    deps = [
        ":access_path_selectors",
        "//src/common/testing:gtest",
        "@com_google_absl//absl/hash:hash_testing",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "access_path_selectors_set_test",
    srcs = ["access_path_selectors_set_test.cc"],
    deps = [
        ":access_path_selectors",
        "//src/common/testing:gtest",
        "@com_google_absl//absl/container:btree",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "field_selector_test",
    srcs = ["field_selector_test.cc"],
    deps = [
        ":access_path_selectors",
        "//src/common/testing:gtest",
        "@com_google_absl//absl/hash:hash_testing",
    ],
)

cc_test(
    name = "selector_test",
    srcs = ["selector_test.cc"],
    deps = [
        ":access_path_selectors",
        "//src/common/testing:gtest",
        "@com_google_absl//absl/hash:hash_testing",
    ],
)

cc_library(
    name = "proto_to_ir",
    srcs = ["proto_to_ir.cc"],
    hdrs = ["proto_to_ir.h"],
    deps = [
        ":block_builder",
        ":ir_context",
        ":module",
        ":ssa_names",
        ":value",
        "//src/common/proto:json_util",
        "//src/ir/attributes:attribute",
        "//src/ir/attributes:float_attribute",
        "//src/ir/attributes:int_attribute",
        "//src/ir/attributes:string_attribute",
        "//src/ir/proto:ir_cc_proto",
        "@com_google_absl//absl/status:statusor",
    ],
)

cc_library(
    name = "ir_to_proto",
    srcs = ["ir_to_proto.cc"],
    hdrs = ["ir_to_proto.h"],
    deps = [
        ":ir_context",
        ":module",
        ":ssa_names",
        ":value",
        "//src/ir/attributes:attribute",
        "//src/ir/attributes:float_attribute",
        "//src/ir/attributes:int_attribute",
        "//src/ir/attributes:string_attribute",
        "//src/ir/proto:ir_cc_proto",
    ],
)

cc_test(
    name = "ir_to_proto_test",
    srcs = ["ir_to_proto_test.cc"],
    copts = ["-fexceptions"],
    features = ["-use_header_modules"],
    deps = [
        ":ir_printer",
        ":ir_to_proto",
        ":proto_to_ir",
        "//src/common/proto:proto_message_differencer",
        "//src/common/proto:protobuf",
        "//src/common/testing:gtest",
        "//src/ir/proto:ir_cc_proto",
        "//src/parser/ir:ir_parser",
    ],
)
