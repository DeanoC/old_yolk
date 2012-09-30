cmake_minimum_required( VERSION 2.8 )

LIST( APPEND protobuf_lite_src_files  
  stubs/common.cc                              
  stubs/once.cc                                
  stubs/hash.h                                 
  stubs/map-util.h                             
  stubs/stl_util-inl.h                         
  extension_set.cc                             
  generated_message_util.cc                    
  message_lite.cc                              
  repeated_field.cc                            
  wire_format_lite.cc                          
  io/coded_stream.cc                           
  io/coded_stream_inl.h                        
  io/zero_copy_stream.cc                       
  io/zero_copy_stream_impl_lite.cc
)
IF( UNIX )
  	LIST( APPEND protobuf_lite_src_files stubs/atomicops_internals_x86_gcc.cc )
ELSEIF( WIN32 )
 	LIST( APPEND protobuf_lite_src_files stubs/atomicops_internals_x86_msvc.cc )
ENDIF()

add_library( protobuf_lite ${protobuf_lite_src_files} )

LIST( APPEND protobuf_src_files
  ${protobuf_lite_src_files} 
  stubs/strutil.cc   
  stubs/strutil.h                
  stubs/substitute.cc            
  stubs/substitute.h             
  stubs/structurally_valid.cc    
  descriptor.cc                  
  descriptor.pb.cc               
  descriptor_database.cc         
  dynamic_message.cc             
  extension_set_heavy.cc         
  generated_message_reflection.cc
  message.cc                     
  reflection_ops.cc              
  service.cc                     
  text_format.cc                 
  unknown_field_set.cc           
  wire_format.cc                 
  io/gzip_stream.cc              
  io/printer.cc                  
  io/tokenizer.cc                
  io/zero_copy_stream_impl.cc    
  compiler/importer.cc           
  compiler/parser.cc
)
add_library( protobuf ${protobuf_src_files} )

LIST( APPEND protoc_src_files 
  compiler/main.cc
  compiler/code_generator.cc           
  compiler/command_line_interface.cc   
  compiler/plugin.cc                   
  compiler/plugin.pb.cc                
  compiler/subprocess.cc               
  compiler/subprocess.h                
  compiler/zip_writer.cc               
  compiler/zip_writer.h                
  compiler/cpp/cpp_enum.cc             
  compiler/cpp/cpp_enum.h              
  compiler/cpp/cpp_enum_field.cc       
  compiler/cpp/cpp_enum_field.h        
  compiler/cpp/cpp_extension.cc        
  compiler/cpp/cpp_extension.h         
  compiler/cpp/cpp_field.cc            
  compiler/cpp/cpp_field.h             
  compiler/cpp/cpp_file.cc             
  compiler/cpp/cpp_file.h              
  compiler/cpp/cpp_generator.cc        
  compiler/cpp/cpp_helpers.cc          
  compiler/cpp/cpp_helpers.h           
  compiler/cpp/cpp_message.cc          
  compiler/cpp/cpp_message.h           
  compiler/cpp/cpp_message_field.cc    
  compiler/cpp/cpp_message_field.h     
  compiler/cpp/cpp_primitive_field.cc  
  compiler/cpp/cpp_primitive_field.h   
  compiler/cpp/cpp_service.cc          
  compiler/cpp/cpp_service.h           
  compiler/cpp/cpp_string_field.cc     
  compiler/cpp/cpp_string_field.h      
  compiler/java/java_enum.cc           
  compiler/java/java_enum.h            
  compiler/java/java_enum_field.cc     
  compiler/java/java_enum_field.h      
  compiler/java/java_extension.cc      
  compiler/java/java_extension.h       
  compiler/java/java_field.cc          
  compiler/java/java_field.h           
  compiler/java/java_file.cc           
  compiler/java/java_file.h            
  compiler/java/java_generator.cc      
  compiler/java/java_helpers.cc        
  compiler/java/java_helpers.h         
  compiler/java/java_message.cc        
  compiler/java/java_message.h         
  compiler/java/java_message_field.cc  
  compiler/java/java_message_field.h   
  compiler/java/java_primitive_field.cc
  compiler/java/java_primitive_field.h 
  compiler/java/java_service.cc        
  compiler/java/java_service.h         
  compiler/java/java_string_field.cc   
  compiler/java/java_string_field.h    
  compiler/python/python_generator.cc
)
add_executable( protoc ${protoc_src_files} )
list( APPEND targ_link_libs protobuf zlib )
if( UNIX )
list( APPEND targ_link_libs pthread dl )
endif()
target_link_libraries( protoc ${targ_link_libs} )

# make a nice hierachy
set_target_properties( protobuf_lite PROPERTIES FOLDER "Libs" )
set_target_properties( protobuf PROPERTIES FOLDER "Libs" )
set_target_properties( protoc PROPERTIES FOLDER "Tools" )
