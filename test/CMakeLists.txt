include_directories(src)
link_directories(src)



ADD_EXECUTABLE(test_tidelog main.cpp)
target_link_libraries(test_tidelog tidelog)
INSTALL(TARGETS test_tidelog DESTINATION ${BINDIR})


