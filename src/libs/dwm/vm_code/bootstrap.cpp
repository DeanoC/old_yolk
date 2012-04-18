
#include "dwm.h"
#include <stdio.h>
extern "C" void bootstrap0() {
   vm_debugOut( "bootstrap0 reporting for duty" );
   printf( "bootstrap0 reporting for duty" );
   printf("%s\n", "texty");
}

extern "C" void bootstrapN() {
}