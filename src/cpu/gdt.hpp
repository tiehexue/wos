//=======================================================================
// Copyright Baptiste Wicht 2013-2018.
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://www.opensource.org/licenses/MIT)
//=======================================================================

#ifndef GDT_H
#define GDT_H

#include "gdt_types.hpp"

namespace gdt {

void setup_gdt();
void flush_tss();
void enable_a20_gate();


} //end of namespace gdt

#endif
