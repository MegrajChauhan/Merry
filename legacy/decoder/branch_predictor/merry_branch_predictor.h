#ifndef _MERRY_BRANCH_PREDICTOR_
#define _MERRY_BRANCH_PREDICTOR_

/*
 This is the branch predictor for Merry.
 Merry is going to have a Local branch predictor with 2-bit saturating counters
*/
#include "../../../../utils/merry_types.h"
#include <stdlib.h>
#include <time.h>

#define RANDOM_VALUE ((mbyte_t)rand())

/*
  [prediction counter] -|
                        |->[Counters: indexed by PC] -|
                        |->                           |->[2-bit counter]
                        |->                           |->
                        |->                           |->
                        |->                           |->
                        |->                           |->
                        |->                           |->
                        |->                           |->
*/

#define _COUNTERS_PER_INDEX_ 8
#define _INDEX_COUNT_ 8

#define BR_TAKEN mtrue
#define BR_NTAKEN mfalse

typedef struct _LocalIndex_
{
    mbyte_t saturation_counter[_COUNTERS_PER_INDEX_];
} _LocalIndex_;

typedef struct MerryPredictor
{
    _LocalIndex_ indexes[_INDEX_COUNT_];
    mbyte_t prediction_counter;
    mbool_t _last_prediction;
    maddress_t last_prediction_point;
} MerryPredictor;

void merry_predictor_init(MerryPredictor *predictor);

mbool_t merry_predict_branch(MerryPredictor *predictor, maddress_t address);

// tell the predictor that the last prediction was wrong
void merry_prediction_wrong(MerryPredictor *predictor);

#endif