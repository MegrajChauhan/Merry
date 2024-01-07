/*
 * Internal component configuration of the Merry VM
 * MIT License
 *
 * Copyright (c) 2024 MegrajChauhan
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef _MERRY_INTERNALS_
#define _MERRY_INTERNALS_

// define the necessary configurations needed for the internal components here
#include "../../utils/merry_config.h"
#include "../../utils/merry_types.h"

// Configurations for memory
#define _MERRY_MEMORY_ADDRESSES_PER_PAGE_ 1048576                   // the number of addresses per page[Equal to 1MB]
#define _MERRY_MEMORY_GET_PAGE_OFFSET_(address) (address & 0xFFFFF) // get the offset from the address
#define _MERRY_MEMORY_GET_PAGE_(address) (address >> 20)            // get the page number from the address
// we currently have no limit set to how many pages Merry can have at max but lets leave it to the OS
#define _MERRY_MEMORY_GENERATE_ADDRESS_(page, offset) ((page << 20) & offset) // generate an address
// the memory follows the same endianness as the host system
#define _MERRY_MEMORY_BYTE_ORDER_ _MERRY_BYTE_ORDER_
#define _MERRY_MEMORY_IS_ACCESS_ERROR_(offset) ((offset + 7) >= _MERRY_MEMORY_ADDRESSES_PER_PAGE_)
#define _MERRY_STACKMEM_BYTE_LEN_ _MERRY_MEMORY_ADDRESSES_PER_PAGE_
#define _MERRY_STACKMEM_SIZE_ 131072 // this is the number of qwords and not the bytes[equals 1MB]

#endif