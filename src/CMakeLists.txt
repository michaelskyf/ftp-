set(SERVER_NAME "${PROJECT_NAME}-server")

add_executable(
	${SERVER_NAME}
	main.cpp
	)

target_link_libraries(
	${SERVER_NAME}
	)
