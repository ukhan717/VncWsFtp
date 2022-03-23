/*********************************************************************
*                     SEGGER Microcontroller GmbH                    *
*                        The Embedded Experts                        *
**********************************************************************
*                                                                    *
*       (c) 2003 - 2019  SEGGER Microcontroller GmbH                 *
*                                                                    *
*       www.segger.com     Support: support@segger.com               *
*                                                                    *
**********************************************************************
----------------------------------------------------------------------
File    : USB_Audio.h
Purpose : Public header of audio device class
--------  END-OF-HEADER  ---------------------------------------------
*/

#ifndef USBD_AUDIO_H          /* Avoid multiple inclusion */
#define USBD_AUDIO_H

#include "SEGGER.h"
#include "USB.h"

#if defined(__cplusplus)
extern "C" {     /* Make sure we have C-declarations in C++ programs */
#endif

/*********************************************************************
*
*       Config defaults
*
**********************************************************************
*/

/*********************************************************************
*
*       Defines, fixed
*
**********************************************************************
*/
#define USB_AUDIO_PLAYBACK_START  0xA1
#define USB_AUDIO_PLAYBACK_STOP   0xA2
#define USB_AUDIO_RECORD_START    0xA3
#define USB_AUDIO_RECORD_STOP     0xA4
//
//Audio Interface Class, Subclass and Protocol Codes
//
#define USB_AUDIO_CLASS                       0x01
#define USB_AUDIO_SUBCLASS_UNDEFINED          0x00
#define USB_AUDIO_SUBCLASS_AUDIO_CONTROL      0x01
#define USB_AUDIO_SUBCLASS_AUDIO_STREAMING    0x02
#define USB_AUDIO_SUBCLASS_MIDI_STREAMING     0x03
#define USB_AUDIO_PROTOCOL_UNDEFINED          0x00

//
// Audio Class-Specific Descriptor Types
//
#define USB_AUDIO_CS_UNDEFINED                0x20
#define USB_AUDIO_CS_DEVICE                   0x21
#define USB_AUDIO_CS_CONFIGURATION            0x22
#define USB_AUDIO_CS_STRING                   0x23
#define USB_AUDIO_CS_INTERFACE                0x24
#define USB_AUDIO_CS_ENDPOINT                 0x25

//
// Audio Class-Specific AC Interface Descriptor Subtypes
//
#define USB_AUDIO_AC_DESCRIPTOR_UNDEFINED     0x00
#define USB_AUDIO_HEADER                      0x01
#define USB_AUDIO_INPUT_TERMINAL              0x02
#define USB_AUDIO_OUTPUT_TERMINAL             0x03
#define USB_AUDIO_MIXER_UNIT                  0x04
#define USB_AUDIO_SELECTOR_UNIT               0x05
#define USB_AUDIO_FEATURE_UNIT                0x06
#define USB_AUDIO_PROCESSING_UNIT             0x07
#define USB_AUDIO_EXTENSION_UNIT              0x08

//
// Audio Class-Specific AS Interface Descriptor Subtypes
//
#define USB_AUDIO_AS_DESCRIPTOR_UNDEFINED     0x00
#define USB_AUDIO_AS_GENERAL                  0x01
#define USB_AUDIO_FORMAT_TYPE                 0x02
#define USB_AUDIO_FORMAT_SPECIFIC             0x03

//
// Processing Unit Process Types
//
#define USB_AUDIO_PROCESS_UNDEFINED           0x00
#define USB_AUDIO_UP_DOWNMIX_PROCESS          0x01
#define USB_AUDIO_DOLBY_PROLOGIC_PROCESS      0x02
#define USB_AUDIO_3D_STEREO_EXTENDER_PROCESS  0x03
#define USB_AUDIO_REVERBERATION_PROCESS       0x04
#define USB_AUDIO_CHORUS_PROCESS              0x05
#define USB_AUDIO_DYN_RANGE_COMP_PROCESS      0x06

//
// Audio Class-Specific Endpoint Descriptor Subtypes
//
#define USB_AUDIO_DESCRIPTOR_UNDEFINED        0x00
#define USB_AUDIO_EP_GENERAL                  0x01

//
// Audio Class-Specific Request Codes
//
#define USB_AUDIO_REQUEST_CODE_UNDEFINED      0x00
#define USB_AUDIO_SET_CUR                     0x01
#define USB_AUDIO_GET_CUR                     0x81
#define USB_AUDIO_SET_MIN                     0x02
#define USB_AUDIO_GET_MIN                     0x82
#define USB_AUDIO_SET_MAX                     0x03
#define USB_AUDIO_GET_MAX                     0x83
#define USB_AUDIO_SET_RES                     0x04
#define USB_AUDIO_GET_RES                     0x84
#define USB_AUDIO_SET_MEM                     0x05
#define USB_AUDIO_GET_MEM                     0x85
#define USB_AUDIO_GET_STAT                    0xFF

//
// Terminal Control Selectors
//
#define USB_AUDIO_TE_CONTROL_UNDEFINED        0x00
#define USB_AUDIO_COPY_PROTECT_CONTROL        0x01

//
// Feature Unit Control Selectors
//
#define USB_AUDIO_FU_CONTROL_UNDEFINED        0x00
#define USB_AUDIO_MUTE_CONTROL                0x01
#define USB_AUDIO_VOLUME_CONTROL              0x02
#define USB_AUDIO_BASS_CONTROL                0x03
#define USB_AUDIO_MID_CONTROL                 0x04
#define USB_AUDIO_TREBLE_CONTROL              0x05
#define USB_AUDIO_GRAPHIC_EQUALIZER_CONTROL   0x06
#define USB_AUDIO_AUTOMATIC_GAIN_CONTROL      0x07
#define USB_AUDIO_DELAY_CONTROL               0x08
#define USB_AUDIO_BASS_BOOST_CONTROL          0x09
#define USB_AUDIO_LOUDNESS_CONTROL            0x0A


//
// Descriptor sizes
//
#define USB_AUDIO_AC_HEADER_SIZE(a)           (8 + a)
#define USB_AUDIO_AC_IN_TER_SIZE              0x0C
#define USB_AUDIO_AC_FE_UNIT_SIZE             0x09
#define USB_AUDIO_AC_OUT_TER_SIZE             0x09
#define USB_AUDIO_AS_GEN_IF_SIZE              0x07
#define USB_AUDIO_AS_FORMAT_ONE_SIZE          0x0B

/*********************************************************************
*
*       Types
*
**********************************************************************
*/
typedef int USBD_AUDIO_HANDLE;

/*********************************************************************
*
*       USBD_AUDIO_RX_FUNC
*
*  Description
*    Definition of the callback which is called when audio data is received. This callback is called in the context of USBD_AUDIO_Read_Task().
*
*  Parameters
*    pUserContext     : User context which is passed to the callback.
*    NumBytesReceived : The number of bytes which have been read in this transaction.
*    ppNextBuffer     : Buffer containing audio samples which should match the configuration from USBD_AUDIO_SPEAKER_CONF . Initially this points to the pBufOut from the USBD_AUDIO_INIT_DATA structure.
*                       The user can change this pointer to a different buffer which will be used in the next transaction
*                       or leave it as it is and copy the data from this buffer elsewhere.
*    pNextBufferSize  : Size of the next buffer.
*/
typedef void USBD_AUDIO_RX_FUNC(void * pUserContext, int NumBytesReceived, U8 ** ppNextBuffer, U32 * pNextBufferSize);

/*********************************************************************
*
*       USBD_AUDIO_TX_FUNC
*
*  Description
*   Definition of the callback which is called when audio data is sent. This callback is called in the context of USBD_AUDIO_Write_Task()
*
*  Parameters
*    pUserContext     : User context which is passed to the callback.
*    ppNextBuffer     : Buffer containing audio samples which should match the configuration from USBD_AUDIO_MIC_CONF .
*                       Initially this points to the pBufIn from the USBD_AUDIO_INIT_DATA structure.
*                       The user can change this pointer to a different buffer which will be used
*                       in the next transaction or fill the same buffer with new data.
*    pNextBufferSize  : Size of the next buffer.
*/
typedef void USBD_AUDIO_TX_FUNC(void * pUserContext, const U8 ** ppNextBuffer, U32 * pNextPacketSize);

/*********************************************************************
*
*       USBD_AUDIO_CONTROL_FUNC
*
*  Description
*    Definition of the callback which is called when audio commands are received. This callback is called in an interrupt context.
*
*  Parameters
*    pUserContext     : User context which is passed to the callback.
*    Event            : Audio event ID.
*    Unit             : ID of the feature unit.
*                       In case of USB_AUDIO_PLAYBACK_* & USB_AUDIO_RECORD_*: 0.
*    ControlSelector  : ID of the control.
*                       In case of USB_AUDIO_PLAYBACK_* & USB_AUDIO_RECORD_*: 0.
*    pBuffer          : In case of GET events: pointer to a buffer into which the callback should write the reply.
*                       In case of SET events: pointer to a buffer containing the command value.
*                       In case of USB_AUDIO_PLAYBACK_* & USB_AUDIO_RECORD_*: NULL.
*    NumBytes         : In case of GET events: requested size of the reply in bytes.
*                       In case of SET events: number of bytes in pBuffer.
*                       In case of USB_AUDIO_PLAYBACK_* & USB_AUDIO_RECORD_*: 0.
*  Return value
*    == 0:              Audio command was handled by the callback. The stack will send the reply.
*    != 0:              Audio command was not handled by the callback. The stack will STALL the request.
*
*  Additional information
*    USB_AUDIO_PLAYBACK_* & USB_AUDIO_RECORD_* events are sent upon receiving a Set Interface USB request
*    for Alternate Setting 1 for the respective interface (microphone or speaker).
*    By default an Audio interface is set to Alternative Setting 0 in which it can not send or receive anything.
*    The host switches the Alternative Setting to 1 when it has to send data to the device,
*    this can be e.g. triggered by pressing "play" in your music player.
*    Normally the host should switch the device back to Alternative Interface 0 when it has stopped sending audio data.
*    This works well on Linux and OS X, but does not work reliably on Windows.
*    When using Windows as a host it seems to depend on the application whether these events are generated or not.
*    E.g. with some applications you will receive USB_AUDIO_PLAYBACK_START when "play" is pressed,
*    but USB_AUDIO_PLAYBACK_STOP will not be sent when "pause" or "stop" is pressed. Relying on these events
*    to check when the host has stopped sending data is not advised, instead set timeouts
*    via USBD_AUDIO_Set_Timeouts and check for timeouts inside your USBD_AUDIO_RX_FUNC and USBD_AUDIO_TX_FUNC .
*/
typedef int USBD_AUDIO_CONTROL_FUNC(void * pUserContext, U8 Event, U8 Unit, U8 ControlSelector, U8 * pBuffer, U32 NumBytes);

/*********************************************************************
*
*       USBD_AUDIO_SPEAKER_CONF
*
*   Description
*     Initialization data for the speaker interface .
*/
typedef struct _USBD_AUDIO_SPEAKER_CONF {
  U8 Controls;      // A bit set to 1 indicates that the mentioned Control is supported:
                    // * b0: Mute
                    // * b1: Volume
                    // * b2: Bass
                    // * b3: Mid
                    // * b4: Treble
                    // * b5: Graphic Equalizer
                    // * b6: Automatic Gain
                    // * b7: Delay
  U8 NrChannels;    // Number of audio channels.
  U8 SubFrameSize;  // Size of an audio frame in bytes. Must be able to hold BitResolution bits.
  U8 BitResolution; // Number of bits inside the audio frame dedicated to audio data. (Any remaining bits are padding.)
  U32 SamFreq;      // Sample frequency.
} USBD_AUDIO_SPEAKER_CONF;

/*********************************************************************
*
*       USBD_AUDIO_MIC_CONF
*
*   Description
*     Initialization data for the microphone interface.
*/
typedef struct _USBD_AUDIO_MIC_CONF {
  U8 Controls;      // Reserved.
  U8 NrChannels;    // Number of audio channels.
  U8 SubFrameSize;  // Size of an audio frame in bytes. Must be able to hold BitResolution bits.
  U8 BitResolution; // Number of bits inside the audio frame dedicated to audio data. (Any remaining bits are padding.)
  U32 SamFreq;      // Sample frequency.
} USBD_AUDIO_MIC_CONF;

/*********************************************************************
*
*       USBD_AUDIO_INIT_DATA
*
*   Description
*     Initialization data for the Audio interface.
*/
typedef struct _USBD_AUDIO_INIT_DATA {
  U8                              EPIn;           // Isochronous endpoint for sending data to the host. If microphone functionality is not desired set this to 0.
  U8                              EPOut;          // Isochronous endpoint for receiving data from the host. If speaker functionality is not desired set this to 0.
  U8                            * pBufOut;        // Buffer used with OUT transfers (speaker interface).
  const U8                      * pBufIn;         // Buffer used with IN transfers (microphone interface).
  unsigned                        InPacketSize;   // Size of a single audio IN packet. Must be calculated as follows: SampleRate * NumChannels * BitsPerSample/8 / 1000
  unsigned                        OutPacketSize;  // Size of a single audio OUT packet. Must be calculated as follows: SampleRate * NumChannels * BitsPerSample/8 / 1000
  USBD_AUDIO_RX_FUNC            * pfOnOut;        // Pointer to a function of type USBD_AUDIO_RX_FUNC which handles incoming audio data. Needs to be set when the speaker interface is used.
  USBD_AUDIO_TX_FUNC            * pfOnIn;         // Pointer to a function of type USBD_AUDIO_TX_FUNC which handles outgoing audio data. Needs to be set when the microphone interface is used.
  USBD_AUDIO_CONTROL_FUNC       * pfOnControl;    // Pointer to a function of type USBD_AUDIO_CONTROL_FUNC which handles audio commands. Always needs to be set.
  void                          * pControlUserContext; // Pointer to a user context which is passed to the pfOnControl function. Optional, can be NULL..
  const USBD_AUDIO_MIC_CONF     * pMicConf;       // Pointer to a structure of type USBD_AUDIO_MIC_CONF which contains configuration data for the microphone interface. If microphone functionality is not desired set this pointer to NULL.
  const USBD_AUDIO_SPEAKER_CONF * pSpeakerConf;   // Pointer to a structure of type USBD_AUDIO_SPEAKER_CONF which contains configuration data for the speaker interface. If speaker functionality is not desired set this pointer to NULL.
  int                             ReadTimeout;    // Initial timeout for read requests (speaker interface). Can be changed via USBD_AUDIO_Set_Timeouts .
  int                             WriteTimeout;   // Initial timeout for write requests (microphone interface). Can be changed via USBD_AUDIO_Set_Timeouts .
} USBD_AUDIO_INIT_DATA;

/*********************************************************************
*
*       API functions
*
**********************************************************************
*/
USBD_AUDIO_HANDLE USBD_AUDIO_Add          (const USBD_AUDIO_INIT_DATA * pInitData);
void              USBD_AUDIO_Read_Task    (void);
void              USBD_AUDIO_Write_Task   (void);
int               USBD_AUDIO_Start_Play   (USBD_AUDIO_HANDLE hInst);
void              USBD_AUDIO_Stop_Play    (USBD_AUDIO_HANDLE hInst);
int               USBD_AUDIO_Start_Listen (USBD_AUDIO_HANDLE hInst);
void              USBD_AUDIO_Stop_Listen  (USBD_AUDIO_HANDLE hInst);
void              USBD_AUDIO_Set_Timeouts (USBD_AUDIO_HANDLE hInst, int ReadTimeout, int WriteTimeout);
#if defined(__cplusplus)
  }              /* Make sure we have C-declarations in C++ programs */
#endif

#endif                 /* Avoid multiple inclusion */

/*************************** End of file ****************************/
