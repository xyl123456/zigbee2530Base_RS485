/**************************************************************************************************
  Filename:       api_events.h
  Revised:        $Date: 2014-04-29 09:01:20 -0800 (Thu, 29 Apr 2014) $
  Revision:       $Revision: 29217 $

  Description:    This file contains set stack events application interface definitions.
**************************************************************************************************/

/**************************************************************************************************
Modify by Sam_Chen
Date:2015-07-31
**************************************************************************************************/

#ifndef API_EVENTS_H
#define API_EVENTS_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */
#include "ZComDef.h"
#include "OSAL.h"


/*********************************************************************
 * CONSTANTS
 */
#define EVENT_MAP_ID		//get event address easily from event map
#define USER_EVENT_MAP_SIZE		16	//the map addr size of user event

#define TIMER_LOWER_LIMIT	99	//lower limit of user event's timer count
#define TIMER_NO_LIMIT	0	//set the timer event immediately

#define TIMER_CLEAR_EXECUTION	0x00	//clear timer event from list
#define TIMER_ONE_EXECUTION		0x01	//set the timer event once
#define TIMER_LOOP_EXECUTION	0x7F	//set the timer event always

#define TIMER_EVENT_RESIDENTS	0x80	//set the timer event and resident always

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * TYPEDEFS
 */
 
/*User Events*/
//type structure of user event callback
//params  (void *)-user defined  (uint16 *)-duration  (uint8 *)-excute count
typedef void (*ssa_ProcessUserTaskCB_t)( void *, uint16 *, uint8 * );

//type structure of user event
typedef struct ssaUserEvent
{
  uint8 task_id;
#ifdef EVENT_MAP_ID
  uint8 map_id;		//mapped id for the array of ssa_ProcessUserTaskCB_t 's address
#endif
  uint16 event;		//event flag for user defined
  uint16 duration;	//excute duration for ther next time
  union 
  {
  	uint8 overall;
	struct
	{
	  uint8 count : 7;
	  uint8 resident : 1;
	} branch;
  } count;		//excute count
  
  void *ptype;	//point to data of user defined
  ssa_ProcessUserTaskCB_t ssa_ProcessUserTaskCB;	//handle callbacks of this event
  struct ssaUserEvent *next;		//point to next structure in list
} ssaUserEvent_t;


/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Set User Defined Events for Application
 */
extern int8 set_user_event(uint8 task_id, uint16 event, 
	ssa_ProcessUserTaskCB_t ssa_ProcessUserTaskCB, uint16 duration, uint8 count, void *ptype);

/*
 * Task Update User Defined Events for Application
 */
extern int8 update_user_event(uint8 task_id, uint16 event, 
	ssa_ProcessUserTaskCB_t ssa_ProcessUserTaskCB, uint16 duration, uint8 count, void *ptype);

/*
* Task event processor.
 */
extern uint16 process_event(uint8 task_id, uint16 events);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* API_EVENTS_H */

