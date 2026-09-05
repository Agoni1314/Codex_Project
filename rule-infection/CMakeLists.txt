cmake_minimum_required(VERSION 3.20)

project(RuleInfection VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 优先使用开发环境中已有的 raylib；没有时只在构建目录下载依赖，避免修改系统。
find_package(raylib 5.5 QUIET)

if(NOT raylib_FOUND)
    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        raylib
        URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP OFF
    )
    FetchContent_MakeAvailable(raylib)
endif()

add_library(rule_infection_core
    src/Creature.cpp
    src/CreatureManager.cpp
    src/Entity.cpp
    src/Food.cpp
    src/FoodManager.cpp
    src/Map.cpp
    src/RuleEconomy.cpp
    src/RuleSystem.cpp
    src/WorldSimulation.cpp
    src/WorldStats.cpp
)

target_include_directories(rule_infection_core PUBLIC src)
target_link_libraries(rule_infection_core PUBLIC raylib)

add_executable(rule_infection
    src/main.cpp
    src/Game.cpp
    src/RuleEditorUI.cpp
    src/WorldStatsUI.cpp
)

target_link_libraries(rule_infection PRIVATE rule_infection_core)

if(WIN32)
    # 保留控制台窗口，便于初学阶段查看 raylib 的启动或错误信息。
    target_link_libraries(rule_infection PRIVATE winmm)
endif()

include(CTest)

if(BUILD_TESTING)
    add_executable(map_tests tests/MapTests.cpp)
    target_link_libraries(map_tests PRIVATE rule_infection_core)
    add_test(NAME map_data_and_coordinates COMMAND map_tests)

    add_executable(creature_tests tests/CreatureTests.cpp)
    target_link_libraries(creature_tests PRIVATE rule_infection_core)
    add_test(NAME creature_creation_rules COMMAND creature_tests)

    add_executable(movement_tests tests/MovementTests.cpp)
    target_link_libraries(movement_tests PRIVATE rule_infection_core)
    add_test(NAME creature_movement_rules COMMAND movement_tests)

    add_executable(faction_perception_tests tests/FactionPerceptionTests.cpp)
    target_link_libraries(faction_perception_tests PRIVATE rule_infection_core)
    add_test(NAME faction_and_perception_rules COMMAND faction_perception_tests)

    add_executable(behavior_tests tests/BehaviorTests.cpp)
    target_link_libraries(behavior_tests PRIVATE rule_infection_core)
    add_test(NAME chase_and_flee_rules COMMAND behavior_tests)

    add_executable(combat_tests tests/CombatTests.cpp)
    target_link_libraries(combat_tests PRIVATE rule_infection_core)
    add_test(NAME creature_combat_rules COMMAND combat_tests)

    add_executable(rule_system_tests tests/RuleSystemTests.cpp)
    target_link_libraries(rule_system_tests PRIVATE rule_infection_core)
    add_test(NAME faction_rule_evaluation COMMAND rule_system_tests)

    add_executable(rule_economy_tests tests/RuleEconomyTests.cpp)
    target_link_libraries(rule_economy_tests PRIVATE rule_infection_core)
    add_test(NAME rule_point_economy COMMAND rule_economy_tests)

    add_executable(hunger_tests tests/HungerTests.cpp)
    target_link_libraries(hunger_tests PRIVATE rule_infection_core)
    add_test(NAME creature_hunger COMMAND hunger_tests)

    add_executable(food_tests tests/FoodTests.cpp)
    target_link_libraries(food_tests PRIVATE rule_infection_core)
    add_test(NAME food_creation_and_seeking COMMAND food_tests)

    add_executable(rule_combination_tests tests/RuleCombinationTests.cpp)
    target_link_libraries(rule_combination_tests PRIVATE rule_infection_core)
    add_test(NAME combined_rule_logic COMMAND rule_combination_tests)

    add_executable(rule_editing_tests tests/RuleEditingTests.cpp)
    target_link_libraries(rule_editing_tests PRIVATE rule_infection_core)
    add_test(NAME safe_rule_editing COMMAND rule_editing_tests)

    add_executable(rule_editor_ui_tests
        tests/RuleEditorUITests.cpp
        src/RuleEditorUI.cpp
    )
    target_link_libraries(rule_editor_ui_tests PRIVATE rule_infection_core)
    add_test(NAME rule_editor_selection_safety COMMAND rule_editor_ui_tests)

    add_executable(rule_list_tests tests/RuleListTests.cpp)
    target_link_libraries(rule_list_tests PRIVATE rule_infection_core)
    add_test(NAME rule_list_management COMMAND rule_list_tests)

    add_executable(world_stats_tests tests/WorldStatsTests.cpp)
    target_link_libraries(world_stats_tests PRIVATE rule_infection_core)
    add_test(NAME world_statistics COMMAND world_stats_tests)

    add_executable(simulation_tests tests/SimulationTests.cpp)
    target_link_libraries(simulation_tests PRIVATE rule_infection_core)
    add_test(NAME simulation_lifecycle COMMAND simulation_tests)
endif()
