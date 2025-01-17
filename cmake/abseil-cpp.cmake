# Copyright 2019 trace_agent authors.
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

if(TARGET absl::strings)
  # If absl is included already, skip including it.
  # (https://github.com/trace_agent/trace_agent/issues/29608)
elseif(trace_agent_ABSL_PROVIDER STREQUAL "module")
  if(NOT ABSL_ROOT_DIR)
    set(ABSL_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/thrid_party/abseil-cpp)
  endif()
  message("ABSL_ROOT_DIR:${ABSL_ROOT_DIR}")
  if(EXISTS "${ABSL_ROOT_DIR}/CMakeLists.txt")
    if(trace_agent_INSTALL)
      # When trace_agent_INSTALL is enabled and Abseil will be built as a module,
      # Abseil will be installed along with trace_agent for convenience.
      set(ABSL_ENABLE_INSTALL ON)
    endif()
    add_subdirectory(${ABSL_ROOT_DIR})
  else()
    message(WARNING "trace_agent_ABSL_PROVIDER is \"module\" but ABSL_ROOT_DIR is wrong")
  endif()
  if(trace_agent_INSTALL AND NOT _trace_agent_INSTALL_SUPPORTED_FROM_MODULE)
    message(WARNING "trace_agent_INSTALL will be forced to FALSE because trace_agent_ABSL_PROVIDER is \"module\" and CMake version (${CMAKE_VERSION}) is less than 3.13.")
    set(trace_agent_INSTALL FALSE)
  endif()
elseif(trace_agent_ABSL_PROVIDER STREQUAL "package")
  # Use "CONFIG" as there is no built-in cmake module for absl.
  find_package(absl REQUIRED CONFIG)
endif()
set(_trace_agent_FIND_ABSL "if(NOT TARGET absl::strings)\n  find_package(absl CONFIG)\nendif()")
