
add_subdirectory(pics) 

########### next target ###############

set(konsolekalendar_SRCS 
    konsolekalendarepoch.cpp
    konsolekalendardelete.cpp
    konsolekalendarchange.cpp
    konsolekalendarvariables.cpp
    konsolekalendaradd.cpp
    konsolekalendarexports.cpp
    konsolekalendar.cpp
    main.cpp )

ecm_qt_declare_logging_category(konsolekalendar_SRCS HEADER konsolekalendar_debug.h IDENTIFIER KONSOLEKALENDAR_LOG CATEGORY_NAME org.kde.pim.konsolekalendar
        DESCRIPTION "console (konsolekalendar)"
        OLD_CATEGORY_NAMES log_konsolekalendar
        EXPORT AKONADICALENDARTOOLS
    )

add_executable(konsolekalendar ${konsolekalendar_SRCS})
if (COMPILE_WITH_UNITY_CMAKE_SUPPORT)
    set_target_properties(konsolekalendar PROPERTIES UNITY_BUILD ON)
endif()

target_link_libraries(konsolekalendar
    Qt::DBus
    KF5::CalendarUtils
    KF5::CalendarCore
    KF5::CalendarSupport
    KF5::AkonadiCalendar
    KF5::I18n
)

install(TARGETS konsolekalendar  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install( PROGRAMS konsolekalendar.desktop  DESTINATION ${KDE_INSTALL_APPDIR})

