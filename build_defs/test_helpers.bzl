#-----------------------------------------------------------------------------
# Copyright 2021 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#----------------------------------------------------------------------------

load("//build_defs:souffle.bzl", "gen_souffle_cxx_code", "souffle_cc_library")
load("//src/analysis/souffle:dl_file_lists.bzl", "core_dl_files_plus_fact_test_helper")

def extracted_datalog_string_test(
        name,
        dl_string_lib,
        visibility = None):
    """Given a C++ library containing test data that contains Datalog strings
    and a function GatherDatalogStrings to collect those into a vector,
    produce a datalog script containing those rule bodies that should be
    parseable if the strings are parseable.

    Args:
      name: String; Name of the library.
      dl_string_lib: String; The library containing the strings to extract.
      visibility: List; List of visibilities.
    """

    native.cc_binary(
        name = "dl_string_test_file_generator_for_" + name,
        testonly = True,
        srcs = ["//src/common/utils/test/dl_string_extractor:dl_string_test_file_generator.cc"],
        copts = [
            "-std=c++17",
            "-fexceptions",
        ],
        # Turn off header modules, as Google precompiled headers use
        # -fno-exceptions, and combining a precompiled header with
        # -fno-exceptions with a binary that uses -fexceptions makes Clang
        # upset.
        features = ["-use_header_modules"],
        deps = [
            dl_string_lib,
            "//src/common/logging:logging",
            "//src/common/utils/test/dl_string_extractor:datalog_string",
        ],
    )

    native.genrule(
        name = name + "_dl",
        outs = [name + ".dl"],
        testonly = True,
        cmd =
            "$(location dl_string_test_file_generator_for_{name}) $@".format(name = name),
        tools = ["dl_string_test_file_generator_for_" + name],
        visibility = visibility,
    )

    gen_souffle_cxx_code(
        name = name + "_cxx",
        src = name + "_dl",
        for_test = True,
        included_dl_scripts = core_dl_files_plus_fact_test_helper,
    )

    souffle_cc_library(
        name = name + "_souffle_lib",
        src = name + "_cxx",
        testonly = True,
    )

    native.cc_test(
        name = name,
        srcs = ["//src/common/utils/test/dl_string_extractor:dl_string_parsing_test_driver.cc"],
        args = [
            name + "_cxx",
        ],
        copts = [
            "-fexceptions",
            "-Iexternal/souffle/src/include/souffle",
        ],
        # Turn off header modules, as Google precompiled headers use
        # -fno-exceptions, and combining a precompiled header with
        # -fno-exceptions with a binary that uses -fexceptions makes Clang
        # upset.
        features = ["-use_header_modules"],
        linkopts = ["-pthread"],
        deps = [
            name + "_souffle_lib",
            "@souffle//:souffle_include_lib",
        ],
    )

def run_taint_exec_compare_check_results(
        name,
        test_bin,
        input_files,
        visibility = None):
    """Run the Souffle-generated taint executable and ensure that the failing
    check output file is empty.

    Args:
      name: String; Name of the test.
      test_bin: List; List with one element containing the tool to be run to get
                      results. This should be a string, but we ran into bizarre
                      buildozer errors.
      input_files: List; List of labels indicatinginput files to taint_exec.
    """

    facts_dir_opts = [
        "--facts=`dirname $(location {})`".format(input_file)
        for input_file in input_files
    ]

    # Run the taint analysis Souffle binary to generate the Datalog output
    # files, then copy the checkAndResult and expectedCheckAndResult output
    # files to the rule outputs.
    native.genrule(
        name = name + "_test_results",
        outs = ["checkAndResult", "expectedCheckAndResult"],
        srcs = input_files,
        testonly = True,
        cmd = (("$(location {}) ".format(test_bin[0])) +
               "--output=$(RULEDIR) {fact_dirs} && " +
               "cp $(RULEDIR)/checkAndResult.csv $(location :checkAndResult) && " +
               "cp $(RULEDIR)/expectedCheckAndResult.csv $(location :expectedCheckAndResult)")
            .format(fact_dirs = " ".join(facts_dir_opts)),
        tools = test_bin,
        visibility = visibility,
    )

    # Sort the checkAndResult contents. Because the check name is the first
    # field, this should sort them by name.
    native.genrule(
        name = name + "_check_and_result_sorted",
        outs = ["checkAndResultSorted"],
        srcs = [":checkAndResult"],
        testonly = True,
        cmd = "sort $< > $@",
        visibility = visibility,
    )

    # Similarly, sort expectedCheckAndResult.
    native.genrule(
        name = name + "_expected_check_and_result_sorted",
        outs = ["expectedCheckAndResultSorted"],
        srcs = [":expectedCheckAndResult"],
        testonly = True,
        cmd = "sort $< > $@",
        visibility = visibility,
    )

    # Compare the two files for diff equality.
    native.sh_test(
        name = name,
        args = ["$(location :expectedCheckAndResultSorted)", "$(location :checkAndResultSorted)"],
        data = [":checkAndResultSorted", ":expectedCheckAndResultSorted"],
        srcs = ["//src/analysis/souffle/tests/arcs_fact_tests:diff_wrapper.sh"],
    )

def output_rule_diff_test(
        name,
        test_bin,
        input_files,
        output_fact,
        expected_contents_csv,
        visibility = None):
    """Run the Souffle-generated taint executable and diff the results contained in a single output relation against those in the expected_contents_csv.

    Args:
      name: String; Name of the test.
      test_bin: List; List with one element containing the tool to be run to get
                      results. This should be a string, but we ran into bizarre
                      buildozer errors.
      input_files: List; List of labels indicating input files to the analysis.
      output_fact: String; The single output relation to be diffed.
      expected_contents_csv: String; The label indicating the expected result.
    """

    facts_dir_opts = [
        "--facts=`dirname $(location {})`".format(input_file)
        for input_file in input_files
    ]

    test_bin_label = test_bin[0]

    # Run the taint analysis Souffle binary to generate the Datalog output
    # files, then copy the checkAndResult and expectedCheckAndResult output
    # files to the rule outputs.
    native.genrule(
        name = name + "_test_results",
        outs = ["resultFact"],
        srcs = input_files,
        testonly = True,
        cmd = (
            ("$(location {}) ".format(test_bin_label)) +
            ("--output=$(RULEDIR) {fact_dirs} && ".format(fact_dirs = " ".join(facts_dir_opts))) +
            ("cp $(RULEDIR)/{}.csv $@".format(output_fact))
        ),
        tools = [test_bin_label],
        visibility = visibility,
    )

    native.genrule(
        name = name + "_result_sorted",
        outs = ["resultFactSorted"],
        srcs = [":resultFact"],
        testonly = True,
        cmd = "sort $< > $@",
        visibility = visibility,
    )

    # Compare the result for diff equality against expected.
    native.sh_test(
        name = name,
        args = ["$(location {})".format(expected_contents_csv), "$(location :resultFact)"],
        data = [expected_contents_csv, ":resultFact"],
        srcs = ["//src/analysis/souffle/tests/arcs_fact_tests:diff_wrapper.sh"],
    )

def convert_ir_to_is_operation_facts(
        name,
        ir_file,
        out_file,
        visibility = None):
    """Convert a file containing Raksha IR into Souffle isOperation facts.

    Args:
      name: String; Name of the file to be produced.
      ir_file: String; the IR file to be converted.
      out_file: String; the resulting isOperation facts file.
      visibility: List; the visibilities of the resulting file.
    """
    ir_to_operation_facts_target = "//src/parser/ir:ir_to_operation_facts"
    native.genrule(
        name = name + "_generate_is_operation",
        outs = [out_file],
        srcs = [ir_file],
        tools = [ir_to_operation_facts_target],
        cmd = "$(location {}) --ir_file $< --out $@".format(ir_to_operation_facts_target),
        visibility = visibility,
    )
