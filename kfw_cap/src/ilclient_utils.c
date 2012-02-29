/*
 *  Copyright (c) 2010-2011, Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *  *  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *  *  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 *  *  Neither the name of Texas Instruments Incorporated nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *  THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 *  PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 *  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 *  OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 *  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contact information for paper mail:
 *  Texas Instruments
 *  Post Office Box 655303
 *  Dallas, Texas 75265
 *  Contact information:
 *  http://www-k.ext.ti.com/sc/technical-support/product-information-centers.htm?
 *  DCMP=TIHomeTracking&HQS=Other+OT+home_d_contact
 *  ============================================================================
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <xdc/std.h>
#include <memory.h>
#include <getopt.h>

/*-------------------------program files -------------------------------------*/
#include "ti/omx/interfaces/openMaxv11/OMX_Core.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Component.h"
#include "OMX_TI_Common.h"
#include "OMX_Video.h"
#include "OMX_TI_Video.h"
#include "ilclient.h"
#include "ilclient_utils.h"
#include "platform_utils.h"
#include <omx_venc.h>
#include <omx_vfpc.h>
#include <omx_vfdc.h>
#include <omx_ctrl.h>
#include <omx_vfcc.h>
#include <OMX_TI_Index.h>
/*---------------------- function prototypes ---------------------------------*/
/* None */

void usage (IL_ARGS *argsp)
{
  printf
    ("capture_encode -f <frame_rate> -m <mode>"
     "-b <bit_rate> -o <output_file> -n <num_frames> -d <0/1>\n\n"
     "-o | --output          output filename \n"
     "-m | --mode            1080p / 720p capture/On-chip HDMI display mode \n"
     "-f | --framerate       encode frame rate \n"
     "-b | --bitrate         encode bit rate \n"
     "-n | --numframes       encode number of frames \n"
     "-d | --display_id      0 - for on-chip HDMI, 1 for Secondary display \n");

  printf(" example -    ./capture_encode_a8host_debug.xv5T -o sample.h264 -m 1080p -f 60 -b 1000000 -d 0 -n 1000 \n");
  printf(" for 720p, set hdmi mode before running example \n echo 720p-60 >/sys/devices/platform/vpss/display0/mode \n");
  
  exit (1);
}

/* ========================================================================== */
/**
* parse_args() : This function parses the input arguments provided to app.
*
* @param argc             : number of args 
* @param argv             : args passed by app
* @param argsp            : parsed data pointer
*
*  @return      
*
*
*/
/* ========================================================================== */

void parse_args (int argc, char *argv[], IL_ARGS *argsp)
{
  const char shortOptions[] = "o:f:b:n:m:d:";
  const struct option longOptions[] =
  {
    {"output", required_argument, NULL, ArgID_OUTPUT_FILE},
    {"framerate", required_argument, NULL, ArgID_FRAMERATE},
    {"bitrate", required_argument, NULL, ArgID_BITRATE},
    {"numframes", required_argument, NULL, ArgID_NUMFRAMES},
    {"mode", required_argument, NULL, ArgID_MODE},
    {"display_id", required_argument, NULL, ArgID_DISPLAYID},
    {0, 0, 0, 0}
  };

  int index, outfile = 0, mode = 0, nframes = 0;
  int display_id = 0;
  int argID;

  for (;;)
  {
    argID = getopt_long (argc, argv, shortOptions, longOptions, &index);

    if (argID == -1)
    {
      break;
    }

    switch (argID)
    {
      case ArgID_OUTPUT_FILE:
      case 'o':
        strncpy (argsp->output_file, optarg, MAX_FILE_NAME_SIZE);
        outfile = 1;
        break;
      case ArgID_FRAMERATE:
      case 'f':
        argsp->frame_rate = atoi (optarg);
        break;
      case ArgID_BITRATE:
      case 'b':
        argsp->bit_rate = atoi (optarg);
        break;
      case ArgID_NUMFRAMES:
      case 'n':
        argsp->num_frames = atoi(optarg);
        nframes = 1;
        break;
      case ArgID_MODE:
      case 'm':
        strncpy (argsp->mode, optarg, MAX_MODE_NAME_SIZE);
        mode = 1;
        break;
      case ArgID_DISPLAYID:
      case 'd':
        argsp->display_id = atoi (optarg);
        display_id = 1;
        break;
      default:
        usage (argsp);
        exit (1);
    }
  }

  if (optind < argc)
  {
    usage (argsp);
    exit (EXIT_FAILURE);
  }

  if (argsp->bit_rate == 0 || !outfile || argsp->frame_rate == 0 || !nframes || !mode || !display_id)
  {
    usage (argsp);
    exit (1);
  }

  printf ("output file: %s\n", argsp->output_file);
  printf ("bit_rate: %d\n", argsp->bit_rate);
  printf ("frame_rate: %d\n", argsp->frame_rate);
  printf ("num_frames: %d\n", argsp->num_frames);
  printf ("mode: %s\n", argsp->mode);
  printf ("display_id: %d\n", argsp->display_id);
}

/* ========================================================================== */
/**
* IL_ClientInit() : This function is to allocate and initialize the application
*                   data structure. It is just to maintain application control.
*
* @param pAppData          : appliaction / client data Handle 
* @param width             : stream width
* @param height            : stream height
* @param frameRate         : encode frame rate
* @param bitrate           : encoder bit rate
* @param numFrames         : encoded number of frames
* @param displayId         : display instance id
*
*  @return      
*
*
*/
/* ========================================================================== */

void IL_ClientInit (IL_Client **pAppData, char *mode, int frameRate,
                    int bitRate, int numFrames, int displayId)
{
  int i;
  IL_Client *pAppDataPtr;
  IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
  IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;

  /* Allocating data structure for IL client structure / buffer management */

  pAppDataPtr = (IL_Client *) malloc (sizeof (IL_Client));
  memset (pAppDataPtr, 0x0, sizeof (IL_Client));

  /* update the user provided parameters */
  pAppDataPtr->nFrameRate = frameRate;
  pAppDataPtr->nBitRate = bitRate;
  pAppDataPtr->nEncodedFrms = numFrames;
  strncpy (pAppDataPtr->mode, mode, MAX_MODE_NAME_SIZE);  
  
  /* based on capture/display mode selected set width and height */
  if (strcmp (mode, "1080p") == 0) {
   pAppDataPtr->nHeight = 1080;
   pAppDataPtr->nWidth =  1920; 
  }
  else  if (strcmp (mode, "720p") == 0) {
   pAppDataPtr->nHeight = 720;
   pAppDataPtr->nWidth =  1280; 
  }
  else
  {
    ERROR ("In correct Mode selected!! \n");
  }
  pAppDataPtr->displayId = displayId;

  /* alloacte data structure for each component used in this IL Client */
  pAppDataPtr->capILComp =
    (IL_CLIENT_COMP_PRIVATE *) malloc (sizeof (IL_CLIENT_COMP_PRIVATE));
  memset (pAppDataPtr->capILComp, 0x0, sizeof (IL_CLIENT_COMP_PRIVATE));

  /* these semaphores are used for tracking the callbacks received from
     component */
  pAppDataPtr->capILComp->eos = malloc (sizeof (semp_t));
  semp_init (pAppDataPtr->capILComp->eos, 0);

  pAppDataPtr->capILComp->done_sem = malloc (sizeof (semp_t));
  semp_init (pAppDataPtr->capILComp->done_sem, 0);

  pAppDataPtr->capILComp->port_sem = malloc (sizeof (semp_t));
  semp_init (pAppDataPtr->capILComp->port_sem, 0);

  /* alloacte data structure for each component used in this IL Client */
  pAppDataPtr->deiILComp =
    (IL_CLIENT_COMP_PRIVATE *) malloc (sizeof (IL_CLIENT_COMP_PRIVATE));
  memset (pAppDataPtr->deiILComp, 0x0, sizeof (IL_CLIENT_COMP_PRIVATE));

  /* these semaphores are used for tracking the callbacks received from
     component */
  pAppDataPtr->deiILComp->eos = malloc (sizeof (semp_t));
  semp_init (pAppDataPtr->deiILComp->eos, 0);

  pAppDataPtr->deiILComp->done_sem = malloc (sizeof (semp_t));
  semp_init (pAppDataPtr->deiILComp->done_sem, 0);

  pAppDataPtr->deiILComp->port_sem = malloc (sizeof (semp_t));
  semp_init (pAppDataPtr->deiILComp->port_sem, 0);

  /* alloacte data structure for each component used in this IL Client */
  pAppDataPtr->encILComp =
    (IL_CLIENT_COMP_PRIVATE *) malloc (sizeof (IL_CLIENT_COMP_PRIVATE));
  memset (pAppDataPtr->encILComp, 0x0, sizeof (IL_CLIENT_COMP_PRIVATE));

  /* these semaphores are used for tracking the callbacks received from
     component */
  pAppDataPtr->encILComp->eos = malloc (sizeof (semp_t));
  semp_init (pAppDataPtr->encILComp->eos, 0);

  pAppDataPtr->encILComp->done_sem = malloc (sizeof (semp_t));
  semp_init (pAppDataPtr->encILComp->done_sem, 0);

  pAppDataPtr->encILComp->port_sem = malloc (sizeof (semp_t));
  semp_init (pAppDataPtr->encILComp->port_sem, 0);

  /* alloacte data structure for each component used in this IL Client */
  pAppDataPtr->disILComp =
    (IL_CLIENT_COMP_PRIVATE *) malloc (sizeof (IL_CLIENT_COMP_PRIVATE));
  memset (pAppDataPtr->disILComp, 0x0, sizeof (IL_CLIENT_COMP_PRIVATE));

  /* these semaphores are used for tracking the callbacks received from
     component */
  pAppDataPtr->disILComp->eos = malloc (sizeof (semp_t));
  semp_init (pAppDataPtr->disILComp->eos, 0);

  pAppDataPtr->disILComp->done_sem = malloc (sizeof (semp_t));
  semp_init (pAppDataPtr->disILComp->done_sem, 0);

  pAppDataPtr->disILComp->port_sem = malloc (sizeof (semp_t));
  semp_init (pAppDataPtr->disILComp->port_sem, 0);

  /* number of ports for each component, which this IL client will handle, this 
     will be equal to number of ports supported by component or less */

  pAppDataPtr->capILComp->numInport = 0;
  /* capture does not has i/p ports */
  pAppDataPtr->capILComp->numOutport = 1;
  pAppDataPtr->capILComp->startOutportIndex = 0;

  pAppDataPtr->deiILComp->numInport = 1;
  pAppDataPtr->deiILComp->numOutport = 2;
  /* VFPC OMX component support max 16 input / output ports, so o/p port index
     starts at 16 */
  pAppDataPtr->deiILComp->startOutportIndex = OMX_VFPC_NUM_INPUT_PORTS;

  pAppDataPtr->encILComp->numInport = 1;
  pAppDataPtr->encILComp->numOutport = 1;
  pAppDataPtr->encILComp->startOutportIndex = 1;

  pAppDataPtr->disILComp->numInport = 1;
  /* display does not has o/p ports */
  pAppDataPtr->disILComp->numOutport = 0;
  pAppDataPtr->disILComp->startOutportIndex = 0;

  /* allocate data structure for input and output port params of IL client
     component, It is for maintaining data structure in IL Client only.
     Components will have its own data structure inside omx components */

  pAppDataPtr->capILComp->outPortParams =
    malloc (sizeof (IL_CLIENT_OUTPORT_PARAMS) *
            pAppDataPtr->capILComp->numOutport);

  memset (pAppDataPtr->capILComp->outPortParams, 0x0,
          sizeof (IL_CLIENT_OUTPORT_PARAMS) *
          pAppDataPtr->capILComp->numOutport);

  pAppDataPtr->deiILComp->inPortParams =
    malloc (sizeof (IL_CLIENT_INPORT_PARAMS) *
            pAppDataPtr->deiILComp->numInport);

  memset (pAppDataPtr->deiILComp->inPortParams, 0x0,
          sizeof (IL_CLIENT_INPORT_PARAMS));

  pAppDataPtr->deiILComp->outPortParams =
    malloc (sizeof (IL_CLIENT_INPORT_PARAMS) *
            pAppDataPtr->deiILComp->numOutport);
  memset (pAppDataPtr->deiILComp->outPortParams, 0x0,
          pAppDataPtr->deiILComp->numOutport *
          sizeof (IL_CLIENT_OUTPORT_PARAMS));

  pAppDataPtr->encILComp->inPortParams =
    malloc (sizeof (IL_CLIENT_INPORT_PARAMS) *
            pAppDataPtr->encILComp->numInport);
  memset (pAppDataPtr->encILComp->inPortParams, 0x0,
          sizeof (IL_CLIENT_INPORT_PARAMS));

  pAppDataPtr->encILComp->outPortParams =
    malloc (sizeof (IL_CLIENT_INPORT_PARAMS) *
            pAppDataPtr->encILComp->numOutport);
  memset (pAppDataPtr->encILComp->outPortParams, 0x0,
          sizeof (IL_CLIENT_OUTPORT_PARAMS));

  pAppDataPtr->disILComp->inPortParams =
    malloc (sizeof (IL_CLIENT_INPORT_PARAMS) *
            pAppDataPtr->disILComp->numInport);
  memset (pAppDataPtr->disILComp->inPortParams, 0x0,
          sizeof (IL_CLIENT_INPORT_PARAMS));

  /* specify some of the parameters, that will be used for initializing OMX
     component parameters */

  for (i = 0; i < pAppDataPtr->capILComp->numOutport; i++)
  {
    outPortParamsPtr = pAppDataPtr->capILComp->outPortParams + i;
    outPortParamsPtr->nBufferCountActual =
      IL_CLIENT_CAPTURE_OUTPUT_BUFFER_COUNT;

    outPortParamsPtr->nBufferSize =
      (pAppDataPtr->nHeight * pAppDataPtr->nWidth * 3) >> 1;

    /* this pipe will not be used in this application, as capture does not read 
       / write into file */
    pipe (outPortParamsPtr->opBufPipe);
  }

  /* each componet will have local pipe to take buffers from other component or 
     its own consumed buffer, so that it can be passed to other connected
     components */
  pipe (pAppDataPtr->capILComp->localPipe);

  for (i = 0; i < pAppDataPtr->deiILComp->numInport; i++)
  {
    inPortParamsPtr = pAppDataPtr->deiILComp->inPortParams + i;
    inPortParamsPtr->nBufferCountActual = IL_CLIENT_DEI_INPUT_BUFFER_COUNT;
    /* since input of DEI is connected to output of capture, size is same as
       capture o/p buffers yuv 420 format */
    inPortParamsPtr->nBufferSize =
      (pAppDataPtr->nHeight * pAppDataPtr->nWidth * 3) >> 1;
    /* this pipe will not be used in this application, as dei does not read /
       write into file */
    pipe (inPortParamsPtr->ipBufPipe);
  }
  for (i = 0; i < pAppDataPtr->deiILComp->numOutport; i++)
  {

    outPortParamsPtr = pAppDataPtr->deiILComp->outPortParams + i;
    outPortParamsPtr->nBufferCountActual = IL_CLIENT_DEI_OUTPUT_BUFFER_COUNT;
    /* DEI one o/p is configured for YUV 422 output, so buffer size is
       calculated as follows */
    if (!(i % 2))
    {
      outPortParamsPtr->nBufferSize =
        pAppDataPtr->nHeight * pAppDataPtr->nWidth * 2;
      outPortParamsPtr->nBufferCountActual = IL_CLIENT_DEI_OUTPUT_BUFFER_COUNT;

     if (1 == pAppDataPtr->displayId) {
      /* configure the buffer size to that of the display size, for custom
         display this can be used to change width and height */
      outPortParamsPtr->nBufferSize = DISPLAY_HEIGHT * DISPLAY_WIDTH * 2;      
    }

    }
    else
    {
      /* DEI one o/p is configured for YUV 420 output for encode, so buffer
         size is calculated as follows */
      outPortParamsPtr->nBufferSize =
        (pAppDataPtr->nHeight * pAppDataPtr->nWidth * 3) >> 1;
      outPortParamsPtr->nBufferCountActual = IL_CLIENT_ENC_INPUT_BUFFER_COUNT;

    }

    /* this pipe will not be used in this application, as dei does not read /
       write into file */
    pipe (outPortParamsPtr->opBufPipe);
  }

  /* each componet will have local pipe to take bufffes from other component or 
     its own consumed buffer, so that it can be passed to other connected
     components */
  pipe (pAppDataPtr->deiILComp->localPipe);

  for (i = 0; i < pAppDataPtr->encILComp->numInport; i++)
  {
    inPortParamsPtr = pAppDataPtr->encILComp->inPortParams + i;
    inPortParamsPtr->nBufferCountActual = IL_CLIENT_ENC_INPUT_BUFFER_COUNT;
    /* input buffers size for yuv buffers, format is YUV420 hence 3/2 */
    inPortParamsPtr->nBufferSize =
      (pAppDataPtr->nHeight * pAppDataPtr->nWidth * 3) >> 1;
    /* this pipe is used for taking buffers from file read thread; in this
       example, file read is not used */
    pipe (inPortParamsPtr->ipBufPipe);
  }
  for (i = 0; i < pAppDataPtr->encILComp->numOutport; i++)
  {
    outPortParamsPtr = pAppDataPtr->encILComp->outPortParams + i;
    outPortParamsPtr->nBufferCountActual = IL_CLIENT_ENC_OUTPUT_BUFFER_COUNT;
    /* this size could be smaller than this value */
    outPortParamsPtr->nBufferSize =
      (pAppDataPtr->nHeight * pAppDataPtr->nWidth * 3) >> 1;

    /* This pipe is used if output is directed to file write thread, */
    pipe (outPortParamsPtr->opBufPipe);
  }
  /* each componet will have local pipe to take bufffers from other component
     or its own consumed buffer, so that it can be passed to other connected
     components */
  pipe (pAppDataPtr->encILComp->localPipe);

  for (i = 0; i < pAppDataPtr->disILComp->numInport; i++)
  {
    inPortParamsPtr = pAppDataPtr->disILComp->inPortParams + i;
    inPortParamsPtr->nBufferCountActual = IL_CLIENT_DISPLAY_INPUT_BUFFER_COUNT;

    inPortParamsPtr->nBufferSize =
      pAppDataPtr->nHeight * pAppDataPtr->nWidth * 2;
    
    /* configuring for custom display parameters */  
     if (1 == pAppDataPtr->displayId) {
      inPortParamsPtr->nBufferSize = DISPLAY_HEIGHT * DISPLAY_WIDTH * 2;     
     }
    
    /* this pipe will not be used in this application, as display does not read 
       / write into file */
    pipe (inPortParamsPtr->ipBufPipe);
  }
  /* each componet will have local pipe to take bufffers from other component
     or its own consumed buffer, so that it can be passed to other connected
     components */
  pipe (pAppDataPtr->disILComp->localPipe);

  /* populate the pointer for allocated data structure */
  *pAppData = pAppDataPtr;
}

/* ========================================================================== */
/**
* IL_ClientInit() : This function is to deinitialize the application
*                   data structure.
*
* @param pAppData          : appliaction / client data Handle 
*  @return      
*
*
*/
/* ========================================================================== */

void IL_ClientDeInit (IL_Client * pAppData)
{
  int i;
  IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
  IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;

  close (pAppData->capILComp->localPipe);

  for (i = 0; i < pAppData->capILComp->numOutport; i++)
  {
    outPortParamsPtr = pAppData->capILComp->outPortParams + i;
    /* this pipe will not be used in this application, as capture does not read 
       / write into file */
    close (outPortParamsPtr->opBufPipe);
  }

  close (pAppData->disILComp->localPipe);

  for (i = 0; i < pAppData->disILComp->numInport; i++)
  {
    inPortParamsPtr = pAppData->disILComp->inPortParams + i;
    /* this pipe will not be used in this application, as display does not read 
       / write into file */
    close (inPortParamsPtr->ipBufPipe);
  }

  close (pAppData->deiILComp->localPipe);

  for (i = 0; i < pAppData->deiILComp->numInport; i++)
  {
    inPortParamsPtr = pAppData->deiILComp->inPortParams + i;
    /* this pipe is not used in this application, as DEI does not read / write
       into file */
    close (inPortParamsPtr->ipBufPipe);
  }
  for (i = 0; i < pAppData->deiILComp->numOutport; i++)
  {
    outPortParamsPtr = pAppData->deiILComp->outPortParams + i;
    /* this pipe is not used in this application, as dei does not read / write
       into file */
    close (outPortParamsPtr->opBufPipe);
  }

  close (pAppData->encILComp->localPipe);

  for (i = 0; i < pAppData->encILComp->numInport; i++)
  {
    inPortParamsPtr = pAppData->encILComp->inPortParams + i;
    /* This pipe is used if output is directed to file write thread, in this
       example, file read is not used */
    close (inPortParamsPtr->ipBufPipe);
  }
  for (i = 0; i < pAppData->encILComp->numOutport; i++)
  {
    outPortParamsPtr = pAppData->encILComp->outPortParams + i;
    close (outPortParamsPtr->opBufPipe);
  }

  free(pAppData->capILComp->outPortParams);

  free (pAppData->disILComp->inPortParams);

  free (pAppData->deiILComp->inPortParams);

  free (pAppData->deiILComp->outPortParams);

  free (pAppData->encILComp->inPortParams);

  free (pAppData->encILComp->outPortParams);

  /* these semaphores are used for tracking the callbacks received from
     component */
  semp_deinit (pAppData->deiILComp->eos);
  free(pAppData->deiILComp->eos);

  semp_deinit (pAppData->deiILComp->done_sem);
  free(pAppData->deiILComp->done_sem);

  semp_deinit (pAppData->deiILComp->port_sem);
  free(pAppData->deiILComp->port_sem);

  semp_deinit (pAppData->capILComp->eos);
  free(pAppData->capILComp->eos);

  semp_deinit (pAppData->capILComp->done_sem);
  free(pAppData->capILComp->done_sem);

  semp_deinit (pAppData->capILComp->port_sem);
  free(pAppData->capILComp->port_sem);

  semp_deinit (pAppData->disILComp->eos);
  free(pAppData->disILComp->eos);

  semp_deinit (pAppData->disILComp->done_sem);
  free(pAppData->disILComp->done_sem);

  semp_deinit (pAppData->disILComp->port_sem);
  free(pAppData->disILComp->port_sem);

  semp_deinit (pAppData->encILComp->eos);
  free(pAppData->encILComp->eos);

  semp_deinit (pAppData->encILComp->done_sem);
  free(pAppData->encILComp->done_sem);

  semp_deinit (pAppData->encILComp->port_sem);
  free(pAppData->encILComp->port_sem);

  free (pAppData->encILComp);

  free (pAppData->deiILComp);

  free (pAppData->capILComp);

  free (pAppData->disILComp);

  free (pAppData);

}

/* ========================================================================== */
/**
* IL_ClientErrorToStr() : Function to map the OMX error enum to string
*
* @param error   : OMX Error type
*
*  @return      
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */

OMX_STRING IL_ClientErrorToStr (OMX_ERRORTYPE error)
{
  OMX_STRING errorString;

  /* used for printing purpose */
  switch (error)
  {
    case OMX_ErrorNone:
      errorString = "OMX_ErrorNone";
      break;
    case OMX_ErrorInsufficientResources:
      errorString = "OMX_ErrorInsufficientResources";
      break;
    case OMX_ErrorUndefined:
      errorString = "OMX_ErrorUndefined";
      break;
    case OMX_ErrorInvalidComponentName:
      errorString = "OMX_ErrorInvalidComponentName";
      break;
    case OMX_ErrorComponentNotFound:
      errorString = "OMX_ErrorComponentNotFound";
      break;
    case OMX_ErrorInvalidComponent:
      errorString = "OMX_ErrorInvalidComponent";
      break;
    case OMX_ErrorBadParameter:
      errorString = "OMX_ErrorBadParameter";
      break;
    case OMX_ErrorNotImplemented:
      errorString = "OMX_ErrorNotImplemented";
      break;
    case OMX_ErrorUnderflow:
      errorString = "OMX_ErrorUnderflow";
      break;
    case OMX_ErrorOverflow:
      errorString = "OMX_ErrorOverflow";
      break;
    case OMX_ErrorHardware:
      errorString = "OMX_ErrorHardware";
      break;
    case OMX_ErrorInvalidState:
      errorString = "OMX_ErrorInvalidState";
      break;
    case OMX_ErrorStreamCorrupt:
      errorString = "OMX_ErrorStreamCorrupt";
      break;
    case OMX_ErrorPortsNotCompatible:
      errorString = "OMX_ErrorPortsNotCompatible";
      break;
    case OMX_ErrorResourcesLost:
      errorString = "OMX_ErrorResourcesLost";
      break;
    case OMX_ErrorNoMore:
      errorString = "OMX_ErrorNoMore";
      break;
    case OMX_ErrorVersionMismatch:
      errorString = "OMX_ErrorVersionMismatch";
      break;
    case OMX_ErrorNotReady:
      errorString = "OMX_ErrorNotReady";
      break;
    case OMX_ErrorTimeout:
      errorString = "OMX_ErrorTimeout";
      break;
    default:
      errorString = "<unknown>";
  }

  return errorString;
}

/* ========================================================================== */
/**
* IL_ClientUtilGetSelfBufHeader() : This util function is to get buffer header
*                                   specific to one component, from the buffer
*                                   received from other component  .
*
* @param thisComp   : application component data structure
* @param pBuffer    : OMX buffer pointer
* @param type       : it is to identfy teh port type
* @param portIndex  : port number of the component
* @param pBufferOut : components buffer header correponding to pBuffer
*
*  @return      
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientUtilGetSelfBufHeader (IL_CLIENT_COMP_PRIVATE *thisComp,
                                             OMX_U8 *pBuffer,
                                             ILCLIENT_PORT_TYPE type,
                                             OMX_U32 portIndex,
                                             OMX_BUFFERHEADERTYPE **pBufferOut)
{
  int i;
  IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
  IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
  OMX_ERRORTYPE eError = OMX_ErrorNone;

  /* Check for input port buffer header queue */
  if (type == ILCLIENT_INPUT_PORT)
  {
    inPortParamsPtr = thisComp->inPortParams + portIndex;
    for (i = 0; i < inPortParamsPtr->nBufferCountActual; i++)
    {
      if (pBuffer == inPortParamsPtr->pInBuff[i]->pBuffer)
      {
        *pBufferOut = inPortParamsPtr->pInBuff[i];
      }
    }
  }
  /* Check for output port buffer header queue */
  else
  {
    outPortParamsPtr =
      thisComp->outPortParams + portIndex - thisComp->startOutportIndex;
    for (i = 0; i < outPortParamsPtr->nBufferCountActual; i++)
    {
      if (pBuffer == outPortParamsPtr->pOutBuff[i]->pBuffer)
      {
        *pBufferOut = outPortParamsPtr->pOutBuff[i];
      }
    }
  }

  return (eError);
}

/* ========================================================================== */
/**
* IL_ClientConnectComponents() : This util function is to update the pipe
*                                information of other connected comonnet, so that
*                                buffers can be passed to connected componnet.
*
* @param handleCompPrivA   : application component data structure for producer
* @param compAPortOut      : port of producer comp
* @param handleCompPrivB   : application component data structure for consumer
* @param compBPortIn       : port number of the consumer component
*
*  @return      
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientConnectComponents (IL_CLIENT_COMP_PRIVATE 
                                            *handleCompPrivA,
                                          unsigned int compAPortOut,
                                          IL_CLIENT_COMP_PRIVATE
                                            *handleCompPrivB,
                                          unsigned int compBPortIn)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  IL_CLIENT_OUTPORT_PARAMS *outPortParamPtr = NULL;
  IL_CLIENT_INPORT_PARAMS *inPortParamPtr = NULL;

  /* update the input port connect structure */
  outPortParamPtr =
    handleCompPrivA->outPortParams + compAPortOut -
    handleCompPrivA->startOutportIndex;

  inPortParamPtr = handleCompPrivB->inPortParams + compBPortIn;

  /* update input port component pipe info with connected port */
  inPortParamPtr->connInfo.remoteClient = handleCompPrivA;
  inPortParamPtr->connInfo.remotePort = compAPortOut;
  inPortParamPtr->connInfo.remotePipe[0] = handleCompPrivA->localPipe[0];
  inPortParamPtr->connInfo.remotePipe[1] = handleCompPrivA->localPipe[1];

  /* update output port component pipe info with connected port */
  outPortParamPtr->connInfo.remoteClient = handleCompPrivB;
  outPortParamPtr->connInfo.remotePort = compBPortIn;
  outPortParamPtr->connInfo.remotePipe[0] = handleCompPrivB->localPipe[0];
  outPortParamPtr->connInfo.remotePipe[1] = handleCompPrivB->localPipe[1];

  return eError;
}

/* ========================================================================== */
/**
* IL_ClientUseInitialOutputResources() :  This function gives initially all
*                                         output buffers to a component.
*                                         after consuming component would keep
*                                         in local pipe for connect thread use. 
*
* @param pAppdata   : application data structure
*
*  @return      
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientUseInitialOutputResources (IL_CLIENT_COMP_PRIVATE 
                                                    *thisComp)
{

  OMX_ERRORTYPE err = OMX_ErrorNone;
  unsigned int i = 0, j;
  IL_CLIENT_OUTPORT_PARAMS *outPortParamPtr = NULL;
  OMX_PARAM_PORTDEFINITIONTYPE param;

  memset (&param, 0, sizeof (param));

  OMX_INIT_PARAM (&param);

  /* Give output buffers to component which is limited by no of output buffers
     available. Rest of the data will be written on the callback from output
     data write thread */
  for (j = 0; j < thisComp->numOutport; j++)
  {

    param.nPortIndex = j + thisComp->startOutportIndex;

    OMX_GetParameter (thisComp->handle, OMX_IndexParamPortDefinition, &param);

    outPortParamPtr = thisComp->outPortParams + j;

    if (OMX_TRUE == param.bEnabled)
    {
      if (outPortParamPtr->connInfo.remotePipe[0] != NULL)
      {

        for (i = 0; i < thisComp->outPortParams->nBufferCountActual; i++)
        {
          /* Pass the output buffer to the component */
          err =
            OMX_FillThisBuffer (thisComp->handle, outPortParamPtr->pOutBuff[i]);

        } /* for (i) */
      } /* if (outPortParamPtr...) */
    } /* if (OMX_TRUE) */
  } /* for (j) */

  return err;
}

/* ========================================================================== */
/**
* IL_ClientEncUseInitialOutputResources() :  This function gives initially all
*                                         output buffers to a component.
*                                         after consuming component would keep
*                                         in local pipe for connect thread use. 
*
* @param pAppdata   : application data structure
*
*  @return      
*  String conversion of the OMX_ERRORTYPE
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientEncUseInitialOutputResources (IL_CLIENT_COMP_PRIVATE 
                                                       *thisComp)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;
  unsigned int i = 0;

  for (i = 0; i < thisComp->outPortParams->nBufferCountActual; i++)
  {
    /* Pass the output buffer to the component */
    err = OMX_FillThisBuffer (thisComp->handle,
                              thisComp->outPortParams->pOutBuff[i]);
  }

  return err;
}

/* ========================================================================== */
/**
* IL_ClientSetDeiParams() : Function to fill the port definition 
* structures and call the Set_Parameter function on to the Capture
* Component
*
* @param pAppData   : Pointer to the application data
*
*  @return      
*  OMX_ErrorNone = Successful 
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientSetCaptureParams (IL_Client *pAppData)
{

  OMX_PARAM_VFCC_HWPORT_PROPERTIES sHwPortParam;

  OMX_PARAM_VFCC_HWPORT_ID sHwPortId;

  OMX_CONFIG_VFCC_FRAMESKIP_INFO sCapSkipFrames;

  OMX_PARAM_CTRL_VIDDECODER_INFO sVidDecParam;

  OMX_PARAM_BUFFER_MEMORYTYPE memTypeCfg;

  OMX_PARAM_PORTDEFINITIONTYPE paramPort;

  OMX_ERRORTYPE eError = OMX_ErrorNone;

  OMX_INIT_PARAM (&paramPort);

  /* set input height/width and color format */
  paramPort.nPortIndex = OMX_VFCC_OUTPUT_PORT_START_INDEX;
  OMX_GetParameter (pAppData->pCapHandle, OMX_IndexParamPortDefinition,
                    &paramPort);
  paramPort.nPortIndex = OMX_VFCC_OUTPUT_PORT_START_INDEX;
  paramPort.format.video.nFrameWidth = pAppData->nWidth;
  paramPort.format.video.nFrameHeight = pAppData->nHeight;
  paramPort.format.video.nStride = pAppData->nWidth;
  paramPort.nBufferCountActual = IL_CLIENT_CAPTURE_OUTPUT_BUFFER_COUNT;
  paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
  /* Capture output in 420 format */
  paramPort.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;

  paramPort.nBufferSize =
    (paramPort.format.video.nStride * pAppData->nHeight * 3) >> 1;
  printf ("Buffer Size computed: %d\n", paramPort.nBufferSize);
  printf ("set input port params (width = %d, height = %d)", pAppData->nWidth,
          pAppData->nHeight);
  OMX_SetParameter (pAppData->pCapHandle, OMX_IndexParamPortDefinition,
                    &paramPort);

  /* Setting Memory type at output port to Raw Memory */
  OMX_INIT_PARAM (&memTypeCfg);
  memTypeCfg.nPortIndex = OMX_VFCC_OUTPUT_PORT_START_INDEX;
  memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
  eError =
    OMX_SetParameter (pAppData->pCapHandle, OMX_TI_IndexParamBuffMemType,
                      &memTypeCfg);

  if (eError != OMX_ErrorNone)
    ERROR ("failed to set memory Type at output port\n");

  OMX_INIT_PARAM (&sHwPortId);
  /* capture on EIO card is component input at VIP1 port */
  sHwPortId.eHwPortId = OMX_VIDEO_CaptureHWPortVIP1_PORTA;
  eError = OMX_SetParameter (pAppData->pCapHandle,
                             (OMX_INDEXTYPE) OMX_TI_IndexParamVFCCHwPortID,
                             (OMX_PTR) & sHwPortId);

  OMX_INIT_PARAM (&sHwPortParam);

  sHwPortParam.eCaptMode = OMX_VIDEO_CaptureModeSC_NON_MUX;
  sHwPortParam.eVifMode = OMX_VIDEO_CaptureVifMode_16BIT;
  sHwPortParam.eInColorFormat = OMX_COLOR_FormatYCbYCr;
  sHwPortParam.eScanType = OMX_VIDEO_CaptureScanTypeProgressive;
  sHwPortParam.nMaxHeight = pAppData->nHeight;
  sHwPortParam.nMaxWidth = pAppData->nWidth;
  sHwPortParam.nMaxChnlsPerHwPort = 1;

  eError = OMX_SetParameter (pAppData->pCapHandle,
                             (OMX_INDEXTYPE)
                             OMX_TI_IndexParamVFCCHwPortProperties,
                             (OMX_PTR) & sHwPortParam);

  if (pAppData->nFrameRate == 30)
  {
    OMX_INIT_PARAM (&sCapSkipFrames);
    printf (" applying skip mask \n");

    sCapSkipFrames.frameSkipMask = 0x2AAAAAAA;
    eError = OMX_SetConfig (pAppData->pCapHandle,
                            (OMX_INDEXTYPE) OMX_TI_IndexConfigVFCCFrameSkip,
                            (OMX_PTR) & sCapSkipFrames);
  }

  /* Set parameters for TVP controller */

  OMX_INIT_PARAM (&sHwPortId);
  /* capture on EIO card is component input at VIP1 port */
  sHwPortId.eHwPortId = OMX_VIDEO_CaptureHWPortVIP1_PORTA;
  eError = OMX_SetParameter (pAppData->pTvpHandle,
                             (OMX_INDEXTYPE) OMX_TI_IndexParamVFCCHwPortID,
                             (OMX_PTR) & sHwPortId);
  OMX_INIT_PARAM (&sHwPortParam);
  sHwPortParam.eCaptMode = OMX_VIDEO_CaptureModeSC_NON_MUX;
  sHwPortParam.eVifMode = OMX_VIDEO_CaptureVifMode_16BIT;
  sHwPortParam.eInColorFormat = OMX_COLOR_FormatYCbYCr;
  sHwPortParam.eScanType = OMX_VIDEO_CaptureScanTypeProgressive;
  sHwPortParam.nMaxHeight = pAppData->nHeight;
  sHwPortParam.nMaxWidth = pAppData->nWidth;
  sHwPortParam.nMaxChnlsPerHwPort = 1;

  eError = OMX_SetParameter (pAppData->pTvpHandle,
                             (OMX_INDEXTYPE)
                             OMX_TI_IndexParamVFCCHwPortProperties,
                             (OMX_PTR) & sHwPortParam);

  OMX_INIT_PARAM (&sVidDecParam);

  /* set the mode based on capture/display device */
  if (strcmp (pAppData->mode, "1080p") == 0) {
  sVidDecParam.videoStandard =  OMX_VIDEO_DECODER_STD_1080P_60;
  }
  else if (strcmp (pAppData->mode, "720p") == 0) {
  sVidDecParam.videoStandard =  OMX_VIDEO_DECODER_STD_720P_60;
  }
  else
  {
    ERROR ("Incorrect Display Mode configured!!\n");
  }
  
  /* setting TVP7002 component input */
  sVidDecParam.videoDecoderId = OMX_VID_DEC_TVP7002_DRV;
  sVidDecParam.videoSystemId = OMX_VIDEO_DECODER_VIDEO_SYSTEM_AUTO_DETECT;
  eError = OMX_SetParameter (pAppData->pTvpHandle,
                             (OMX_INDEXTYPE) OMX_TI_IndexParamCTRLVidDecInfo,
                             (OMX_PTR) & sVidDecParam);
  if (eError != OMX_ErrorNone)
    ERROR ("failed to set Ctrl Vid dec info \n");

}

/* ========================================================================== */
/**
* IL_ClientSetEncodeParams() : Function to fill the port definition 
* structures and call the Set_Parameter function on to the Encode
* Component
*
* @param pAppData   : Pointer to the application data
*
*  @return      
*  OMX_ErrorNone = Successful 
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientSetEncodeParams (IL_Client *pAppData)
{
  OMX_ERRORTYPE eError = OMX_ErrorUndefined;
  OMX_HANDLETYPE pHandle = NULL;
  OMX_VIDEO_PARAM_PROFILELEVELTYPE tProfileLevel;
  OMX_VIDEO_PARAM_ENCODER_PRESETTYPE tEncoderPreset;
  OMX_VIDEO_PARAM_BITRATETYPE tVidEncBitRate;
  OMX_VIDEO_PARAM_PORTFORMATTYPE tVideoParams;
  OMX_PARAM_PORTDEFINITIONTYPE tPortDef;
  OMX_VIDEO_CONFIG_DYNAMICPARAMS tDynParams;
  OMX_VIDEO_PARAM_STATICPARAMS   tStaticParam;

  pHandle = pAppData->pEncHandle;

  /* Number of frames to be encoded */
  pAppData->encILComp->numFrames = pAppData->nEncodedFrms;

  /* Set the profile and level for H264 */
  OMX_INIT_PARAM (&tProfileLevel);
  tProfileLevel.nPortIndex = OMX_VIDENC_OUTPUT_PORT;

  eError = OMX_GetParameter (pHandle, OMX_IndexParamVideoProfileLevelCurrent,
                             &tProfileLevel);

  /* set as baseline 4.2 level */

  tProfileLevel.eProfile = OMX_VIDEO_AVCProfileBaseline;
  tProfileLevel.eLevel = OMX_VIDEO_AVCLevel42;

  eError = OMX_SetParameter (pHandle, OMX_IndexParamVideoProfileLevelCurrent,
                             &tProfileLevel);
  if (eError != OMX_ErrorNone)
    ERROR ("failed to set encoder pfofile \n");

  /* Encoder Preset settings */
  OMX_INIT_PARAM (&tEncoderPreset);
  tEncoderPreset.nPortIndex = OMX_VIDENC_OUTPUT_PORT;
  eError = OMX_GetParameter (pHandle, OMX_TI_IndexParamVideoEncoderPreset,
                             &tEncoderPreset);

  tEncoderPreset.eEncodingModePreset = OMX_Video_Enc_Med_Speed_High_Quality;
  tEncoderPreset.eRateControlPreset = OMX_Video_RC_Low_Delay;

  eError = OMX_SetParameter (pHandle, OMX_TI_IndexParamVideoEncoderPreset,
                             &tEncoderPreset);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to Encoder Preset \n");
  }

  OMX_INIT_PARAM (&tPortDef);
  /* Get the Number of Ports */

  tPortDef.nPortIndex = OMX_VIDENC_INPUT_PORT;
  eError = OMX_GetParameter (pHandle, OMX_IndexParamPortDefinition, &tPortDef);
  /* set the actual number of buffers required */
  tPortDef.nBufferCountActual = IL_CLIENT_ENC_INPUT_BUFFER_COUNT;
  /* set the video format settings */
  tPortDef.format.video.nFrameWidth = pAppData->nWidth;
  tPortDef.format.video.nStride = pAppData->nWidth;
  tPortDef.format.video.nFrameHeight = pAppData->nHeight;
  tPortDef.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  /* settings for OMX_IndexParamVideoPortFormat */
  tPortDef.nBufferSize = (pAppData->nWidth * pAppData->nHeight * 3) >> 1;
  eError = OMX_SetParameter (pHandle, OMX_IndexParamPortDefinition, &tPortDef);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set Encode OMX_IndexParamPortDefinition for input \n");
  }

  OMX_INIT_PARAM (&tPortDef);

  tPortDef.nPortIndex = OMX_VIDENC_OUTPUT_PORT;
  eError = OMX_GetParameter (pHandle, OMX_IndexParamPortDefinition, &tPortDef);
  /* settings for OMX_IndexParamPortDefinition */
  /* set the actual number of buffers required */
  tPortDef.nBufferCountActual = IL_CLIENT_ENC_OUTPUT_BUFFER_COUNT;
  tPortDef.format.video.nFrameWidth = pAppData->nWidth;
  tPortDef.format.video.nFrameHeight = pAppData->nHeight;
  tPortDef.format.video.eCompressionFormat = OMX_VIDEO_CodingAVC;
  tPortDef.format.video.xFramerate = (pAppData->nFrameRate << 16);
  tVideoParams.xFramerate = (pAppData->nFrameRate << 16);
  tPortDef.format.video.nBitrate = pAppData->nBitRate;
  /* settings for OMX_IndexParamVideoPortFormat */

  eError = OMX_SetParameter (pHandle, OMX_IndexParamPortDefinition, &tPortDef);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set Encode OMX_IndexParamPortDefinition for output \n");
  }

  /* For changing bit rate following index can be used */
  OMX_INIT_PARAM (&tVidEncBitRate);

  tVidEncBitRate.nPortIndex = OMX_DirOutput;
  eError = OMX_GetParameter (pHandle, OMX_IndexParamVideoBitrate,
                             &tVidEncBitRate);

  tVidEncBitRate.eControlRate = OMX_Video_ControlRateDisable;
  tVidEncBitRate.nTargetBitrate = pAppData->nBitRate;
  eError = OMX_SetParameter (pHandle, OMX_IndexParamVideoBitrate,
                             &tVidEncBitRate);

  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set Encode bitrate \n");
  }

  /* before creating use set_parameters, for run-time change use set_config
     all codec supported parameters can be set using this index       */

  OMX_INIT_PARAM (&tDynParams);

  tDynParams.nPortIndex = OMX_VIDENC_OUTPUT_PORT;
  
  eError = OMX_GetParameter (pHandle, OMX_TI_IndexParamVideoDynamicParams,
                             &tDynParams);
  
  /* setting I frame interval */
  tDynParams.videoDynamicParams.h264EncDynamicParams.videnc2DynamicParams.intraFrameInterval = 90;
                         
  eError = OMX_SetParameter (pHandle, OMX_TI_IndexParamVideoDynamicParams,
                             &tDynParams);
                             
  

  return eError;
}

/* ========================================================================== */
/**
* IL_ClientSetDeiParams() : Function to fill the port definition 
* structures and call the Set_Parameter function on to the DEI
* Component
*
* @param pAppData   : Pointer to the application data
*
*  @return      
*  OMX_ErrorNone = Successful 
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientSetDeiParams (IL_Client *pAppData)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  OMX_PARAM_BUFFER_MEMORYTYPE memTypeCfg;
  OMX_PARAM_PORTDEFINITIONTYPE paramPort;
  OMX_PARAM_VFPC_NUMCHANNELPERHANDLE sNumChPerHandle;
  OMX_CONFIG_ALG_ENABLE algEnable;
  OMX_CONFIG_VIDCHANNEL_RESOLUTION chResolution;

  OMX_CONFIG_SUBSAMPLING_FACTOR sSubSamplinginfo = {NULL};
  
  OMX_INIT_PARAM(&sSubSamplinginfo);

  sSubSamplinginfo.nSubSamplingFactor = 1;
  eError = OMX_SetConfig ( pAppData->pDeiHandle, ( OMX_INDEXTYPE )
                           ( OMX_TI_IndexConfigSubSamplingFactor ),
                             &sSubSamplinginfo );

  OMX_INIT_PARAM (&memTypeCfg);
  memTypeCfg.nPortIndex = OMX_VFPC_INPUT_PORT_START_INDEX;
  memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
  eError = OMX_SetParameter (pAppData->pDeiHandle, OMX_TI_IndexParamBuffMemType,
                             &memTypeCfg);

  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set memory Type at input port\n");
  }

  /* Setting Memory type at output port to Raw Memory */
  OMX_INIT_PARAM (&memTypeCfg);
  memTypeCfg.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX;
  memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
  eError = OMX_SetParameter (pAppData->pDeiHandle, OMX_TI_IndexParamBuffMemType,
                             &memTypeCfg);

  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set memory Type at output port\n");
  }

  OMX_INIT_PARAM (&memTypeCfg);
  memTypeCfg.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX + 1;
  memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
  eError = OMX_SetParameter (pAppData->pDeiHandle, OMX_TI_IndexParamBuffMemType,
                             &memTypeCfg);

  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set memory Type at output port\n");
  }

  /* set input height/width and color format */
  OMX_INIT_PARAM (&paramPort);
  paramPort.nPortIndex = OMX_VFPC_INPUT_PORT_START_INDEX;

  OMX_GetParameter (pAppData->pDeiHandle, OMX_IndexParamPortDefinition,
                    &paramPort);
  paramPort.nPortIndex = OMX_VFPC_INPUT_PORT_START_INDEX;
  paramPort.format.video.nFrameWidth = pAppData->nWidth;
  paramPort.format.video.nFrameHeight = pAppData->nHeight/2;

  paramPort.format.video.nStride = pAppData->nWidth;
  paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
  paramPort.format.video.eColorFormat = OMX_COLOR_FormatYUV420SemiPlanar;
  paramPort.nBufferSize =
    (paramPort.format.video.nStride * pAppData->nHeight * 3) >> 1;
  paramPort.nBufferAlignment = 0;
  paramPort.bBuffersContiguous = 0;
  paramPort.nBufferCountActual = IL_CLIENT_DEI_INPUT_BUFFER_COUNT;
  printf ("set input port params (width = %u, height = %u) \n",
          pAppData->nWidth, pAppData->nHeight);
  OMX_SetParameter (pAppData->pDeiHandle, OMX_IndexParamPortDefinition,
                    &paramPort);

  /* set output height/width and color format */
  OMX_INIT_PARAM (&paramPort);
  paramPort.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX;
  OMX_GetParameter (pAppData->pDeiHandle, OMX_IndexParamPortDefinition,
                    &paramPort);

  paramPort.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX;
  paramPort.format.video.nFrameWidth = pAppData->nWidth;
  paramPort.format.video.nFrameHeight = pAppData->nHeight;
  paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
  paramPort.format.video.eColorFormat = OMX_COLOR_FormatYCbYCr;
  paramPort.nBufferAlignment = 0;
  paramPort.nBufferCountActual = IL_CLIENT_DEI_OUTPUT_BUFFER_COUNT;
  /* This port is connected to display and provides 422 o/p */
  paramPort.format.video.nStride = pAppData->nWidth * 2;
  paramPort.nBufferSize = paramPort.format.video.nStride *
                          paramPort.format.video.nFrameHeight;

    if (1 == pAppData->displayId) {
    /*For the case of On-chip HDMI as display device*/
    paramPort.format.video.nFrameWidth = DISPLAY_WIDTH;
    paramPort.format.video.nFrameHeight = DISPLAY_HEIGHT;
    paramPort.format.video.nStride = DISPLAY_HEIGHT * 2;
  }
  
  paramPort.nBufferSize =
    paramPort.format.video.nStride * paramPort.format.video.nFrameHeight;
                          

  printf ("set output port params (width = %d, height = %d)", pAppData->nWidth,
          pAppData->nHeight);

  OMX_SetParameter (pAppData->pDeiHandle, OMX_IndexParamPortDefinition,
                    &paramPort);

  OMX_INIT_PARAM (&paramPort);
  paramPort.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX + 1;
  OMX_GetParameter (pAppData->pDeiHandle, OMX_IndexParamPortDefinition,
                    &paramPort);

  paramPort.nPortIndex = OMX_VFPC_OUTPUT_PORT_START_INDEX + 1;
  paramPort.format.video.nFrameWidth = pAppData->nWidth;
  paramPort.format.video.nFrameHeight = pAppData->nHeight;
  paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
  paramPort.nBufferAlignment = 0;
  paramPort.nBufferCountActual = IL_CLIENT_ENC_INPUT_BUFFER_COUNT;
  paramPort.format.video.nStride = pAppData->nWidth;

  /* This port is connected to encoder and provides 420 o/p */
  paramPort.nBufferSize =
    (paramPort.format.video.nStride * paramPort.format.video.nFrameHeight *
     3) >> 1;

  printf ("set output port params (width = %d, height = %d)", pAppData->nWidth,
          pAppData->nHeight);

  OMX_SetParameter (pAppData->pDeiHandle, OMX_IndexParamPortDefinition,
                    &paramPort);

  /* set number of channles */
  printf ("set number of channels");

  OMX_INIT_PARAM (&sNumChPerHandle);
  sNumChPerHandle.nNumChannelsPerHandle = 1;
  eError = OMX_SetParameter (pAppData->pDeiHandle,
                             (OMX_INDEXTYPE)
                               OMX_TI_IndexParamVFPCNumChPerHandle,
                             &sNumChPerHandle);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set num of channels\n");
  }

  /* set VFPC input and output resolution information */
  printf ("set input resolution");

  OMX_INIT_PARAM (&chResolution);
  chResolution.Frm0Width = pAppData->nWidth;
  chResolution.Frm0Height = pAppData->nHeight;
  chResolution.Frm0Pitch = pAppData->nWidth;;
  chResolution.Frm1Width = 0;
  chResolution.Frm1Height = 0;
  chResolution.Frm1Pitch = 0;
  chResolution.FrmStartX = 0;
  chResolution.FrmStartY = 0;
  chResolution.FrmCropWidth = pAppData->nWidth;
  chResolution.FrmCropHeight = pAppData->nHeight;
  chResolution.eDir = OMX_DirInput;
  chResolution.nChId = 0;

  eError = OMX_SetConfig (pAppData->pDeiHandle,
                          (OMX_INDEXTYPE) OMX_TI_IndexConfigVidChResolution,
                          &chResolution);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set input channel resolution\n");
  }

  printf ("set output resolution");
  OMX_INIT_PARAM (&chResolution);
  /* first output to display */
  chResolution.Frm0Width = pAppData->nWidth;
  chResolution.Frm0Height = pAppData->nHeight;
  chResolution.Frm0Pitch = pAppData->nWidth * 2;

   if (1 == pAppData->displayId) {
    /* on secondary display, it is scaled to display size */  
    chResolution.Frm0Width = DISPLAY_WIDTH;
    chResolution.Frm0Height = DISPLAY_HEIGHT;
    chResolution.Frm0Pitch = DISPLAY_WIDTH * 2;  
  }
  
  
  /* second output to encode */
  chResolution.Frm1Width = pAppData->nWidth;
  chResolution.Frm1Height = pAppData->nHeight;
  chResolution.Frm1Pitch = pAppData->nWidth;
  chResolution.FrmStartX = 0;
  chResolution.FrmStartY = 0;
  chResolution.FrmCropWidth = 0;
  chResolution.FrmCropHeight = 0;
  chResolution.eDir = OMX_DirOutput;
  chResolution.nChId = 0;

  eError = OMX_SetConfig (pAppData->pDeiHandle,
                          (OMX_INDEXTYPE) OMX_TI_IndexConfigVidChResolution,
                          &chResolution);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set output channel resolution\n");
  }

  /* disable algo bypass mode */
  OMX_INIT_PARAM (&algEnable);
  algEnable.nPortIndex = 0;
  algEnable.nChId = 0;
  /* capture providing progressive input, alg is bypassed */
  algEnable.bAlgBypass = 1;

  eError = OMX_SetConfig (pAppData->pDeiHandle,
                          (OMX_INDEXTYPE) OMX_TI_IndexConfigAlgEnable,
                          &algEnable);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to disable algo by pass mode\n");
  }

}

/* ========================================================================== */
/**
* IL_ClientSetDisplayParams() : Function to fill the port definition 
* structures and call the Set_Parameter function on to the display
* Component
*
* @param pAppData   : Pointer to the application data
*
*  @return      
*  OMX_ErrorNone = Successful 
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientSetDisplayParams (IL_Client *pAppData)
{
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  OMX_PARAM_BUFFER_MEMORYTYPE memTypeCfg;
  OMX_PARAM_PORTDEFINITIONTYPE paramPort;
  OMX_PARAM_VFPC_NUMCHANNELPERHANDLE sNumChPerHandle;
  OMX_PARAM_VFDC_DRIVERINSTID driverId;
  OMX_PARAM_VFDC_CREATEMOSAICLAYOUT mosaicLayout;
  OMX_CONFIG_VFDC_MOSAICLAYOUT_PORT2WINMAP port2Winmap;

  OMX_INIT_PARAM (&paramPort);

  /* set input height/width and color format */
  paramPort.nPortIndex = OMX_VFDC_INPUT_PORT_START_INDEX;
  OMX_GetParameter (pAppData->pDisHandle, OMX_IndexParamPortDefinition,
                    &paramPort);
  paramPort.nPortIndex = OMX_VFDC_INPUT_PORT_START_INDEX;
  paramPort.format.video.nFrameWidth = pAppData->nWidth;
  paramPort.format.video.nFrameHeight = pAppData->nHeight;
  paramPort.format.video.nStride = pAppData->nWidth * 2;
  paramPort.nBufferCountActual = IL_CLIENT_DISPLAY_INPUT_BUFFER_COUNT;
  paramPort.format.video.eCompressionFormat = OMX_VIDEO_CodingUnused;
  paramPort.format.video.eColorFormat = OMX_COLOR_FormatYCbYCr;

  paramPort.nBufferSize = paramPort.format.video.nStride * pAppData->nHeight;
  
   if (1 == pAppData->displayId) {
    /* on secondary display, it is scaled to display size */  
    paramPort.format.video.nFrameWidth = DISPLAY_WIDTH;
    paramPort.format.video.nFrameHeight = DISPLAY_HEIGHT;
    paramPort.format.video.nStride = DISPLAY_WIDTH * 2;  
  }
  paramPort.nBufferSize = paramPort.format.video.nStride * pAppData->nHeight;
  
  printf ("Buffer Size computed: %d\n", paramPort.nBufferSize);
  printf ("set input port params (width = %d, height = %d)", pAppData->nWidth,
          pAppData->nHeight);
  OMX_SetParameter (pAppData->pDisHandle, OMX_IndexParamPortDefinition,
                    &paramPort);
  /* --------------------------------------------------------------------------*
     Supported display IDs by VFDC and DC are below The names will be renamed in
     future releases as some of the driver names & interfaces will be changed in
     future @ param OMX_VIDEO_DISPLAY_ID_HD0: 422P On-chip HDMI @ param
     OMX_VIDEO_DISPLAY_ID_HD1: 422P HDDAC component output @ param
     OMX_VIDEO_DISPLAY_ID_SD0: 420T/422T SD display (NTSC): Not supported yet.
     ------------------------------------------------------------------------ */

  /* set the parameter to the disaply component to 1080P @60 mode */
  OMX_INIT_PARAM (&driverId);
  /* Configured to use on-chip HDMI */

  if (0 == pAppData->displayId) {
    /* Configured to use on-chip HDMI */
     driverId.nDrvInstID = OMX_VIDEO_DISPLAY_ID_HD0;
     
    /* Depending on the mode parameter set via cmd line, set the mode*/
    if (strcmp (pAppData->mode, "1080p") == 0) {
      driverId.eDispVencMode = OMX_DC_MODE_1080P_60;
    }
    else if (strcmp (pAppData->mode, "720p") == 0) {
      driverId.eDispVencMode = OMX_DC_MODE_720P_60;
    }
    else {
      ERROR ("Incorrect Display Mode configured!!\n");
    }
    
  } 
  else if (1 == pAppData->displayId) {
    /* Configured to use LCD Display */
    driverId.nDrvInstID = OMX_VIDEO_DISPLAY_ID_HD1;
    driverId.eDispVencMode = DISPLAY_VENC_MODE;
  } 
  else {
    ERROR ("Incorrect Display Id configured\n");
  }
  
  eError = OMX_SetParameter (pAppData->pDisHandle,
                             (OMX_INDEXTYPE) OMX_TI_IndexParamVFDCDriverInstId,
                             &driverId);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set driver mode to 1080P@60\n");
  }

  /* set the parameter to the disaply controller component to 1080P @60 mode */
  OMX_INIT_PARAM (&driverId);
  /* Configured to use on-chip HDMI */

  if (0 == pAppData->displayId) {
    /* Configured to use on-chip HDMI */
     driverId.nDrvInstID = OMX_VIDEO_DISPLAY_ID_HD0;
     
    /* Depending on the mode parameter set via cmd line, set the mode*/
    if (strcmp (pAppData->mode, "1080p") == 0) {
      driverId.eDispVencMode = OMX_DC_MODE_1080P_60;
    }
    else if (strcmp (pAppData->mode, "720p") == 0) {
      driverId.eDispVencMode = OMX_DC_MODE_720P_60;
    }
    else {
      ERROR ("Incorrect Display Mode configured!!\n");
    }
    
  } 
  else if (1 == pAppData->displayId) {
    /* Configured to use LCD Display */
    driverId.nDrvInstID = OMX_VIDEO_DISPLAY_ID_HD1;
    driverId.eDispVencMode = DISPLAY_VENC_MODE;
  } 
  else {
    ERROR ("Incorrect Display Id configured\n");
  }

  eError = OMX_SetParameter (pAppData->pctrlHandle,
                             (OMX_INDEXTYPE) OMX_TI_IndexParamVFDCDriverInstId,
                             &driverId);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set driver mode to 1080P@60\n");
  }

  if (1 == pAppData->displayId) {
   IL_ClientSetSecondaryDisplayParams(pAppData);
   }

  /* set mosaic layout info */

  OMX_INIT_PARAM (&mosaicLayout);
  /* Configuring the first (and only) window */
  mosaicLayout.sMosaicWinFmt[0].winStartX = 0;
  mosaicLayout.sMosaicWinFmt[0].winStartY = 0;
  mosaicLayout.sMosaicWinFmt[0].winWidth = pAppData->nWidth;
  mosaicLayout.sMosaicWinFmt[0].winHeight = pAppData->nHeight;
  mosaicLayout.sMosaicWinFmt[0].pitch[VFDC_YUV_INT_ADDR_IDX] =
    pAppData->nWidth * 2;
  mosaicLayout.sMosaicWinFmt[0].dataFormat = VFDC_DF_YUV422I_YVYU;
  mosaicLayout.sMosaicWinFmt[0].bpp = VFDC_BPP_BITS16;
  mosaicLayout.sMosaicWinFmt[0].priority = 0;
  mosaicLayout.nDisChannelNum = 0;
  /* Only one window in this layout, hence setting it to 1 */
  mosaicLayout.nNumWindows = 1;

  if (1 == pAppData->displayId) {
    /* For secondary Display, start the window at (0,0), since it is 
       scaled to display device size */
    mosaicLayout.sMosaicWinFmt[0].winStartX = 0;
    mosaicLayout.sMosaicWinFmt[0].winStartY = 0;
    
    /*If LCD is chosen, fir the mosaic window to the size of the LCD display*/
    mosaicLayout.sMosaicWinFmt[0].winWidth = DISPLAY_WIDTH;
    mosaicLayout.sMosaicWinFmt[0].winHeight = DISPLAY_HEIGHT;
    mosaicLayout.sMosaicWinFmt[0].pitch[VFDC_YUV_INT_ADDR_IDX] = 
                                       DISPLAY_WIDTH * 2;  
  }

  eError = OMX_SetParameter (pAppData->pDisHandle, (OMX_INDEXTYPE)
                             OMX_TI_IndexParamVFDCCreateMosaicLayout,
                             &mosaicLayout);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set mosaic window parameter\n");
  }

  /* map OMX port to window */
  OMX_INIT_PARAM (&port2Winmap);
  /* signifies the layout id this port2win mapping refers to */
  port2Winmap.nLayoutId = 0;
  /* Just one window in this layout, hence setting the value to 1 */
  port2Winmap.numWindows = 1;
  /* Only 1st input port used here */
  port2Winmap.omxPortList[0] = OMX_VFDC_INPUT_PORT_START_INDEX + 0;
  eError = OMX_SetConfig (pAppData->pDisHandle,
                          (OMX_INDEXTYPE)
                            OMX_TI_IndexConfigVFDCMosaicPort2WinMap,
                          &port2Winmap);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to map port to windows\n");
  }

  /* Setting Memory type at input port to Raw Memory */
  printf ("setting input and output memory type to default");
  OMX_INIT_PARAM (&memTypeCfg);
  memTypeCfg.nPortIndex = OMX_VFPC_INPUT_PORT_START_INDEX;
  memTypeCfg.eBufMemoryType = OMX_BUFFER_MEMORY_DEFAULT;
  eError = OMX_SetParameter (pAppData->pDisHandle, OMX_TI_IndexParamBuffMemType,
                             &memTypeCfg);

  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to set memory Type at input port\n");
  }

}

/* Nothing beyond this point */

