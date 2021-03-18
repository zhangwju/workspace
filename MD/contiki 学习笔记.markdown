#Contiki 学习笔记
PROCESS_BEGIN() : Declares the beginning of a process' protothread.
PROCESS_END() : Declares the end of a process' protothread.
PROCESS_EXIT() : Exit the process.
PROCESS_WAIT_EVENT() : Wait for any event.
PROCESS_WAIT_EVENT_UNTIL() : Wait for an event, but with condition.
PROCESS_YIELD() : Wait for any event, equivalent to PROCESS_WAIT_EVENT().
PROCESS_WAIT_UNTIL() : Wait for a given condition; may not yield the micro-controller.
PROCESS_PAUSE() : Temporarily yield the micro-controller.