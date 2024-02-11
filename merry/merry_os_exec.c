#include "internals/merry_os_exec.h"

_os_exec_(halt)
{
    // here we have to stop the core first
    // since the core is waiting, we don't have to worry about mutex locks.
    // since the core's decoder won't mess with other fields of the core, we can freely make changes
    _llog_(_OS_, "Request", " Fulfilling the halt request: Requester %d", request->id);
    os->cores[request->id]->stop_running = mtrue; // this will automatically halt the core's decoder as well
    if (os->core_count == 1) // I am an idiot
    {
        // we had only one core to begin with then stop any further execution
        os->stop = mtrue;
        // the core that makes this request should have the return value in Ma register
        os->ret = os->cores[request->id]->registers[Ma];
    }
    printf("Halting.\n"); /// TODO: remove this
    _llog_(_OS_, "REQ_SUCCESS", "Halt request successfully fulfilled for core ID %lu", os->cores[request->id]->core_id);
    return RET_SUCCESS; // for mitigating compiler's warning
}

_os_exec_(new_core)
{
    // generate a new core
    _llog_(_OS_, "Request", " Creating a new core: Requester %d", request->id);
    if (merry_os_add_core() == RET_FAILURE)
    {
        // let the core know that its request was a failure
        os->cores[request->id]->registers[Ma] = 1; // Ma should contain the address and it will be updated with the result of the request
        _llog_(_OS_, "Request", "Creation of a new core failed: Requester %d", request->id);
    }
    else
    {
        os->cores[request->id]->registers[Ma] = merry_os_boot_core(os->core_count - 1, os->cores[request->id]->registers[Ma]);
        _llog_(_OS_, "Request", " Successfully Created a new core: Requester %d", request->id);
    }
    return RET_SUCCESS; // for now
}