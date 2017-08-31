# CMakeLists.txt

add_executable(tcp_server tcp_server.c)

target_include_directories(
  tcp_server
  PRIVATE ${PROJECT_SOURCE_DIR}/export)

target_link_libraries(tcp_server ax)

add_executable(chat_server chat_server.c)

target_include_directories(
  chat_server
  PRIVATE ${PROJECT_SOURCE_DIR}/export)

target_link_libraries(chat_server ax)
