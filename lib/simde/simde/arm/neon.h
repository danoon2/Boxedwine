/* SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Copyright:
 *   2020      Evan Nemerson <evan@nemerson.com>
 */

#if !defined(SIMDE_ARM_NEON_H)
#define SIMDE_ARM_NEON_H

#include "neon/types.h"

#include "neon/aba.h"
#include "neon/abd.h"
#include "neon/abdl.h"
#include "neon/abs.h"
#include "neon/add.h"
#include "neon/addl.h"
#include "neon/addlv.h"
#include "neon/addl_high.h"
#include "neon/addv.h"
#include "neon/addw.h"
#include "neon/addw_high.h"
#include "neon/and.h"
#include "neon/bic.h"
#include "neon/bsl.h"
#include "neon/cagt.h"
#include "neon/ceq.h"
#include "neon/ceqz.h"
#include "neon/cge.h"
#include "neon/cgez.h"
#include "neon/cgt.h"
#include "neon/cgtz.h"
#include "neon/cle.h"
#include "neon/clez.h"
#include "neon/cls.h"
#include "neon/clt.h"
#include "neon/cltz.h"
#include "neon/clz.h"
#include "neon/cnt.h"
#include "neon/cvt.h"
#include "neon/combine.h"
#include "neon/create.h"
#include "neon/dot.h"
#include "neon/dot_lane.h"
#include "neon/dup_lane.h"
#include "neon/dup_n.h"
#include "neon/eor.h"
#include "neon/ext.h"
#include "neon/get_high.h"
#include "neon/get_lane.h"
#include "neon/get_low.h"
#include "neon/hadd.h"
#include "neon/hsub.h"
#include "neon/ld1.h"
#include "neon/ld3.h"
#include "neon/ld4.h"
#include "neon/max.h"
#include "neon/maxnm.h"
#include "neon/maxv.h"
#include "neon/min.h"
#include "neon/minnm.h"
#include "neon/minv.h"
#include "neon/mla.h"
#include "neon/mla_n.h"
#include "neon/mlal.h"
#include "neon/mlal_high.h"
#include "neon/mlal_n.h"
#include "neon/mls.h"
#include "neon/mlsl.h"
#include "neon/mlsl_high.h"
#include "neon/mlsl_n.h"
#include "neon/movl.h"
#include "neon/movl_high.h"
#include "neon/movn.h"
#include "neon/movn_high.h"
#include "neon/mul.h"
#include "neon/mul_lane.h"
#include "neon/mul_n.h"
#include "neon/mull.h"
#include "neon/mull_high.h"
#include "neon/mull_n.h"
#include "neon/mvn.h"
#include "neon/neg.h"
#include "neon/orn.h"
#include "neon/orr.h"
#include "neon/padal.h"
#include "neon/padd.h"
#include "neon/paddl.h"
#include "neon/pmax.h"
#include "neon/pmin.h"
#include "neon/qabs.h"
#include "neon/qadd.h"
#include "neon/qdmulh.h"
#include "neon/qdmull.h"
#include "neon/qrdmulh.h"
#include "neon/qrdmulh_n.h"
#include "neon/qmovn.h"
#include "neon/qmovun.h"
#include "neon/qmovn_high.h"
#include "neon/qneg.h"
#include "neon/qsub.h"
#include "neon/qshl.h"
#include "neon/qtbl.h"
#include "neon/qtbx.h"
#include "neon/rbit.h"
#include "neon/reinterpret.h"
#include "neon/rev16.h"
#include "neon/rev32.h"
#include "neon/rev64.h"
#include "neon/rhadd.h"
#include "neon/rnd.h"
#include "neon/rshl.h"
#include "neon/rshr_n.h"
#include "neon/rsra_n.h"
#include "neon/set_lane.h"
#include "neon/shl.h"
#include "neon/shl_n.h"
#include "neon/shr_n.h"
#include "neon/sra_n.h"
#include "neon/st1.h"
#include "neon/st1_lane.h"
#include "neon/st3.h"
#include "neon/st4.h"
#include "neon/sub.h"
#include "neon/subl.h"
#include "neon/subw.h"
#include "neon/subw_high.h"
#include "neon/tbl.h"
#include "neon/tbx.h"
#include "neon/trn.h"
#include "neon/trn1.h"
#include "neon/trn2.h"
#include "neon/tst.h"
#include "neon/uqadd.h"
#include "neon/uzp.h"
#include "neon/uzp1.h"
#include "neon/uzp2.h"
#include "neon/zip.h"
#include "neon/zip1.h"
#include "neon/zip2.h"

#endif /* SIMDE_ARM_NEON_H */
