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

/**
 *******************************************************************************
 *  @file  capture_encode_test.c
 *  @brief This file contains all Functions related to Test Application
 *
 *         This is the example IL Client support to create, configure & chaining
 *         of single channel omx-components using non tunneling 
 *         mode
 *
 *  @rev 1.0
 *******************************************************************************
 */

/*******************************************************************************
*                             Compilation Control Switches
*******************************************************************************/
/* None */

/*******************************************************************************
*                             INCLUDE FILES
*******************************************************************************/

/*--------------------- system and platform files ----------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include <xdc/std.h>

/*-------------------------program files -------------------------------------*/
#include "ti/omx/interfaces/openMaxv11/OMX_Core.h"
#include "ti/omx/interfaces/openMaxv11/OMX_Component.h"
#include "ilclient.h"
#include "ilclient_utils.h"
#include <omx_venc.h>
#include <omx_vfpc.h>
#include <omx_vfdc.h>
#include <omx_vfcc.h>
#include <omx_ctrl.h>
#include "../DeviceInterface/DeviceInterface.h"
#include "../DeviceInterface/internal.h"
#include "serv.h"

comm_res common_res;

/* ========================================================================== */
/**
* IL_ClientCbEventHandler() : This method is the event handler implementation to 
* handle events from the OMX Derived component
*
* @param hComponent        : Handle to the component
* @param ptrAppData        : 
* @param eEvent            :
* @param nData1            :
* @param nData2            :
* @param pEventData        :
*
*  @return      
*  OMX_ErrorNone = Successful 
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */
static OMX_ERRORTYPE IL_ClientCbEventHandler (OMX_HANDLETYPE hComponent,
                                              OMX_PTR ptrAppData,
                                              OMX_EVENTTYPE eEvent,
                                              OMX_U32 nData1, OMX_U32 nData2, 
                                              OMX_PTR pEventData)
{
  IL_CLIENT_COMP_PRIVATE *comp;

  comp = ptrAppData;

  printf ("got event");
  if (eEvent == OMX_EventCmdComplete)
  {
    if (nData1 == OMX_CommandStateSet)
    {
      printf ("State changed to: ");
      switch ((int) nData2)
      {
        case OMX_StateInvalid:
          printf ("OMX_StateInvalid \n");
          break;
        case OMX_StateLoaded:
          printf ("OMX_StateLoaded \n");
          break;
        case OMX_StateIdle:
          printf ("OMX_StateIdle \n");
          break;
        case OMX_StateExecuting:
          printf ("OMX_StateExecuting \n");
          break;
        case OMX_StatePause:
          printf ("OMX_StatePause\n");
          break;
        case OMX_StateWaitForResources:
          printf ("OMX_StateWaitForResources\n");
          break;
      }
      /* post an semaphore, so that in IL Client we can confirm the state
         change */
      semp_post (comp->done_sem);
    }
    else if (OMX_CommandPortEnable || OMX_CommandPortDisable)
    {
      printf ("Enable/Disable Event \n");
      semp_post (comp->port_sem);
    }
  }
  else if (eEvent == OMX_EventBufferFlag)
  {
    printf ("OMX_EventBufferFlag \n");
    if ((int) nData2 == OMX_BUFFERFLAG_EOS)
    {
      printf ("got EOS event \n");
      semp_post (comp->eos);
    }
  }
  else if (eEvent == OMX_EventError)
  {
    printf ("*** unrecoverable error: %s (0x%lx) \n",
            IL_ClientErrorToStr (nData1), nData1);
    printf ("Press a key to proceed\n");
  }
  else
  {
    printf ("unhandled event, param1 = %i, param2 = %i \n", (int) nData1,
            (int) nData2);
  }

  return OMX_ErrorNone;
}

/* ========================================================================== */
/**
* IL_ClientCbEmptyBufferDone() : This method is the callback implementation to 
* handle EBD events from the OMX Derived component
*
* @param hComponent        : Handle to the component
* @param ptrAppData        : app pointer, which was passed during the getHandle
* @param pBuffer           : buffer header, for the buffer which is consumed
*
*  @return      
*  OMX_ErrorNone = Successful 
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientCbEmptyBufferDone (OMX_HANDLETYPE hComponent,
                                          OMX_PTR ptrAppData, 
                                          OMX_BUFFERHEADERTYPE *pBuffer)
{

  IL_CLIENT_COMP_PRIVATE *thisComp = (IL_CLIENT_COMP_PRIVATE *) ptrAppData;
  IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
  IL_CLIENT_PIPE_MSG localPipeMsg;

  OMX_ERRORTYPE eError = OMX_ErrorNone;
  int retVal = 0;

  inPortParamsPtr = thisComp->inPortParams + pBuffer->nInputPortIndex;

  /* if the buffer is from file i/o, write the free buffer header into ipbuf
     pipe, else keep it in its local pipe. From local pipe It would be given
     to remote component as "consumed buffer " */
  if (inPortParamsPtr->connInfo.remotePipe[0] == NULL)
  {
    /* write the empty buffer pointer to input pipe */
    retVal = write (inPortParamsPtr->ipBufPipe[1], &pBuffer, sizeof (pBuffer));

    if (sizeof (pBuffer) != retVal)
    {
      printf ("Error writing to Input buffer i/p Pipe!\n");
      eError = OMX_ErrorNotReady;
      return eError;
    }
  }
  else
  {
    /* Create a message that EBD is done and this buffer is ready to be
       recycled. This message will be read in buffer processing thread and and 
       remote component will be indicated about its status */
    localPipeMsg.cmd = IL_CLIENT_PIPE_CMD_EBD;
    localPipeMsg.pbufHeader = pBuffer;
    retVal = write (thisComp->localPipe[1],
                    &localPipeMsg, sizeof (IL_CLIENT_PIPE_MSG));
    if (sizeof (IL_CLIENT_PIPE_MSG) != retVal)
    {
      printf ("Error writing to local Pipe!\n");
      eError = OMX_ErrorNotReady;
      return eError;
    }

  }

  return eError;
}

/* ========================================================================== */
/**
* IL_ClientCbFillBufferDone() : This method is the callback implementation to 
* handle FBD events from the OMX Derived component
*
* @param hComponent        : Handle to the component
* @param ptrAppData        : app pointer, which was passed during the getHandle
* @param pBuffer           : buffer header, for the buffer which is produced
*
*  @return      
*  OMX_ErrorNone = Successful 
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientCbFillBufferDone (OMX_HANDLETYPE hComponent,
                                         OMX_PTR ptrAppData, 
                                         OMX_BUFFERHEADERTYPE *pBuffer)
{

  IL_CLIENT_COMP_PRIVATE *thisComp = (IL_CLIENT_COMP_PRIVATE *) ptrAppData;
  IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
  IL_CLIENT_PIPE_MSG localPipeMsg;

  OMX_ERRORTYPE eError = OMX_ErrorNone;
  int retVal = 0;

  /* get the pipe corrsponding to this port, portIndex is part of bufferheader
     structure */
  outPortParamsPtr =
    thisComp->outPortParams + (pBuffer->nOutputPortIndex -
                               thisComp->startOutportIndex);

  /* if the buffer is from file i/o, write the free buffer header into outbuf
     pipe, else keep it in its local pipe. From local pipe It would be given
     to remote component as "filled buffer " */

  if (outPortParamsPtr->connInfo.remotePipe[0] == NULL)
  {
    /* write the empty buffer pointer to input pipe */
    retVal = write (outPortParamsPtr->opBufPipe[1], &pBuffer, sizeof (pBuffer));

    if (sizeof (pBuffer) != retVal)
    {
      printf ("Error writing to Input buffer i/p Pipe!\n");
      eError = OMX_ErrorNotReady;
      return eError;
    }
  }
  else
  {
    /* Create a message that FBD is done and this buffer is ready to be used
       by other compoenent. This message will be read in buffer processing
       thread and and remote component will be indicated about its status */
    localPipeMsg.cmd = IL_CLIENT_PIPE_CMD_FBD;
    localPipeMsg.pbufHeader = pBuffer;
    retVal = write (thisComp->localPipe[1],
                    &localPipeMsg, sizeof (IL_CLIENT_PIPE_MSG));
    if (sizeof (IL_CLIENT_PIPE_MSG) != retVal)
    {
      printf ("Error writing to local Pipe!\n");
      eError = OMX_ErrorNotReady;
      return eError;
    }

  }
  return eError;
}

/* ========================================================================== */
/**
* IL_ClientOutputBitStreamWriteTask() : This task function is file writetask for
* encoder component. 
*
* @param threadsArg        : Handle to the application
*
*/
/* ========================================================================== */

void IL_ClientOutputBitStreamWriteTask (void *threadsArg)
{
  unsigned int dataRead = 0;
  OMX_ERRORTYPE err = OMX_ErrorNone;
  IL_CLIENT_COMP_PRIVATE *encILComp = NULL;
  OMX_BUFFERHEADERTYPE *pBufferOut = NULL;
  static unsigned int frameCounter = 0;
	pipe_cmd p_cmd;
  encILComp = ((IL_Client *) threadsArg)->encILComp;

  /* use the initial i/p buffers and make empty this buffer calls */
  err = IL_ClientEncUseInitialOutputResources (encILComp);

  while (1)
  {
    /* Read filled buffer pointer from the pipe */
    read (encILComp->outPortParams->opBufPipe[0],
          &pBufferOut, sizeof (pBufferOut));

    /* write data to output file */
    /* fwrite (pBufferOut->pBuffer,
            sizeof (char),
            pBufferOut->nFilledLen, ((IL_Client *) threadsArg)->fOut);
		*/


//*******************************************************************
 		p_cmd.bf_offset=1024*1024*(frameCounter%2);
		p_cmd.bf_size=pBufferOut->nFilledLen;
		printf("IL Client write thread %d\n", frameCounter);	
		if(write_comm_data((char*)pBufferOut->pBuffer, &p_cmd, &common_res ) < 0){

			fprintf(stderr, "IL Capture thread error\n");
			pthread_exit(NULL);

		}


//*******************************************************************
    frameCounter++;
    if(frameCounter == encILComp->numFrames)
    {
      frameCounter = 0;
      semp_post(encILComp->eos);
      pthread_exit(encILComp);
    }

    /* Pass the input buffer to the component */
    err = OMX_FillThisBuffer (encILComp->handle, pBufferOut);

    if (OMX_ErrorNone != err)
    {
      /* put back the frame in pipe and wait for state change */
      write (encILComp->outPortParams->opBufPipe[1],
             &pBufferOut, sizeof (pBufferOut));
      printf (" waiting for action from IL Client \n");

      /* since in this example we are changing states in other thread it will
         return error for giving ETB/FTB calls in non-execute state. Since
         example is shutting down, we exit the thread */

      pthread_exit (encILComp);

    }
  }

}

/* ========================================================================== */
/**
* IL_ClientConnInConnOutTask() : This task function is for passing buffers from
* one component to other connected component. This functions reads from local
* pipe of a perticular component , and takes action based on the message in the
* pipe. This pipe is written by callback ( EBD/FBD) function from component and
* from other component threads, which writes into this pipe for buffer 
* communication.
*
* @param threadsArg        : Handle to a particular component
*
*/
/* ========================================================================== */

void IL_ClientConnInConnOutTask (void *threadsArg)
{
  IL_CLIENT_PIPE_MSG pipeMsg;
  IL_CLIENT_COMP_PRIVATE *thisComp = (IL_CLIENT_COMP_PRIVATE *) threadsArg;
  OMX_ERRORTYPE err = OMX_ErrorNone;

  /* Initially pipes will not have any buffers, so components needs to be given 
     empty buffers for output ports. Input bufefrs are given by other
     component, or file read task */
  IL_ClientUseInitialOutputResources (thisComp);

  for (;;)
  {
    /* Read from its own local Pipe */
    read (thisComp->localPipe[0], &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

    /* check the function type */

    switch (pipeMsg.cmd)
    {

      case IL_CLIENT_PIPE_CMD_EXIT:
        printf ("exiting thread \n ");
        pthread_exit (thisComp);
        break;
      case IL_CLIENT_PIPE_CMD_ETB:
        err = IL_ClientProcessPipeCmdETB (thisComp, &pipeMsg);
        /* If not in proper state, bufers may not be accepted by component */
        if (OMX_ErrorNone != err)
        {
          write (thisComp->localPipe[1], &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));
          printf (" ETB: wait \n");
          /* since in this example we are changing states in other thread it
             will return error for giving ETB/FTB calls in non-execute state.
             Since example is shutting down, we exit the thread */
          pthread_exit (thisComp);
          /* if error is incorrect state operation, wait for state to change */
          /* waiting mechanism should be implemented here */
        }

        break;
      case IL_CLIENT_PIPE_CMD_FTB:
        err = IL_ClientProcessPipeCmdFTB (thisComp, &pipeMsg);

        if (OMX_ErrorNone != err)
        {
          write (thisComp->localPipe[1], &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));
          printf (" FTB: wait \n");
          /* if error is incorrect state operation, wait for state to change */
          /* waiting mechanism should be implemented here */
          /* since in this example we are changing states in other thread it
             will return error for giving ETB/FTB calls in non-execute state.
             Since example is shutting down, we exit the thread */
          pthread_exit (thisComp);

        }
        break;

      case IL_CLIENT_PIPE_CMD_EBD:
        IL_ClientProcessPipeCmdEBD (thisComp, &pipeMsg);

        break;
      case IL_CLIENT_PIPE_CMD_FBD:
        IL_ClientProcessPipeCmdFBD (thisComp, &pipeMsg);
        break;
      default:
        break;
    }
  }
}

/* ========================================================================== */
/**
* IL_ClientProcessPipeCmdETB() : This function passes the bufefrs to component
* for consuming. This buffer will come from other component as an output. To 
* consume it, IL client finds its bufefr header (for consumer component), and 
* calls ETB call.
* @param thisComp        : Handle to a particular component
* @param pipeMsg         : message structure, which is written in response to 
*                          callbacks
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientProcessPipeCmdETB (IL_CLIENT_COMP_PRIVATE *thisComp,
                                          IL_CLIENT_PIPE_MSG *pipeMsg)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;
  OMX_BUFFERHEADERTYPE *pBufferIn;

  /* search its own buffer header based on submitted by connected comp */
  IL_ClientUtilGetSelfBufHeader (thisComp, pipeMsg->bufHeader.pBuffer,
                                 ILCLIENT_INPUT_PORT,
                                 pipeMsg->bufHeader.nInputPortIndex,
                                 &pBufferIn);

  /* populate buffer header */
  pBufferIn->nFilledLen = pipeMsg->bufHeader.nFilledLen;
  pBufferIn->nOffset = pipeMsg->bufHeader.nOffset;
  pBufferIn->nTimeStamp = pipeMsg->bufHeader.nTimeStamp;
  pBufferIn->nFlags = pipeMsg->bufHeader.nFlags;
  pBufferIn->hMarkTargetComponent = pipeMsg->bufHeader.hMarkTargetComponent;
  pBufferIn->pMarkData = pipeMsg->bufHeader.pMarkData;
  pBufferIn->nTickCount = 0;

  /* call etb to the component */
  err = OMX_EmptyThisBuffer (thisComp->handle, pBufferIn);
  return (err);
}

/* ========================================================================== */
/**
* IL_ClientProcessPipeCmdFTB() : This function passes the bufefrs to component
* for consuming. This buffer will come from other component as consumed at input
* To  consume it, IL client finds its bufefr header (for consumer component),
* and calls FTB call.
* @param thisComp        : Handle to a particular component
* @param pipeMsg         : message structure, which is written in response to 
*                          callbacks
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientProcessPipeCmdFTB (IL_CLIENT_COMP_PRIVATE *thisComp,
                                          IL_CLIENT_PIPE_MSG *pipeMsg)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;
  OMX_BUFFERHEADERTYPE *pBufferOut;

  /* search its own buffer header based on submitted by connected comp */
  IL_ClientUtilGetSelfBufHeader (thisComp, pipeMsg->bufHeader.pBuffer,
                                 ILCLIENT_OUTPUT_PORT,
                                 pipeMsg->bufHeader.nOutputPortIndex,
                                 &pBufferOut);

  /* call etb to the component */
  err = OMX_FillThisBuffer (thisComp->handle, pBufferOut);

  return (err);
}

/* ========================================================================== */
/**
* IL_ClientProcessPipeCmdEBD() : This function passes the bufefrs to component
* for consuming. This empty buffer will go to other component to be reused at 
* output port.
* @param thisComp        : Handle to a particular component
* @param pipeMsg         : message structure, which is written in response to 
*                          callbacks
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientProcessPipeCmdEBD (IL_CLIENT_COMP_PRIVATE *thisComp,
                                          IL_CLIENT_PIPE_MSG *pipeMsg)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;
  OMX_BUFFERHEADERTYPE *pBufferIn;
  IL_CLIENT_PIPE_MSG remotePipeMsg;
  IL_CLIENT_INPORT_PARAMS *inPortParamsPtr;
  int retVal = 0;

  pBufferIn = pipeMsg->pbufHeader;

  /* find the input port structure (pipe) */
  inPortParamsPtr = thisComp->inPortParams + pBufferIn->nInputPortIndex;

  remotePipeMsg.cmd = IL_CLIENT_PIPE_CMD_FTB;
  remotePipeMsg.bufHeader.pBuffer = pBufferIn->pBuffer;
  remotePipeMsg.bufHeader.nOutputPortIndex =
    inPortParamsPtr->connInfo.remotePort;

  /* write the fill buffer message to remote pipe */
  retVal = write (inPortParamsPtr->connInfo.remotePipe[1],
                  &remotePipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

  if (sizeof (IL_CLIENT_PIPE_MSG) != retVal)
  {
    printf ("Error writing to remote Pipe!\n");
    err = OMX_ErrorNotReady;
    return err;
  }

  return (err);
}

/* ========================================================================== */
/**
* IL_ClientProcessPipeCmdFBD() : This function passes the bufefrs to component
* for consuming. This buffer will go to other component to be consumed at input
* port.
* @param thisComp        : Handle to a particular component
* @param pipeMsg         : message structure, which is written in response to 
*                          callbacks
*
*/
/* ========================================================================== */

OMX_ERRORTYPE IL_ClientProcessPipeCmdFBD (IL_CLIENT_COMP_PRIVATE *thisComp,
                                          IL_CLIENT_PIPE_MSG *pipeMsg)
{
  OMX_ERRORTYPE err = OMX_ErrorNone;
  OMX_BUFFERHEADERTYPE *pBufferOut;
  IL_CLIENT_PIPE_MSG remotePipeMsg;
  IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr;
  int retVal = 0;
  pBufferOut = pipeMsg->pbufHeader;

  remotePipeMsg.cmd = IL_CLIENT_PIPE_CMD_ETB;
  remotePipeMsg.bufHeader.pBuffer = pBufferOut->pBuffer;

  outPortParamsPtr =
    thisComp->outPortParams + (pBufferOut->nOutputPortIndex -
                               thisComp->startOutportIndex);

  /* populate buffer header */
  remotePipeMsg.bufHeader.nFilledLen = pBufferOut->nFilledLen;
  remotePipeMsg.bufHeader.nOffset = pBufferOut->nOffset;
  remotePipeMsg.bufHeader.nTimeStamp = pBufferOut->nTimeStamp;
  remotePipeMsg.bufHeader.nFlags = pBufferOut->nFlags;
  remotePipeMsg.bufHeader.hMarkTargetComponent =
    pBufferOut->hMarkTargetComponent;
  remotePipeMsg.bufHeader.pMarkData = pBufferOut->pMarkData;
  remotePipeMsg.bufHeader.nTickCount = pBufferOut->nTickCount;
  remotePipeMsg.bufHeader.nInputPortIndex =
    outPortParamsPtr->connInfo.remotePort;

  /* write the fill buffer message to remote pipe */
  retVal = write (outPortParamsPtr->connInfo.remotePipe[1],
                  &remotePipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

  if (sizeof (IL_CLIENT_PIPE_MSG) != retVal)
  {
    printf ("Error writing to remote Pipe!\n");
    err = OMX_ErrorNotReady;
    return err;
  }

  return (err);
}

/* ========================================================================== */
/**
* Capture_Encode_Example() : This method is the IL Client implementation for 
* connecting capture, dei and display, and Encode OMX components. This function
*  creates configures, and connects the components.
* it manages the buffer communication.
*
* @param args         : parameters( widt,height,frame rate etc) for this function
*
*  @return      
*  OMX_ErrorNone = Successful 
*
*  Other_value = Failed (Error code is returned)
*
*/
/* ========================================================================== */

/* Main IL Client application to create , intiate and connect components */
int Capture_Encode_Example (IL_ARGS *args)
{
  IL_Client *pAppData = NULL;
  OMX_ERRORTYPE eError = OMX_ErrorNone;
  OMX_U32 i, j;
  OMX_S32 ret_value;
  IL_CLIENT_PIPE_MSG pipeMsg;
  IL_CLIENT_OUTPORT_PARAMS *outPortParamsPtr = NULL;

  /* Initialize application specific data structures and buffer management
     data */
  IL_ClientInit (&pAppData, args->mode, args->frame_rate,
                 args->bit_rate, args->num_frames, args->display_id);

//  printf (" openeing file \n");

  /* Open the file of data to be rendered.  */
// pAppData->fOut = fopen (args->output_file, "wb");

/*   if (pAppData->fOut == NULL)
  {
    printf ("Error: failed to open the file %s for writing \n",
            args->output_file);
    goto EXIT;
  }
*/
//*****************************************************************	
	if(init_comm_resource(&common_res) < 0){

		printf("IL Client: resouce initialization failed\n");
		goto EXIT;
	}


//****************************************************************




  /* Initialize application / IL Client callback functions */
  /* Callbacks are passed during getHandle call to component, component uses
     these callabacks to communicate with IL Client */
  /* event handler is to handle the state changes , omx commands and any
     message for IL client */
  pAppData->pCb.EventHandler = IL_ClientCbEventHandler;

  /* Empty buffer done is data callback at the input port, where component let
     the application know that buffer has been consumed, this is not applicabel 
     if there is no input port in the component */
  pAppData->pCb.EmptyBufferDone = IL_ClientCbEmptyBufferDone;

  /* fill buffer done is callback at the output port, where component lets the
     application know that an output buffer is available with the processed data 
   */
  pAppData->pCb.FillBufferDone = IL_ClientCbFillBufferDone;

/******************************************************************************/
  /* Create the capture Component, component handle would be returned component 
     name is unique and fixed for a componnet, callback are passed to
     component in this function. component would be in loaded state post this
     call */

  eError =
    OMX_GetHandle (&pAppData->pCapHandle,
                   (OMX_STRING) "OMX.TI.VPSSM3.VFCC", pAppData->capILComp,
                   &pAppData->pCb);

  printf (" capture compoenent is created \n");

  if ((eError != OMX_ErrorNone) || (pAppData->pCapHandle == NULL))
  {
    printf ("Error in Get Handle function : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  pAppData->capILComp->handle = pAppData->pCapHandle;

  /* This is control component, without ports. It is implemented as OMX
     component */
  eError =
    OMX_GetHandle (&pAppData->pTvpHandle,
                   (OMX_STRING) "OMX.TI.VPSSM3.CTRL.TVP",
                   pAppData->capILComp, &pAppData->pCb);

  printf (" control TVP compoenent is created \n");

  if ((eError != OMX_ErrorNone) || (pAppData->pTvpHandle == NULL))
  {
    printf ("Error in Get Handle function : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  /* Configute the capture componet */
  /* calling OMX_Setparam in this function */
  IL_ClientSetCaptureParams (pAppData);

  printf ("enable capture output port \n");

  OMX_SendCommand (pAppData->pCapHandle, OMX_CommandPortEnable,
                   OMX_VFCC_OUTPUT_PORT_START_INDEX, NULL);

  semp_pend (pAppData->capILComp->port_sem);

/******************************************************************************/
  /* Create DEI component, it creatd OMX compponent for dei writeback, Int
     this client we are passing the same callbacks to all the components */

  eError =
    OMX_GetHandle (&pAppData->pDeiHandle,
                   (OMX_STRING) "OMX.TI.VPSSM3.VFPC.DEIMDUALOUT",
                   pAppData->deiILComp, &pAppData->pCb);

  if ((eError != OMX_ErrorNone) || (pAppData->pDeiHandle == NULL))
  {
    printf ("Error in Get Handle function : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  pAppData->deiILComp->handle = pAppData->pDeiHandle;

  printf (" dei compoenent is created \n");

  /* omx calls are made in this function for setting the parameters for DEI
     component, For clarity purpose it is written as separate function */

  IL_ClientSetDeiParams (pAppData);

  /* enable input and output port */
  /* as per openmax specs all the ports should be enabled by default but EZSDK
     OMX component does not enable it hence we manually need to enable it. */

  printf ("enable dei input port \n");
  OMX_SendCommand (pAppData->pDeiHandle, OMX_CommandPortEnable,
                   OMX_VFPC_INPUT_PORT_START_INDEX, NULL);

  /* wait for both ports to get enabled, event handler would be notified from
     the component after enabling the port, which inturn would post this
     semaphore */
  semp_pend (pAppData->deiILComp->port_sem);

  printf ("enable dei output port 0 \n");
  OMX_SendCommand (pAppData->pDeiHandle, OMX_CommandPortEnable,
                   OMX_VFPC_OUTPUT_PORT_START_INDEX, NULL);
  semp_pend (pAppData->deiILComp->port_sem);

  printf ("enable dei output port 1 \n");
  OMX_SendCommand (pAppData->pDeiHandle, OMX_CommandPortEnable,
                   OMX_VFPC_OUTPUT_PORT_START_INDEX + 1, NULL);
  semp_pend (pAppData->deiILComp->port_sem);

/******************************************************************************/
  /* Create the H264 encoder Component, component handle would be returned
     component name is unique and fixed for a componnet, callback are passed
     to componnet in this function. component would be loaded state post this
     call */

  eError =
    OMX_GetHandle (&pAppData->pEncHandle,
                   (OMX_STRING) "OMX.TI.DUCATI.VIDENC", pAppData->encILComp,
                   &pAppData->pCb);

  printf (" encoder compoenent is created \n");

  if ((eError != OMX_ErrorNone) || (pAppData->pEncHandle == NULL))
  {
    printf ("Error in Get Handle function : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  pAppData->encILComp->handle = pAppData->pEncHandle;

  /* Configute the encode componet, ports are default enabled for encode comp,
     so no need to enable from IL Client */
  /* calling OMX_Setparam in this function */
  IL_ClientSetEncodeParams (pAppData);

/******************************************************************************/

/******************************************************************************/
/* Create and Configure the display component. It will use VFDC component on  */
/* media controller.                                                          */
/******************************************************************************/

  /* Create the display component */
  /* getting display component handle */
  eError =
    OMX_GetHandle (&pAppData->pDisHandle, "OMX.TI.VPSSM3.VFDC",
                   pAppData->disILComp, &pAppData->pCb);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to get handle\n");
  }

  printf ("found handle %p for component %s \n", pAppData->pDisHandle,
          "OMX.TI.VPSSM3.VFDC");

  pAppData->disILComp->handle = pAppData->pDisHandle;

  printf (" got display handle \n");
  /* getting display controller component handle, Display contrller is
     implemented as an OMX component, however it does not has any input or
     output ports. It is used only for controling display hw */
  eError =
    OMX_GetHandle (&pAppData->pctrlHandle, "OMX.TI.VPSSM3.CTRL.DC",
                   pAppData->disILComp, &pAppData->pCb);
  if (eError != OMX_ErrorNone)
  {
    ERROR ("failed to get handle\n");
  }

  printf ("found handle %p for component %s\n", pAppData->pctrlHandle,
          "OMX.TI.VPSSM3.CTRL.DC");

  /* omx calls are made in this function for setting the parameters for display 
     component, For clarity purpose it is written as separate function */

  IL_ClientSetDisplayParams (pAppData);

  /* as per openmax specs all the ports should be enabled by default but EZSDK 
     OMX component does not enable it hence we manually need to enable it */
  printf ("enable input port \n");

  OMX_SendCommand (pAppData->pDisHandle, OMX_CommandPortEnable,
                   OMX_VFDC_INPUT_PORT_START_INDEX, NULL);

  /* wait for port to get enabled, event handler would be notified from the
     component after enabling the port, which inturn would post this 
     semaphore */

  semp_pend (pAppData->disILComp->port_sem);

/******************************************************************************/

  /* Connect the capture to dei, This application uses 'pipe' to pass the
     buffers between different components. each compponent has a local pipe,
     which It reads for taking buffers. By connecting this functions informs
     about local pipe to other component, so that other component can pass
     buffers to this 'remote' pipe */

  printf (" connect call for capture-Dei \n ");

  IL_ClientConnectComponents (pAppData->capILComp,
                              OMX_VFCC_OUTPUT_PORT_START_INDEX,
                              pAppData->deiILComp,
                              OMX_VFPC_INPUT_PORT_START_INDEX);

  printf (" connect call for dei- encoder \n ");

  IL_ClientConnectComponents (pAppData->deiILComp,
                              OMX_VFPC_OUTPUT_PORT_START_INDEX + 1,
                              pAppData->encILComp, OMX_VIDENC_INPUT_PORT);

  printf (" connect call for dei-display \n ");

  /* Connect the dei to display, even ports are concted to encoder, while odd
     ports would be connected to display */
  IL_ClientConnectComponents (pAppData->deiILComp,
                              OMX_VFPC_OUTPUT_PORT_START_INDEX,
                              pAppData->disILComp,
                              OMX_VFDC_INPUT_PORT_START_INDEX);

/******************************************************************************/

  /* OMX_SendCommand expecting OMX_StateIdle, after this command component will 
     wait for all buffers to be allocated as per omx buffers are created during 
     loaded to Idle transition IF ports are enabled ) */

  eError =
    OMX_SendCommand (pAppData->pTvpHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->capILComp->done_sem);

  eError =
    OMX_SendCommand (pAppData->pCapHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  /* Allocate I/O Buffers; componnet would allocated buffers and would return
     the buffer header containing the pointer to buffer */
  for (i = 0; i < pAppData->capILComp->outPortParams->nBufferCountActual; i++)
  {
    eError = OMX_AllocateBuffer (pAppData->pCapHandle,
                                 &pAppData->capILComp->outPortParams->
                                 pOutBuff[i],
                                 OMX_VFCC_OUTPUT_PORT_START_INDEX, pAppData,
                                 pAppData->capILComp->outPortParams->
                                 nBufferSize);

    if (eError != OMX_ErrorNone)
    {
      printf
        ("Capture: Error in OMX_AllocateBuffer() : %s \n",
         IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }
  printf (" Capture outport buffers allocated \n ");

  semp_pend (pAppData->capILComp->done_sem);

  printf (" Capture is in IDLE state \n");
/******************************************************************************/

  eError =
    OMX_SendCommand (pAppData->pDeiHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  /* Since DEI is connected to capture output, buffers would be used from
     capture output port */
  for (i = 0; i < pAppData->deiILComp->inPortParams->nBufferCountActual; i++)
  {

    eError = OMX_UseBuffer (pAppData->pDeiHandle,
                            &pAppData->deiILComp->inPortParams->pInBuff[i],
                            OMX_VFPC_INPUT_PORT_START_INDEX,
                            pAppData->deiILComp,
                            pAppData->capILComp->outPortParams->nBufferSize,
                            pAppData->capILComp->outPortParams->
                            pOutBuff[i]->pBuffer);

    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_UseBuffer()-input Port State set : %s \n",
              IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }
  printf (" Dei input port use buffer done \n ");

  /* DEI is dual o/p port OMX component; allocate buffers on both ports */
  for (j = 0; j < pAppData->deiILComp->numOutport; j++)
  {
    outPortParamsPtr = pAppData->deiILComp->outPortParams + j;
    /* buffer alloaction for output port */
    for (i = 0; i < outPortParamsPtr->nBufferCountActual; i++)
    {
      eError = OMX_AllocateBuffer (pAppData->pDeiHandle,
                                   &outPortParamsPtr->pOutBuff[i],
                                   OMX_VFPC_OUTPUT_PORT_START_INDEX + j,
                                   pAppData, outPortParamsPtr->nBufferSize);
      if (eError != OMX_ErrorNone)
      {
        printf
          ("Error in OMX_AllocateBuffer()-Output Port State set : %s \n",
           IL_ClientErrorToStr (eError));
        goto EXIT;
      }
    }
  }
  printf (" DEI outport buffers allocated \n ");

  /* Wait for initialization to complete.. Wait for Idle stete of component
     after all buffers are alloacted componet would chnage to idle */

  semp_pend (pAppData->deiILComp->done_sem);

  printf (" DEI is in IDLE state \n");

/*******************************************************************************/

  /* OMX_SendCommand expecting OMX_StateIdle, after this command component
     would create codec, and will wait for all buffers to be allocated */
  eError =
    OMX_SendCommand (pAppData->pEncHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  /* since encoder is connected to dei, buffers are supplied by dei to
     encoder, so encoder does not allocate the buffers. However it is informed
     to use the buffers created by dei. encode component would create only
     buffer headers corresponding to these buffers */

  for (i = 0; i < pAppData->encILComp->inPortParams->nBufferCountActual; i++)
  {

    outPortParamsPtr = pAppData->deiILComp->outPortParams + 1;

    eError = OMX_UseBuffer (pAppData->pEncHandle,
                            &pAppData->encILComp->inPortParams->pInBuff[i],
                            OMX_VIDENC_INPUT_PORT,
                            pAppData->encILComp,
                            outPortParamsPtr->nBufferSize,
                            outPortParamsPtr->pOutBuff[i]->pBuffer);

    if (eError != OMX_ErrorNone)
    {
      printf ("Error in encode OMX_UseBuffer(): %s \n",
              IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }
  printf (" encoder input port use buffer done \n ");

  /* in SDK conventionally output port allocates the buffers, encode would
     create the buffers which would be consumed by filewrite thread */
  /* buffer alloaction for output port */
  for (i = 0; i < pAppData->encILComp->outPortParams->nBufferCountActual; i++)
  {
    eError = OMX_AllocateBuffer (pAppData->pEncHandle,
                                 &pAppData->encILComp->outPortParams->
                                 pOutBuff[i], OMX_VIDENC_OUTPUT_PORT,
                                 pAppData,
                                 pAppData->encILComp->outPortParams->
                                 nBufferSize);
    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_AllocateBuffer()-Output Port State set : %s \n",
              IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }

  printf (" encoder outport buffers allocated \n ");

  semp_pend (pAppData->encILComp->done_sem);

  printf (" Encoder state IDLE \n ");
/******************************************************************************/

  /* control component does not allocate any data buffers, It's interface is
     though as it is omx componenet */
  eError =
    OMX_SendCommand (pAppData->pctrlHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->disILComp->done_sem);

  printf (" ctrl-dc state IDLE \n ");

  eError =
    OMX_SendCommand (pAppData->pDisHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  /* Since display has only input port and buffers are already created by DEI
     component, only use_buffer call is used at input port. there is no output
     port in the display component */
  for (i = 0; i < pAppData->disILComp->inPortParams->nBufferCountActual; i++)
  {

    outPortParamsPtr = pAppData->deiILComp->outPortParams;

    eError = OMX_UseBuffer (pAppData->pDisHandle,
                            &pAppData->disILComp->inPortParams->pInBuff[i],
                            OMX_VFDC_INPUT_PORT_START_INDEX,
                            pAppData->disILComp,
                            outPortParamsPtr->nBufferSize,
                            outPortParamsPtr->pOutBuff[i]->pBuffer);

    if (eError != OMX_ErrorNone)
    {
      printf ("Error in Display OMX_UseBuffer()- %s \n",
              IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }
  printf (" display buffers allocated \n waiting for IDLE");
  semp_pend (pAppData->disILComp->done_sem);

  printf (" display state IDLE \n ");
/******************************************************************************/

  /* change state tho execute, so that component can accept buffers from IL
     client. Please note the ordering of components is from consumer to
     producer component i.e. capture-dei-encoder/display */
  eError =
    OMX_SendCommand (pAppData->pctrlHandle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error in SendCommand()-OMX_StateIdle State set : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->disILComp->done_sem);

  printf (" display control state execute \n ");

  /* change state to execute so that buffers processing can start */
  eError =
    OMX_SendCommand (pAppData->pDisHandle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Executing State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->disILComp->done_sem);

  printf (" display state execute \n ");
/******************************************************************************/

  /* change state to execute so that buffers processing can start */
  eError =
    OMX_SendCommand (pAppData->pEncHandle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Executing State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->encILComp->done_sem);

  printf (" encoder state execute \n ");
/******************************************************************************/

  /* change state to execute so that buffers processing can start */
  eError =
    OMX_SendCommand (pAppData->pDeiHandle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Executing State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->deiILComp->done_sem);

  printf (" dei state execute \n ");
/******************************************************************************/

  /* change state to execute so that buffers processing can start */
  eError =
    OMX_SendCommand (pAppData->pCapHandle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Executing State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->capILComp->done_sem);

  printf (" capture state execute \n ");

  eError =
    OMX_SendCommand (pAppData->pTvpHandle, OMX_CommandStateSet,
                     OMX_StateExecuting, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Executing State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->capILComp->done_sem);

  printf (" capture control ( TVP ) state execute \n ");

/******************************************************************************/

  /* Create thread for writing bitstream and passing the buffers to encoder
     component */
  pthread_attr_init (&pAppData->encILComp->ThreadAttr);

  if (0 !=
      pthread_create (&pAppData->encILComp->outDataStrmThrdId,
                      &pAppData->encILComp->ThreadAttr,
                      IL_ClientOutputBitStreamWriteTask, pAppData))
  {
    printf ("Create_Task failed !");
    goto EXIT;
  }

  printf (" file write thread created \n ");

  pthread_attr_init (&pAppData->encILComp->ThreadAttr);

  /* These threads are created for each component to pass the buffers to each
     other. this thread function reads the buffers from pipe and feeds it to
     component or for processed buffers, passes the buffers to connected
     component */
  if (0 !=
      pthread_create (&pAppData->encILComp->connDataStrmThrdId,
                      &pAppData->encILComp->ThreadAttr,
                      IL_ClientConnInConnOutTask, pAppData->encILComp))
  {
    printf ("Create_Task failed !");
    goto EXIT;
  }

  printf (" encode connect thread created \n ");

  pthread_attr_init (&pAppData->deiILComp->ThreadAttr);

  if (0 !=
      pthread_create (&pAppData->deiILComp->connDataStrmThrdId,
                      &pAppData->deiILComp->ThreadAttr,
                      IL_ClientConnInConnOutTask, pAppData->deiILComp))
  {
    printf ("Create_Task failed !");
    goto EXIT;
  }

  printf (" dei connect thread created \n ");

  pthread_attr_init (&pAppData->capILComp->ThreadAttr);

  if (0 !=
      pthread_create (&pAppData->capILComp->connDataStrmThrdId,
                      &pAppData->capILComp->ThreadAttr,
                      IL_ClientConnInConnOutTask, pAppData->capILComp))
  {
    printf ("Create_Task failed !");
    goto EXIT;
  }
  printf (" capture connect thread created \n ");

  if (0 !=
      pthread_create (&pAppData->disILComp->connDataStrmThrdId,
                      &pAppData->disILComp->ThreadAttr,
                      IL_ClientConnInConnOutTask, pAppData->disILComp))
  {
    printf ("Create_Task failed !");
    goto EXIT;
  }
  printf (" display connect thread created \n ");

  printf (" executing the application now!! \n");

/******************************************************************************/
  /* Waiting for this semaphore to be posted by the bitstream write thread */
  semp_pend(pAppData->encILComp->eos);
/******************************************************************************/
  printf(" tearing down the capture-encode example\n ");

  /* tear down sequence */

  /* change the state to idle */
  /* before changing state to idle, buffer communication to component should be 
     stoped , writing an exit message to threads */

  pipeMsg.cmd = IL_CLIENT_PIPE_CMD_EXIT;

  write (pAppData->deiILComp->localPipe[1],
         &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

  write (pAppData->capILComp->localPipe[1],
         &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

  write (pAppData->disILComp->localPipe[1],
         &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

  write (pAppData->encILComp->localPipe[1],
         &pipeMsg, sizeof (IL_CLIENT_PIPE_MSG));

  /* change state to idle so that buffers processing would stop */
  eError =
    OMX_SendCommand (pAppData->pCapHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->capILComp->done_sem);
  printf (" capture state idle \n ");

  /* change state to idle so that buffers processing would stop */
  eError =
    OMX_SendCommand(pAppData->pTvpHandle, OMX_CommandStateSet,
                    OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf("Error from SendCommand-Idle State set :%s \n",
           IL_ClientErrorToStr(eError));
    goto EXIT;
  }

  semp_pend(pAppData->capILComp->done_sem);
  printf(" control tvp state idle \n ");


  /* change state to idle so that buffers processing can stop */
  eError =
    OMX_SendCommand (pAppData->pDeiHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->deiILComp->done_sem);

  printf (" DEI state idle \n ");

  /* change state to execute so that buffers processing can stop */
  eError =
    OMX_SendCommand (pAppData->pDisHandle, OMX_CommandStateSet,
                     OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->disILComp->done_sem);

  printf (" display state idle \n ");

  /* change state to execute so that buffers processing can stop */
  eError =
    OMX_SendCommand(pAppData->pctrlHandle, OMX_CommandStateSet,
                    OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf("Error from SendCommand-Idle State set :%s \n",
           IL_ClientErrorToStr(eError));
    goto EXIT;
  }

  semp_pend(pAppData->disILComp->done_sem);

  printf(" display control state idle \n ");

  /* change state to execute so that buffers processing can stop */
  eError =
  OMX_SendCommand (pAppData->pEncHandle, OMX_CommandStateSet,
                   OMX_StateIdle, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  semp_pend (pAppData->encILComp->done_sem);

  printf (" Encoder state idle \n ");

/******************************************************************************/

  eError =
    OMX_SendCommand (pAppData->pDisHandle, OMX_CommandStateSet,
                     OMX_StateLoaded, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  /* During idle-> loaded state transition buffers need to be freed up */
  for (i = 0; i < pAppData->disILComp->inPortParams->nBufferCountActual; i++)
  {
    eError =
      OMX_FreeBuffer (pAppData->pDisHandle,
                      OMX_VFDC_INPUT_PORT_START_INDEX,
                      pAppData->disILComp->inPortParams->pInBuff[i]);
    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_FreeBuffer : %s \n", IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }

  semp_pend (pAppData->disILComp->done_sem);

  printf (" display state loaded \n ");

  /* control component does not alloc/free any data buffers, It's interface
     is though as it is omx componenet */
  eError =
    OMX_SendCommand(pAppData->pctrlHandle, OMX_CommandStateSet,
                    OMX_StateLoaded, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf("Error in SendCommand()-OMX_StateLoaded State set : %s \n",
           IL_ClientErrorToStr(eError));
    goto EXIT;
  }

  semp_pend(pAppData->disILComp->done_sem);

  printf(" ctrl-dc state loaded \n ");

/******************************************************************************/

  /* change the encoder state to loded */
  eError =
    OMX_SendCommand (pAppData->pEncHandle, OMX_CommandStateSet,
                     OMX_StateLoaded, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  /* During idle-> loaded state transition buffers need to be freed up */
  for (i = 0; i < pAppData->encILComp->inPortParams->nBufferCountActual; i++)
  {
    eError =
      OMX_FreeBuffer (pAppData->pEncHandle, OMX_VIDENC_INPUT_PORT,
                      pAppData->encILComp->inPortParams->pInBuff[i]);
    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_FreeBuffer : %s \n", IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }

  for (i = 0; i < pAppData->encILComp->outPortParams->nBufferCountActual; i++)
  {
    eError =
      OMX_FreeBuffer (pAppData->pEncHandle, OMX_VIDENC_OUTPUT_PORT,
                      pAppData->encILComp->outPortParams->pOutBuff[i]);
    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_FreeBuffer : %s \n", IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }

  semp_pend (pAppData->encILComp->done_sem);

  printf (" encoder state loaded \n ");
/******************************************************************************/

  eError =
    OMX_SendCommand (pAppData->pDeiHandle, OMX_CommandStateSet,
                     OMX_StateLoaded, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  /* During idle-> loaded state transition buffers need to be freed up */
  for (i = 0; i < pAppData->deiILComp->inPortParams->nBufferCountActual; i++)
  {
    eError =
      OMX_FreeBuffer (pAppData->pDeiHandle,
                      OMX_VFPC_INPUT_PORT_START_INDEX,
                      pAppData->deiILComp->inPortParams->pInBuff[i]);
    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_FreeBuffer : %s \n", IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }

  /* DEI is dual o/p port OMX component; allocate buffers on both ports */
  for (j = 0; j < pAppData->deiILComp->numOutport; j++)
  {
    outPortParamsPtr = pAppData->deiILComp->outPortParams + j;
    /* buffer alloaction for output port */
    for (i = 0; i < outPortParamsPtr->nBufferCountActual; i++)
    {
      eError = OMX_FreeBuffer (pAppData->pDeiHandle,
                               OMX_VFPC_OUTPUT_PORT_START_INDEX + j,
                               outPortParamsPtr->pOutBuff[i]);
      if (eError != OMX_ErrorNone)
      {
        printf ("Error in OMX_AllocateBuffer()-Output Port State set : %s \n",
                IL_ClientErrorToStr (eError));
        goto EXIT;
      } /* if (eError) */
    } /* for (i) */
  } /* for (j) */

  semp_pend (pAppData->deiILComp->done_sem);

  printf (" dei state loaded \n ");

/******************************************************************************/

  eError =
    OMX_SendCommand (pAppData->pCapHandle, OMX_CommandStateSet,
                     OMX_StateLoaded, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf ("Error from SendCommand-Idle State set :%s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  /* During idle-> loaded state transition buffers need to be freed up */
  for (i = 0; i < pAppData->capILComp->outPortParams->nBufferCountActual; i++)
  {
    eError =
      OMX_FreeBuffer (pAppData->pCapHandle,
                      OMX_VFCC_OUTPUT_PORT_START_INDEX,
                      pAppData->capILComp->outPortParams->pOutBuff[i]);
    if (eError != OMX_ErrorNone)
    {
      printf ("Error in OMX_FreeBuffer : %s \n", IL_ClientErrorToStr (eError));
      goto EXIT;
    }
  }

  semp_pend (pAppData->capILComp->done_sem);

  printf (" capture state loaded \n ");

  /* ctrl tvp component does not alloc/free any data buffers, It's interface
     is though as it is omx componenet */
  eError =
    OMX_SendCommand(pAppData->pTvpHandle, OMX_CommandStateSet,
                    OMX_StateLoaded, NULL);
  if (eError != OMX_ErrorNone)
  {
    printf("Error in SendCommand()-OMX_StateLoaded State set : %s \n",
           IL_ClientErrorToStr(eError));
    goto EXIT;
  }

  semp_pend(pAppData->capILComp->done_sem);

  printf(" ctrl-tvp state loaded \n ");

/******************************************************************************/

  /* free handle for all component */

  eError = OMX_FreeHandle (pAppData->pCapHandle);
  if ((eError != OMX_ErrorNone))
  {
    printf ("Error in Free Handle function : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  printf (" capture free handle \n");

  eError = OMX_FreeHandle(pAppData->pTvpHandle);
  if ((eError != OMX_ErrorNone))
  {
    printf("Error in Free Handle function : %s \n",
           IL_ClientErrorToStr(eError));
    goto EXIT;
  }

  printf(" ctrl-tvp free handle \n");

  eError = OMX_FreeHandle (pAppData->pEncHandle);
  if ((eError != OMX_ErrorNone))
  {
    printf ("Error in Free Handle function : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }
  printf (" encoder free handle \n");

  eError = OMX_FreeHandle (pAppData->pDeiHandle);
  if ((eError != OMX_ErrorNone))
  {
    printf ("Error in Free Handle function : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  printf (" dei free handle \n");

  eError = OMX_FreeHandle (pAppData->pDisHandle);
  if ((eError != OMX_ErrorNone))
  {
    printf ("Error in Free Handle function : %s \n",
            IL_ClientErrorToStr (eError));
    goto EXIT;
  }

  printf (" display free handle \n");

  eError = OMX_FreeHandle(pAppData->pctrlHandle);
  if ((eError != OMX_ErrorNone))
  {
    printf("Error in Free Handle function : %s \n",
           IL_ClientErrorToStr(eError));
    goto EXIT;
  }

  printf(" ctrl-dc free handle \n");

/*   if (pAppData->fOut != NULL)
  {
    fclose(pAppData->fOut);
    pAppData->fOut = NULL;
  }
*/
  /* terminate the threads */

  pthread_join (pAppData->encILComp->connDataStrmThrdId, &ret_value);

  pthread_join (pAppData->deiILComp->connDataStrmThrdId, &ret_value);

  pthread_join (pAppData->capILComp->connDataStrmThrdId, &ret_value);

  pthread_join (pAppData->disILComp->connDataStrmThrdId, &ret_value);

  pthread_join(pAppData->encILComp->outDataStrmThrdId, &ret_value);

  IL_ClientDeInit (pAppData);

  printf ("IL Client deinitialized \n");

  printf (" example exit \n");

EXIT:
  return (0);
}

/* Nothing beyond this point */
