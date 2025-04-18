#ifndef _MERRY_OS_
#define _MERRY_OS_

// This module is the backbone of the VM and controls everything

#include <merry_config.h>
#include <merry_types.h>
#include <merry_utils.h>
#include <merry_nreader.h>
#include <merry_dmemory.h>
#include <merry_memory.h>
#include <merry_request_hdlr.h>
#include <merry_core.h>
#include <merry_console.h>
#include <merry_process.h>
#include "merry_commands.h"
// #include "merry_dbg.h"
#include <stdatomic.h>
#include <stdlib.h>
#include "merry_temp.h"
#include "merry_dynl.h"
#include "merry_traps.h"
#include "merry_ihdlr.h"
#include "merry_syscall_hdlr.h"
#include "merry_helpers.h"
#include "merry_subsystem.h"
#include "merry_debug.h"

typedef struct Merry Merry;

struct Merry
{
  MerryReader *reader;
  MerryCore **cores;          // the vcores
  MerryThread **core_threads; // the vcore's threads
  MerryMemory *inst_mem;      // the instruction memory that every vcore shares
  MerryDMemory *data_mem;     // the data memory that every vcore shares
  MerryMutex *_lock;          // the Manager's lock
  MerryCond *_cond;           // the Manager's cond
  msize_t core_count;         // the number of vcores
  msize_t active_core_count;  // the number of vcores currently active
  mbool_t stop;               // tell the manager to stop the VM and exit
  msize_t ret;
  msize_t _os_id;
  mbool_t dump_on_error;
  mstr_t dump_file;
  msize_t err_core_id;
  mbool_t wait_for_conn;
  // MerryListener *listener;
  // MerrySender *sender;
  // MerryThread *listener_th;
  // MerryThread *sender_th;
  // volatile mbool_t listener_running, listener_stopped;
  // volatile mbool_t sender_running, sender_stopped;
  MerryDebug *dbg;
  MerryThread *dbg_th;
  mbool_t dbg_running;
  volatile mbool_t _subsystem_running;
  volatile mbool_t _subsystem_failure;
  MerryPipe *os_pipe;
  MerryThread *subsys_thread;
};

#define _MERRY_REQUEST_QUEUE_LEN_ 20 // for now
#define _MERRY_SUBSYS_LEN_ 10        // initial

#define _MERRY_REQUEST_INTERNAL_ERROR_(request_id) (request_id >= 0 && request_id <= 9)
#define _MERRY_REQUEST_OTHER_(request_id) (request_id >= 10 && request_id <= 50)
#define _MERRY_REQUEST_PROGRAM_ERROR_(request_id) (request_id >= 51 && request_id <= 150)
#define _MERRY_REQUEST_VALID_(req_id) (req_id >= 151)

#define merry_mem_error(msg) mreportA("MEM ERROR: %s", msg)
#define merry_internal_module_error(msg) mreportA("MODULE ERROR: %s", msg)
#define merry_general_error(type, msg) mreportA("%s: %s", type, msg)

#define merry_error(msg, ...) merry_log(_MERRY_LOG_ERR_, msg, __VA_ARGS__)

#define _MERRY_DEFAULT_DUMP_FILE_ "mdump"

#define _MERRY_EXIT_FAILURE_ 1
#define _MERRY_EXIT_SUCCESS_ 0
#define _MERRY_INITIAL_DYNLOAD_COUNT_ 10 // 10 libraries should be enough

/*
 The Manager assigns core ids to every core it manages which then helps in identifying the cores.
*/

// this only initializes an instance of Merry while leaving inst_mem, data_mem uninitialized which is valid as we need to know the input and how many
// pages to start with

static Merry os;

#define merry_manager_mem_read_inst(inst_mem, address, store_in) merry_memory_read(inst_mem, address, store_in)

mret_t merry_os_init(mcstr_t _inp_file, char **options, msize_t count, mbool_t _wait_for_conn);
mret_t merry_os_init_reader_provided(MerryReader *r);
mret_t merry_os_start_subsys();

void merry_os_start_dbg();

void merry_os_produce_dump(mstr_t _output_filename);

// This may sound NUTS but if any errors are encountered whilst writing the dump file, nothing will be reported and we continue as if nothing has happened.
mret_t merry_os_error_dump();
void merry_os_dump_mem_dets(FILE *f);
void merry_os_dump_core_dets(FILE *f);

_THRET_T_ merry_os_start_vm(mptr_t some_arg);

mret_t merry_os_add_core();
mret_t merry_os_boot_core(msize_t core_id, maddress_t start_addr);

// destroy the OS
void merry_os_destroy();

void merry_os_new_proc_cleanup();

// print the suitable error message and exit the VM
void merry_os_handle_error(merrot_t error, msize_t id);

// handle others
void merry_os_handle_others(merrot_t _id, msize_t id);

void merry_os_handle_internal_module_error(merrot_t error_num);

void merry_os_notify_dbg(mqword_t sig, mqword_t arg);

mqword_t merry_os_get_ret();

void merry_os_subsys_stopped();
void merry_os_dbg_stopped();

void merry_os_give_id(msize_t id);

#define _os_exec_(reqname) mret_t merry_os_execute_request_##reqname(Merry *os, MerryOSRequest *request)

// handle the halt request
_os_exec_(halt);
_os_exec_(new_core);
_os_exec_(mem);
_os_exec_(newprocess);
_os_exec_(bp);

_os_exec_(dynl);
_os_exec_(dynul);
_os_exec_(dyncall);

_os_exec_(syscall);
_os_exec_(get_func_addr);
_os_exec_(call_loaded_func);

_os_exec_(start_subsys);
_os_exec_(add_channel);
_os_exec_(close_channel);
_os_exec_(send);
_os_exec_(send_wait);
_os_exec_(subsys_status);
_os_exec_(geterrno);

_os_exec_(get_core_count);
_os_exec_(get_active_core_count);
_os_exec_(get_data_mem_page_count);
_os_exec_(get_inst_mem_page_count);
_os_exec_(add_breakpoint);
_os_exec_(inst_at);
_os_exec_(data_at);
_os_exec_(sp_of);
_os_exec_(bp_of);
_os_exec_(pc_of);
_os_exec_(regr_of);
_os_exec_(continue_core);
_os_exec_(bt_of);

_os_exec_(read_data_pg);
_os_exec_(read_inst_pg);

// _os_exec_(fopen);
// _os_exec_(fclose);
// _os_exec_(fread);
// _os_exec_(fwrite);
// _os_exec_(feof);
// _os_exec_(fseek);
// _os_exec_(ftell);
// _os_exec_(rewind);

#endif