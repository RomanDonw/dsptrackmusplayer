/*
    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at https://mozilla.org/MPL/2.0/.
*/

#include "dspmodule.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <libopenmpt/libopenmpt.h>

const unsigned short dspmodule_requiredAPIversion = 4;

static float ampmod = 0, volmod = 0.1;
static void *outportl, *outportr;
static openmpt_module *mod;
static bool quitonsongend = true;

static void dummycallback(void) {}

DSPModuleReturnCode dspmodule_startup(const DSPLoaderAPI * restrict lapi, DSPModuleStartupFunctionOptions * restrict opts)
{
    char *infilepath = NULL;
    int repeatcount = -1;
    {
        int p;
        while ((p = getopt(opts->argc, opts->argv, "a:v:f:sl:")) != -1)
        {
            switch (p)
            {
                case 'a':
                    if (sscanf(optarg, "%f", &ampmod) < 1) goto parseoptfail;
                    break;

                case 'v':
                    if (sscanf(optarg, "%f", &volmod) < 1) goto parseoptfail;
                    break;

                case 'f':
                    infilepath = optarg;
                    break;

                case 's': // suppress quitting/stay connected.
                    quitonsongend = false;
                    break;

                case 'l':
                    if (sscanf(optarg, "%i", &repeatcount) < 0) goto parseoptfail;
                    if (repeatcount < 0) repeatcount = -1;
                    break;

                parseoptfail:
                    lapi->log(DSPLogLevel_Fatal, "failed parsing option -%c", (char)p);
                return DSPModuleReturnCode_Stop;
            }
        }
    }

    if (!infilepath) { lapi->log(DSPLogLevel_Fatal, "specify source tracker module file through -f parameter"); return DSPModuleReturnCode_Stop; }

    if (!(outportl = lapi->addport("left", NULL, DSPPortDirection_Output, 0))) 
    { lapi->log(DSPLogLevel_Fatal, "error creating output port for left audio channel"); return DSPModuleReturnCode_Stop; }
    if (!(outportr = lapi->addport("right", NULL, DSPPortDirection_Output, 0)))
    { lapi->log(DSPLogLevel_Fatal, "error creating output port for right audio channel)"); return DSPModuleReturnCode_Stop; }

    FILE *f = fopen(infilepath, "rb");
    if (!f) { lapi->log(DSPLogLevel_Fatal, "unable to open specified file"); return DSPModuleReturnCode_Stop; }

    fseek(f, 0, SEEK_END);
    size_t infilesize = ftell(f);
    fseek(f, 0, SEEK_SET);

    void *infiledata = malloc(infilesize);
    if (!infiledata) { lapi->log(DSPLogLevel_Fatal, "memory allocation failed"); fclose(f); return DSPModuleReturnCode_Stop; }

    if (fread(infiledata, 1, infilesize, f) < infilesize || ferror(f))
    {
        lapi->log(DSPLogLevel_Fatal, "failed reading file");
        free(infiledata);
        fclose(f);
        return DSPModuleReturnCode_Stop;
    }

    fclose(f);

    const char *errmsg = NULL;
    mod = openmpt_module_create_from_memory2(infiledata, infilesize, (void *)dummycallback, NULL, (void *)dummycallback, NULL, NULL, &errmsg, NULL);
    free(infiledata);
    if (!mod)
    {
        lapi->log(DSPLogLevel_Fatal, "failed to parse specified track music file: %s", errmsg);
        openmpt_free_string(errmsg);
        return DSPModuleReturnCode_Stop;
    }

    openmpt_module_set_repeat_count(mod, repeatcount);

    lapi->log(DSPLogLevel_Info, "in file: \"%s\"\nampmod: %f\nvolmod: %f\nquitonsongend: %s", infilepath, ampmod, volmod, quitonsongend ? "yes" : "no");
    if (repeatcount < 0) lapi->log(DSPLogLevel_Info, "repeat count: forever");
    else if (!repeatcount) lapi->log(DSPLogLevel_Info, "repeat count: play once");
    else lapi->log(DSPLogLevel_Info, "repeat count: %i", repeatcount);

    opts->sysname = "trackmusicplayer";
    opts->dispname = "simple track music player";
    return DSPModuleReturnCode_Continue;
}

DSPModuleReturnCode dspmodule_process(const DSPLoaderAPI * restrict lapi, DSPModuleProcessFunctionOptions * restrict opts)
{
    float *leftch = lapi->getportbuffer(outportl, opts->duration);
    if (!leftch) return DSPModuleReturnCode_Continue;
    float *rightch = lapi->getportbuffer(outportr, opts->duration);
    if (!rightch) return DSPModuleReturnCode_Continue;

    size_t frames = openmpt_module_read_float_stereo(mod, opts->rate, opts->duration, leftch, rightch);
    if (!frames && quitonsongend) return DSPModuleReturnCode_Quit;
    else if (frames < opts->duration)
    {
        memset(&leftch[frames], 0, (opts->duration - frames) * sizeof(float));
        memset(&rightch[frames], 0, (opts->duration - frames) * sizeof(float));
    }
    
    for (unsigned long long i = 0; i < opts->duration; i++)
    {
        leftch[i] = adjf(leftch[i], ampmod) * volmod;
        rightch[i] = adjf(rightch[i], ampmod) * volmod;
    }
    
    return DSPModuleReturnCode_Continue;
}

void dspmodule_cleanup(DSPModuleCleanupFunctionOptions * restrict opts)
{
    openmpt_module_destroy(mod);
}
