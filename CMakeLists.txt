cmake_minimum_required(VERSION 2.8)
project(process_tool)

file(GLOB_RECURSE src "src/*.cpp")
file(GLOB_RECURSE headers "include/*.h")

# Source groupping
foreach(f ${src})
  file(RELATIVE_PATH SRCGR ${CMAKE_CURRENT_SOURCE_DIR} ${f})
  set(SRCGR "Sources/${SRCGR}")
  string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})
  string(REPLACE / \\ SRCGR ${SRCGR})
  source_group("${SRCGR}" FILES ${f})
endforeach()
foreach(f ${headers})
  file(RELATIVE_PATH SRCGR ${CMAKE_CURRENT_SOURCE_DIR} ${f})
  set(SRCGR "Headers/${SRCGR}")
  string(REGEX REPLACE "(.*)(/[^/]*)$" "\\1" SRCGR ${SRCGR})
  string(REPLACE / \\ SRCGR ${SRCGR})
  source_group("${SRCGR}" FILES ${f})
endforeach()
                
add_executable(${PROJECT_NAME} ${src} ${headers})

# Select crt version for windows builds
# http://stackoverflow.com/questions/10199904/how-can-i-set-specific-compiler-flags-for-for-a-specific-target-in-a-specific-bu
target_compile_options(${PROJECT_NAME} PRIVATE "/MT$<$<CONFIG:Debug>:d>")
target_compile_definitions(${PROJECT_NAME} PRIVATE "_VARIADIC_MAX=10")

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
