cmake_minimum_required(VERSION 3.20)
project(SangokushiRules LANGUAGES CXX)

# Portable simulation target: no Siv3D SDK, textures, audio or windowing required.
# The native game remains in Sangokushi.sln. This library can later be linked
# into another engine through a deliberately small adapter.
add_library(sangokushi_rules STATIC
    NewGame/TacticChain.cpp
    NewGame/TacticCatalog.cpp
    NewGame/TacticChainActions.cpp
    NewGame/BattleIntelligence.cpp
    NewGame/StrategistJourney.cpp
    NewGame/StrategistJourneyAI.cpp
    NewGame/JourneyEnemyPlan.cpp
    NewGame/JourneyRealtime.cpp
    NewGame/JourneyRealtimeRouting.cpp
    NewGame/JourneyMarch.cpp
    NewGame/JourneySave.cpp
    NewGame/JourneySaveValidation.cpp
    NewGame/JourneyArchiveWorld.cpp
    NewGame/JourneyArchiveBattle.cpp
    NewGame/JourneyArchiveSession.cpp
    NewGame/JourneyOpening.cpp
    NewGame/JourneyBattleStory.cpp
    NewGame/JourneyAssignment.cpp
    NewGame/JourneyAssignmentOrders.cpp
    NewGame/JourneyPlanExample.cpp
    NewGame/JourneyCampaign.cpp
    NewGame/JourneyCampaignDrama.cpp
    NewGame/JourneyDrama.cpp
    NewGame/JourneyCouncilDrama.cpp
    NewGame/JourneyEpisode.cpp
    NewGame/JourneyEpisodeCouncil.cpp
    NewGame/JourneyEpisodeAftermath.cpp
    NewGame/JourneyChainSignal.cpp
    NewGame/JourneyBattleWatch.cpp
    NewGame/JourneyOrderRead.cpp
    NewGame/StrategistJourneyObjectives.cpp
    NewGame/StrategistBattle.cpp
    NewGame/StrategistBattleIntelligence.cpp
    NewGame/StrategistBattleOperations.cpp
    NewGame/Campaign.cpp
    NewGame/CampaignRoster.cpp
    NewGame/CampaignRegions.cpp
    NewGame/CampaignAI.cpp
    NewGame/CampaignAIPlanning.cpp
    NewGame/CampaignAIArmies.cpp
    NewGame/CampaignAIOperations.cpp
    NewGame/CampaignAssignments.cpp
    NewGame/CampaignLogistics.cpp
    NewGame/CampaignCombat.cpp
    NewGame/CampaignBattleRules.cpp
    NewGame/CampaignBattlefield.cpp
    NewGame/CampaignDailyCombat.cpp
    NewGame/CampaignDiplomacy.cpp
    NewGame/CampaignEconomy.cpp
    NewGame/CampaignTraining.cpp
    NewGame/CampaignGovernance.cpp
    NewGame/CampaignDevelopment.cpp
    NewGame/CampaignOfficers.cpp
    NewGame/CampaignOrders.cpp
    NewGame/CampaignReturn.cpp
    NewGame/CampaignSupply.cpp
    NewGame/CampaignTurn.cpp
)
target_compile_features(sangokushi_rules PUBLIC cxx_std_17)
target_include_directories(sangokushi_rules PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/NewGame")
set_target_properties(sangokushi_rules PROPERTIES POSITION_INDEPENDENT_CODE ON)
if(MSVC)
    target_compile_options(sangokushi_rules PUBLIC /utf-8)
endif()

include(CTest)
if(BUILD_TESTING)
    function(add_rules_test name source)
        add_executable(${name} ${source})
        target_link_libraries(${name} PRIVATE sangokushi_rules)
        # Tests use assert, including Release configurations.
        if(MSVC)
            target_compile_options(${name} PRIVATE /UNDEBUG)
            target_link_options(${name} PRIVATE /STACK:8388608)
        else()
            target_compile_options(${name} PRIVATE -UNDEBUG)
            if(WIN32)
                target_link_options(${name} PRIVATE -Wl,--stack,8388608)
            endif()
        endif()
        add_test(NAME ${name} COMMAND ${name})
    endfunction()
    add_rules_test(tactic_chain tests/TacticChainTests.cpp)
    add_rules_test(tactic_repertoire tests/TacticRepertoireTests.cpp)
    add_rules_test(tactic_combo tests/TacticComboTests.cpp)
    add_rules_test(journey_battle_story tests/JourneyBattleStoryTests.cpp)
    add_rules_test(journey_realtime tests/JourneyRealtimeTests.cpp)
    add_rules_test(journey_march tests/JourneyMarchTests.cpp)
    add_rules_test(journey_save tests/JourneySaveTests.cpp)
    add_rules_test(journey_assignments tests/JourneyAssignmentTests.cpp)
    add_rules_test(journey_accessibility tests/JourneyAccessibilityTests.cpp)
    add_rules_test(journey_campaign_pacing tests/JourneyCampaignPacingTests.cpp)
    add_rules_test(journey_opening tests/JourneyOpeningTests.cpp)
    add_rules_test(journey_chain_signal tests/JourneyChainSignalTests.cpp)
    add_rules_test(journey_battle_watch tests/JourneyBattleWatchTests.cpp)
    add_rules_test(journey_enemy_plans tests/JourneyEnemyPlanTests.cpp)
    add_rules_test(journey_missions tests/JourneyMissionTests.cpp)
    add_rules_test(journey_battlefields tests/JourneyBattlefieldTests.cpp)
    add_rules_test(journey_playthrough tests/JourneyPlaythroughTests.cpp)
    add_test(NAME journey_story_playthrough COMMAND journey_playthrough --story)
    add_rules_test(campaign_regression tests/CampaignTests.cpp)
    add_rules_test(observation_and_operations tests/StrategistOperationsTests.cpp)
endif()
