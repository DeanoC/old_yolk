cmake_minimum_required( VERSION 2.8 )

LIST( APPEND jpeg_src_files
	jcapimin.c
	jcapistd.c
	jdapimin.c
	jdapistd.c
	jcomapi.c
	jcparam.c
	jctrans.c
	jdtrans.c
	jcinit.c
	jcmaster.c
	jcmainct.c
	jcprepct.c
	jccoefct.c
	jccolor.c
	jcsample.c
	jcdctmgr.c
	jfdctint.c
	jfdctfst.c
	jfdctflt.c
	jchuff.c
	jcarith.c
	jcmarker.c
	jdatadst.c
	jdmaster.c
	jdinput.c
	jdmainct.c
	jdcoefct.c
	jdpostct.c
	jdmarker.c
	jdhuff.c
	jdarith.c
	jddctmgr.c
	jidctint.c
	jidctfst.c
	jidctflt.c
	jdsample.c
	jdcolor.c
	jdmerge.c
	jquant1.c
	jquant2.c
	jdatasrc.c
	jaricom.c
	jerror.c
	jmemmgr.c
	jutils.c
	jmemmgr.c
	jmemnobs.c
)

add_library( jpeg ${jpeg_src_files} )
#list( APPEND targ_link_libs  )
target_link_libraries( jpeg ${targ_link_libs} )

# make a nice hierachy
set_target_properties( jpeg PROPERTIES FOLDER "Libs" )
