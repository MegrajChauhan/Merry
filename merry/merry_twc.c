#include <merry_twc.h>

mret_t merry_twc_base_form(MerryTWCBase *base, MerryState *state) {
  merry_check_ptr(base);
  if (merry_open_pipe(&base->lines._read_line_for_child,
                      &base->lines._write_line_for_parent) != RET_FAILURE) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_OPEN_PIPE_);
    return RET_FAILURE;
  }
  if (merry_open_pipe(&base->lines._read_line_for_parent,
                      &base->lines._write_line_for_child) != RET_FAILURE) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_OPEN_PIPE_);
    close(base->lines._read_line_for_child);
    close(base->lines._write_line_for_parent);
    return RET_FAILURE;
  }
  return RET_SUCCESS;
}

MerryTWC *merry_twc_create(mdataline_t rline, mdataline_t wline,
                           MerryState *state) {
  MerryTWC *twc = (MerryTWC *)malloc(sizeof(MerryTWC));
  if (!twc) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    return RET_NULL;
  }
  merry_owc_init(&twc->rc, rline, 0, mfalse, mtrue);
  merry_owc_init(&twc->wc, 0, wline, mtrue, mfalse);
  merry_assign_state(twc->state, _MERRY_ORIGIN_NONE_, 0);
  twc->state.child_state = NULL;
  return twc;
}

mret_t merry_twc_send(MerryTWC *twc, mbptr_t data, msize_t len) {
  merry_check_ptr(twc);
  merry_check_ptr(data);

  if (surelyF(len == 0))
    return RET_SUCCESS;

  mret_t res = merry_owc_speak(&twc->wc, data, len);
  twc->state = twc->wc.state;
  return res;
}

mret_t merry_twc_receive(MerryTWC *twc, mbptr_t buf, msize_t n) {
  merry_check_ptr(twc);
  merry_check_ptr(buf);

  if (surelyF(n == 0))
    return RET_SUCCESS;

  mret_t res = merry_owc_listen(&twc->rc, buf, n);
  twc->state = twc->rc.state;
  return res;
}

void merry_twc_destroy(MerryTWC *twc) {
  merry_check_ptr(twc);
  merry_owc_only_listen(&twc->wc);
  merry_owc_only_speak(&twc->rc);
  free(twc);
}
