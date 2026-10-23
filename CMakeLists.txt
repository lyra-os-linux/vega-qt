cmake_minimum_required(VERSION 3.24)
project(vega-qt VERSION 0.1.7 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_SKIP_INSTALL_RPATH TRUE)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Qml Quick DBus Network LinguistTools)
find_package(KF6Kirigami REQUIRED)
qt_standard_project_setup(REQUIRES 6.6)
qt_add_executable(vega-qt src/main.cpp src/systembackend.cpp src/systembackend.h
    src/assistantbackend.cpp src/assistantbackend.h src/translations.h)
qt_add_executable(vega-update-notifier-qt src/update_notifier.cpp src/translations.h)
qt_add_resources(vega-qt "vega-icons" PREFIX "/" FILES packaging/vega.svg)
qt_add_qml_module(vega-qt URI org.lyraos.vega.qt VERSION 1.0 QML_FILES
    qml/Main.qml
    qml/components/LyraProgressBar.qml
    qml/pages/SoftwarePage.qml
    qml/pages/ServicesPage.qml
    qml/pages/PlaceholderPage.qml
    qml/pages/HardwarePage.qml
    qml/pages/StoragePage.qml
    qml/pages/UsersPage.qml
    qml/pages/NetworkPage.qml
    qml/pages/BluetoothPage.qml
    qml/pages/LogsPage.qml
        qml/pages/MonitorPage.qml
        qml/pages/DateTimePage.qml
        qml/pages/BackupPage.qml
        qml/pages/PersonalizationPage.qml
        qml/pages/AssistantPage.qml
)
target_link_libraries(vega-qt PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::DBus Qt6::Network KF6::Kirigami)
target_link_libraries(vega-update-notifier-qt PRIVATE Qt6::Core Qt6::DBus)

# English is the source language; pt-BR and es-ES are embedded in both binaries.
set(VEGA_TS_FILES translations/vega-qt_pt_BR.ts translations/vega-qt_es_ES.ts)
qt_add_lrelease(vega-qt TS_FILES ${VEGA_TS_FILES} QM_FILES_OUTPUT_VARIABLE VEGA_QM_FILES)
qt_add_resources(vega-qt "vega-translations" PREFIX "/i18n" BASE "${CMAKE_CURRENT_BINARY_DIR}"
    FILES ${VEGA_QM_FILES})
qt_add_resources(vega-update-notifier-qt "vega-notifier-translations" PREFIX "/i18n"
    BASE "${CMAKE_CURRENT_BINARY_DIR}" FILES ${VEGA_QM_FILES})
add_dependencies(vega-update-notifier-qt vega-qt_lrelease)
install(TARGETS vega-qt RUNTIME DESTINATION bin)
install(TARGETS vega-update-notifier-qt RUNTIME DESTINATION bin)
install(FILES packaging/org.lyraos.Vega.Qt.desktop DESTINATION share/applications)
install(FILES packaging/vega-update-notifier-qt.desktop DESTINATION /etc/xdg/autostart)
install(FILES packaging/vega.svg DESTINATION share/icons/hicolor/scalable/apps)
