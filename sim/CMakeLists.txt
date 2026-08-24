cmake_minimum_required(VERSION 3.16)
project(pi7_leg_sim C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
add_compile_options(-Wall -Wextra)

# Projeto CMake independente do CMakeLists.txt raiz (que faz cross-compile
# arm-none-eabi para o Pico via pico_sdk_import.cmake). Nunca adicionar este
# diretorio via add_subdirectory() do root -- configure/compile a parte:
#   cmake -S sim -B sim/build && cmake --build sim/build

set(MUJOCO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/third_party/mujoco" CACHE PATH
    "Caminho do SDK do MuJoCo extraido (ver sim/README.md)")

# O tarball oficial de release do MuJoCo (linux-x86_64) nao traz um
# mujocoConfig.cmake para find_package(CONFIG) -- so include/ + lib/ soltos.
# Monta-se aqui um alvo IMPORTED equivalente manualmente.
find_path(MUJOCO_INCLUDE_DIR mujoco/mujoco.h PATHS "${MUJOCO_DIR}/include" NO_DEFAULT_PATH)
find_library(MUJOCO_LIBRARY mujoco PATHS "${MUJOCO_DIR}/lib" NO_DEFAULT_PATH)
if(NOT MUJOCO_INCLUDE_DIR OR NOT MUJOCO_LIBRARY)
  message(FATAL_ERROR "MuJoCo SDK nao encontrado em ${MUJOCO_DIR} -- ver sim/README.md")
endif()
add_library(mujoco::mujoco SHARED IMPORTED)
set_target_properties(mujoco::mujoco PROPERTIES
  IMPORTED_LOCATION "${MUJOCO_LIBRARY}"
  INTERFACE_INCLUDE_DIRECTORIES "${MUJOCO_INCLUDE_DIR}"
)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

add_library(pi7_leg_core STATIC
  src/kinematics.c
  src/pid.c
  src/trajectory.c
  src/sim_common.c
)
target_include_directories(pi7_leg_core PUBLIC include)
target_link_libraries(pi7_leg_core PUBLIC mujoco::mujoco m)

add_executable(leg_view src/main_viewer.c)
target_link_libraries(leg_view PRIVATE pi7_leg_core glfw OpenGL::GL)

add_executable(leg_record src/main_record.c)
target_link_libraries(leg_record PRIVATE pi7_leg_core glfw OpenGL::GL)

# para os binarios acharem libmujoco.so.* em runtime sem precisar de LD_LIBRARY_PATH
set_target_properties(leg_view leg_record PROPERTIES
  BUILD_RPATH   "${MUJOCO_DIR}/lib"
  INSTALL_RPATH "${MUJOCO_DIR}/lib"
)

add_executable(test_kinematics tests/test_kinematics.c src/kinematics.c)
target_include_directories(test_kinematics PRIVATE include)
target_link_libraries(test_kinematics PRIVATE m)

enable_testing()
add_test(NAME kinematics_sanity COMMAND test_kinematics)
