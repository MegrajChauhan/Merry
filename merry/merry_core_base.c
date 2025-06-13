#include "merry_core_base.h"

void merry_core_base_clean(MerryCoreBase *base) {
  merry_check_ptr(base);
  merry_mutex_destroy(&base->lock);
  merry_cond_destroy(&base->cond);
  //  merry_dynamic_queue_destroy(base->execution_queue);
  merry_destroy_dynamic_list(base->execution_states);
  free(base);
}

void merry_compare_f32(float a, float b, MerryFFlagsRegr *regr) {
  regr->zf = (a == b && a == 0.0f);
  regr->sf = ((a - b) < 0.0f);
  regr->uof = isnan(a) || isnan(b);
  regr->inv = isnan(a - b) || (a == 0.0f && signbit(a) != signbit(b));

  float diff = a - b;
  regr->of = diff > FLT_MAX;
  regr->uf = fabsf(diff) < FLT_MIN && diff != 0.0f;
}

void merry_compare_f64(double a, double b, MerryFFlagsRegr *regr) {
  regr->zf = (a == b && a == 0.0);
  regr->sf = ((a - b) < 0.0);
  regr->uof = isnan(a) || isnan(b);
  regr->inv = isnan(a - b) || (a == 0.0 && signbit(a) != signbit(b));

  double diff = a - b;
  regr->of = diff > DBL_MAX;
  regr->uf = fabs(diff) < DBL_MIN && diff != 0.0;
}
