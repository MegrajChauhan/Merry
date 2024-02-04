#include "internals/merry_os_exec.h"

_os_exec_(halt)
{
    // here we have to stop the core first
    // since the core is waiting, we don't have to worry about mutex locks.
    // since the core's decoder won't mess with other fields of the core, we can freely make changes
    _llog_(_OS_, "Request", " Fulfilling the halt request: Requester %d", request->id);
    os->cores[request->id]->stop_running = mtrue; // this will automatically halt the core's decoder as well
    if (os->core_count == 0)
    {
        // we had only one core to begin with then stop any further execution
        os->stop = mtrue;
    }
    printf("Halting.\n"); /// TODO: remove this
    _llog_(_OS_, "REQ_SUCCESS", "Halt request successfully fulfilled for core ID %lu", os->cores[request->id]->core_id);
    return RET_SUCCESS;                     // for mitigating compiler's warning
}