
########### next target ###############

set(calendarjanitor_SRCS
    backuper.cpp
    calendarjanitor.cpp
    collectionloader.cpp
    main.cpp
    options.cpp)

add_executable(calendarjanitor ${calendarjanitor_SRCS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(calendarjanitor PROPERTIES UNITY_BUILD ON)
endif()
target_link_libraries(calendarjanitor
    KF5::CalendarUtils
    KF5::CalendarCore
    KF5::CalendarSupport
    KF5::AkonadiCalendar)

install(TARGETS calendarjanitor  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
