/*******************************************************************
** File        : Types.h                                          **
********************************************************************
**                                                                **
** Version     : V 1.0                                            **
**                                                                **
** Writen by   : Miguel Luis                                      **
**                                                                **
** Date        : 12-01-2000                                       **
**                                                                **
** Project     : -                                                **
**                                                                **
********************************************************************
** Changes     :                                                  **
********************************************************************
** Description : Standard types definition                        **
*******************************************************************/
#ifndef __TYPES_H__
#define __TYPES_H__

/*******************************************************************
** BOOL - Boolean values                                          **
*******************************************************************/
#define false 0
#define true  1
#define FALSE 0
#define TRUE  1

/*******************************************************************
** _U8 - 8 bit quantity                                           **
*******************************************************************/
#ifdef _U8
  #undef _U8
#endif
typedef unsigned char _U8;  /* 8 bit quantity         */

/*******************************************************************
** _S8 - 8 bit signed quanity                                     **
*******************************************************************/
#ifdef _S8
  #undef _S8
#endif
typedef signed char _S8;        /* 8 bit signed quantity  */ 

/*******************************************************************
** _U16 - 16 bit quantity                                         **
*******************************************************************/
#ifdef _U16
  #undef _U16
#endif
typedef unsigned short _U16; /* 16 bit quantity        */

/*******************************************************************
** _S16 - 16 bit signed quanity                                   **
*******************************************************************/
#ifdef _S16
  #undef _S16
#endif
typedef short _S16;        /* 16 bit signed quantity */

/*******************************************************************
** _U32 - 32 bit quantity                                         **
*******************************************************************/
#ifdef _U32
  #undef _U32
#endif
typedef unsigned long _U32; /* 32 bit quantity        */

/*******************************************************************
** _S32 - 32 bit signed quanity                                   **
*******************************************************************/
#ifdef _S32
  #undef _S32
#endif
typedef long  _S32;        /* 32 bit signed quantity */


/*******************************************************************
** _F32 - 32 bit float quanity                                    **
*******************************************************************/
#ifdef _F32
  #undef _F32
#endif
typedef double _F32;        /* 32 bit float quantity */

#endif // __TYPES_H__

