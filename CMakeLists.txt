cmake_minimum_required(VERSION 3.24)
project(vega-qt VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
find_package(Qt6 6.6 REQUIRED COMPONENTS Core Gui Qml Quick DBus)
find_package(KF6Kirigami REQUIRED)
qt_standard_project_setup(REQUIRES 6.6)
qt_add_executable(vega-qt src/main.cpp src/systembackend.cpp src/systembackend.h)
qt_add_qml_module(vega-qt URI org.lyraos.vega.qt VERSION 1.0 QML_FILES
    qml/Main.qml
    qml/pages/SoftwarePage.qml
    qml/pages/ServicesPage.qml
    qml/pages/PlaceholderPage.qml
    qml/pages/HardwarePage.qml
    qml/pages/StoragePage.qml
    qml/pages/UsersPage.qml
    qml/pages/NetworkPage.qml
)
target_link_libraries(vega-qt PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick Qt6::DBus KF6::Kirigami)
install(TARGETS vega-qt RUNTIME DESTINATION bin)
install(FILES packaging/org.lyraos.Vega.Qt.desktop DESTINATION share/applications)
