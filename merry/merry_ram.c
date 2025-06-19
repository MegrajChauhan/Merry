#include "merry_ram.h"

MerryRAM *merry_create_RAM(msize_t number_of_pages, MerryState *state) {
  merry_assert(number_of_pages != 0);

  MerryRAM *ram = (MerryRAM *)malloc(sizeof(MerryRAM));

  if (!ram) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    return RET_NULL;
  }

  MerryNormalMemoryPage **pages = (MerryNormalMemoryPage **)malloc(
      sizeof(MerryNormalMemoryPage *) * number_of_pages);
  if (!pages) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    free(ram);
    return RET_NULL;
  }

  msize_t i = 0;
  for (; i < number_of_pages; i++) {
    if ((pages[i] = merry_create_normal_memory_page()) == RET_NULL) {
      merry_obtain_memory_interface_state(state);
      goto __rid_of_during_error;
    }
  }

  ram->pages = pages;
  ram->page_count = number_of_pages;
  if (merry_mutex_init(&ram->lock) == RET_FAILURE) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FAILED_TO_OBTAIN_LOCK_);
    goto __rid_of_during_error;
  }

  return ram;
__rid_of_during_error: // not the best names :(
  for (msize_t j = 0; j < i; j++) {
    merry_return_normal_memory_page(pages[j]);
  }
  free(pages);
  free(ram);
  return RET_NULL;
}

mret_t merry_RAM_add_pages(MerryRAM *ram, msize_t num, MerryState *state) {
  // if failed, we won't invalidate the current page buffer
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);

  if (surelyF(num == 0))
    return RET_SUCCESS;

  register msize_t temp = ram->page_count + num;
  MerryNormalMemoryPage **pages =
      (MerryNormalMemoryPage **)malloc(sizeof(MerryNormalMemoryPage *) * temp);
  if (!pages) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    merry_obtain_memory_interface_state(state->child_state);
    return RET_FAILURE;
  }

  msize_t i = 0;
  for (i = ram->page_count; i < temp; i++) {
    if ((pages[i] = merry_create_normal_memory_page()) == RET_NULL) {
      merry_obtain_memory_interface_state(state);
      goto __rid_of_during_error;
    }
  }

  memcpy(pages, ram->pages, sizeof(MerryNormalMemoryPage *) * ram->page_count);
  for (msize_t j = ram->page_count; j < temp; j++) {
    if (merry_initialize_normal_memory_page(pages[j]) == RET_FAILURE) {
      merry_obtain_memory_interface_state(state);
      goto __rid_of_during_error;
    }
  }
  ram->page_count = temp;
  free(ram->pages);
  ram->pages = pages;
  return RET_SUCCESS;

__rid_of_during_error:
  for (msize_t j = ram->page_count; j < i; j++) {
    merry_return_normal_memory_page(pages[j]);
  }
  free(pages);
  return RET_FAILURE;
}

mret_t merry_RAM_read_byte(MerryRAM *ram, maddress_t address, mbptr_t store_in,
                           MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_check_ptr(store_in);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  *store_in = ram->pages[page_num]->buf[page_off];
  return RET_SUCCESS;
}

mret_t merry_RAM_read_word(MerryRAM *ram, maddress_t address, mwptr_t store_in,
                           MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_check_ptr(store_in);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;
  register MerryHostMemLayout layout;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  switch (page_off) {
  case _MERRY_PAGE_LEN_ - 1: {
    if (surelyF((page_num + 1) > ram->page_count) ||
        !ram->pages[page_num + 1]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }
    layout.bytes.b0 = ram->pages[page_num]->buf[page_off];
    layout.bytes.b1 = ram->pages[page_num + 1]->buf[0];
    break;
  }
  default:
    layout.half_half_words.w0 =
        *(mwptr_t)(ram->pages[page_num]->buf + page_off);
    break;
  }

  *store_in = layout.half_half_words.w0;
  return RET_SUCCESS;
}

mret_t merry_RAM_read_dword(MerryRAM *ram, maddress_t address, mdptr_t store_in,
                            MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_check_ptr(store_in);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;
  register MerryHostMemLayout layout;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  switch (page_off) {

  case _MERRY_PAGE_LEN_ - 3:
  case _MERRY_PAGE_LEN_ - 2:
  case _MERRY_PAGE_LEN_ - 1: {
    if (surelyF((page_num + 1) > ram->page_count) ||
        !ram->pages[page_num + 1]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }
    for (msize_t i = page_off; i < _MERRY_PAGE_LEN_; i++) {
      layout.half_words.w0 |= ram->pages[page_num]->buf[i];
      layout.half_words.w0 <<= 8;
    }
    page_num++;
    msize_t i = 0;
    for (; i < 3 - (_MERRY_PAGE_LEN_ - page_off); i++) {
      layout.half_words.w0 |= ram->pages[page_num]->buf[i];
      layout.half_words.w0 <<= 8;
    }
    layout.half_words.w0 |= ram->pages[page_num]->buf[i];
    break;
  }
  default:
    layout.half_words.w0 = *(mdptr_t)(ram->pages[page_num]->buf + page_off);
    break;
  }

  *store_in = layout.half_words.w0;
  return RET_SUCCESS;
}

mret_t merry_RAM_read_qword(MerryRAM *ram, maddress_t address, mqptr_t store_in,
                            MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_check_ptr(store_in);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;
  register MerryHostMemLayout layout;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  switch (page_off) {

  case _MERRY_PAGE_LEN_ - 7:
  case _MERRY_PAGE_LEN_ - 6:
  case _MERRY_PAGE_LEN_ - 5:
  case _MERRY_PAGE_LEN_ - 4:
  case _MERRY_PAGE_LEN_ - 3:
  case _MERRY_PAGE_LEN_ - 2:
  case _MERRY_PAGE_LEN_ - 1: {
    if (surelyF((page_num + 1) > ram->page_count) ||
        !ram->pages[page_num + 1]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }
    for (msize_t i = page_off; i < _MERRY_PAGE_LEN_; i++) {
      layout.whole_word |= ram->pages[page_num]->buf[i];
      layout.whole_word <<= 8;
    }
    page_num++;
    msize_t i = 0;
    for (; i < 7 - (_MERRY_PAGE_LEN_ - page_off); i++) {
      layout.whole_word |= ram->pages[page_num]->buf[i];
      layout.whole_word <<= 8;
    }
    layout.whole_word |= ram->pages[page_num]->buf[i];
    break;
  }
  default:
    layout.whole_word = *(mqptr_t)(ram->pages[page_num]->buf + page_off);
    break;
  }
  *store_in = layout.whole_word;
  return RET_SUCCESS;
}

mret_t merry_RAM_write_byte(MerryRAM *ram, maddress_t address, mbyte_t value,
                            MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count || !ram->pages[page_num]->init)) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  ram->pages[page_num]->buf[page_off] = value;
  return RET_SUCCESS;
}

mret_t merry_RAM_write_word(MerryRAM *ram, maddress_t address, mword_t value,
                            MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  switch (page_off) {
  case _MERRY_PAGE_LEN_ - 1: {
    register MerryHostMemLayout layout;
    layout.half_half_words.w0 = value;

    if (surelyF((page_num + 1) > ram->page_count) ||
        !ram->pages[page_num + 1]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }
    ram->pages[page_num]->buf[page_off] = layout.bytes.b0;
    ram->pages[page_num + 1]->buf[0] = layout.bytes.b1;
    break;
  }
  default:
    *(mwptr_t)(ram->pages[page_num]->buf + page_off) = value;
    break;
  }

  return RET_SUCCESS;
}

mret_t merry_RAM_write_dword(MerryRAM *ram, maddress_t address, mdword_t value,
                             MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  switch (page_off) {

  case _MERRY_PAGE_LEN_ - 3:
  case _MERRY_PAGE_LEN_ - 2:
  case _MERRY_PAGE_LEN_ - 1: {
    register MerryHostMemLayout layout;
    layout.half_words.w0 = value;

    if (surelyF((page_num + 1) > ram->page_count) ||
        !ram->pages[page_num + 1]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }
    for (msize_t i = page_off; i < _MERRY_PAGE_LEN_; i++) {
      ram->pages[page_num]->buf[i] = layout.half_words.w0 & 255;
      layout.half_words.w0 >>= 8;
    }
    page_num++;
    msize_t i = 0;
    for (; i < 4 - (_MERRY_PAGE_LEN_ - page_off); i++) {
      ram->pages[page_num]->buf[i] = layout.half_words.w0 & 255;
      layout.half_words.w0 >>= 8;
    }
    break;
  }
  default:
    *(mdptr_t)(ram->pages[page_num]->buf + page_off) = value;
    break;
  }

  return RET_SUCCESS;
}

mret_t merry_RAM_write_qword(MerryRAM *ram, maddress_t address, mqword_t value,
                             MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  switch (page_off) {

  case _MERRY_PAGE_LEN_ - 7:
  case _MERRY_PAGE_LEN_ - 6:
  case _MERRY_PAGE_LEN_ - 5:
  case _MERRY_PAGE_LEN_ - 4:
  case _MERRY_PAGE_LEN_ - 3:
  case _MERRY_PAGE_LEN_ - 2:
  case _MERRY_PAGE_LEN_ - 1: {
    register MerryHostMemLayout layout;
    layout.whole_word = value;

    if (surelyF((page_num + 1) > ram->page_count) ||
        !ram->pages[page_num + 1]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }

    for (msize_t i = page_off; i < _MERRY_PAGE_LEN_; i++) {
      ram->pages[page_num]->buf[i] = layout.whole_word & 255;
      layout.whole_word >>= 8;
    }
    page_num++;
    msize_t i = 0;
    for (; i < 8 - (_MERRY_PAGE_LEN_ - page_off); i++) {
      ram->pages[page_num]->buf[i] = layout.whole_word & 255;
      layout.whole_word >>= 8;
    }
    break;
  }
  default:
    *(mqptr_t)(ram->pages[page_num]->buf + page_off) = value;
    break;
  }

  return RET_SUCCESS;
}

void merry_destroy_RAM(MerryRAM *ram) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);

  for (msize_t i = 0; i < ram->page_count; i++) {
    merry_check_ptr(ram->pages[i]);
    merry_return_normal_memory_page(ram->pages[i]);
  }
  merry_mutex_destroy(&ram->lock);
  free(ram->pages);
  free(ram);
}

mbptr_t merry_RAM_bulk_read(MerryRAM *ram, maddress_t address, msize_t length,
                            MerryState *state) {
  merry_check_ptr(ram);
  if (length == 0) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_BULK_OPERATION_CANNOT_BE_DONE_ON_LENGTH_ZERO_);
    return RET_NULL;
  }

  mbptr_t buf = (mbptr_t)malloc(length);
  mbptr_t iter = buf;

  if (!buf) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    return RET_NULL;
  }

  msize_t pg_num;
  msize_t off;

  merry_deduce_address(address, pg_num, off);
  while (length > 0) {
    if (surelyF(pg_num >= ram->page_count) ||
        ram->pages[pg_num]->init == mfalse) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      free(buf);
      return RET_NULL;
    }

    if (length > _MERRY_PAGE_LEN_) {
      memcpy(iter, (char *)ram->pages[pg_num]->buf + off, _MERRY_PAGE_LEN_);
      pg_num++;
      length -= _MERRY_PAGE_LEN_;
      off = 0;
      iter += _MERRY_PAGE_LEN_;
    } else {
      memcpy(iter, (char *)ram->pages[pg_num]->buf + off, length);
      pg_num++;
      iter += length;
      length -= length;
      off = 0;
    }
  }
  return buf;
}

mret_t merry_RAM_bulk_write(MerryRAM *ram, maddress_t address, msize_t length,
                            mbptr_t to_write, MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(to_write);

  if (length == 0) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_BULK_OPERATION_CANNOT_BE_DONE_ON_LENGTH_ZERO_);
    return RET_FAILURE;
  }

  mbptr_t iter = to_write;

  msize_t pg_num;
  msize_t off;

  merry_deduce_address(address, pg_num, off);

  while (length > 0) {
    if (surelyF(pg_num >= ram->page_count) || !ram->pages[pg_num]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }

    if (length > _MERRY_PAGE_LEN_) {
      memcpy((char *)ram->pages[pg_num]->buf + off, iter, _MERRY_PAGE_LEN_);
      ;
      pg_num++;
      length -= _MERRY_PAGE_LEN_;
      off = 0;
      iter += _MERRY_PAGE_LEN_;
    } else {
      memcpy((char *)ram->pages[pg_num]->buf + off, iter, length);
      pg_num++;
      iter += length;
      length -= length;
      off = 0;
    }
  }

  return RET_SUCCESS;
}

mret_t merry_RAM_read_byte_atm(MerryRAM *ram, maddress_t address,
                               mbptr_t store_in, MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_check_ptr(store_in);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  *store_in = atomic_load((atomic_char *)&ram->pages[page_num]->buf[page_off]);
  return RET_SUCCESS;
}

mret_t merry_RAM_read_word_atm(MerryRAM *ram, maddress_t address,
                               mwptr_t store_in, MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_check_ptr(store_in);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;
  register MerryHostMemLayout layout;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  switch (page_off) {
  case _MERRY_PAGE_LEN_ - 1: {
    if (surelyF((page_num + 1) > ram->page_count) ||
        !ram->pages[page_num + 1]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }
    layout.bytes.b0 =
        atomic_load((atomic_char *)&ram->pages[page_num]->buf[page_off]);
    layout.bytes.b1 =
        atomic_load((atomic_char *)&ram->pages[page_num + 1]->buf[0]);
    break;
  }
  default:
    layout.half_half_words.w0 =
        atomic_load((atomic_char16_t *)(ram->pages[page_num]->buf + page_off));
    break;
  }

  *store_in = layout.half_half_words.w0;
  return RET_SUCCESS;
}

mret_t merry_RAM_read_dword_atm(MerryRAM *ram, maddress_t address,
                                mdptr_t store_in, MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_check_ptr(store_in);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;
  register MerryHostMemLayout layout;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  switch (page_off) {

  case _MERRY_PAGE_LEN_ - 3:
  case _MERRY_PAGE_LEN_ - 2:
  case _MERRY_PAGE_LEN_ - 1: {
    if (surelyF((page_num + 1) > ram->page_count) ||
        !ram->pages[page_num + 1]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }
    for (msize_t i = page_off; i < _MERRY_PAGE_LEN_; i++) {
      layout.half_words.w0 |=
          atomic_load((atomic_char *)&ram->pages[page_num]->buf[i]);
      layout.half_words.w0 <<= 8;
    }
    page_num++;
    msize_t i = 0;
    for (; i < 3 - (_MERRY_PAGE_LEN_ - page_off); i++) {
      layout.half_words.w0 |=
          atomic_load((atomic_char *)&ram->pages[page_num]->buf[i]);
      layout.half_words.w0 <<= 8;
    }
    layout.half_words.w0 |=
        atomic_load((atomic_char *)&ram->pages[page_num]->buf[i]);
    break;
  }
  default:
    layout.half_words.w0 =
        atomic_load((atomic_int *)(ram->pages[page_num]->buf + page_off));
    break;
  }

  *store_in = layout.half_words.w0;
  return RET_SUCCESS;
}

mret_t merry_RAM_read_qword_atm(MerryRAM *ram, maddress_t address,
                                mqptr_t store_in, MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_check_ptr(store_in);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;
  register MerryHostMemLayout layout;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  switch (page_off) {

  case _MERRY_PAGE_LEN_ - 7:
  case _MERRY_PAGE_LEN_ - 6:
  case _MERRY_PAGE_LEN_ - 5:
  case _MERRY_PAGE_LEN_ - 4:
  case _MERRY_PAGE_LEN_ - 3:
  case _MERRY_PAGE_LEN_ - 2:
  case _MERRY_PAGE_LEN_ - 1: {
    if (surelyF((page_num + 1) > ram->page_count) ||
        !ram->pages[page_num + 1]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }
    for (msize_t i = page_off; i < _MERRY_PAGE_LEN_; i++) {
      layout.whole_word |=
          atomic_load((atomic_char *)&ram->pages[page_num]->buf[i]);
      layout.whole_word <<= 8;
    }
    page_num++;
    msize_t i = 0;
    for (; i < 7 - (_MERRY_PAGE_LEN_ - page_off); i++) {
      layout.whole_word |=
          atomic_load((atomic_char *)&ram->pages[page_num]->buf[i]);
      layout.whole_word <<= 8;
    }
    layout.whole_word |=
        atomic_load((atomic_char *)&ram->pages[page_num]->buf[i]);
    break;
  }
  default:
    layout.whole_word =
        atomic_load((atomic_long *)(ram->pages[page_num]->buf + page_off));
    break;
  }

  *store_in = layout.whole_word;
  return RET_SUCCESS;
}

mret_t merry_RAM_write_byte_atm(MerryRAM *ram, maddress_t address,
                                mbyte_t value, MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count || !ram->pages[page_num]->init)) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }
  atomic_store((atomic_char *)&ram->pages[page_num]->buf[page_off], value);
  return RET_SUCCESS;
}

mret_t merry_RAM_write_word_atm(MerryRAM *ram, maddress_t address,
                                mword_t value, MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  switch (page_off) {
  case _MERRY_PAGE_LEN_ - 1: {
    register MerryHostMemLayout layout;
    layout.half_half_words.w0 = value;

    if (surelyF((page_num + 1) > ram->page_count) ||
        !ram->pages[page_num + 1]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }
    atomic_store((atomic_char *)&ram->pages[page_num]->buf[page_off],
                 layout.bytes.b0);
    atomic_store((atomic_char *)&ram->pages[page_num + 1]->buf[0],
                 layout.bytes.b1);
    break;
  }
  default:
    atomic_store((atomic_short *)(ram->pages[page_num]->buf + page_off), value);
    break;
  }

  return RET_SUCCESS;
}

mret_t merry_RAM_write_dword_atm(MerryRAM *ram, maddress_t address,
                                 mdword_t value, MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  switch (page_off) {

  case _MERRY_PAGE_LEN_ - 3:
  case _MERRY_PAGE_LEN_ - 2:
  case _MERRY_PAGE_LEN_ - 1: {
    register MerryHostMemLayout layout;
    layout.half_words.w0 = value;

    if (surelyF((page_num + 1) > ram->page_count) ||
        !ram->pages[page_num + 1]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }
    for (msize_t i = page_off; i < _MERRY_PAGE_LEN_; i++) {
      atomic_store((atomic_char *)&ram->pages[page_num]->buf[i],
                   layout.half_words.w0 & 255);
      layout.half_words.w0 >>= 8;
    }
    page_num++;
    msize_t i = 0;
    for (; i < 4 - (_MERRY_PAGE_LEN_ - page_off); i++) {
      atomic_store((atomic_char *)&ram->pages[page_num]->buf[i],
                   layout.half_words.w0 & 255);
      layout.half_words.w0 >>= 8;
    }
    break;
  }
  default:
    atomic_store((atomic_int *)(ram->pages[page_num]->buf + page_off), value);
    break;
  }

  return RET_SUCCESS;
}

mret_t merry_RAM_write_qword_atm(MerryRAM *ram, maddress_t address,
                                 mqword_t value, MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  switch (page_off) {

  case _MERRY_PAGE_LEN_ - 7:
  case _MERRY_PAGE_LEN_ - 6:
  case _MERRY_PAGE_LEN_ - 5:
  case _MERRY_PAGE_LEN_ - 4:
  case _MERRY_PAGE_LEN_ - 3:
  case _MERRY_PAGE_LEN_ - 2:
  case _MERRY_PAGE_LEN_ - 1: {
    register MerryHostMemLayout layout;
    layout.whole_word = value;

    if (surelyF((page_num + 1) > ram->page_count) ||
        !ram->pages[page_num + 1]->init) {
      merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_PAGE_FAULT_);
      return RET_FAILURE;
    }

    for (msize_t i = page_off; i < _MERRY_PAGE_LEN_; i++) {
      atomic_store((atomic_char *)&ram->pages[page_num]->buf[i],
                   layout.whole_word & 255);
      layout.whole_word >>= 8;
    }
    page_num++;
    msize_t i = 0;
    for (; i < 8 - (_MERRY_PAGE_LEN_ - page_off); i++) {
      atomic_store((atomic_char *)&ram->pages[page_num]->buf[i],
                   layout.whole_word & 255);
      layout.whole_word >>= 8;
    }
    break;
  }
  default:
    atomic_store((atomic_long *)(ram->pages[page_num]->buf + page_off), value);
    break;
  }

  return RET_SUCCESS;
}

mret_t merry_RAM_cmpxchg(MerryRAM *ram, maddress_t address, mbyte_t expected,
                         mbyte_t desired, MerryState *state) {
  merry_check_ptr(ram);
  merry_check_ptr(ram->pages);
  merry_assert(ram->page_count != 0);

  register msize_t page_num;
  register msize_t page_off;

  merry_deduce_address(address, page_num, page_off);

  if (surelyF(page_num >= ram->page_count) || !ram->pages[page_num]->init) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  atomic_compare_exchange_strong(
      (atomic_char *)&ram->pages[page_num]->buf[page_off], (char *)&expected,
      desired);
  return RET_SUCCESS;
}
