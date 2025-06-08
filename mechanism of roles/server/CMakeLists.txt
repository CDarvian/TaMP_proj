cmake_minimum_required(VERSION 3.16)
project(ServerProject LANGUAGES CXX)

# Настройка Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Network Sql Test)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Network Sql Test)


# Основной сервер
add_executable(server
    main_serv.cpp
    mytcpserver.cpp
    mytcpserver.h
    functions_to_server.cpp
    functions_to_server.h
    databasemanager.cpp
    databasemanager.h
)

target_link_libraries(server PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Test
)

# DB сервер
add_executable(db_server
    main_db.cpp
    dbtcpserver.cpp
    dbtcpserver.h
    databasemanager.cpp
    databasemanager.h
)

target_link_libraries(db_server PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Test
)

# Тесты
add_executable(test_backend
    test_backend.cpp
    functions_to_server.cpp
    functions_to_server.h
    databasemanager.cpp
    databasemanager.h
)

target_link_libraries(test_backend PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Test
)

if(WIN64)
    # Копируем SQLite драйвер
    add_custom_command(TARGET test_backend POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${Qt6_DIR}/../../../plugins/sqldrivers/qsqlite.dll"
        $<TARGET_FILE_DIR:test_backend>)

    # Копируем зависимые DLL
    add_custom_command(TARGET test_backend POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${Qt6_DIR}/../../../bin/Qt6Sql.dll"
        $<TARGET_FILE_DIR:test_backend>)
endif()


include(GNUInstallDirs)
install(TARGETS server
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
