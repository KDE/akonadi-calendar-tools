
add_subdirectory(pics) 

add_executable(konsolekalendar)
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(konsolekalendar PROPERTIES UNITY_BUILD ON)
endif()

target_sources(konsolekalendar PRIVATE
    konsolekalendarepoch.cpp
    konsolekalendardelete.cpp
    konsolekalendarchange.cpp
    konsolekalendarvariables.cpp
    konsolekalendaradd.cpp
    konsolekalendarexports.cpp
    konsolekalendar.cpp
    konsolekalendarepoch.h
    konsolekalendardelete.h
    konsolekalendarchange.h
    konsolekalendarvariables.h
    konsolekalendaradd.h
    konsolekalendarexports.h
    konsolekalendar.h

    main.cpp )

ecm_qt_declare_logging_category(konsolekalendar
    HEADER konsolekalendar_debug.h
    IDENTIFIER KONSOLEKALENDAR_LOG
    CATEGORY_NAME org.kde.pim.konsolekalendar
    DESCRIPTION "console (konsolekalendar)"
    OLD_CATEGORY_NAMES log_konsolekalendar
    EXPORT AKONADICALENDARTOOLS
    )

target_link_libraries(konsolekalendar
    Qt::DBus
    KPim${KF_MAJOR_VERSION}::CalendarUtils
    KF${KF_MAJOR_VERSION}::CalendarCore
    KF5::CalendarSupport
    KF5::AkonadiCalendar
    KF${KF_MAJOR_VERSION}::I18n
)

install(TARGETS konsolekalendar  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( PROGRAMS konsolekalendar.desktop  DESTINATION ${KDE_INSTALL_APPDIR})

