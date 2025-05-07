#ifndef _MERRY_CORE_BASE_
#define _MERRY_CORE_BASE_

/**
 * What the basic structure of a core be?
 * First of all, all cores are going to be of different types. Mainly of two types to be precise:
 * 1) Built-in cores
 * 2) Custom cores
 *
 * Built-in cores are the cores that Merry will provide by default. Based on the needs of the users, Merry may expand it's built-in core types to cover different necessities.
 * The Custom cores are what the user may provide tailored to their specific needs. The Custom core must adhere to the conventions that Merry will provide.
 * 
 * Graves is the manager for the VM. It has absolutely no idea of what a core is. Graves just knows that it must manage the VM and that's all.
 * Each core must have a basic structure and a set of functions that Graves will use to initialize it.
 * 
 * */

#include "merry_types.h"
#include "merry_state.h"
#include "merry_core_types.h"
#include "merry_ram.h"
#include "merry_platform.h"
#include <float.h>
#include <math.h>

#define flags_res(x, size) unsigned long x : size

typedef struct MerryCoreBase MerryCoreBase;
typedef struct MerryFlagsRegr MerryFlagsRegr;
typedef struct MerryFFlagsRegr MerryFFlagsRegr;
// Some of the functions that the cores have to provide
// cptr is the core: Get the core details
_MERRY_DEFINE_FUNC_PTR_(MerryCoreBase*, mcoredetails_t, MerryState *state);

// Initialize the core
_MERRY_DEFINE_FUNC_PTR_(void*, mcoreinit_t, MerryCoreBase* base, MerryRAM *ram, MerryRAM *iram, maddress_t start_point);

// Destroy the core
_MERRY_DEFINE_FUNC_PTR_(void, mcoredest_t, void*);


_MERRY_DEFINE_FUNC_PTR_(_THRET_T_, mcoreexec_t, void* arg);

struct MerryCoreBase
{
  mcoreinit_t init_func;
  mcoredest_t free_func;
  mcoreexec_t exec_func;
  
  unsigned int core_id; // assigned by Graves for every core
  mbool_t stop;

  mcond_t cond;
  mmutex_t lock;
  
  mcore_t core_type;
  MerryState state; // The state of the core

  mqword_t wild_request;
  mqword_t wild_request_hdlr;
  mbool_t wild_request_hdlr_set;
};

struct MerryFFlagsRegr
{
   flags_res(zf, 1); // zero flag
   flags_res(sf, 1); // sign flag
   flags_res(uof, 1); // unordered flag
   flags_res(of, 1); // overflow flag
   flags_res(uf, 1); // underflow flag
   flags_res(inv, 1); // invalid flag
   flags_res(res, 2); // reserved
};

struct MerryFlagsRegr
{
#if defined(_MERRY_HOST_CPU_x86_64_ARCH_)
    // This defines the flags structure for the AMD64 processors
    unsigned long carry : 1;     /*0th bit is the CF in AMD64 EFlags*/
    flags_res(r1, 1);            /*1 bit reserved here*/
    unsigned long parity : 1;    /*2th bit is the PF*/
    flags_res(r2, 1);            /*1 bit reserved here*/
    unsigned long aux_carry : 1; /*4th bit Aux Carry flag[NOT REALLY NEEDED AS BCD INSTRUCTIONS ARE NOT SUPPORTED]*/
    flags_res(r3, 1);            /*1 bit reserved here*/
    unsigned long zero : 1;      /*6th bit ZF*/
    unsigned long negative : 1;  /*7th bit SF or NG*/
    flags_res(r4, 2);            /*2 bit reserved here*/
    unsigned long direction : 1; /*11th bit is the DF[NOT REALLY USEFUL YET BUT MAYBE WHEN IMPLEMENTING STRING RELATED INSTRUCTIONS]*/
    unsigned long overflow : 1;  /*10th bit is the OF*/
    flags_res(rem_32, 20);
    flags_res(top_32, 32);
#endif
};

extern void merry_update_flags_regr(MerryFlagsRegr *reg);
extern void merry_compare_two_values(mqword_t v1, mqword_t v2, MerryFlagsRegr *reg);

void merry_core_base_clean(MerryCoreBase *base);

void merry_compare_f32(float a, float b, MerryFFlagsRegr *regr);
void merry_compare_f64(double a, double b, MerryFFlagsRegr *regr);

#endif
