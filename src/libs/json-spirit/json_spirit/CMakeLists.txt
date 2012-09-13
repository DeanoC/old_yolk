SET(JSON_SPIRIT_SRCS
json_spirit_reader.cpp
json_spirit_value.cpp
json_spirit_writer.cpp)

FIND_PACKAGE(Boost 1.34 REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})

add_definitions( "/bigobj" )
ADD_LIBRARY(json_spirit STATIC ${JSON_SPIRIT_SRCS})

# make a nice hierachy
set_target_properties( json_spirit PROPERTIES FOLDER "Libs")
