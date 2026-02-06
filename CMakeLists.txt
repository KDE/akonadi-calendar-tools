cmake_minimum_required(VERSION 3.27)
set(PIM_VERSION "6.6.40")
project(akonadi-calendar-tools VERSION ${PIM_VERSION})

set(KF_MIN_VERSION "6.21.0")
set(QT_REQUIRED_VERSION "6.9.0")
find_package(ECM ${KF_MIN_VERSION} CONFIG REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(ECMInstallIcons)


include(ECMSetupVersion)
include(ECMAddTests)

include(GenerateExportHeader)
include(ECMGenerateHeaders)

include(KDEGitCommitHooks)
include(KDEClangFormat)
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
include(CheckFunctionExists)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMAddAppIcon)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMFeatureSummary)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)
# Do NOT add quote
set(KDEPIM_DEV_VERSION alpha)

# add an extra space
if(DEFINED KDEPIM_DEV_VERSION)
    set(KDEPIM_DEV_VERSION " ${KDEPIM_DEV_VERSION}")
endif()

set(KDEPIM_VERSION "${PIM_VERSION}${KDEPIM_DEV_VERSION}")


set(AKONADICALENDAR_LIB_VERSION "6.6.40")
set(AKONADI_VERSION "6.6.40")
set(CALENDARSUPPORT_LIB_VERSION "6.6.40")
set(KDEPIM_LIB_VERSION "${PIM_VERSION}")
set(KDEPIM_LIB_SOVERSION "6")

find_package(Qt6 ${QT_REQUIRED_VERSION} CONFIG REQUIRED Widgets)


# Find KF6 package
find_package(KF6I18n ${KF_MIN_VERSION} REQUIRED)
find_package(KF6DocTools ${KF_MIN_VERSION})
set_package_properties(KF6DocTools PROPERTIES DESCRIPTION
    "Tools to generate documentation"
    TYPE OPTIONAL
)

# Find KdepimLibs Package
find_package(KPim6Akonadi ${AKONADI_VERSION} CONFIG REQUIRED)
find_package(KF6CalendarCore ${KF_MIN_VERSION}  CONFIG REQUIRED)
find_package(KPim6AkonadiCalendar ${AKONADICALENDAR_LIB_VERSION} CONFIG REQUIRED)
find_package(KPim6CalendarSupport ${CALENDARSUPPORT_LIB_VERSION} CONFIG REQUIRED)

include_directories(${akonadi-calendar-tools_SOURCE_DIR} ${akonadi-calendar-tools_BINARY_DIR})

configure_file(console-version.h.in ${CMAKE_CURRENT_BINARY_DIR}/console-version.h @ONLY)


add_definitions(-DQT_NO_CONTEXTLESS_CONNECT)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Not setting for GNU due to too many warnings related to private members of base classes or around lambdas
    # see e.g. https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56556 or https://gcc.gnu.org/bugzilla/show_bug.cgi?id=79328
    string(APPEND CMAKE_CXX_FLAGS " -Wshadow")
endif()
ecm_set_disabled_deprecation_versions(QT 6.8.1 KF 6.23.0)

add_definitions(-DQT_NO_EMIT)
option(USE_UNITY_CMAKE_SUPPORT "Use UNITY cmake support (speedup compile time)" OFF)

set(COMPILE_WITH_UNITY_CMAKE_SUPPORT OFF)
if (USE_UNITY_CMAKE_SUPPORT)
    set(COMPILE_WITH_UNITY_CMAKE_SUPPORT ON)
    add_definitions(-DCOMPILE_WITH_UNITY_CMAKE_SUPPORT)
endif()

add_subdirectory(calendarjanitor)
add_subdirectory(konsolekalendar)

ecm_qt_install_logging_categories(
        EXPORT AKONADICALENDARTOOLS
        FILE console.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ki18n_install(po)
if(KF6DocTools_FOUND)
    kdoctools_install(po)
    add_subdirectory(doc)
endif()


ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
