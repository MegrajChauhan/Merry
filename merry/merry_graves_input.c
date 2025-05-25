#include "merry_graves_input.h"

mbool_t merry_graves_reader_confirm_input_file(MerryGravesInput *reader) {
  merry_check_ptr(reader);

  reader->fd = fopen(reader->file_path, "rb");
  if (!reader->fd) {
    if (errno == EISDIR) {
      merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_INPUT_FILE_IS_A_DIR_);
      return mfalse;
    }
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_INPUT_FILE_DOESNT_EXIST_);
    return mfalse;
  }

  fseek(reader->fd, 0, SEEK_END);
  reader->file_size = ftell(reader->fd);
  rewind(reader->fd);

  if (reader->file_size < 64) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_INPUT_FILE_DOESNT_FIT_FORMAT_);
    fclose(reader->fd);
    return mfalse;
  }

  for (msize_t i = 0; i < __CORE_TYPE_COUNT; i++) {
    reader->_instruction_for_core_already_read[i] = mfalse;
  }

  return mtrue;
}

MerryGravesInput *merry_graves_initialize_reader(mstr_t inp_path,
                                                 MerryState *state) {
  merry_check_ptr(inp_path);

  MerryGravesInput *reader =
      (MerryGravesInput *)malloc(sizeof(MerryGravesInput));

  if (!reader) {
    merry_assign_state(*state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    return RET_NULL;
  }

  reader->file_path = inp_path;

  if (!merry_graves_reader_confirm_input_file(reader)) {
    *state = reader->state;
    free(reader);
    return RET_NULL;
  }

  for (msize_t i = 0; i < __CORE_TYPE_COUNT; i++) {
    reader->iram[i] = NULL;
    reader->instruction_offsets[i] = NULL;
    reader->_instruction_for_core_already_read[i] = mfalse;
  }
  reader->data_ram = NULL;
  reader->data_offsets = NULL;
  reader->string_offsets = NULL;
  merry_assign_state(reader->state, _MERRY_ORIGIN_NONE_, 0);
  reader->state.child_state = NULL;
  return reader;
}

mret_t merry_graves_reader_read_input(MerryGravesInput *reader) {
  merry_check_ptr(reader);

  if (merry_graves_reader_parse_identification_header(reader) == RET_FAILURE)
    return RET_FAILURE;
  if (merry_graves_reader_parse_ITIT_header(reader) == RET_FAILURE)
    return RET_FAILURE;
  if (merry_graves_reader_parse_data_and_string_header(reader) == RET_FAILURE)
    return RET_FAILURE;
  if (merry_graves_reader_perform_checksum(reader) == RET_FAILURE)
    return RET_FAILURE;
  if (merry_graves_reader_parse_ITIT(reader) == RET_FAILURE)
    return RET_FAILURE;
  if (merry_graves_reader_parse_instruction_sections(reader) == RET_FAILURE)
    return RET_FAILURE;
  if (merry_graves_reader_parse_data_type_metadata(reader) == RET_FAILURE)
    return RET_FAILURE;
  if (merry_graves_reader_parse_data_and_string_section(reader) == RET_FAILURE)
    return RET_FAILURE;
  if (merry_graves_reader_prep_memory(reader) == RET_FAILURE)
    return RET_FAILURE;
  if (merry_graves_reader_load_instructions(
          reader, reader->itit.entries[0].type, 0) == RET_FAILURE)
    return RET_FAILURE;
  if (merry_graves_reader_load_data(reader, 0) == RET_FAILURE)
    return RET_FAILURE;
  return RET_SUCCESS;
}

void merry_graves_reader_destroy(MerryGravesInput *reader) {
  merry_check_ptr(reader);
  merry_check_ptr(reader->fd);

  for (msize_t i = 0; i < __CORE_TYPE_COUNT; i++) {
    if (reader->iram[i] != NULL)
      merry_destroy_RAM(reader->iram[i]);
    if (reader->instruction_offsets[i] != NULL)
      free(reader->instruction_offsets[i]);
  }

  if (reader->data_ram != NULL)
    merry_destroy_RAM(reader->data_ram);
  if (reader->itit.entries != NULL)
    free(reader->itit.entries);
  if (reader->data_offsets != NULL)
    free(reader->data_offsets);
  if (reader->string_offsets != NULL)
    free(reader->string_offsets);

  fclose(reader->fd);
  free(reader);
}

mret_t
merry_graves_reader_parse_identification_header(MerryGravesInput *reader) {
  merry_check_ptr(reader);

  mbyte_t magic_bytes[4] = {0};
  fread((void *)magic_bytes, 1, 3, reader->fd);

  if (strcmp((mstr_t)magic_bytes, "beb") != 0) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_UNKNOWN_INPUT_FILE_);
    return RET_FAILURE;
  }

  fread(&reader->metadata.type, 1, 1, reader->fd);

  switch (reader->metadata.type) {
  case _NORMAL_INPUT_FILE:
    break;
  default:
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_CANNOT_DEDUCE_FILE_TYPE_);
    return RET_FAILURE;
  }

  msize_t tmp = 0;

  fread(&tmp, 1, 4, reader->fd);

  return RET_SUCCESS;
}

mret_t merry_graves_reader_parse_ITIT_header(MerryGravesInput *reader) {
  merry_check_ptr(reader);
  MerryHostMemLayout le;
  le.whole_word = 0;
  fread(&le.whole_word, 8, 1, reader->fd);
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
  merry_LITTLE_ENDIAN_to_BIG_ENDIAN(&le);
#endif
  reader->metadata.ITIT_len = le.whole_word;

  if ((reader->metadata.ITIT_len % 16) != 0) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MISALIGNED_ITIT_HEADER_);
    return RET_FAILURE;
  }
  if ((reader->metadata.ITIT_len / 16) > __CORE_TYPE_COUNT) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_FRAGMENTED_INSTRUCTION_SECTION_);
    return RET_FAILURE;
  }

  return RET_SUCCESS;
}

mret_t
merry_graves_reader_parse_data_and_string_header(MerryGravesInput *reader) {
  merry_check_ptr(reader);
  MerryHostMemLayout le;
  fread(&le.whole_word, 8, 1, reader->fd);
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
  merry_LITTLE_ENDIAN_to_BIG_ENDIAN(&le);
#endif
  reader->metadata.data_section_len = le.whole_word;
  le.whole_word = 0;
  fread(&le.whole_word, 8, 1, reader->fd);
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
  merry_LITTLE_ENDIAN_to_BIG_ENDIAN(&le);
#endif
  reader->metadata.string_section_len = le.whole_word;

  return RET_SUCCESS;
}

mret_t merry_graves_reader_perform_checksum(MerryGravesInput *reader) {
  merry_check_ptr(reader);

  // We read the DI length in here since only the length is of interest to us.
  MerryHostMemLayout le;
  fread(&le.whole_word, 8, 1, reader->fd);
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
  merry_LITTLE_ENDIAN_to_BIG_ENDIAN(&le);
#endif
  reader->metadata.DI_len = le.whole_word;

  if ((reader->metadata.DI_len + reader->metadata.ITIT_len +
       reader->metadata.data_section_len + reader->metadata.string_section_len +
       56) >= reader->file_size) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_INVALID_FILE_STRUCTURE_DOESNT_MATCH_HEADER_INFO_);
    return RET_FAILURE;
  }
  /// NOTE: This is not the correct check actually.
  /// We haven't even read the length of each instruction section.
  /// Thus, this is a pre-check.
  return RET_SUCCESS;
}

mret_t merry_graves_reader_parse_ITIT(MerryGravesInput *reader) {
  merry_check_ptr(reader);

  reader->itit.entry_count = reader->metadata.ITIT_len / 16;
  reader->itit.entries = (MerryITITEntry *)malloc(sizeof(MerryITITEntry) *
                                                  reader->itit.entry_count);
  if (!reader->itit.entries) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    return RET_FAILURE;
  }

  reader->metadata.total_instructions_len = 0;

  // Now we have to parse each of the section.
  for (msize_t i = 0; i < reader->itit.entry_count; i++) {
    MerryHostMemLayout c_type;
    MerryHostMemLayout section_len;
    fread(&c_type.whole_word, 8, 1, reader->fd);
    fread(&section_len.whole_word, 8, 1, reader->fd);
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
    merry_LITTLE_ENDIAN_to_BIG_ENDIAN(&c_type);
    merry_LITTLE_ENDIAN_to_BIG_ENDIAN(&section_len);
#endif
    c_type.bytes.b7 = c_type.bytes.b7 & __CORE_TYPE_COUNT;
    if (reader->_instruction_for_core_already_read[c_type.bytes.b7] == mtrue) {
      merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_FRAGMENTED_INSTRUCTION_SECTION_);
      return RET_FAILURE;
    }
    if ((section_len.whole_word % 8) != 0) {
      merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_MISALIGNED_INSTRUCTION_SECTION_);
      return RET_FAILURE;
    }
    if (section_len.whole_word == 0) {
      merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_INSTRUCTION_SECTION_CANNOT_BE_ZERO_);
      return RET_FAILURE;
    }
    reader->_instruction_for_core_already_read[c_type.bytes.b7] = mtrue;
    reader->itit.entries[i].type = c_type.bytes.b7;
    reader->itit.entries[i].section_len = section_len.whole_word;
    reader->metadata.total_instructions_len += section_len.whole_word;
  }

  if ((reader->metadata.DI_len + reader->metadata.ITIT_len +
       reader->metadata.data_section_len + reader->metadata.string_section_len +
       reader->metadata.total_instructions_len + 40) > reader->file_size) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_INVALID_FILE_STRUCTURE_DOESNT_MATCH_HEADER_INFO_);
    return RET_FAILURE;
  }
  return RET_SUCCESS;
}

mret_t
merry_graves_reader_parse_instruction_sections(MerryGravesInput *reader) {
  merry_check_ptr(reader);

  msize_t off = ftell(reader->fd);
  for (msize_t i = 0; i < reader->itit.entry_count; i++) {
    MerryITITEntry entry = reader->itit.entries[i];
    msize_t curr_section_len = entry.section_len;
    msize_t section_count =
        entry.section_len / _MERRY_PAGE_LEN_ +
        ((entry.section_len % _MERRY_PAGE_LEN_) > 0 ? 1 : 0);
    reader->instruction_offsets_count[entry.type] = section_count;

    if ((reader->instruction_offsets[entry.type] = (MerrySection *)malloc(
             sizeof(MerrySection) * section_count)) == NULL) {
      merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                         _MERRY_MEM_ALLOCATION_FAILURE_);
      return RET_FAILURE;
    }
    for (msize_t j = 0; j < section_count; j++) {
      reader->instruction_offsets[entry.type][j].offset = off;
      reader->instruction_offsets[entry.type][j].section_length =
          ((curr_section_len - _MERRY_PAGE_LEN_) < 0 ? curr_section_len
                                                     : _MERRY_PAGE_LEN_);
      curr_section_len -= _MERRY_PAGE_LEN_;
      off += reader->instruction_offsets[entry.type][j].section_length;
    }
    fseek(reader->fd, entry.section_len, SEEK_CUR);
  }

  return RET_SUCCESS;
}

mret_t merry_graves_reader_parse_data_type_metadata(MerryGravesInput *reader) {
  merry_check_ptr(reader);

  // Exactly 24 bytes
  fread(&reader->qword.off_ed, 8, 1, reader->fd);
  fread(&reader->dword.off_ed, 8, 1, reader->fd);
  fread(&reader->word.off_ed, 8, 1, reader->fd);
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
  MerryHostMemLayout l;
  l.whole_word = reader->qword.off_ed;
  merry_LITTLE_ENDIAN_to_BIG_ENDIAN(&l);
  reader->qword = l.whole_word;

  l.whole_word = reader->dword.off_ed;
  merry_LITTLE_ENDIAN_to_BIG_ENDIAN(&l);
  reader->dword = l.whole_word;

  l.whole_word = reader->word.off_ed;
  merry_LITTLE_ENDIAN_to_BIG_ENDIAN(&l);
  reader->word = l.whole_word;
#endif
  if ((reader->qword.off_ed + reader->dword.off_ed + reader->word.off_ed) !=
      reader->metadata.data_section_len) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_INVALID_FILE_STRUCTURE_DOESNT_MATCH_HEADER_INFO_);
    return RET_FAILURE;
  }
  if ((reader->qword.off_ed % 8) != 0) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MISALIGNED_DATA_SECTION_);
    return RET_FAILURE;
  }
  if ((reader->dword.off_ed % 4) != 0) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MISALIGNED_DATA_SECTION_);
    return RET_FAILURE;
  }
  if ((reader->word.off_ed % 2) != 0) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MISALIGNED_DATA_SECTION_);
    return RET_FAILURE;
  }
  return RET_SUCCESS;
}

mret_t
merry_graves_reader_parse_data_and_string_section(MerryGravesInput *reader) {
  merry_check_ptr(reader);
  msize_t off = ftell(reader->fd);
  msize_t section_count =
      reader->metadata.data_section_len / _MERRY_PAGE_LEN_ +
      ((reader->metadata.data_section_len % _MERRY_PAGE_LEN_) > 0 ? 1 : 0);
  reader->data_offsets_count = section_count;

  if ((reader->data_offsets = (MerrySection *)malloc(sizeof(MerrySection) *
                                                     section_count)) == NULL) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_MEM_ALLOCATION_FAILURE_);
    return RET_FAILURE;
  }

  reader->qword.off_st = off;
  reader->qword.off_ed = reader->qword.off_ed + off;

  reader->dword.off_st = reader->qword.off_ed;
  reader->dword.off_ed = reader->dword.off_ed + reader->dword.off_st;

  reader->word.off_st = reader->dword.off_ed;
  reader->word.off_ed = reader->word.off_ed + reader->word.off_st;

  msize_t data_section_len = reader->metadata.data_section_len;

  for (msize_t i = 0; i < section_count; i++) {
    reader->data_offsets[i].offset = off;
    reader->data_offsets[i].section_length =
        ((int64_t)(data_section_len - _MERRY_PAGE_LEN_) < 0 ? data_section_len
                                                            : _MERRY_PAGE_LEN_);
    data_section_len -= _MERRY_PAGE_LEN_;
    off += reader->data_offsets[i].section_length;
  }
  fseek(reader->fd, reader->metadata.data_section_len, SEEK_CUR);
  if (reader->metadata.string_section_len > 0) {
    off = ftell(reader->fd);
    msize_t i = 0;
    msize_t last_pg_len = reader->metadata.data_section_len % _MERRY_PAGE_LEN_;
    data_section_len = reader->metadata.string_section_len;
    if (last_pg_len > 0) {
      msize_t diff = _MERRY_PAGE_LEN_ - last_pg_len;
      if (data_section_len > diff) {
        data_section_len -= diff;
      }
      section_count = data_section_len / _MERRY_PAGE_LEN_ +
                      ((data_section_len % _MERRY_PAGE_LEN_) > 0 ? 1 : 0);

      if ((reader->string_offsets = (MerrySection *)malloc(
               sizeof(MerrySection) * (section_count + 1))) == NULL) {
        merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                           _MERRY_MEM_ALLOCATION_FAILURE_);
        return RET_FAILURE;
      }
      reader->string_offsets[i].section_length = diff;
      reader->string_offsets[i].offset = off;
      off += reader->data_offsets[i].section_length;
      reader->string_offsets_count = section_count + 1;
      i++;
    } else {
      section_count = data_section_len / _MERRY_PAGE_LEN_ +
                      ((data_section_len % _MERRY_PAGE_LEN_) > 0 ? 1 : 0);

      if ((reader->string_offsets = (MerrySection *)malloc(
               sizeof(MerrySection) * section_count)) == NULL) {
        merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                           _MERRY_MEM_ALLOCATION_FAILURE_);
        return RET_FAILURE;
      }
      reader->string_offsets_count = section_count;
    }
    for (; i < section_count; i++) {
      reader->string_offsets[i].offset = off;
      reader->string_offsets[i].section_length =
          ((data_section_len - _MERRY_PAGE_LEN_) < 0 ? data_section_len
                                                     : _MERRY_PAGE_LEN_);
      reader->string_offsets[i].section_length -= _MERRY_PAGE_LEN_;
      off += reader->string_offsets[i].section_length;
      data_section_len -= reader->string_offsets[i].section_length;
    }
    fseek(reader->fd, reader->metadata.string_section_len, SEEK_CUR);
  }
  return RET_SUCCESS;
}

mret_t merry_graves_reader_prep_memory(MerryGravesInput *reader) {
  merry_check_ptr(reader);

  if ((reader->data_ram = merry_create_RAM(reader->data_offsets_count +
                                               reader->string_offsets_count,
                                           &reader->state)) == RET_NULL)
    return RET_FAILURE;

  for (msize_t i = 0; i < reader->itit.entry_count; i++) {
    MerryITITEntry e = reader->itit.entries[i];
    if ((reader->iram[e.type] = merry_create_RAM(
             reader->data_offsets_count + reader->string_offsets_count,
             &reader->state)) == RET_NULL)
      return RET_FAILURE;
  }

  return RET_SUCCESS;
}

mret_t merry_graves_reader_load_instructions(MerryGravesInput *reader,
                                             mcore_t c_type, msize_t pgnum) {
  merry_check_ptr(reader);

  if (surelyF(pgnum >= reader->instruction_offsets_count[c_type])) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }

  MerryRAM *ram = reader->iram[c_type];
  if (ram->pages[pgnum]->init == mtrue)
    return RET_SUCCESS;
  if (merry_initialize_normal_memory_page(ram->pages[pgnum]) == RET_FAILURE) {
    merry_obtain_memory_interface_state(&reader->state);
    return RET_FAILURE;
  }

  MerrySection section = reader->instruction_offsets[c_type][pgnum];

  fseek(reader->fd, section.offset, SEEK_SET);

  fread(ram->pages[pgnum]->buf, 8, section.section_length / 8, reader->fd);
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
  mqptr_t buf = (mqptr_t)ram->pages[pgnum]->buf;
  for (msize_t i = 0; i < (section.section_len / 8); i++) {
    MerryHostMemLayout le;
    le.whole_word = buf[i];
    merry_LITTLE_ENDIAN_to_BIG_ENDIAN(&le);
    buf[i] = le.whole_word;
  }
#endif

  return RET_SUCCESS;
}

mret_t merry_graves_reader_load_data(MerryGravesInput *reader, msize_t pgnum) {
  merry_check_ptr(reader);

  if (surelyF(pgnum >=
              (reader->data_offsets_count + reader->string_offsets_count))) {
    merry_assign_state(reader->state, _MERRY_INTERNAL_SYSTEM_ERROR_,
                       _MERRY_PAGE_FAULT_);
    return RET_FAILURE;
  }
  MerryRAM *ram = reader->data_ram;
  MerrySection section;

  if (ram->pages[pgnum]->init == mtrue)
    return RET_SUCCESS;
  if (merry_initialize_normal_memory_page(ram->pages[pgnum]) == RET_FAILURE) {
    merry_obtain_memory_interface_state(&reader->state);
    return RET_FAILURE;
  }

  if (pgnum < (reader->data_offsets_count - 1)) {
    // This implies everything is in the data section
    section = reader->data_offsets[pgnum];
    fseek(reader->fd, section.offset, SEEK_SET);

    msize_t bytes_to_read = 0;
    msize_t bytes_already_read = 0;

    if (section.offset >= reader->qword.off_st &&
        section.offset < reader->qword.off_ed) {
      bytes_to_read = (reader->qword.off_ed - section.offset);
      if (bytes_to_read > section.section_length) {
        bytes_to_read = section.section_length;
      }
      fread(ram->pages[pgnum]->buf, 8, (bytes_to_read) / 8, reader->fd);
      bytes_already_read += bytes_to_read;
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
      mqptr_t buf = (mqptr_t)ram->pages[pgnum]->buf;
      for (msize_t i = 0; i < (bytes_to_read / 8); i++) {
        MerryHostMemLayout le;
        le.whole_word = buf[i];
        merry_LITTLE_ENDIAN_to_BIG_ENDIAN(&le);
        buf[i] = le.whole_word;
      }
#endif
      section.section_length -= bytes_to_read;
    }
    section.offset += bytes_already_read;
    if (section.offset >= reader->dword.off_st &&
        section.offset < reader->dword.off_ed) {
      bytes_to_read = (reader->dword.off_ed - section.offset);
      fread((mbptr_t)ram->pages[pgnum]->buf + bytes_already_read, 4,
            (bytes_to_read) / 4, reader->fd);
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
      mdptr_t buf =
          (mdptr_t)((mbptr_t)ram->pages[pgnum]->buf + bytes_already_read);
      for (msize_t i = 0; i < (bytes_to_read / 4); i++) {
        MerryHostMemLayout l, r;
        l.whole_word = *buf;
        r.bytes.b7 = l.bytes.b4;
        r.bytes.b6 = l.bytes.b5;
        r.bytes.b5 = l.bytes.b6;
        r.bytes.b4 = l.bytes.b7;
        *buf = r.w1;
      }
#endif
      bytes_already_read += bytes_to_read;
      section.section_length -= bytes_to_read;
    }
    section.offset += bytes_already_read;
    if (section.offset >= reader->word.off_st &&
        section.offset < reader->word.off_ed) {
      bytes_to_read = (reader->word.off_ed - section.offset);
      fread((mbptr_t)ram->pages[pgnum]->buf + bytes_already_read, 2,
            (bytes_to_read) / 2, reader->fd);
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
      mwptr_t buf =
          (mwptr_t)((mbptr_t)ram->pages[pgnum]->buf + bytes_already_read);
      for (msize_t i = 0; i < (bytes_to_read / 2); i++) {
        MerryHostMemLayout l, r;
        l.whole_word = *buf;
        r.bytes.b7 = l.bytes.b6;
        r.bytes.b6 = l.bytes.b7;
        *buf = r.word.w4;
      }
#endif
      bytes_already_read += bytes_to_read;
      section.section_length -= bytes_to_read;
    }
  } else if (pgnum == (reader->data_offsets_count - 1)) {
    section = reader->data_offsets[pgnum];
    msize_t actual_len = section.section_length;
    fseek(reader->fd, section.offset, SEEK_SET);
    msize_t bytes_to_read = 0;
    msize_t bytes_already_read = 0;

    if (section.offset >= reader->qword.off_st &&
        section.offset < reader->qword.off_ed) {
      bytes_to_read = (reader->qword.off_ed - section.offset);
      if (bytes_to_read > section.section_length) {
        bytes_to_read = section.section_length;
      }
      fread(ram->pages[pgnum]->buf, 8, (bytes_to_read) / 8, reader->fd);
      bytes_already_read += bytes_to_read;
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
      mqptr_t buf = (mqptr_t)ram->pages[pgnum]->buf;
      for (msize_t i = 0; i < (bytes_to_read / 8); i++) {
        MerryHostMemLayout le;
        le.whole_word = buf[i];
        merry_LITTLE_ENDIAN_to_BIG_ENDIAN(&le);
        buf[i] = le.whole_word;
      }
#endif
      section.section_length -= bytes_to_read;
    }
    section.offset += bytes_already_read;
    if (section.offset >= reader->dword.off_st &&
        section.offset < reader->dword.off_ed) {
      bytes_to_read = (reader->dword.off_ed - section.offset);
      fread((mbptr_t)ram->pages[pgnum]->buf + bytes_already_read, 4,
            (bytes_to_read) / 4, reader->fd);
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
      mdptr_t buf =
          (mdptr_t)((mbptr_t)ram->pages[pgnum]->buf + bytes_already_read);
      for (msize_t i = 0; i < (bytes_to_read / 4); i++) {
        MerryHostMemLayout l, r;
        l.whole_word = *buf;
        r.bytes.b7 = l.bytes.b4;
        r.bytes.b6 = l.bytes.b5;
        r.bytes.b5 = l.bytes.b6;
        r.bytes.b4 = l.bytes.b7;
        *buf = r.w1;
      }
#endif
      bytes_already_read += bytes_to_read;
      section.section_length -= bytes_to_read;
    }
    section.offset += bytes_already_read;
    if (section.offset >= reader->word.off_st &&
        section.offset < reader->word.off_ed) {
      bytes_to_read = (reader->word.off_ed - section.offset);
      fread((mbptr_t)ram->pages[pgnum]->buf + bytes_already_read, 2,
            (bytes_to_read) / 2, reader->fd);
#if _MERRY_BYTE_ORDER_ == _MERRY_BIG_ENDIAN_
      mwptr_t buf =
          (mwptr_t)((mbptr_t)ram->pages[pgnum]->buf + bytes_already_read);
      for (msize_t i = 0; i < (bytes_to_read / 2); i++) {
        MerryHostMemLayout l, r;
        l.whole_word = *buf;
        r.bytes.b7 = l.bytes.b6;
        r.bytes.b6 = l.bytes.b7;
        *buf = r.word.w4;
      }
#endif
      bytes_already_read += bytes_to_read;
      section.section_length -= bytes_to_read;
    }
    if (actual_len != _MERRY_PAGE_LEN_) {
      section = reader->string_offsets[0];
      fseek(reader->fd, section.offset, SEEK_SET);
      fread((mbptr_t)ram->pages[pgnum]->buf + actual_len, 1,
            section.section_length, reader->fd);
    }
  } else {
    section =
        reader->string_offsets[pgnum - reader->data_offsets_count -
                               (reader->string_offsets_count > 0 ? 1 : 0)];
    fseek(reader->fd, section.offset, SEEK_SET);
    fread(ram->pages[pgnum]->buf, 1, section.section_length, reader->fd);
  }

  return RET_SUCCESS;
}
