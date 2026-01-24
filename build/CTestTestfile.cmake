# CMake generated Testfile for 
# Source directory: E:/airs/Simulation
# Build directory: E:/airs/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
if(CTEST_CONFIGURATION_TYPE MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
  add_test(harness_tests "E:/airs/build/Debug/harness_tests.exe")
  set_tests_properties(harness_tests PROPERTIES  LABELS "harness" _BACKTRACE_TRIPLES "E:/airs/Simulation/CMakeLists.txt;456;add_test;E:/airs/Simulation/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
  add_test(harness_tests "E:/airs/build/Release/harness_tests.exe")
  set_tests_properties(harness_tests PROPERTIES  LABELS "harness" _BACKTRACE_TRIPLES "E:/airs/Simulation/CMakeLists.txt;456;add_test;E:/airs/Simulation/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
  add_test(harness_tests "E:/airs/build/MinSizeRel/harness_tests.exe")
  set_tests_properties(harness_tests PROPERTIES  LABELS "harness" _BACKTRACE_TRIPLES "E:/airs/Simulation/CMakeLists.txt;456;add_test;E:/airs/Simulation/CMakeLists.txt;0;")
elseif(CTEST_CONFIGURATION_TYPE MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
  add_test(harness_tests "E:/airs/build/RelWithDebInfo/harness_tests.exe")
  set_tests_properties(harness_tests PROPERTIES  LABELS "harness" _BACKTRACE_TRIPLES "E:/airs/Simulation/CMakeLists.txt;456;add_test;E:/airs/Simulation/CMakeLists.txt;0;")
else()
  add_test(harness_tests NOT_AVAILABLE)
endif()
subdirs("_deps/googletest-build")
subdirs("dase_cli")
