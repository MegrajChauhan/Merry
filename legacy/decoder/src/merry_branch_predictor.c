#include "../branch_predictor/merry_branch_predictor.h"
#include <stdio.h>

mbool_t merry_predict_branch(MerryPredictor *predictor, maddress_t address)
{
    register mbyte_t counter = predictor->indexes[predictor->prediction_counter & 7].saturation_counter[address & 7];
    register mbool_t prediction = BR_TAKEN;
    if ((counter & 3) == 3)
    {
        prediction = BR_NTAKEN;
        predictor->prediction_counter--;
    }
    else
        predictor->prediction_counter++;
    predictor->last_prediction_point = address;
    predictor->_last_prediction = prediction;
    return prediction;
}

void merry_prediction_wrong(MerryPredictor *predictor)
{
    // since the last prediction turned out to be wrong
    /// NOTE: This function has a major flaw. If two or more predictions were made before the wrong prediction was discovered, we "correct" the wrong counter
    /// TODO: remove this
    printf("Oh crap!\n"); 
    predictor->indexes[(predictor->prediction_counter + (predictor->_last_prediction == BR_TAKEN) ? 1 : -1) & 7].saturation_counter[predictor->last_prediction_point & 7]++;
}

void merry_predictor_init(MerryPredictor *predictor)
{
    // Seed the random number generator with the current time
    srand((unsigned int)time(NULL));

    // Initialize prediction_counter to a random number
    predictor->prediction_counter = RANDOM_VALUE;

    // Initialize saturation counters in each index to random values
    for (int i = 0; i < 8; ++i) {
        for (int j = 0; j < 8; ++j) {
            predictor->indexes[i].saturation_counter[j] = RANDOM_VALUE;
        }
    }

    // Initialize other fields
    predictor->_last_prediction = 0; 
    predictor->last_prediction_point = 0;
}
