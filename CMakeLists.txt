cmake_minimum_required(VERSION 3.10)

project(butterfly C)

find_library(READLINE_LIB NAMES readline)

if(NOT READLINE_LIB)
  message(FATAL_ERROR, "GNU Readline library not found.")
endif()

add_executable(butterfly main.c parser.c backgroundJobs.c builtin.c utility.c execution.c)

target_link_libraries(butterfly PRIVATE ${READLINE_LIB})
