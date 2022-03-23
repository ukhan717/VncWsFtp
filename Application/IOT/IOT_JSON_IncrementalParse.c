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
-------------------------- END-OF-HEADER -----------------------------

File        : IOT_JSON_IncrementalParse.c
Purpose     : Incremental parse of a JSON object.

*/

/*********************************************************************
*
*       #include Section
*
**********************************************************************
*/

#include "IOT.h"

/*********************************************************************
*
*       Prototypes
*
**********************************************************************
*/

static void _BeginObject(IOT_JSON_CONTEXT *pContext);
static void _EndObject  (IOT_JSON_CONTEXT *pContext);
static void _BeginArray (IOT_JSON_CONTEXT *pContext);
static void _EndArray   (IOT_JSON_CONTEXT *pContext);
static void _Key        (IOT_JSON_CONTEXT *pContext, const char *sText);
static void _String     (IOT_JSON_CONTEXT *pContext, const char *sText);
static void _Number     (IOT_JSON_CONTEXT *pContext, const char *sText);
static void _Literal    (IOT_JSON_CONTEXT *pContext, const char *sText);

/*********************************************************************
*
*       Static const data
*
**********************************************************************
*/

static const IOT_JSON_EVENTS _EventAPI = {
  _BeginObject,
  _EndObject,
  _BeginArray,
  _EndArray,
  _Key,
  _String,
  _Number,
  _Literal
};

static const char _sJSONValue[] = {
  "{                                                   "
  "    \"name\": \"My TV\",                            "
  "    \"resolutions\": [                              "
  "        { \"width\": 1280, \"height\": 720 },       "
  "        { \"width\": 1920, \"height\": 1080 },      "
  "        { \"width\": 3840, \"height\": 2160 }       "
  "    ]                                               "
  "}                                                   "
};

/*********************************************************************
*
*       Static code
*
**********************************************************************
*/

/*********************************************************************
*
*       _BeginObject()
*
*  Function description
*    Handle Begin Object event.
*
*  Parameters
*    pContext - Pointer to JSON context.
*/
static void _BeginObject(IOT_JSON_CONTEXT *pContext) {
  IOT_USE_PARA(pContext);
  printf("Begin object\n");
}

/*********************************************************************
*
*       _EndObject()
*
*  Function description
*    Handle End Object event.
*
*  Parameters
*    pContext - Pointer to JSON context.
*/
static void _EndObject(IOT_JSON_CONTEXT *pContext) {
  IOT_USE_PARA(pContext);
  printf("End object\n");
}

/*********************************************************************
*
*       _BeginArray()
*
*  Function description
*    Handle Begin Array event.
*
*  Parameters
*    pContext - Pointer to JSON context.
*/
static void _BeginArray(IOT_JSON_CONTEXT *pContext) {
  IOT_USE_PARA(pContext);
  printf("Begin array\n");
}

/*********************************************************************
*
*       _EndArray()
*
*  Function description
*    Handle End Array event.
*
*  Parameters
*    pContext - Pointer to JSON context.
*/
static void _EndArray(IOT_JSON_CONTEXT *pContext) {
  IOT_USE_PARA(pContext);
  printf("End array\n");
}

/*********************************************************************
*
*       _Key()
*
*  Function description
*    Handle Key event.
*
*  Parameters
*    pContext - Pointer to JSON context.
*    sText    - Zero-terminated key ID.
*/
static void _Key(IOT_JSON_CONTEXT *pContext, const char *sText) {
  IOT_USE_PARA(pContext);
  printf("Key = \"%s\"\n", sText);
}

/*********************************************************************
*
*       _String()
*
*  Function description
*    Handle Key event.
*
*  Parameters
*    pContext - Pointer to JSON context.
*    sText    - Zero-terminated string value.
*/
static void _String(IOT_JSON_CONTEXT *pContext, const char *sText) {
  IOT_USE_PARA(pContext);
  printf("String = \"%s\"\n", sText);
}

/*********************************************************************
*
*       _Number()
*
*  Function description
*    Handle Number event.
*
*  Parameters
*    pContext - Pointer to JSON context.
*    sText    - Zero-terminated numeric value.
*/
static void _Number(IOT_JSON_CONTEXT *pContext, const char *sText) {
  IOT_USE_PARA(pContext);
  printf("Number = \"%s\"\n", sText);
}

/*********************************************************************
*
*       _Literal()
*
*  Function description
*    Handle Literal event.
*
*  Parameters
*    pContext - Pointer to JSON context.
*    sText    - Zero-terminated literal name.
*/
static void _Literal(IOT_JSON_CONTEXT *pContext, const char *sText) {
  IOT_USE_PARA(pContext);
  printf("Literal = \"%s\"\n", sText);
}

/*********************************************************************
*
*       Public code
*
**********************************************************************
*/

/*********************************************************************
*
*       MainTask()
*
*  Function description
*    Application entry point.
*/
void MainTask(void);
void MainTask(void) {
  IOT_JSON_CONTEXT JSON;
  char             acBuf[32];
  unsigned         i;
  int              Status;
  //
  IOT_JSON_Init(&JSON, &_EventAPI, &acBuf[0], sizeof(acBuf));
  //
  i = 0;
  do {
    Status = IOT_JSON_Parse(&JSON, &_sJSONValue[i++], 1);
  } while (Status > 0);
  //
  if (Status == 0) {
    printf("\nParse OK\n");
  } else {
    printf("\nParse FAIL\n");
  }
}

/*************************** End of file ****************************/
