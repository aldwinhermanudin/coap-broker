cmake_minimum_required(VERSION 2.8.9)
project(coap_broker)

#Bring the headers, such as Student.h into the project
include_directories(include)

#Can manually add the sources using the set command as follows:
set(SOURCES src/coap-broker.c) 
#However, the file(GLOB...) allows for wildcard additions:
file(GLOB SOURCES "src/*.c")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -I /usr/local/include/coap/ -lcoap-1 -lpaho-mqtt3c -lpthread -O2 -D_GNU_SOURCE -DWITH_POSIX -std=c99")

add_executable(CoAPBroker ${SOURCES})
target_link_libraries(CoAPBroker
   coap-1
   paho-mqtt3c
   pthread
 ) 
set_property(TARGET CoAPBroker PROPERTY C_STANDARD 99)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(CoAPBrokerCPP src/coap-broker.cpp src/CoAPRD.cpp src/LinkedListDB.cpp src/LinkFormatParser.cpp src/LibcoapMod.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I /usr/local/include/coap/ -lcoap-1 -lpaho-mqtt3c -lpthread -O2 -D_GNU_SOURCE -DWITH_POSIX")
target_link_libraries(CoAPBrokerCPP
  coap-1
  paho-mqtt3c
  pthread
) 