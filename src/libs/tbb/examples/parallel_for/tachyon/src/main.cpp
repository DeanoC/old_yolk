/*
    Copyright 2005-2011 Intel Corporation.  All Rights Reserved.

    This file is part of Threading Building Blocks.

    Threading Building Blocks is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    Threading Building Blocks is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty
    of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Threading Building Blocks; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    As a special exception, you may use this file as part of a free software
    library without restriction.  Specifically, if other files instantiate
    templates or use macros or inline functions from this file, or you compile
    this file and link it with other files to produce an executable, this
    file does not by itself cause the resulting executable to be covered by
    the GNU General Public License.  This exception does not however
    invalidate any other reasons why the executable file might be covered by
    the GNU General Public License.
*/

/*
    The original source for this example is
    Copyright (c) 1994-2008 John E. Stone
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:
    1. Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
    2. Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
    3. The name of the author may not be used to endorse or promote products
       derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
    OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    SUCH DAMAGE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VIDEO_WINMAIN_ARGS
#include "types.h"
#include "api.h"       /* The ray tracing library API */
#include "parse.h"     /* Support for my own file format */
#include "ui.h"
#include "util.h"
#include "tachyon_video.h"
#include "../../../common/utility/utility.h"

SceneHandle global_scene;
int global_xsize;     /*  size of graphic image rendered in window (from hres, vres)  */
int global_ysize;
int global_xwinsize;  /*  size of window (may be larger than above)  */
int global_ywinsize;
char *global_window_title;
bool global_usegraphics;

bool silent_mode = false; /* silent mode */

class video *video = 0;

typedef struct {
  int foundfilename;      /* was a model file name found in the args? */
  char filename[1024];    /* model file to render */
  int useoutfilename;     /* command line override of output filename */
  char outfilename[1024]; /* name of output image file */
  int verbosemode;        /* verbose flags */
  int antialiasing;       /* antialiasing setting */
  int displaymode;        /* display mode */
  int boundmode;          /* bounding mode */
  int boundthresh;        /* bounding threshold */
  int usecamfile;         /* use camera file */
  char camfilename[1024]; /* camera filename */
} argoptions;

static char *window_title_string (int argc, const char **argv)
{
    int i;
    char *name;

    name = (char *) malloc (8192);
    
    if(strrchr(argv[0], '\\')) strcpy (name, strrchr(argv[0], '\\')+1);
    else if(strrchr(argv[0], '/')) strcpy (name, strrchr(argv[0], '/')+1);
    else strcpy (name, *argv[0]?argv[0]:"Tachyon");
    for (i = 1; i < argc; i++) {
        strcat (name, " ");
        strcat (name, argv[i]);
    }
#ifdef _DEBUG
    strcat (name, " (DEBUG BUILD)");
#endif
    return name;
}

void rt_finalize(void) {
    timer t0, t1;
    t0 = gettimer();
    if(global_usegraphics)
        do { rt_sleep(10); t1 = gettimer(); }
        while(timertime(t0, t1) < 10 && video->next_frame());
#ifdef _WINDOWS
    else rt_sleep(10000);
#endif
}

void initoptions(argoptions * opt) {
    memset(opt, 0, sizeof(argoptions));
    opt->foundfilename = -1;
    opt->useoutfilename = -1;
    opt->verbosemode = -1;
    opt->antialiasing = -1;
    opt->displaymode = -1;
    opt->boundmode = -1; 
    opt->boundthresh = -1; 
    opt->usecamfile = -1;
}

int useoptions(argoptions * opt, SceneHandle scene) {
  if (opt->useoutfilename == 1) {
    rt_outputfile(scene, opt->outfilename);
  }

  if (opt->verbosemode == 1) {
    rt_verbose(scene, 1);
  }

  if (opt->antialiasing != -1) {
    /* need new api code for this */
  } 

  if (opt->displaymode != -1) {
    rt_displaymode(scene, opt->displaymode);
  }

  if (opt->boundmode != -1) {
    rt_boundmode(scene, opt->boundmode);
  }

  if (opt->boundthresh != -1) {
    rt_boundthresh(scene, opt->boundthresh);
  }

  return 0;
}    

argoptions ParseCommandLine(int argc, const char *argv[]) {
    argoptions opt;

    initoptions(&opt);

    bool nobounding = false;
    bool nodisp = false;

    string filename;

    utility::parse_cli_arguments(argc,argv,
        utility::cli_argument_pack()
        .positional_arg(filename,"dataset", "Model file")
        .positional_arg(opt.boundthresh,"boundthresh","bounding threshold value")
        .arg(nodisp,"no-display-updating","disable run-time display updating")
        .arg(nobounding,"no-bounding","disable bounding technique")
        .arg(silent_mode,"silent","no output except elapsed time")
    );

    strcpy(opt.filename, filename.c_str());

    opt.displaymode = nodisp ? RT_DISPLAY_DISABLED : RT_DISPLAY_ENABLED;
    opt.boundmode = nobounding ? RT_BOUNDING_DISABLED : RT_BOUNDING_ENABLED;

    return opt;
}

int CreateScene(argoptions &opt) {
    char *filename;

    global_scene = rt_newscene();
    rt_initialize();

    /* process command line overrides */
    useoptions(&opt, global_scene);

#ifdef DEFAULT_MODELFILE
#if  _WIN32||_WIN64
#define _GLUE_FILENAME(x) "..\\dat\\" #x
#else
#define _GLUE_FILENAME(x) #x
#endif
#define GLUE_FILENAME(x) _GLUE_FILENAME(x)
    if(opt.foundfilename == -1)
        filename = GLUE_FILENAME(DEFAULT_MODELFILE);
    else
#endif//DEFAULT_MODELFILE
        filename = opt.filename;

    if ( readmodel(filename, global_scene) != 0 ) {
        fprintf(stderr, "Parser returned a non-zero error code reading %s\n", filename);
        fprintf(stderr, "Aborting Render...\n");
        rt_finalize();
        return -1;
    }

    // need these early for create_graphics_window() so grab these here...
    scenedef *scene = (scenedef *) global_scene;
    global_xsize = scene->hres;
    global_ysize = scene->vres;
    global_xwinsize = global_xsize;
    global_ywinsize = global_ysize;  // add some here to leave extra blank space on bottom for status etc.
    global_usegraphics = (scene->displaymode == RT_DISPLAY_ENABLED);

    return 0;
}

int main (int argc, char *argv[]) {
    try {
        timer mainStartTime = gettimer();

        global_window_title = window_title_string (argc, (const char**)argv);

        argoptions opt = ParseCommandLine(argc, (const char**)argv);

        if ( CreateScene(opt) != 0 )
            return -1;

        tachyon_video tachyon;
        tachyon.threaded = true;
        tachyon.init_console();

        tachyon.title = global_window_title;
        tachyon.updating = global_usegraphics;
        // always using window even if(!global_usegraphics)
        global_usegraphics = 
            tachyon.init_window(global_xwinsize, global_ywinsize);
        if(!tachyon.running)
            return -1;

        video = &tachyon;
        tachyon.main_loop();

        utility::report_elapsed_time(timertime(mainStartTime, gettimer()));
        return 0;
    } catch ( std::exception& e ) {
        std::cerr<<"error occurred. error text is :\"" <<e.what()<<"\"\n";
        return 1;
    }
}
