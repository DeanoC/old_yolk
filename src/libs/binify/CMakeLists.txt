cmake_minimum_required( VERSION 2.8 )

SET( cpp_src_files binify.cpp emit.cpp lex.yy.cc parser.tab.cpp )
file( GLOB header_files *.h )

list( APPEND src_files ${cpp_src_files} ${c_src_files} )

add_library( binify ${src_files} ${header_files} )

# make a nice hierachy
set_target_properties( binify PROPERTIES FOLDER "Libs")
