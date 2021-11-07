add_executable(calendarjanitor)
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(calendarjanitor PROPERTIES UNITY_BUILD ON)
endif()

target_sources(calendarjanitor PRIVATE
    main.cpp
    backuper.cpp
    calendarjanitor.cpp
    collectionloader.cpp
    options.cpp
    backuper.h
    calendarjanitor.h
    collectionloader.h
    options.h
    )

target_link_libraries(calendarjanitor
    KF5::CalendarUtils
    KF5::CalendarCore
    KF5::CalendarSupport
    KF5::AkonadiCalendar)

install(TARGETS calendarjanitor  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
