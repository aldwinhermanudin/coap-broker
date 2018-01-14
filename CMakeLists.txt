cmake_minimum_required(VERSION 2.8.9)
project(coap_broker)


set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
find_package(CoAP REQUIRED)

include_directories(${COAP_INCLUDE_DIR})
#Bring the headers, such as Student.h into the project
include_directories(include/c include/cpp)

#Can manually add the sources using the set command as follows:
#set(C_SOURCES src/c/main/coap-broker.c) 
#However, the file(GLOB...) allows for wildcard additions:
file(GLOB C_SOURCES "src/c/*.c")

find_package (Boost 1.55.0 REQUIRED COMPONENTS system filesystem)
if (Boost_FOUND)
  include_directories (SYSTEM ${Boost_INCLUDE_DIR})
endif ()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I /usr/local/include/coap/ -lcoap-1 -lpaho-mqtt3c -lpthread -O2 -D_GNU_SOURCE -DWITH_POSIX -std=c99")

add_executable(CoAPBroker src/c/main/coap-broker.c ${C_SOURCES})
target_link_libraries(CoAPBroker
   coap-1
   paho-mqtt3c
   pthread
 ) 
set_property(TARGET CoAPBroker PROPERTY C_STANDARD 99)

#set(CPP_SOURCES src/cpp/coap-broker.cpp) 
#However, the file(GLOB...) allows for wildcard additions:
file(GLOB CPP_SOURCES "src/cpp/*.cpp")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(CoAPBrokerCPP src/cpp/main/coap-broker.cpp ${CPP_SOURCES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX")
target_link_libraries(CoAPBrokerCPP
coap-1
  ${Boost_LIBRARIES}
  pthread
) 

add_executable(experimental src/cpp/main/experimental.cpp ${CPP_SOURCES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I /usr/local/include/coap/ -O2 -D_GNU_SOURCE -DWITH_POSIX")
target_link_libraries(experimental
coap-1
  ${Boost_LIBRARIES}
  pthread
) 