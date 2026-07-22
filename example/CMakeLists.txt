project(bnet)

add_executable(bnet-server
    src/server.cpp
)

target_link_libraries(bnet-server PRIVATE
    bnet::bnet
)


add_executable(bnet-client
    src/client.cpp
)

target_link_libraries(bnet-client PRIVATE
    bnet::bnet
)