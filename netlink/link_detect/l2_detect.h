/* Filename: l2_detect.h,  created by Teidor.Tien, on 2016/12/12  */
#ifndef __L2_DETECT_H__
#define __L2_DETECT_H__


/************************************************************************
  *                                                     DEFINITION                                               *
  ************************************************************************/
enum {
	UBUS_INTERFACE_EVENT,
	UBUS_DEVICE_NAME_EVENT,
	UBUS_METHOD_EVENT,
	UBUS_EVENT_MAX
};


/************************************************************************
  *                                                     FUNCTIONS                                               *
  ************************************************************************/
void l2_thread_handler(void* arg);

#endif /* __L2_DETECT_H__ */
