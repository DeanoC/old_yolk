
extern "C" void DebugOut( const char* txt );


extern "C" void bootstrap0() {
   DebugOut( "bootstrap0 reporting for duty" );
}

extern "C" void bootstrapN() {
}