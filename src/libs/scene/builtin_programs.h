
DECLARE_SHARED( shared_structs );
DECLARE_SHARED( constant_blocks );
DECLARE_SHARED( vtstructs );

DECLARE_SHADER( vs_null );
DECLARE_SHADER( vs_basic );
DECLARE_SHADER( vs_sprite_basic );
DECLARE_SHADER( vs_passthrough_pos );
DECLARE_SHADER( vs_passthrough_pos_col );
DECLARE_SHADER( vs_passthrough_pos_uv );
DECLARE_SHADER( vs_passthrough_pos_col_uv );
DECLARE_SHADER( vs_passthrough_pos_col_uv );
DECLARE_SHADER( vs_wvp_pos_col );
DECLARE_SHADER( vs_vp_pos_col );
DECLARE_SHADER( vs_vtsolidwireframe );


DECLARE_SHADER( gs_fullscreen );
DECLARE_SHADER( gs_vtsolidwireframe );

DECLARE_SHADER( fs_basic );
DECLARE_SHADER( fs_copy );
DECLARE_SHADER( fs_col );
DECLARE_SHADER( fs_tex );
DECLARE_SHADER( fs_tex_col );
DECLARE_SHADER( fs_vtsolidwireframe );
DECLARE_SHADER( fs_vtresolve );

DECLARE_PROGRAM( basic );
DECLARE_PROGRAM( rendertarget_copy );
DECLARE_PROGRAM( sprite_basic );
DECLARE_PROGRAM( 2dcolour );
DECLARE_PROGRAM( debugline );
DECLARE_PROGRAM( vtsolidwireframe );
DECLARE_PROGRAM( vtresolve );