/**************************************************************************************************
  Filename:       api_events.c
  Revised:        $Date: 2014-04-29 09:03:06 -0800 (Thu, 29 Apr 2014) $
  Revision:       $Revision: 29217 $

  Description:    Set Events Application Interface
**************************************************************************************************/

/**************************************************************************************************
Modify by Sam_Chen
Date:2014-07-31
**************************************************************************************************/


/*********************************************************************
 * INCLUDES
 */
#include "api_events.h"

/*********************************************************************
 * MACROS
 */

/*********************************************************************
 * CONSTANTS
 */

/*********************************************************************
 * EXTERNAL VARIABLES
 */

/*********************************************************************
 * EXTERNAL FUNCTIONS
 */
 
/*********************************************************************
 * GLOBAL VARIABLES
 */

/*********************************************************************
 * LOCAL VARIABLES
 */
//chain list header
static ssaUserEvent_t *puser_event = NULL;

//event map
#ifdef EVENT_MAP_ID
static ssaUserEvent_t *userevent_map[USER_EVENT_MAP_SIZE] = {NULL};
#endif

/*********************************************************************
 * LOCAL FUNCTIONS
 */

#ifdef EVENT_MAP_ID
static uint8 event_to_id(uint16 event);
#endif
static int8 Add_User_Event(ssaUserEvent_t *ssaUserEvent);
static ssaUserEvent_t *Query_User_Event(uint16 event);
static int8 Del_User_Event(uint16 event);

static void set_app_event(ssaUserEvent_t *ssaUserEvent);

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

#ifdef EVENT_MAP_ID
/*********************************************************************
 * @fn      event_to_id
 *
 * @brief   get map id from user event flag
 *
 * @param   user process event
 *
 * @return  user map id
 */
uint8 event_to_id(uint16 event)
{
  uint8 i = 0;
  
  for(i=0; i<16; i++)
  	if(event & 1<<i) break;
	
  return i;
}
#endif
 

/*********************************************************************
 * @fn      Add_User_Event
 *
 * @brief   add user event to chain list
 *
 * @param   user process event
 *
 * @return  result(susses or failed)
 */
int8 Add_User_Event(ssaUserEvent_t *ssaUserEvent)
{
  if(ssaUserEvent != NULL)
  {
  	ssaUserEvent->next = NULL;
#ifdef EVENT_MAP_ID
	ssaUserEvent->map_id = event_to_id(ssaUserEvent->event);
#endif
  }
 
  if(puser_event == NULL)
  {
	puser_event = ssaUserEvent;
#ifdef EVENT_MAP_ID
	if(ssaUserEvent->map_id < USER_EVENT_MAP_SIZE)
	{
	  userevent_map[ssaUserEvent->map_id] = ssaUserEvent;
	}
#endif
  }
  else
  {
	ssaUserEvent_t *pre_event = NULL;
	ssaUserEvent_t *t_event = puser_event;
	if(ssaUserEvent->event == t_event->event)
	{
	  return -1;
	}
	else if(ssaUserEvent->duration < t_event->duration)
	{
	  ssaUserEvent->next = t_event;
	  puser_event = ssaUserEvent;
#ifdef EVENT_MAP_ID
	  if(ssaUserEvent->map_id < USER_EVENT_MAP_SIZE)
	  {
	    userevent_map[ssaUserEvent->map_id] = ssaUserEvent;
	  }
#endif
	  return 0;
	}

	while(t_event->next != NULL)
	{
	  pre_event = t_event;
	  t_event = t_event->next;
	  if(ssaUserEvent->event == t_event->event)
	  {
		return -1;
	  }
	  else if(ssaUserEvent->duration < t_event->duration)
	  {
		ssaUserEvent->next = t_event;
		pre_event->next = ssaUserEvent;
#ifdef EVENT_MAP_ID
		if(ssaUserEvent->map_id < USER_EVENT_MAP_SIZE)
		{
		  userevent_map[ssaUserEvent->map_id] = ssaUserEvent;
		}
#endif

		return 0;
	  }
	}
	t_event->next = ssaUserEvent;
  }

  return 0;
}


/*********************************************************************
 * @fn      Query_User_Event
 *
 * @brief   query user event from chain list
 *
 * @param   event flag
 *
 * @return  result(event obj or NULL)
 */
ssaUserEvent_t *Query_User_Event(uint16 event)
{
  ssaUserEvent_t *t_event;
#ifdef EVENT_MAP_ID
  uint8 map_id = event_to_id(event);
  if(map_id<USER_EVENT_MAP_SIZE && userevent_map[map_id]!=NULL)
  	return userevent_map[map_id];
#endif

  t_event = puser_event;
  while(t_event->next!=NULL && t_event->event!=event)
  	t_event = t_event->next;

  if(t_event->event == event)
  	return t_event;
  else
  	return NULL;
}


/*********************************************************************
 * @fn      Del_User_Event
 *
 * @brief   delete user event from chain list
 *
 * @param   event flag
 *
 * @return  result(susses or failed)
 */
int8 Del_User_Event(uint16 event)
{
  ssaUserEvent_t *pre_event = NULL;
  ssaUserEvent_t *t_event = puser_event;
  if(t_event == NULL)
  	return -1;

  while(t_event->next!=NULL && t_event->event != event)
  {
	pre_event = t_event;
	t_event = t_event->next;
  }

  if(t_event->event == event)
  {
	if(pre_event == NULL)
		puser_event = t_event->next;
	else
		pre_event->next = t_event->next;

#ifdef EVENT_MAP_ID
	if(t_event->map_id<USER_EVENT_MAP_SIZE)
		userevent_map[t_event->map_id] = NULL;
#endif
	
	osal_mem_free(t_event);
	return 0;
  }
  else
  	return -2;
}

/*********************************************************************
 * @fn      set_app_event
 *
 * @brief   set osal event from app event
 *
 * @param   user process event
 *
 * @return  none
 */
void set_app_event(ssaUserEvent_t *ssaUserEvent)
{
  if(ssaUserEvent->count.branch.count)
  {
    if(ssaUserEvent->duration > TIMER_LOWER_LIMIT)
    {
	  osal_stop_timerEx(ssaUserEvent->task_id, ssaUserEvent->event);
	  osal_start_timerEx(ssaUserEvent->task_id, 
	  		ssaUserEvent->event, ssaUserEvent->duration);
    }
    else
    {
	  osal_set_event(ssaUserEvent->task_id, ssaUserEvent->event);
    }
  }
}


/*********************************************************************
 * @fn      set_user_event
 *
 * @brief   set user defined events
 *
 * @param   none
 *
 * @return  none
 */
int8 set_user_event(uint8 task_id, uint16 event, 
	ssa_ProcessUserTaskCB_t ssa_ProcessUserTaskCB, uint16 duration, uint8 count, void *ptype)
{
	ssaUserEvent_t *m_ssaUserEvent;
	m_ssaUserEvent = (ssaUserEvent_t *)osal_mem_alloc(sizeof(ssaUserEvent_t));

	m_ssaUserEvent->task_id = task_id;
	m_ssaUserEvent->event = event;
	m_ssaUserEvent->duration = duration;
	m_ssaUserEvent->count.overall = count;
	m_ssaUserEvent->ptype = ptype;
	m_ssaUserEvent->ssa_ProcessUserTaskCB = ssa_ProcessUserTaskCB;

	if(Add_User_Event(m_ssaUserEvent) < 0)
	{
	  osal_mem_free(m_ssaUserEvent);
	  return -1;
	}

	set_app_event(m_ssaUserEvent);
	return 0;
}


/*********************************************************************
 * @fn      update_user_event
 *
 * @brief   update user defined events
 *
 * @param   none
 *
 * @return  none
 */
int8 update_user_event(uint8 task_id, uint16 event, 
	ssa_ProcessUserTaskCB_t ssa_ProcessUserTaskCB, uint16 duration, uint8 count, void *ptype)
{
  ssaUserEvent_t *m_ssaUserEvent;
  m_ssaUserEvent = Query_User_Event(event);

  if(m_ssaUserEvent == NULL)
  {
	return set_user_event(task_id, event, 
		ssa_ProcessUserTaskCB, duration, count, ptype);
  }

  m_ssaUserEvent->duration = duration;
  m_ssaUserEvent->count.overall = count;
  m_ssaUserEvent->ptype = ptype;
  m_ssaUserEvent->ssa_ProcessUserTaskCB = ssa_ProcessUserTaskCB;

  set_app_event(m_ssaUserEvent);
  
  return 0;
}


/*********************************************************************
 * @fn      process_event
 *
 * @brief   Task event processor. 
 *
 * @param   task_id  - The OSAL assigned task ID.
 * @param   events - events to process.  This is a bit map and can
 *                   contain more than one event.
 *
 * @return  none
 */
uint16 process_event(uint8 task_id, uint16 events)
{
  ssaUserEvent_t *t_ssaUserEvent = puser_event;
  
  while(t_ssaUserEvent)
  {
    if(t_ssaUserEvent->count.branch.count>0
		|| t_ssaUserEvent->count.branch.resident==1)
    {
	  if((t_ssaUserEvent->task_id == task_id )
	  		&& (events & t_ssaUserEvent->event))
	  {
		if(t_ssaUserEvent->ssa_ProcessUserTaskCB != NULL)
      	  t_ssaUserEvent->ssa_ProcessUserTaskCB(t_ssaUserEvent->ptype, 
	    		&t_ssaUserEvent->duration, &t_ssaUserEvent->count.overall);

	    if(t_ssaUserEvent->count.branch.count != TIMER_LOOP_EXECUTION)
          t_ssaUserEvent->count.branch.count--;

	    if(t_ssaUserEvent->count.branch.count > 0)
		  set_app_event(t_ssaUserEvent);

		return (events ^ t_ssaUserEvent->event);
	  }
  	}
  	else
  	{
	  Del_User_Event(t_ssaUserEvent->event);
  	}
  
  	t_ssaUserEvent = t_ssaUserEvent->next;
  }	

  // Discard unknown events
   return process_event(task_id, events);
}

