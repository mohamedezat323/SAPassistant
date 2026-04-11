cmake_minimum_required(VERSION 3.19)
project(SAPassistant LANGUAGES CXX)

set(CMAKE_AUTORCC ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Network)

qt_standard_project_setup()

qt_add_executable(SAPassistant
    WIN32 MACOSX_BUNDLE
    main.cpp
    widget.cpp
    widget.h
    widget.ui
    resources.qrc
    appicon.rc
)

set_target_properties(SAPassistant PROPERTIES AUTORCC ON)

target_link_libraries(SAPassistant
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::Network
)

# Qt 6 loads TLS from <appDir>/plugins/tls. Debug runs from the build tree have no
# plugins there, so HTTPS fails with "No functional TLS backend". Schannel needs
# no OpenSSL DLLs on Windows.
if(WIN32)
    get_filename_component(QT6_ROOT "${Qt6_DIR}/../../.." ABSOLUTE)
    set(QT_TLS_PLUGINS_DIR "${QT6_ROOT}/plugins/tls")
    add_custom_command(
        TARGET SAPassistant
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:SAPassistant>/plugins/tls"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_TLS_PLUGINS_DIR}/qschannelbackend.dll"
                "$<TARGET_FILE_DIR:SAPassistant>/plugins/tls/"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${QT_TLS_PLUGINS_DIR}/qcertonlybackend.dll"
                "$<TARGET_FILE_DIR:SAPassistant>/plugins/tls/"
        COMMENT "Copying Qt TLS plugins (Schannel) for HTTPS"
        VERBATIM
    )
endif()

install(TARGETS SAPassistant
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET SAPassistant
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})

find_package(Qt6 REQUIRED COMPONENTS Network)
target_link_libraries(SAPassistant PRIVATE Qt6::Network)
