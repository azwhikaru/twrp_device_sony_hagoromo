/*
* Copyright (C) 2011-2015 MediaTek Inc.
*
* This program is free software: you can redistribute it and/or modify it under the terms of the
* GNU General Public License version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with this program.
* If not, see <http://www.gnu.org/licenses/>.
*/
//
//
#include "vdec_verify_mpv_prov.h"
#include "../hal/vdec_hal_if_common.h"
#include "vdec_verify_keydef.h"
#include "vdec_verify_vparser_vp9.h"
#include "vdec_verify_file_common.h"
#include "vdec_verify_filesetting.h"
#include "../hal/vdec_hal_if_vp9.h"

#include <linux/string.h>
#include <linux/delay.h>
#include <linux/vmalloc.h>


//#ifdef CONFIG_TV_DRV_VFY 
//#include <mach/cache_operation.h>
//#endif // CONFIG_TV_DRV_VFY

static const vp9_coeff_probs_model default_coef_probs_4x4[PLANE_TYPES] = {
  {  // Y plane
    {  // Intra
      {  // Band 0
        { 195,  29, 183 }, {  84,  49, 136 }, {   8,  42,  71 }
      }, {  // Band 1
        {  31, 107, 169 }, {  35,  99, 159 }, {  17,  82, 140 },
        {   8,  66, 114 }, {   2,  44,  76 }, {   1,  19,  32 }
      }, {  // Band 2
        {  40, 132, 201 }, {  29, 114, 187 }, {  13,  91, 157 },
        {   7,  75, 127 }, {   3,  58,  95 }, {   1,  28,  47 }
      }, {  // Band 3
        {  69, 142, 221 }, {  42, 122, 201 }, {  15,  91, 159 },
        {   6,  67, 121 }, {   1,  42,  77 }, {   1,  17,  31 }
      }, {  // Band 4
        { 102, 148, 228 }, {  67, 117, 204 }, {  17,  82, 154 },
        {   6,  59, 114 }, {   2,  39,  75 }, {   1,  15,  29 }
      }, {  // Band 5
        { 156,  57, 233 }, { 119,  57, 212 }, {  58,  48, 163 },
        {  29,  40, 124 }, {  12,  30,  81 }, {   3,  12,  31 }
      }
    }, {  // Inter
      {  // Band 0
        { 191, 107, 226 }, { 124, 117, 204 }, {  25,  99, 155 }
      }, {  // Band 1
        {  29, 148, 210 }, {  37, 126, 194 }, {   8,  93, 157 },
        {   2,  68, 118 }, {   1,  39,  69 }, {   1,  17,  33 }
      }, {  // Band 2
        {  41, 151, 213 }, {  27, 123, 193 }, {   3,  82, 144 },
        {   1,  58, 105 }, {   1,  32,  60 }, {   1,  13,  26 }
      }, {  // Band 3
        {  59, 159, 220 }, {  23, 126, 198 }, {   4,  88, 151 },
        {   1,  66, 114 }, {   1,  38,  71 }, {   1,  18,  34 }
      }, {  // Band 4
        { 114, 136, 232 }, {  51, 114, 207 }, {  11,  83, 155 },
        {   3,  56, 105 }, {   1,  33,  65 }, {   1,  17,  34 }
      }, {  // Band 5
        { 149,  65, 234 }, { 121,  57, 215 }, {  61,  49, 166 },
        {  28,  36, 114 }, {  12,  25,  76 }, {   3,  16,  42 }
      }
    }
  }, {  // UV plane
    {  // Intra
      {  // Band 0
        { 214,  49, 220 }, { 132,  63, 188 }, {  42,  65, 137 }
      }, {  // Band 1
        {  85, 137, 221 }, { 104, 131, 216 }, {  49, 111, 192 },
        {  21,  87, 155 }, {   2,  49,  87 }, {   1,  16,  28 }
      }, {  // Band 2
        {  89, 163, 230 }, {  90, 137, 220 }, {  29, 100, 183 },
        {  10,  70, 135 }, {   2,  42,  81 }, {   1,  17,  33 }
      }, {  // Band 3
        { 108, 167, 237 }, {  55, 133, 222 }, {  15,  97, 179 },
        {   4,  72, 135 }, {   1,  45,  85 }, {   1,  19,  38 }
      }, {  // Band 4
        { 124, 146, 240 }, {  66, 124, 224 }, {  17,  88, 175 },
        {   4,  58, 122 }, {   1,  36,  75 }, {   1,  18,  37 }
      }, {  //  Band 5
        { 141,  79, 241 }, { 126,  70, 227 }, {  66,  58, 182 },
        {  30,  44, 136 }, {  12,  34,  96 }, {   2,  20,  47 }
      }
    }, {  // Inter
      {  // Band 0
        { 229,  99, 249 }, { 143, 111, 235 }, {  46, 109, 192 }
      }, {  // Band 1
        {  82, 158, 236 }, {  94, 146, 224 }, {  25, 117, 191 },
        {   9,  87, 149 }, {   3,  56,  99 }, {   1,  33,  57 }
      }, {  // Band 2
        {  83, 167, 237 }, {  68, 145, 222 }, {  10, 103, 177 },
        {   2,  72, 131 }, {   1,  41,  79 }, {   1,  20,  39 }
      }, {  // Band 3
        {  99, 167, 239 }, {  47, 141, 224 }, {  10, 104, 178 },
        {   2,  73, 133 }, {   1,  44,  85 }, {   1,  22,  47 }
      }, {  // Band 4
        { 127, 145, 243 }, {  71, 129, 228 }, {  17,  93, 177 },
        {   3,  61, 124 }, {   1,  41,  84 }, {   1,  21,  52 }
      }, {  // Band 5
        { 157,  78, 244 }, { 140,  72, 231 }, {  69,  58, 184 },
        {  31,  44, 137 }, {  14,  38, 105 }, {   8,  23,  61 }
      }
    }
  }
};

static const vp9_coeff_probs_model default_coef_probs_8x8[PLANE_TYPES] = {
  {  // Y plane
    {  // Intra
      {  // Band 0
        { 125,  34, 187 }, {  52,  41, 133 }, {   6,  31,  56 }
      }, {  // Band 1
        {  37, 109, 153 }, {  51, 102, 147 }, {  23,  87, 128 },
        {   8,  67, 101 }, {   1,  41,  63 }, {   1,  19,  29 }
      }, {  // Band 2
        {  31, 154, 185 }, {  17, 127, 175 }, {   6,  96, 145 },
        {   2,  73, 114 }, {   1,  51,  82 }, {   1,  28,  45 }
      }, {  // Band 3
        {  23, 163, 200 }, {  10, 131, 185 }, {   2,  93, 148 },
        {   1,  67, 111 }, {   1,  41,  69 }, {   1,  14,  24 }
      }, {  // Band 4
        {  29, 176, 217 }, {  12, 145, 201 }, {   3, 101, 156 },
        {   1,  69, 111 }, {   1,  39,  63 }, {   1,  14,  23 }
      }, {  // Band 5
        {  57, 192, 233 }, {  25, 154, 215 }, {   6, 109, 167 },
        {   3,  78, 118 }, {   1,  48,  69 }, {   1,  21,  29 }
      }
    }, {  // Inter
      {  // Band 0
        { 202, 105, 245 }, { 108, 106, 216 }, {  18,  90, 144 }
      }, {  // Band 1
        {  33, 172, 219 }, {  64, 149, 206 }, {  14, 117, 177 },
        {   5,  90, 141 }, {   2,  61,  95 }, {   1,  37,  57 }
      }, {  // Band 2
        {  33, 179, 220 }, {  11, 140, 198 }, {   1,  89, 148 },
        {   1,  60, 104 }, {   1,  33,  57 }, {   1,  12,  21 }
      }, {  // Band 3
        {  30, 181, 221 }, {   8, 141, 198 }, {   1,  87, 145 },
        {   1,  58, 100 }, {   1,  31,  55 }, {   1,  12,  20 }
      }, {  // Band 4
        {  32, 186, 224 }, {   7, 142, 198 }, {   1,  86, 143 },
        {   1,  58, 100 }, {   1,  31,  55 }, {   1,  12,  22 }
      }, {  // Band 5
        {  57, 192, 227 }, {  20, 143, 204 }, {   3,  96, 154 },
        {   1,  68, 112 }, {   1,  42,  69 }, {   1,  19,  32 }
      }
    }
  }, {  // UV plane
    {  // Intra
      {  // Band 0
        { 212,  35, 215 }, { 113,  47, 169 }, {  29,  48, 105 }
      }, {  // Band 1
        {  74, 129, 203 }, { 106, 120, 203 }, {  49, 107, 178 },
        {  19,  84, 144 }, {   4,  50,  84 }, {   1,  15,  25 }
      }, {  // Band 2
        {  71, 172, 217 }, {  44, 141, 209 }, {  15, 102, 173 },
        {   6,  76, 133 }, {   2,  51,  89 }, {   1,  24,  42 }
      }, {  // Band 3
        {  64, 185, 231 }, {  31, 148, 216 }, {   8, 103, 175 },
        {   3,  74, 131 }, {   1,  46,  81 }, {   1,  18,  30 }
      }, {  // Band 4
        {  65, 196, 235 }, {  25, 157, 221 }, {   5, 105, 174 },
        {   1,  67, 120 }, {   1,  38,  69 }, {   1,  15,  30 }
      }, {  // Band 5
        {  65, 204, 238 }, {  30, 156, 224 }, {   7, 107, 177 },
        {   2,  70, 124 }, {   1,  42,  73 }, {   1,  18,  34 }
      }
    }, {  // Inter
      {  // Band 0
        { 225,  86, 251 }, { 144, 104, 235 }, {  42,  99, 181 }
      }, {  // Band 1
        {  85, 175, 239 }, { 112, 165, 229 }, {  29, 136, 200 },
        {  12, 103, 162 }, {   6,  77, 123 }, {   2,  53,  84 }
      }, {  // Band 2
        {  75, 183, 239 }, {  30, 155, 221 }, {   3, 106, 171 },
        {   1,  74, 128 }, {   1,  44,  76 }, {   1,  17,  28 }
      }, {  // Band 3
        {  73, 185, 240 }, {  27, 159, 222 }, {   2, 107, 172 },
        {   1,  75, 127 }, {   1,  42,  73 }, {   1,  17,  29 }
      }, {  // Band 4
        {  62, 190, 238 }, {  21, 159, 222 }, {   2, 107, 172 },
        {   1,  72, 122 }, {   1,  40,  71 }, {   1,  18,  32 }
      }, {  // Band 5
        {  61, 199, 240 }, {  27, 161, 226 }, {   4, 113, 180 },
        {   1,  76, 129 }, {   1,  46,  80 }, {   1,  23,  41 }
      }
    }
  }
};

static const vp9_coeff_probs_model default_coef_probs_16x16[PLANE_TYPES] = {
  {  // Y plane
    {  // Intra
      {  // Band 0
        {   7,  27, 153 }, {   5,  30,  95 }, {   1,  16,  30 }
      }, {  // Band 1
        {  50,  75, 127 }, {  57,  75, 124 }, {  27,  67, 108 },
        {  10,  54,  86 }, {   1,  33,  52 }, {   1,  12,  18 }
      }, {  // Band 2
        {  43, 125, 151 }, {  26, 108, 148 }, {   7,  83, 122 },
        {   2,  59,  89 }, {   1,  38,  60 }, {   1,  17,  27 }
      }, {  // Band 3
        {  23, 144, 163 }, {  13, 112, 154 }, {   2,  75, 117 },
        {   1,  50,  81 }, {   1,  31,  51 }, {   1,  14,  23 }
      }, {  // Band 4
        {  18, 162, 185 }, {   6, 123, 171 }, {   1,  78, 125 },
        {   1,  51,  86 }, {   1,  31,  54 }, {   1,  14,  23 }
      }, {  // Band 5
        {  15, 199, 227 }, {   3, 150, 204 }, {   1,  91, 146 },
        {   1,  55,  95 }, {   1,  30,  53 }, {   1,  11,  20 }
      }
    }, {  // Inter
      {  // Band 0
        {  19,  55, 240 }, {  19,  59, 196 }, {   3,  52, 105 }
      }, {  // Band 1
        {  41, 166, 207 }, { 104, 153, 199 }, {  31, 123, 181 },
        {  14, 101, 152 }, {   5,  72, 106 }, {   1,  36,  52 }
      }, {  // Band 2
        {  35, 176, 211 }, {  12, 131, 190 }, {   2,  88, 144 },
        {   1,  60, 101 }, {   1,  36,  60 }, {   1,  16,  28 }
      }, {  // Band 3
        {  28, 183, 213 }, {   8, 134, 191 }, {   1,  86, 142 },
        {   1,  56,  96 }, {   1,  30,  53 }, {   1,  12,  20 }
      }, {  // Band 4
        {  20, 190, 215 }, {   4, 135, 192 }, {   1,  84, 139 },
        {   1,  53,  91 }, {   1,  28,  49 }, {   1,  11,  20 }
      }, {  // Band 5
        {  13, 196, 216 }, {   2, 137, 192 }, {   1,  86, 143 },
        {   1,  57,  99 }, {   1,  32,  56 }, {   1,  13,  24 }
      }
    }
  }, {  // UV plane
    {  // Intra
      {  // Band 0
        { 211,  29, 217 }, {  96,  47, 156 }, {  22,  43,  87 }
      }, {  // Band 1
        {  78, 120, 193 }, { 111, 116, 186 }, {  46, 102, 164 },
        {  15,  80, 128 }, {   2,  49,  76 }, {   1,  18,  28 }
      }, {  // Band 2
        {  71, 161, 203 }, {  42, 132, 192 }, {  10,  98, 150 },
        {   3,  69, 109 }, {   1,  44,  70 }, {   1,  18,  29 }
      }, {  // Band 3
        {  57, 186, 211 }, {  30, 140, 196 }, {   4,  93, 146 },
        {   1,  62, 102 }, {   1,  38,  65 }, {   1,  16,  27 }
      }, {  // Band 4
        {  47, 199, 217 }, {  14, 145, 196 }, {   1,  88, 142 },
        {   1,  57,  98 }, {   1,  36,  62 }, {   1,  15,  26 }
      }, {  // Band 5
        {  26, 219, 229 }, {   5, 155, 207 }, {   1,  94, 151 },
        {   1,  60, 104 }, {   1,  36,  62 }, {   1,  16,  28 }
      }
    }, {  // Inter
      {  // Band 0
        { 233,  29, 248 }, { 146,  47, 220 }, {  43,  52, 140 }
      }, {  // Band 1
        { 100, 163, 232 }, { 179, 161, 222 }, {  63, 142, 204 },
        {  37, 113, 174 }, {  26,  89, 137 }, {  18,  68,  97 }
      }, {  // Band 2
        {  85, 181, 230 }, {  32, 146, 209 }, {   7, 100, 164 },
        {   3,  71, 121 }, {   1,  45,  77 }, {   1,  18,  30 }
      }, {  // Band 3
        {  65, 187, 230 }, {  20, 148, 207 }, {   2,  97, 159 },
        {   1,  68, 116 }, {   1,  40,  70 }, {   1,  14,  29 }
      }, {  // Band 4
        {  40, 194, 227 }, {   8, 147, 204 }, {   1,  94, 155 },
        {   1,  65, 112 }, {   1,  39,  66 }, {   1,  14,  26 }
      }, {  // Band 5
        {  16, 208, 228 }, {   3, 151, 207 }, {   1,  98, 160 },
        {   1,  67, 117 }, {   1,  41,  74 }, {   1,  17,  31 }
      }
    }
  }
};

static const vp9_coeff_probs_model default_coef_probs_32x32[PLANE_TYPES] = {
  {  // Y plane
    {  // Intra
      {  // Band 0
        {  17,  38, 140 }, {   7,  34,  80 }, {   1,  17,  29 }
      }, {  // Band 1
        {  37,  75, 128 }, {  41,  76, 128 }, {  26,  66, 116 },
        {  12,  52,  94 }, {   2,  32,  55 }, {   1,  10,  16 }
      }, {  // Band 2
        {  50, 127, 154 }, {  37, 109, 152 }, {  16,  82, 121 },
        {   5,  59,  85 }, {   1,  35,  54 }, {   1,  13,  20 }
      }, {  // Band 3
        {  40, 142, 167 }, {  17, 110, 157 }, {   2,  71, 112 },
        {   1,  44,  72 }, {   1,  27,  45 }, {   1,  11,  17 }
      }, {  // Band 4
        {  30, 175, 188 }, {   9, 124, 169 }, {   1,  74, 116 },
        {   1,  48,  78 }, {   1,  30,  49 }, {   1,  11,  18 }
      }, {  // Band 5
        {  10, 222, 223 }, {   2, 150, 194 }, {   1,  83, 128 },
        {   1,  48,  79 }, {   1,  27,  45 }, {   1,  11,  17 }
      }
    }, {  // Inter
      {  // Band 0
        {  36,  41, 235 }, {  29,  36, 193 }, {  10,  27, 111 }
      }, {  // Band 1
        {  85, 165, 222 }, { 177, 162, 215 }, { 110, 135, 195 },
        {  57, 113, 168 }, {  23,  83, 120 }, {  10,  49,  61 }
      }, {  // Band 2
        {  85, 190, 223 }, {  36, 139, 200 }, {   5,  90, 146 },
        {   1,  60, 103 }, {   1,  38,  65 }, {   1,  18,  30 }
      }, {  // Band 3
        {  72, 202, 223 }, {  23, 141, 199 }, {   2,  86, 140 },
        {   1,  56,  97 }, {   1,  36,  61 }, {   1,  16,  27 }
      }, {  // Band 4
        {  55, 218, 225 }, {  13, 145, 200 }, {   1,  86, 141 },
        {   1,  57,  99 }, {   1,  35,  61 }, {   1,  13,  22 }
      }, {  // Band 5
        {  15, 235, 212 }, {   1, 132, 184 }, {   1,  84, 139 },
        {   1,  57,  97 }, {   1,  34,  56 }, {   1,  14,  23 }
      }
    }
  }, {  // UV plane
    {  // Intra
      {  // Band 0
        { 181,  21, 201 }, {  61,  37, 123 }, {  10,  38,  71 }
      }, {  // Band 1
        {  47, 106, 172 }, {  95, 104, 173 }, {  42,  93, 159 },
        {  18,  77, 131 }, {   4,  50,  81 }, {   1,  17,  23 }
      }, {  // Band 2
        {  62, 147, 199 }, {  44, 130, 189 }, {  28, 102, 154 },
        {  18,  75, 115 }, {   2,  44,  65 }, {   1,  12,  19 }
      }, {  // Band 3
        {  55, 153, 210 }, {  24, 130, 194 }, {   3,  93, 146 },
        {   1,  61,  97 }, {   1,  31,  50 }, {   1,  10,  16 }
      }, {  // Band 4
        {  49, 186, 223 }, {  17, 148, 204 }, {   1,  96, 142 },
        {   1,  53,  83 }, {   1,  26,  44 }, {   1,  11,  17 }
      }, {  // Band 5
        {  13, 217, 212 }, {   2, 136, 180 }, {   1,  78, 124 },
        {   1,  50,  83 }, {   1,  29,  49 }, {   1,  14,  23 }
      }
    }, {  // Inter
      {  // Band 0
        { 197,  13, 247 }, {  82,  17, 222 }, {  25,  17, 162 }
      }, {  // Band 1
        { 126, 186, 247 }, { 234, 191, 243 }, { 176, 177, 234 },
        { 104, 158, 220 }, {  66, 128, 186 }, {  55,  90, 137 }
      }, {  // Band 2
        { 111, 197, 242 }, {  46, 158, 219 }, {   9, 104, 171 },
        {   2,  65, 125 }, {   1,  44,  80 }, {   1,  17,  91 }
      }, {  // Band 3
        { 104, 208, 245 }, {  39, 168, 224 }, {   3, 109, 162 },
        {   1,  79, 124 }, {   1,  50, 102 }, {   1,  43, 102 }
      }, {  // Band 4
        {  84, 220, 246 }, {  31, 177, 231 }, {   2, 115, 180 },
        {   1,  79, 134 }, {   1,  55,  77 }, {   1,  60,  79 }
      }, {  // Band 5
        {  43, 243, 240 }, {   8, 180, 217 }, {   1, 115, 166 },
        {   1,  84, 121 }, {   1,  51,  67 }, {   1,  16,   6 }
      }
    }
  }
};

static const vp9_prob default_if_y_probs[BLOCK_SIZE_GROUPS][INTRA_MODES - 1] = {
  {  65,  32,  18, 144, 162, 194,  41,  51,  98 },  // block_size < 8x8
  { 132,  68,  18, 165, 217, 196,  45,  40,  78 },  // block_size < 16x16
  { 173,  80,  19, 176, 240, 193,  64,  35,  46 },  // block_size < 32x32
  { 221, 135,  38, 194, 248, 121,  96,  85,  29 }   // block_size >= 32x32
};

static const vp9_prob default_if_uv_probs[INTRA_MODES][INTRA_MODES - 1] = {
  { 120,   7,  76, 176, 208, 126,  28,  54, 103 },  // y = dc
  {  48,  12, 154, 155, 139,  90,  34, 117, 119 },  // y = v
  {  67,   6,  25, 204, 243, 158,  13,  21,  96 },  // y = h
  {  97,   5,  44, 131, 176, 139,  48,  68,  97 },  // y = d45
  {  83,   5,  42, 156, 111, 152,  26,  49, 152 },  // y = d135
  {  80,   5,  58, 178,  74,  83,  33,  62, 145 },  // y = d117
  {  86,   5,  32, 154, 192, 168,  14,  22, 163 },  // y = d153
  {  85,   5,  32, 156, 216, 148,  19,  29,  73 },  // y = d207
  {  77,   7,  64, 116, 132, 122,  37, 126, 120 },  // y = d63
  { 101,  21, 107, 181, 192, 103,  19,  67, 125 }   // y = tm
};

static const vp9_prob default_skip_probs[SKIP_CONTEXTS] = {
  192, 128, 64
};

static const vp9_prob default_switchable_interp_prob[SWITCHABLE_FILTER_CONTEXTS]
                                                    [SWITCHABLE_FILTERS - 1] = {
  { 235, 162, },
  { 36, 255, },
  { 34, 3, },
  { 149, 144, },
};

static const vp9_prob default_partition_probs[PARTITION_CONTEXTS]
                                             [PARTITION_TYPES - 1] = {
  // 8x8 -> 4x4
  { 199, 122, 141 },  // a/l both not split
  { 147,  63, 159 },  // a split, l not split
  { 148, 133, 118 },  // l split, a not split
  { 121, 104, 114 },  // a/l both split
  // 16x16 -> 8x8
  { 174,  73,  87 },  // a/l both not split
  {  92,  41,  83 },  // a split, l not split
  {  82,  99,  50 },  // l split, a not split
  {  53,  39,  39 },  // a/l both split
  // 32x32 -> 16x16
  { 177,  58,  59 },  // a/l both not split
  {  68,  26,  63 },  // a split, l not split
  {  52,  79,  25 },  // l split, a not split
  {  17,  14,  12 },  // a/l both split
  // 64x64 -> 32x32
  { 222,  34,  30 },  // a/l both not split
  {  72,  16,  44 },  // a split, l not split
  {  58,  32,  12 },  // l split, a not split
  {  10,   7,   6 },  // a/l both split
};

static const vp9_prob default_inter_mode_probs[INTER_MODE_CONTEXTS]
                                              [INTER_MODES - 1] = {
  {2,       173,   34},  // 0 = both zero mv
  {7,       145,   85},  // 1 = one zero mv + one a predicted mv
  {7,       166,   63},  // 2 = two predicted mvs
  {7,       94,    66},  // 3 = one predicted/zero and one new mv
  {8,       64,    46},  // 4 = two new mvs
  {17,      81,    31},  // 5 = one intra neighbour + x
  {25,      29,    30},  // 6 = two intra neighbours
};

static const vp9_prob default_intra_inter_p[INTRA_INTER_CONTEXTS] = {
  9, 102, 187, 225
};

static const vp9_prob default_comp_inter_p[COMP_INTER_CONTEXTS] = {
  239, 183, 119,  96,  41
};

static const vp9_prob default_comp_ref_p[REF_CONTEXTS] = {
  50, 126, 123, 221, 226
};

static const vp9_prob default_single_ref_p[REF_CONTEXTS][2] = {
  {  33,  16 },
  {  77,  74 },
  { 142, 142 },
  { 172, 170 },
  { 238, 247 }
};

static const struct tx_probs default_tx_probs = {
  { { 3, 136, 37 },
    { 5, 52,  13 } },

  { { 20, 152 },
    { 15, 101 } },

  { { 100 },
    { 66  } }
};

static const nmv_context default_nmv_context = {
  {32, 64, 96},
  { // NOLINT
    { /* vert component */ // NOLINT
      128,                                                  /* sign */
      {224, 144, 192, 168, 192, 176, 192, 198, 198, 245},   /* class */
      {216},                                                /* class0 */
      {136, 140, 148, 160, 176, 192, 224, 234, 234, 240},   /* bits */
      {{128, 128, 64}, {96, 112, 64}},                      /* class0_fp */
      {64, 96, 64},                                         /* fp */
      160,                                                  /* class0_hp bit */
      128,                                                  /* hp */
    },
    { /* hor component */ // NOLINT
      128,                                                  /* sign */
      {216, 128, 176, 160, 176, 176, 192, 198, 198, 208},   /* class */
      {208},                                                /* class0 */
      {136, 140, 148, 160, 176, 192, 224, 234, 234, 240},   /* bits */
      {{128, 128, 64}, {96, 112, 64}},                      /* class0_fp */
      {64, 96, 64},                                         /* fp */
      160,                                                  /* class0_hp bit */
      128,                                                  /* hp */
    }
  },
};

const vp9_tree_index vp9_mv_joint_tree[TREE_SIZE(MV_JOINTS)] = {
  -MV_JOINT_ZERO, 2,
  -MV_JOINT_HNZVZ, 4,
  -MV_JOINT_HZVNZ, -MV_JOINT_HNZVNZ
};

const vp9_tree_index vp9_mv_class_tree[TREE_SIZE(MV_CLASSES)] = {
  -MV_CLASS_0, 2,
  -MV_CLASS_1, 4,
  6, 8,
  -MV_CLASS_2, -MV_CLASS_3,
  10, 12,
  -MV_CLASS_4, -MV_CLASS_5,
  -MV_CLASS_6, 14,
  16, 18,
  -MV_CLASS_7, -MV_CLASS_8,
  -MV_CLASS_9, -MV_CLASS_10,
};

const vp9_tree_index vp9_mv_class0_tree[TREE_SIZE(CLASS0_SIZE)] = {
  -0, -1,
};

const vp9_tree_index vp9_mv_fp_tree[TREE_SIZE(MV_FP_SIZE)] = {
  -0, 2,
  -1, 4,
  -2, -3
};

void tx_counts_to_branch_counts_32x32(const unsigned int *tx_count_32x32p,
                                      unsigned int (*ct_32x32p)[2]) {
  ct_32x32p[0][0] = tx_count_32x32p[TX_4X4];
  ct_32x32p[0][1] = tx_count_32x32p[TX_8X8] +
                    tx_count_32x32p[TX_16X16] +
                    tx_count_32x32p[TX_32X32];
  ct_32x32p[1][0] = tx_count_32x32p[TX_8X8];
  ct_32x32p[1][1] = tx_count_32x32p[TX_16X16] +
                    tx_count_32x32p[TX_32X32];
  ct_32x32p[2][0] = tx_count_32x32p[TX_16X16];
  ct_32x32p[2][1] = tx_count_32x32p[TX_32X32];
}

void tx_counts_to_branch_counts_16x16(const unsigned int *tx_count_16x16p,
                                      unsigned int (*ct_16x16p)[2]) {
  ct_16x16p[0][0] = tx_count_16x16p[TX_4X4];
  ct_16x16p[0][1] = tx_count_16x16p[TX_8X8] + tx_count_16x16p[TX_16X16];
  ct_16x16p[1][0] = tx_count_16x16p[TX_8X8];
  ct_16x16p[1][1] = tx_count_16x16p[TX_16X16];
}

void tx_counts_to_branch_counts_8x8(const unsigned int *tx_count_8x8p,
                                    unsigned int (*ct_8x8p)[2]) {
  ct_8x8p[0][0] = tx_count_8x8p[TX_4X4];
  ct_8x8p[0][1] = tx_count_8x8p[TX_8X8];
}


//static inline int is_inter_mode(PREDICTION_MODE mode) {
  static int is_inter_mode(PREDICTION_MODE mode) {
  return mode >= NEARESTMV && mode <= NEWMV;
}

/* Array indices are identical to previously-existing INTRAMODECONTEXTNODES. */
const vp9_tree_index vp9_intra_mode_tree[TREE_SIZE(INTRA_MODES)] = {
  -DC_PRED, 2,                      /* 0 = DC_NODE */
  -TM_PRED, 4,                      /* 1 = TM_NODE */
  -V_PRED, 6,                       /* 2 = V_NODE */
  8, 12,                            /* 3 = COM_NODE */
  -H_PRED, 10,                      /* 4 = H_NODE */
  -D135_PRED, -D117_PRED,           /* 5 = D135_NODE */
  -D45_PRED, 14,                    /* 6 = D45_NODE */
  -D63_PRED, 16,                    /* 7 = D63_NODE */
  -D153_PRED, -D207_PRED             /* 8 = D153_NODE */
};

const vp9_tree_index vp9_inter_mode_tree[TREE_SIZE(INTER_MODES)] = {
  -INTER_OFFSET(ZEROMV), 2,
  -INTER_OFFSET(NEARESTMV), 4,
  -INTER_OFFSET(NEARMV), -INTER_OFFSET(NEWMV)
};

const vp9_tree_index vp9_partition_tree[TREE_SIZE(PARTITION_TYPES)] = {
  -PARTITION_NONE, 2,
  -PARTITION_HORZ, 4,
  -PARTITION_VERT, -PARTITION_SPLIT
};

const vp9_tree_index vp9_switchable_interp_tree
                         [TREE_SIZE(SWITCHABLE_FILTERS)] = {
  -EIGHTTAP, 2,
  -EIGHTTAP_SMOOTH, -EIGHTTAP_SHARP
};

const INTERP_FILTER literal_to_filter[] = { EIGHTTAP_SMOOTH,
                                            EIGHTTAP,
                                            EIGHTTAP_SHARP,
                                            BILINEAR };

static const int seg_feature_data_signed[SEG_LVL_MAX] = { 1, 1, 0, 0 };

static const int seg_feature_data_max[SEG_LVL_MAX] = {
  MAXQ, MAX_LOOP_FILTER, 3, 0 };

static const UINT32 dc_qlookup[QINDEX_RANGE] = {
  4,       8,    8,    9,   10,   11,   12,   12,
  13,     14,   15,   16,   17,   18,   19,   19,
  20,     21,   22,   23,   24,   25,   26,   26,
  27,     28,   29,   30,   31,   32,   32,   33,
  34,     35,   36,   37,   38,   38,   39,   40,
  41,     42,   43,   43,   44,   45,   46,   47,
  48,     48,   49,   50,   51,   52,   53,   53,
  54,     55,   56,   57,   57,   58,   59,   60,
  61,     62,   62,   63,   64,   65,   66,   66,
  67,     68,   69,   70,   70,   71,   72,   73,
  74,     74,   75,   76,   77,   78,   78,   79,
  80,     81,   81,   82,   83,   84,   85,   85,
  87,     88,   90,   92,   93,   95,   96,   98,
  99,    101,  102,  104,  105,  107,  108,  110,
  111,   113,  114,  116,  117,  118,  120,  121,
  123,   125,  127,  129,  131,  134,  136,  138,
  140,   142,  144,  146,  148,  150,  152,  154,
  156,   158,  161,  164,  166,  169,  172,  174,
  177,   180,  182,  185,  187,  190,  192,  195,
  199,   202,  205,  208,  211,  214,  217,  220,
  223,   226,  230,  233,  237,  240,  243,  247,
  250,   253,  257,  261,  265,  269,  272,  276,
  280,   284,  288,  292,  296,  300,  304,  309,
  313,   317,  322,  326,  330,  335,  340,  344,
  349,   354,  359,  364,  369,  374,  379,  384,
  389,   395,  400,  406,  411,  417,  423,  429,
  435,   441,  447,  454,  461,  467,  475,  482,
  489,   497,  505,  513,  522,  530,  539,  549,
  559,   569,  579,  590,  602,  614,  626,  640,
  654,   668,  684,  700,  717,  736,  755,  775,
  796,   819,  843,  869,  896,  925,  955,  988,
  1022, 1058, 1098, 1139, 1184, 1232, 1282, 1336,
};

static const UINT32 ac_qlookup[QINDEX_RANGE] = {
  4,       8,    9,   10,   11,   12,   13,   14,
  15,     16,   17,   18,   19,   20,   21,   22,
  23,     24,   25,   26,   27,   28,   29,   30,
  31,     32,   33,   34,   35,   36,   37,   38,
  39,     40,   41,   42,   43,   44,   45,   46,
  47,     48,   49,   50,   51,   52,   53,   54,
  55,     56,   57,   58,   59,   60,   61,   62,
  63,     64,   65,   66,   67,   68,   69,   70,
  71,     72,   73,   74,   75,   76,   77,   78,
  79,     80,   81,   82,   83,   84,   85,   86,
  87,     88,   89,   90,   91,   92,   93,   94,
  95,     96,   97,   98,   99,  100,  101,  102,
  104,   106,  108,  110,  112,  114,  116,  118,
  120,   122,  124,  126,  128,  130,  132,  134,
  136,   138,  140,  142,  144,  146,  148,  150,
  152,   155,  158,  161,  164,  167,  170,  173,
  176,   179,  182,  185,  188,  191,  194,  197,
  200,   203,  207,  211,  215,  219,  223,  227,
  231,   235,  239,  243,  247,  251,  255,  260,
  265,   270,  275,  280,  285,  290,  295,  300,
  305,   311,  317,  323,  329,  335,  341,  347,
  353,   359,  366,  373,  380,  387,  394,  401,
  408,   416,  424,  432,  440,  448,  456,  465,
  474,   483,  492,  501,  510,  520,  530,  540,
  550,   560,  571,  582,  593,  604,  615,  627,
  639,   651,  663,  676,  689,  702,  715,  729,
  743,   757,  771,  786,  801,  816,  832,  848,
  864,   881,  898,  915,  933,  951,  969,  988,
  1007, 1026, 1046, 1066, 1087, 1108, 1129, 1151,
  1173, 1196, 1219, 1243, 1267, 1292, 1317, 1343,
  1369, 1396, 1423, 1451, 1479, 1508, 1537, 1567,
  1597, 1628, 1660, 1692, 1725, 1759, 1793, 1828,
};


#define ALIGN_POWER_OF_TWO(value, n) \
    (((value) + ((1 << (n)) - 1)) & ~((1 << (n)) - 1))
    
#define MIN_TILE_WIDTH_B64 4
#define MAX_TILE_WIDTH_B64 64

//static inline int mi_cols_aligned_to_sb(int n_mis) {
  static  int mi_cols_aligned_to_sb(int n_mis) {

  return ALIGN_POWER_OF_TWO(n_mis, MI_BLOCK_SIZE_LOG2);
}

void vp9_get_tile_n_bits(int mi_cols,
                         int *min_log2_tile_cols, int *max_log2_tile_cols) {
    const int sb_cols = mi_cols_aligned_to_sb(mi_cols) >> MI_BLOCK_SIZE_LOG2;
    int min_log2 = 0, max_log2 = 0;

    // max
    while ((sb_cols >> max_log2) >= MIN_TILE_WIDTH_B64)
    ++max_log2;
    --max_log2;
    if (max_log2 < 0)
    max_log2 = 0;

    // min
    while ((MAX_TILE_WIDTH_B64 << min_log2) < sb_cols)
    ++min_log2;

    ASSERT(min_log2 <= max_log2);

    *min_log2_tile_cols = min_log2;
    *max_log2_tile_cols = max_log2;
}


#define COEF_COUNT_SAT 24
#define COEF_MAX_UPDATE_FACTOR 112
#define COEF_COUNT_SAT_KEY 24
#define COEF_MAX_UPDATE_FACTOR_KEY 112
#define COEF_COUNT_SAT_AFTER_KEY 24
#define COEF_MAX_UPDATE_FACTOR_AFTER_KEY 128



#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))

#define ROUND_POWER_OF_TWO(value, n) \
    (((value) + (1 << ((n) - 1))) >> (n))

#define ALIGN_POWER_OF_TWO(value, n) \
    (((value) + ((1 << (n)) - 1)) & ~((1 << (n)) - 1))
    
#define COEFF_CONTEXTS 6
#define BAND_COEFF_CONTEXTS(band) ((band) == 0 ? 3 : COEFF_CONTEXTS)
#define EOB_MODEL_TOKEN 3
//extern const vp9_tree_index vp9_coefmodel_tree[];

//static inline vp9_prob clip_prob(int p) {
static vp9_prob clip_prob(int p) {
  return (p > 255) ? 255u : (p < 1) ? 1u : p;
}

// int64 is not needed for normal frame level calculations.
// However when outputting entropy stats accumulated over many frames
// or even clips we can overflow int math.
//#ifdef ENTROPY_STATS
#if 0
static inline vp9_prob get_prob(int num, int den) {
  return (den == 0) ? 128u : clip_prob(((int64_t)num * 256 + (den >> 1)) / den);
}
#else
//static inline vp9_prob get_prob(int num, int den) {
  static vp9_prob get_prob(int num, int den) {
  return (den == 0) ? 128u : clip_prob((num * 256 + (den >> 1)) / den);
}
#endif

//static inline vp9_prob get_binary_prob(int n0, int n1) {
  static vp9_prob get_binary_prob(int n0, int n1) {
  return get_prob(n0, n0 + n1);
}

/* This function assumes prob1 and prob2 are already within [1,255] range. */
//static inline vp9_prob weighted_prob(int prob1, int prob2, int factor) {
  static vp9_prob weighted_prob(int prob1, int prob2, int factor) {
  return ROUND_POWER_OF_TWO(prob1 * (256 - factor) + prob2 * factor, 8);
}

//static inline vp9_prob merge_probs(vp9_prob pre_prob,
static vp9_prob merge_probs(vp9_prob pre_prob,
                                   const unsigned int ct[2],
                                   unsigned int count_sat,
                                   unsigned int max_update_factor) {
  const vp9_prob prob = get_binary_prob(ct[0], ct[1]);
  const unsigned int count = MIN(ct[0] + ct[1], count_sat);
  const unsigned int factor = max_update_factor * count / count_sat;
  return weighted_prob(pre_prob, prob, factor);
}


static void adapt_coef_probs(VP9_COMMON_T * prCommon, TX_SIZE tx_size,
                             unsigned int count_sat,
                             unsigned int update_factor) {
  const FRAME_CONTEXT *pre_fc = &prCommon->frame_contexts[prCommon->rUnCompressedHeader.u4FrameContextIdx];
  vp9_coeff_probs_model *const probs = prCommon->fc.coef_probs[tx_size];
  const vp9_coeff_probs_model *const pre_probs = pre_fc->coef_probs[tx_size];
  vp9_coeff_count_model *counts = prCommon->counts.coef[tx_size];
  unsigned int (*eob_counts)[REF_TYPES][COEF_BANDS][COEFF_CONTEXTS] =
      prCommon->counts.eob_branch[tx_size];
  int i, j, k, l, m;

  for (i = 0; i < PLANE_TYPES; ++i)
    for (j = 0; j < REF_TYPES; ++j)
      for (k = 0; k < COEF_BANDS; ++k)
        for (l = 0; l < BAND_COEFF_CONTEXTS(k); ++l) {
          const int n0 = counts[i][j][k][l][ZERO_TOKEN];
          const int n1 = counts[i][j][k][l][ONE_TOKEN];
          const int n2 = counts[i][j][k][l][TWO_TOKEN];
          const int neob = counts[i][j][k][l][EOB_MODEL_TOKEN];
          const unsigned int branch_ct[UNCONSTRAINED_NODES][2] = {
            { neob, eob_counts[i][j][k][l] - neob },
            { n0, n1 + n2 },
            { n1, n2 }
          };
          for (m = 0; m < UNCONSTRAINED_NODES; ++m)
            probs[i][j][k][l][m] = merge_probs(pre_probs[i][j][k][l][m],
                                               branch_ct[m],
                                               count_sat, update_factor);
        }
}

void vVP9AdaptCoefProbs(VP9_COMMON_T *prCommon) {
    TX_SIZE t;
    unsigned int count_sat, update_factor;
    VP9_PRINT_INFO("vVP9AdaptCoefProbs\n");

    if (fgVP9IntraOnly(prCommon)) {
        update_factor = COEF_MAX_UPDATE_FACTOR_KEY;
        count_sat = COEF_COUNT_SAT_KEY;
    } else if (prCommon->u4LastFrameType == KEY_FRAME) {
        update_factor = COEF_MAX_UPDATE_FACTOR_AFTER_KEY;  /* adapt quickly */
        count_sat = COEF_COUNT_SAT_AFTER_KEY;
    } else {
        update_factor = COEF_MAX_UPDATE_FACTOR;
        count_sat = COEF_COUNT_SAT;
    }
    for (t = TX_4X4; t <= TX_32X32; t++)
        adapt_coef_probs(prCommon, t, count_sat, update_factor);
}

#define COUNT_SAT 20
#define MAX_UPDATE_FACTOR 128

static int adapt_prob(vp9_prob pre_prob, const unsigned int ct[2]) {
  return merge_probs(pre_prob, ct, COUNT_SAT, MAX_UPDATE_FACTOR);
}

static unsigned int tree_merge_probs_impl(unsigned int i,
                                          const vp9_tree_index *tree,
                                          const vp9_prob *pre_probs,
                                          const unsigned int *counts,
                                          unsigned int count_sat,
                                          unsigned int max_update,
                                          vp9_prob *probs) {
  const int l = tree[i];
  const unsigned int left_count = (l <= 0)
                 ? counts[-l]
                 : tree_merge_probs_impl(l, tree, pre_probs, counts,
                                         count_sat, max_update, probs);
  const int r = tree[i + 1];
  const unsigned int right_count = (r <= 0)
                 ? counts[-r]
                 : tree_merge_probs_impl(r, tree, pre_probs, counts,
                                         count_sat, max_update, probs);
  const unsigned int ct[2] = { left_count, right_count };
  probs[i >> 1] = merge_probs(pre_probs[i >> 1], ct,
                              count_sat, max_update);
  return left_count + right_count;
}

void vp9_tree_merge_probs(const vp9_tree_index *tree, const vp9_prob *pre_probs,
                          const unsigned int *counts, unsigned int count_sat,
                          unsigned int max_update_factor, vp9_prob *probs) {
  tree_merge_probs_impl(0, tree, pre_probs, counts, count_sat,
                        max_update_factor, probs);
}

static void adapt_probs(const vp9_tree_index *tree,
                        const vp9_prob *pre_probs, const unsigned int *counts,
                        vp9_prob *probs) {
  vp9_tree_merge_probs(tree, pre_probs, counts, COUNT_SAT, MAX_UPDATE_FACTOR,
                   probs);
}

void vVP9AdaptModeProbs(VP9_COMMON_T *prCommon) {
  int i, j;
  FRAME_CONTEXT *fc = &prCommon->fc;
  const FRAME_CONTEXT *pre_fc = &prCommon->frame_contexts[prCommon->rUnCompressedHeader.u4FrameContextIdx];
  const FRAME_COUNTS *counts = &prCommon->counts;
  VP9_PRINT_INFO("vVP9AdaptModeProbs\n");

  for (i = 0; i < INTRA_INTER_CONTEXTS; i++)
    fc->intra_inter_prob[i] = adapt_prob(pre_fc->intra_inter_prob[i],
                                         counts->intra_inter[i]);
  for (i = 0; i < COMP_INTER_CONTEXTS; i++)
    fc->comp_inter_prob[i] = adapt_prob(pre_fc->comp_inter_prob[i],
                                        counts->comp_inter[i]);
  for (i = 0; i < REF_CONTEXTS; i++)
    fc->comp_ref_prob[i] = adapt_prob(pre_fc->comp_ref_prob[i],
                                      counts->comp_ref[i]);
  for (i = 0; i < REF_CONTEXTS; i++)
    for (j = 0; j < 2; j++)
      fc->single_ref_prob[i][j] = adapt_prob(pre_fc->single_ref_prob[i][j],
                                             counts->single_ref[i][j]);

  for (i = 0; i < INTER_MODE_CONTEXTS; i++)
    adapt_probs(vp9_inter_mode_tree, pre_fc->inter_mode_probs[i],
                counts->inter_mode[i], fc->inter_mode_probs[i]);

  for (i = 0; i < BLOCK_SIZE_GROUPS; i++)
    adapt_probs(vp9_intra_mode_tree, pre_fc->y_mode_prob[i],
                counts->y_mode[i], fc->y_mode_prob[i]);

  for (i = 0; i < INTRA_MODES; ++i)
    adapt_probs(vp9_intra_mode_tree, pre_fc->uv_mode_prob[i],
                counts->uv_mode[i], fc->uv_mode_prob[i]);

  for (i = 0; i < PARTITION_CONTEXTS; i++)
    adapt_probs(vp9_partition_tree, pre_fc->partition_prob[i],
                counts->partition[i], fc->partition_prob[i]);

  if (prCommon->rUnCompressedHeader.eInterpFilterType == SWITCHABLE) {
    for (i = 0; i < SWITCHABLE_FILTER_CONTEXTS; i++)
      adapt_probs(vp9_switchable_interp_tree, pre_fc->switchable_interp_prob[i],
                  counts->switchable_interp[i], fc->switchable_interp_prob[i]);
  }

  if (prCommon->eTxMode == TX_MODE_SELECT) {
    int j;
    unsigned int branch_ct_8x8p[TX_SIZES - 3][2];
    unsigned int branch_ct_16x16p[TX_SIZES - 2][2];
    unsigned int branch_ct_32x32p[TX_SIZES - 1][2];

    for (i = 0; i < TX_SIZE_CONTEXTS; ++i) {
      tx_counts_to_branch_counts_8x8(counts->tx.p8x8[i], branch_ct_8x8p);
      for (j = 0; j < TX_SIZES - 3; ++j)
        fc->tx_probs.p8x8[i][j] = adapt_prob(pre_fc->tx_probs.p8x8[i][j],
                                             branch_ct_8x8p[j]);

      tx_counts_to_branch_counts_16x16(counts->tx.p16x16[i], branch_ct_16x16p);
      for (j = 0; j < TX_SIZES - 2; ++j)
        fc->tx_probs.p16x16[i][j] = adapt_prob(pre_fc->tx_probs.p16x16[i][j],
                                               branch_ct_16x16p[j]);

      tx_counts_to_branch_counts_32x32(counts->tx.p32x32[i], branch_ct_32x32p);
      for (j = 0; j < TX_SIZES - 1; ++j)
        fc->tx_probs.p32x32[i][j] = adapt_prob(pre_fc->tx_probs.p32x32[i][j],
                                               branch_ct_32x32p[j]);
    }
  }

  for (i = 0; i < SKIP_CONTEXTS; ++i)
    fc->skip_probs[i] = adapt_prob(pre_fc->skip_probs[i], counts->skip[i]);
}

void vVP9AdaptMvProbs(VP9_COMMON_T *prCommon, int allow_hp) {
  int i, j;

  nmv_context *fc = &prCommon->fc.nmvc;
  const nmv_context *pre_fc = &prCommon->frame_contexts[prCommon->rUnCompressedHeader.u4FrameContextIdx].nmvc;
  const nmv_context_counts *counts = &prCommon->counts.mv;
  VP9_PRINT_INFO("vVP9AdaptMvProbs\n");

  adapt_probs(vp9_mv_joint_tree, pre_fc->joints, counts->joints, fc->joints);

  for (i = 0; i < 2; ++i) {
    nmv_component *comp = &fc->comps[i];
    const nmv_component *pre_comp = &pre_fc->comps[i];
    const nmv_component_counts *c = &counts->comps[i];

    comp->sign = adapt_prob(pre_comp->sign, c->sign);
    adapt_probs(vp9_mv_class_tree, pre_comp->classes, c->classes,
                comp->classes);
    adapt_probs(vp9_mv_class0_tree, pre_comp->class0, c->class0, comp->class0);

    for (j = 0; j < MV_OFFSET_BITS; ++j)
      comp->bits[j] = adapt_prob(pre_comp->bits[j], c->bits[j]);

    for (j = 0; j < CLASS0_SIZE; ++j)
      adapt_probs(vp9_mv_fp_tree, pre_comp->class0_fp[j], c->class0_fp[j],
                  comp->class0_fp[j]);

    adapt_probs(vp9_mv_fp_tree, pre_comp->fp, c->fp, comp->fp);

    if (allow_hp) {
      comp->class0_hp = adapt_prob(pre_comp->class0_hp, c->class0_hp);
      comp->hp = adapt_prob(pre_comp->hp, c->hp);
    }
  }
}

static UINT32 VP9_Clamp(INT32 value, INT32 low, INT32 high)
{
    return value < low ? low : (value > high ? high : value);
}

UINT32 vp9_dc_quant(UINT32 qindex, UINT32 delta)
{
    return dc_qlookup[VP9_Clamp(qindex + delta, 0, MAXQ)];
}

UINT32 vp9_ac_quant(UINT32 qindex, UINT32 delta) 
{
    return ac_qlookup[VP9_Clamp(qindex + delta, 0, MAXQ)];
}

void vVP9_Init_Dequantizer(VP9_COMMON_T *prCommon) 
{        
    UINT32 q;
    for (q = 0; q < QINDEX_RANGE; q++) 
    {
        prCommon->au4Y_Dequant[q][0] = vp9_dc_quant(q, prCommon->rUnCompressedHeader.u4Y_DC_DELTA_Q);
        prCommon->au4Y_Dequant[q][1] = vp9_ac_quant(q, 0);
        
        prCommon->au4UV_Dequant[q][0] = vp9_dc_quant(q, prCommon->rUnCompressedHeader.u4C_DC_DELTA_Q);
        prCommon->au4UV_Dequant[q][1] = vp9_ac_quant(q, prCommon->rUnCompressedHeader.u4C_AC_DELTA_Q);

        //printk("Y[0] %d Y[1] %d, UV[0] %d UV[1] %d\n", prCommon->au4Y_Dequant[q][0] , prCommon->au4Y_Dequant[q][1],
        //prCommon->au4UV_Dequant[q][0] , prCommon->au4UV_Dequant[q][1]   );
     }
}

UINT32 vp9_segfeature_active(SEGMENTATION *seg, UINT32 segment_id, SEG_LVL_FEATURES feature_id) 
{
    return seg->enabled && (seg->feature_mask[segment_id] & (1 << feature_id));
}

UINT32 vp9_get_segdata(SEGMENTATION *seg, UINT32 segment_id, SEG_LVL_FEATURES feature_id) 
{
    return seg->feature_data[segment_id][feature_id];
}

UINT32 vp9_get_qindex(SEGMENTATION *seg, UINT32 segment_id, UINT32 base_qindex) 
{
    if (vp9_segfeature_active(seg, segment_id, SEG_LVL_ALT_Q)) 
    {
        INT32 data = vp9_get_segdata(seg, segment_id, SEG_LVL_ALT_Q);
        VP9_PRINT_VERBOSE("vp9_get_segdata %d", data);
        INT32 seg_qindex = seg->abs_delta == SEGMENT_ABSDATA ? data : base_qindex + data;
        VP9_PRINT_VERBOSE("seg_qindex %d", seg_qindex);
        return VP9_Clamp(seg_qindex, 0, MAXQ);
    }
    else 
    {
        return base_qindex;
    }
}

void vVP9_Set_Default_LF_Deltas( VP9_LOOP_FILTER_INFO_T* lf) 
{
    lf->mode_ref_delta_enabled = 1;
    lf->mode_ref_delta_update = 1;

    lf->ref_deltas[INTRA_FRAME] = 1;
    lf->ref_deltas[LAST_FRAME] = 0;
    lf->ref_deltas[GOLDEN_FRAME] = -1;
    lf->ref_deltas[ALTREF_FRAME] = -1;

    lf->mode_deltas[0] = 0;
    lf->mode_deltas[1] = 0;
}

//-------------------------------------------------------------------------------------

extern BOOL fgGoldenCmp_VP9(ULONG u4DecBuf,ULONG u4GoldenBuf,UINT32 u4Size);

VP9_COMMON_T* prVP9GetCommon(UINT32 u4InstID)
{
    if(u4InstID >= 2)
    {
        VP9_PRINT_ERROR("prVP9GetCommon u4InstID(%d) > VDEC_INST_MAX, oops...\n", u4InstID);
        return NULL;
    }
    return &(_rVdecVp9Common[u4InstID]);
}

BOOL fgVP9IntraOnly(VP9_COMMON_T * prCommon)
{
    if ((prCommon->rUnCompressedHeader.u4FrameType == KEY_FRAME || prCommon->rUnCompressedHeader.u4IntraOnly))
    {
        return TRUE;
    }
    
    return FALSE;
}

static UINT32 u4VP9IVFReadFrame(VP9_INPUT_CTX_T * prInputCtx) 
{    
    VDEC_INFO_VERIFY_FILE_INFO_T rFileInfo;
    UCHAR ucBitstreamName[256];
    UINT32 u4FifoOffset = prInputCtx->u4FileOffset % VP9_V_FIFO_SZ;
    
    memset(ucBitstreamName, 0, 256);
    strncpy (ucBitstreamName , _bFileStr1[0][1], (strlen(_bFileStr1[0][1]) - 21));

    if((prInputCtx->u4FileOffset + VP9_IVF_FRAME_HEADER_SZ) >= prInputCtx->u4FileLength)
    {        
        VP9_PRINT_INFO("------ivf eof-------");
        //VP9_PRINT_INFO(" @@ VP9 Decode Completed [%s]",ucBitstreamName);
        VP9_PRINT_INFO(" @@ VP9 Decode Completed [%s]",_bFileStr1[0][1]);
        return VP9_FAIL;
    }

    if(prInputCtx->u4FileLength > VP9_V_FIFO_SZ)
    {
        if((prInputCtx->u4BitstreamLoadingCnt & 0x1) && (u4FifoOffset > VP9_V_FIFO_SZ/2))
        {
            rFileInfo.fgGetFileInfo = TRUE;
            rFileInfo.u4FileOffset = ((VP9_V_FIFO_SZ * (prInputCtx->u4BitstreamLoadingCnt+ 1))/2);
            rFileInfo.u4TargetSz = (VP9_V_FIFO_SZ/2);
            rFileInfo.u4FileLength = 0;
            rFileInfo.pucTargetAddr = (UCHAR *)prInputCtx->ulVaFifoStart; //qianqian@20150215       
			
			//fgOpenFile(0, prInputCtx->ucBitStreamName, "r+b", &rFileInfo);
#ifdef SATA_HDD_FS_SUPPORT
			if((fgOpenHDDFile(0, prInputCtx->ucBitStreamName, "r+b", &rFileInfo)) == FALSE)
#elif defined(IDE_READ_SUPPORT)
			if((fgOpenIdeFile(0, prInputCtx->ucBitStreamName, "r+b", &rFileInfo)) == FALSE)
#else
			if((fgOpenFile(0, prInputCtx->ucBitStreamName, "r+b", &rFileInfo)) == FALSE)
#endif

		   {
			  
			  VP9_PRINT_INFO("------1.u4VP9IVFReadFrame read file fail-------");
		   }
            prInputCtx->u4BitstreamLoadingCnt++;
        }
        else if(!(prInputCtx->u4BitstreamLoadingCnt & 0x1) && (u4FifoOffset < VP9_V_FIFO_SZ/2))
        {   
        
		    VP9_PRINT_INFO("------2.u4VP9IVFReadFrame -------");
            rFileInfo.fgGetFileInfo = TRUE;
            rFileInfo.u4FileOffset = ((VP9_V_FIFO_SZ * (prInputCtx->u4BitstreamLoadingCnt+ 1))/2);
            rFileInfo.u4TargetSz = (VP9_V_FIFO_SZ/2);
            rFileInfo.u4FileLength = 0;        
            rFileInfo.pucTargetAddr = (UCHAR *)(prInputCtx->ulVaFifoStart + VP9_V_FIFO_SZ/2);//qianqian@20150215  
            rFileInfo.u4FileOffset = ((VP9_V_FIFO_SZ * (prInputCtx->u4BitstreamLoadingCnt + 1))/2);
            //fgOpenFile(0, prInputCtx->ucBitStreamName, "r+b", &rFileInfo);            
            
#ifdef SATA_HDD_FS_SUPPORT
			if((fgOpenHDDFile(0, prInputCtx->ucBitStreamName, "r+b", &rFileInfo)) == FALSE)
#elif defined(IDE_READ_SUPPORT)
			if((fgOpenIdeFile(0, prInputCtx->ucBitStreamName, "r+b", &rFileInfo)) == FALSE)
#else
			if((fgOpenFile(0, prInputCtx->ucBitStreamName, "r+b", &rFileInfo)) == FALSE)
#endif

		   {
			  
			  VP9_PRINT_INFO("------2.u4VP9IVFReadFrame read file fail-------");
		   }
            prInputCtx->u4BitstreamLoadingCnt++;
        }
    }
    UCHAR* pCHAR = (UCHAR*)(u4FifoOffset + prInputCtx->ulVaFifoStart);
    
    UINT32 u4Byte1 = *(pCHAR++);
    if(((ULONG)pCHAR) == prInputCtx->ulVaFifoEnd)////qianqian@20150215  
    {        
        VP9_PRINT_INFO("Frame Size Header Cross Fifo End!!!");
        pCHAR = (UCHAR*)(prInputCtx->ulVaFifoStart);
    }
    UINT32 u4Byte2 = *(pCHAR++);    
    if(((ULONG)pCHAR) == prInputCtx->ulVaFifoEnd)
    {        
        VP9_PRINT_INFO("Frame Size Header Cross Fifo End!!!");
        pCHAR = (UCHAR*)(prInputCtx->ulVaFifoStart);
    }
    UINT32 u4Byte3 = *(pCHAR++);    
    if(((ULONG)pCHAR) == prInputCtx->ulVaFifoEnd)
    {        
        VP9_PRINT_INFO("Frame Size Header Cross Fifo End!!!");
        pCHAR = (UCHAR*)(prInputCtx->ulVaFifoStart);
    }
    UINT32 u4Byte4 = *(pCHAR++);
    if(((ULONG)pCHAR) == prInputCtx->ulVaFifoEnd)
    {        
        VP9_PRINT_INFO("Frame Size Header Cross Fifo End!!!");
        pCHAR =(UCHAR*) (prInputCtx->ulVaFifoStart);
    }
    
    prInputCtx->u4FrameSize = (u4Byte1) | (u4Byte2<<8) | (u4Byte3<<16) | (u4Byte4<<24);
    prInputCtx->ulVaFrameStart = prInputCtx->ulVaFifoStart + u4FifoOffset + VP9_IVF_FRAME_HEADER_SZ;
    prInputCtx->ulVaFrameStart = (prInputCtx->ulVaFrameStart >= prInputCtx->ulVaFifoEnd) ? (prInputCtx->ulVaFrameStart - VP9_V_FIFO_SZ) : prInputCtx->ulVaFrameStart;
    prInputCtx->ulVaFrameEnd = prInputCtx->ulVaFrameStart + prInputCtx->u4FrameSize;
    prInputCtx->ulVaFrameEnd = prInputCtx->ulVaFrameEnd >= prInputCtx->ulVaFifoEnd ? (prInputCtx->ulVaFrameEnd - VP9_V_FIFO_SZ) : prInputCtx->ulVaFrameEnd;
    prInputCtx->u4FileOffset += (VP9_IVF_FRAME_HEADER_SZ + prInputCtx->u4FrameSize);
    
    VP9_PRINT_INFO("ivf frame vaddr:[0x%lx -- 0x%lx],  size: 0x%x", prInputCtx->ulVaFrameStart, prInputCtx->ulVaFrameEnd, prInputCtx->u4FrameSize);
    if(prInputCtx->u4FileOffset > prInputCtx->u4FileLength)
    {        
        VP9_PRINT_ERROR("[QQ]frame exceed file end");
        return VP9_FAIL;
    }
    
    return VP9_OK;
}

static void vVP9CheckInputWindow(u4CoreId) {
    VP9_PRINT_INFO("[input window : 0x%x]", u4VDEC_HAL_VP9_Get_Input_Window(u4CoreId));
}

static void vVP9SuperFrameParse(VP9_INPUT_CTX_T * prInputCtx) {
    VP9_SUPER_FRAME_INFO_T* prSuperFrame = &prInputCtx->rSuperFrame;
    
    ASSERT(prInputCtx->u4FrameSize);
	UCHAR* pu1Data3;
    UCHAR* pu1Data = (UCHAR *)(prInputCtx->ulVaFrameStart);	
    UCHAR* pu1Data2 = (UCHAR *)(prInputCtx->ulVaFrameEnd-1);
    //UCHAR u1Marker = pu1Data[prInputCtx->u4FrameSize - 1];
	UCHAR u1Marker;
	UCHAR u1MarkerIndexSz;
	if((prInputCtx->ulVaFrameStart + prInputCtx->u4FrameSize -1) >= prInputCtx->ulVaFifoEnd)
    {
      u1Marker = pu1Data2[0];
    }else
    {
      u1Marker = pu1Data[prInputCtx->u4FrameSize - 1];	  
	  VP9_PRINT_INFO("------super frame : %x,%x,-------", pu1Data2[0],u1Marker);
    }
    if ((u1Marker & 0xe0) == 0xc0) {
        UCHAR u1FrameCount = (u1Marker & 0x7) + 1;
        UINT32 u4Mag = ((u1Marker >> 3) & 0x3) + 1;
        UINT32 u4IndexSz = 2 + u4Mag * u1FrameCount;
		
        if((prInputCtx->ulVaFrameStart + prInputCtx->u4FrameSize - u4IndexSz) >= prInputCtx->ulVaFifoEnd)
	    {
	    
		  pu1Data3 = (UCHAR *)(prInputCtx->ulVaFrameEnd-u4IndexSz);
	      u1MarkerIndexSz = pu1Data3[0];
	    }else
	    {
	      u1MarkerIndexSz = pu1Data[prInputCtx->u4FrameSize - u4IndexSz];	  
		  VP9_PRINT_INFO("------2. super frame : %x,%x,-------", pu1Data3[0],u1MarkerIndexSz);
	    }
        if (prInputCtx->u4FrameSize >= u4IndexSz && u1MarkerIndexSz == u1Marker) {
            // found a valid superframe index
            UINT32 i, j;
            UCHAR *x = pu1Data + prInputCtx->u4FrameSize - u4IndexSz + 1;
            if(((ULONG)x) >= prInputCtx->ulVaFifoEnd)
            {
               x = (UCHAR *)(prInputCtx->ulVaFrameEnd-u4IndexSz+1);
			   //pu1Data + prInputCtx->u4FrameSize - u4IndexSz + 1;
            }else if(((ULONG)x) == prInputCtx->ulVaFifoEnd-1)
            {
               x = (UCHAR *)prInputCtx->ulVaFifoStart - 1;
            }
            for (i = 0; i < u1FrameCount; i++) {
                UINT32 u4ThisSz = 0;
                for (j = 0; j < u4Mag; j++)
                    u4ThisSz |= (*x++) << (j * 8);
                prSuperFrame->u4SuperFrmSizes[i] = u4ThisSz;
            }

            if(u1FrameCount)
            {                
                VP9_PRINT_INFO("------super frame found[need to do ringfifo], count: %d-------", u1FrameCount);
                prSuperFrame->fgInSuperFrame = TRUE;
                prSuperFrame->u4SuperFrmIndex = 0;                
                prSuperFrame->u4SuperFrmCount = u1FrameCount;
                prInputCtx->u4FrameSize = prSuperFrame->u4SuperFrmSizes[0];
                prInputCtx->ulVaFrameEnd = prInputCtx->ulVaFrameStart + prInputCtx->u4FrameSize;
                if(prSuperFrame->u4SuperFrmCount == 1)
                {                    
                    //reach super frame end
                    //memset(prSuperFrame, 0, sizeof(VP9_SUPER_FRAME_INFO_T));
                    memset(prSuperFrame, 0, sizeof(VP9_SUPER_FRAME_INFO_T));
                }
            }
        }
    }
}


static INT32 i4VP9CheckSyncCode(UINT32 u4CoreId) {
  if (u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 8) != VP9_SYNC_CODE_0 ||
      u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 8) != VP9_SYNC_CODE_1 ||
      u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 8) != VP9_SYNC_CODE_2) 
  {
      VP9_PRINT_ERROR("Invalid frame sync code");
      return VP9_FAIL;
  }
  return VP9_OK;
}

static void vVP9SetupLoopFilter(UINT32 u4CoreId, VP9_LOOP_FILTER_INFO_T* prLoopFilter) 
{
    VP9_PRINT_INFO("setup loop filter");
    
    prLoopFilter->filter_level = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 6);
    prLoopFilter->sharpness_level = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 3);

    // Read in loop filter deltas applied at the MB level based on mode or ref frame.
    prLoopFilter->mode_ref_delta_update = 0;
    prLoopFilter->mode_ref_delta_enabled = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);

    if(prLoopFilter->mode_ref_delta_enabled) 
    {    
       prLoopFilter->mode_ref_delta_update = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
        
        if(prLoopFilter->mode_ref_delta_update) 
        {
            UINT32 i;

            for (i = 0; i < MAX_REF_LF_DELTAS_VP9; i++)
            {                
                if (u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId))
                {
                   prLoopFilter->ref_deltas[i] = i4VDEC_HAL_VP9_Read_Signed_Literal_Raw(u4CoreId, 6);
                }
            }

            for (i = 0; i < MAX_MODE_LF_DELTAS_VP9; i++)
            {                
                if (u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId))
                {
                   prLoopFilter->mode_deltas[i] = i4VDEC_HAL_VP9_Read_Signed_Literal_Raw(u4CoreId, 6);
                }
            }
        }
    }
}

//static inline INT32 get_msb(UINT32 n) {
  static inline INT32 get_msb(UINT32 n) {

  return 31 ^ __builtin_clz(n);
}

static INT32 i4VP9DecodeUnsignedMax(UINT32 u4CoreId, INT32 i4Max) 
{
    //VP9_PRINT_INFO("i4VP9DecodeUnsignedMax Shift bit =  %d", (i4Max > 0 ? get_msb(i4Max) + 1 : 0));
    // possible shift 0 bit here.
    INT32 i4Ret = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, (i4Max > 0 ? get_msb(i4Max) + 1 : 0));
    return i4Ret > i4Max ? i4Max : i4Ret;
}


static INT32 i4VP9Read_Delta_Q(UINT32 u4CoreId, INT32 *pDelta_Q) {
    INT32 u4Old = *pDelta_Q;
    *pDelta_Q = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId) ? i4VDEC_HAL_VP9_Read_Signed_Literal_Raw(u4CoreId, 4) : 0;
    return u4Old != *pDelta_Q;
}

static void vVP9SetupQuantization(VP9_COMMON_T * prCommon, VP9_UNCOMPRESSED_HEADER_T* prUnCompressed, MACROBLOCKD* prMBD, UINT32 u4CoreId) 
{
    INT32 u4Update = 0;    
    VP9_PRINT_INFO("setup quantization");
    prUnCompressed->u4BaseQIdx = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, QINDEX_BITS);
    
    VP9_PRINT_VERBOSE("Base Q = %d", prUnCompressed->u4BaseQIdx);
    
    u4Update |= i4VP9Read_Delta_Q(u4CoreId, &prUnCompressed->u4Y_DC_DELTA_Q);
    u4Update |= i4VP9Read_Delta_Q(u4CoreId, &prUnCompressed->u4C_DC_DELTA_Q);
    u4Update |= i4VP9Read_Delta_Q(u4CoreId, &prUnCompressed->u4C_AC_DELTA_Q);
    
    if (u4Update)
    {
        vVP9_Init_Dequantizer(prCommon);
    }
    
    prUnCompressed->u4Lossless = prUnCompressed->u4BaseQIdx == 0 &&
                 prUnCompressed->u4Y_DC_DELTA_Q == 0 &&
                 prUnCompressed->u4C_DC_DELTA_Q == 0 &&
                 prUnCompressed->u4C_AC_DELTA_Q == 0;

    //    prMBD->itxm_add = prMBD->lossless ? vp9_iwht4x4_add : vp9_idct4x4_add;
}


static void vVP9SetupSegmentation(SEGMENTATION *seg, UINT32 u4CoreId) 
{    
    int i, j;
    
    VP9_PRINT_INFO("setup segmentation");
    
    seg->update_map = 0;
    seg->update_data = 0;

    seg->enabled = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
    if (!seg->enabled)
    {    
        VP9_PRINT_INFO("segmentation not enabled");
        return;
    }
    // Segmentation map update
    seg->update_map = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
    if (seg->update_map) {
        for (i = 0; i < SEG_TREE_PROBS; i++)
            seg->tree_probs[i] = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId) ? u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 8)
                                                   : MAX_PROB;

        seg->temporal_update = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
        if (seg->temporal_update) {
            for (i = 0; i < PREDICTION_PROBS; i++)
                seg->pred_probs[i] = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId) ? u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 8)
                                                     : MAX_PROB;
        } else {
            for (i = 0; i < PREDICTION_PROBS; i++)
                seg->pred_probs[i] = MAX_PROB;
        }
    }
    
    //--------------------------------
    if((!vp9_segfeature_active(seg,0,SEG_LVL_ALT_LF)) &&
       (!vp9_segfeature_active(seg,1,SEG_LVL_ALT_LF)) &&
       (!vp9_segfeature_active(seg,2,SEG_LVL_ALT_LF)) &&
       (!vp9_segfeature_active(seg,3,SEG_LVL_ALT_LF)) &&
       (!vp9_segfeature_active(seg,4,SEG_LVL_ALT_LF)) &&
       (!vp9_segfeature_active(seg,5,SEG_LVL_ALT_LF)) &&
       (!vp9_segfeature_active(seg,6,SEG_LVL_ALT_LF)) &&
       (!vp9_segfeature_active(seg,7,SEG_LVL_ALT_LF)))
    {
      seg->u4SegCtr = 0;
    }
    //--------------------------------

  // Segmentation data update
    seg->update_data = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
    if (seg->update_data) 
    {
        seg->abs_delta = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
        VP9_PRINT_INFO(" seg->abs_delta %d", seg->abs_delta);
        //--------------------------------
        seg->u4SegCtr  = 0;
        seg->u4SegCtr  = (seg->abs_delta & 0x1) << 8;
        seg->u4SegFeature_0_3 = 0;
        seg->u4SegFeature_4_7 = 0;
        //--------------------------------

        memset(seg->feature_data, 0, sizeof(seg->feature_data));    
        memset(seg->feature_mask, 0, sizeof(seg->feature_mask));

        for (i = 0; i < MAX_SEGMENTS; i++) 
        {
            for (j = 0; j < SEG_LVL_MAX; j++) 
            {
                INT32 data = 0;
                UINT32 feature_enabled = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
                
                if (feature_enabled) 
                {
                    //vp9 enable segfeature
                    seg->feature_mask[i] |= 1 << j;
                   // VP9_PRINT_INFO("seg %d. Enable Feature %d", i ,j);
                    data =  i4VP9DecodeUnsignedMax(u4CoreId, seg_feature_data_max[j]);
                    if (seg_feature_data_signed[j])
                        data = ((INT32)u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId)) ? -data : data;
                }
                
                ASSERT(data <= seg_feature_data_max[j]);
                if (data < 0) 
                {
                //   ASSERT(seg_feature_data_signed[j]);
                //    ASSERT(-data <= seg_feature_data_max[j]);
                }
                seg->feature_data[i][j] = data;
                
                //--------------------------------
        		if(j == SEG_LVL_ALT_LF)
        		{
		        	seg->u4SegCtr |= feature_enabled << i;
			
        			if(i < 4)
			        	seg->u4SegFeature_0_3 |= ((UINT32)(data & 0xff)) << i*8;
	        		else
	        			seg->u4SegFeature_4_7 |= ((UINT32)(data & 0xff)) << (i-4)*8;
        		}
		        //--------------------------------
            }
        }
    }
}

    
static void vVP9SetupTileInfo(VP9_UNCOMPRESSED_HEADER_T * prUnCompressed, UINT32 u4CoreId)
{
    int min_log2_tile_cols, max_log2_tile_cols, max_ones;
    vp9_get_tile_n_bits(prUnCompressed->u4MiCols, &min_log2_tile_cols, &max_log2_tile_cols);
   
    // columns
    max_ones = max_log2_tile_cols - min_log2_tile_cols;
    prUnCompressed->u4Log2TileCols = min_log2_tile_cols;
    while (max_ones-- && u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId))
        prUnCompressed->u4Log2TileCols++;

    // rows
    prUnCompressed->u4Log2TileRows = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
    if (prUnCompressed->u4Log2TileRows)
        prUnCompressed->u4Log2TileRows += u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
}

static INT32 iVP9GetFreeFb(VP9_COMMON_T * prCommon) 
{
    UINT32 i;
     VP9_PRINT_INFO("---iVP9GetFreeFb enter-----");
    //for (i = 0; i < FRAME_BUFFERS; i++)
    for (i = 0; i < MAX_FRAMES-1; i++)
    if (prCommon->FRAME_BUFS[i].u4RefCount == 0)
      break;

    VP9_PRINT_INFO("Got Free Frame, idx:%d", i);
    //ASSERT(i < FRAME_BUFFERS);  
   
    prCommon->FRAME_BUFS[i].u4RefCount = 1;
    return i;
}

static void vVP9RefCntFb(VP9_REF_CNT_BUF_T *bufs, int *idx, int new_idx) {
  const int ref_index = *idx;

  if (ref_index >= 0 && bufs[ref_index].u4RefCount > 0)
    bufs[ref_index].u4RefCount--;

  *idx = new_idx;

  bufs[new_idx].u4RefCount++;
}

static void vVP9FrameBufferConfig(VP9_COMMON_T * prCommon)
{
    UINT32 i;
    UINT32 u4SBWidth, u4SBHeight;    
    UINT32 u4SB4KWidth, u4SB4KHeight;
    UINT32 PIC_SIZE4K_Y,PIC_SIZE4K_C, Y_OFFSET;    
    UINT32 PIC_SIZE_Y,PIC_SIZE_C;
    UINT32 UFO_LEN_SIZE4K_Y, UFO_LEN_SIZE4K_C;
    UINT32 UFO_LEN_SIZE_Y, UFO_LEN_SIZE_C;
    UINT32 u4MV_BUF_SIZE;
    UINT32 u4DramPic4KY_Y_LENSize;
    UINT32 u4DramPic4KC_C_LENSize;
    UINT32 u4DramPicY_Y_LENSize;
    UINT32 u4DramPicC_C_LENSize;
    VP9_REF_CNT_BUF_T* prFrameBuffer;

    // reserved 4k
   u4SB4KWidth = ((4096 + 63) >> 6); // # of SB
    u4SB4KHeight = ((2304 + 63) >> 6); // # of SB

 // u4SB4KWidth = ((4096/2 + 63) >> 6); // # of SB
 // u4SB4KHeight = ((2304/2 + 63) >> 6); // # of SB

    //u4SB4KWidth = ((2048 + 63) >> 6); // # of SB
    //u4SB4KHeight = ((1152 + 63) >> 6); // # of SB
	
    u4MV_BUF_SIZE = u4SB4KWidth * u4SB4KHeight * 36 * 16;
    
    PIC_SIZE4K_Y = (u4SB4KWidth * u4SB4KHeight) << (6 + 6);    
    PIC_SIZE4K_C = PIC_SIZE4K_Y >> 1;
    Y_OFFSET = (MAX_FRAMES * PIC_SIZE4K_Y);
    
    // actual pic 
    u4SBWidth = ((prCommon->rUnCompressedHeader.u4Width + 63) >> 6);
    u4SBHeight = ((prCommon->rUnCompressedHeader.u4Height+ 63) >> 6);

    PIC_SIZE_Y =  (u4SBWidth * u4SBHeight) << (6 + 6);    
    PIC_SIZE_C = PIC_SIZE_Y >> 1;

    prFrameBuffer =  prCommon->FRAME_BUFS;

    if(prCommon->fgUFOModeEnable)
    {
        // 4k
        UFO_LEN_SIZE4K_Y = ((((PIC_SIZE4K_Y + 255)>> 8)+ 63 + (16*8)) >> 6 ) << 6;
        UFO_LEN_SIZE4K_C = (((UFO_LEN_SIZE4K_Y >> 1) + 15 + (16*8)) >> 4) << 4;
        u4DramPic4KY_Y_LENSize  =  (((PIC_SIZE4K_Y + UFO_LEN_SIZE4K_Y) + 8191) >> 13) << 13;
        u4DramPic4KC_C_LENSize  = (((PIC_SIZE4K_C + UFO_LEN_SIZE4K_C ) + 8191) >> 13) << 13;

        // actual
        UFO_LEN_SIZE_Y = ((((PIC_SIZE_Y + 255)>> 8)+ 63 + (16*8)) >> 6 ) << 6;
        UFO_LEN_SIZE_C = (((UFO_LEN_SIZE_Y >> 1) + 15 + (16*8)) >> 4) << 4;
        u4DramPicY_Y_LENSize  =  (((PIC_SIZE_Y + UFO_LEN_SIZE_Y) + 8191) >> 13) << 13;
        u4DramPicC_C_LENSize  = (((PIC_SIZE_C + UFO_LEN_SIZE_C ) + 8191) >> 13) << 13;

        // won't use...
        prFrameBuffer[prCommon->u4NewFbIdx].rBuf.u4DramPicY_Y_LENSize = u4DramPicY_Y_LENSize;
        prFrameBuffer[prCommon->u4NewFbIdx].rBuf.u4DramPicC_C_LENSize = u4DramPicC_C_LENSize;
        // ~
        prFrameBuffer[prCommon->u4NewFbIdx].rBuf.rUFO_LEN_Y.u4BufSize = UFO_LEN_SIZE_Y;
        prFrameBuffer[prCommon->u4NewFbIdx].rBuf.rUFO_LEN_C.u4BufSize = UFO_LEN_SIZE_C;
        
        VP9_PRINT_INFO("u4DramPicY_Y_LENSize 0x%08X", u4DramPicY_Y_LENSize);
        VP9_PRINT_INFO("u4DramPicC_C_LENSize 0x%08X", u4DramPicC_C_LENSize);
        VP9_PRINT_INFO("u4UFO_LEN_SIZE_Y 0x%08X", UFO_LEN_SIZE_Y);
        VP9_PRINT_INFO("u4UFO_LEN_SIZE_C 0x%08X", UFO_LEN_SIZE_C);
        Y_OFFSET = (MAX_FRAMES * u4DramPic4KY_Y_LENSize);
    }
    prFrameBuffer[prCommon->u4NewFbIdx].rBuf.rBufY.u4BufSize = PIC_SIZE_Y;
    prFrameBuffer[prCommon->u4NewFbIdx].rBuf.rBufC.u4BufSize = PIC_SIZE_C;
    VP9_PRINT_INFO("(Actual) PIC_SIZE_Y %d(0x%08X), PIC_SIZE_C %d(0x%08X)",PIC_SIZE_Y,PIC_SIZE_Y,PIC_SIZE_C,PIC_SIZE_C);
    if(prCommon->fgFrameBufferConfiged)
    {
        return;
    }
    
    VP9_PRINT_INFO("=========== VP9 Frame Buffer Configure ===========");

    VP9_PRINT_INFO("(4K) PIC_SIZE_Y %d(0x%08X), PIC_SIZE_C %d(0x%08X)",PIC_SIZE4K_Y,PIC_SIZE4K_Y,PIC_SIZE4K_C,PIC_SIZE4K_C);
    VP9_PRINT_INFO("Y_OFFSET %d(0x%08X)", Y_OFFSET, Y_OFFSET);
    
    for ( i = 0; i < MAX_FRAMES; i++ )
    {
        // UFO Mode reserved
        if(prCommon->fgUFOModeEnable)
        {
            prFrameBuffer[i].rBuf.rBufY.ulBufVAddr = prCommon->rDramDpb.ulBufVAddr + (i * u4DramPic4KY_Y_LENSize);
            prFrameBuffer[i].rBuf.rUFO_LEN_Y.ulBufVAddr = prFrameBuffer[i].rBuf.rBufY.ulBufVAddr + PIC_SIZE4K_Y;
            
            prFrameBuffer[i].rBuf.rBufC.ulBufVAddr = prCommon->rDramDpb.ulBufVAddr + Y_OFFSET + (i * u4DramPic4KC_C_LENSize);
            prFrameBuffer[i].rBuf.rUFO_LEN_C.ulBufVAddr = prFrameBuffer[i].rBuf.rBufC.ulBufVAddr + PIC_SIZE4K_C;
            
            VP9_PRINT_INFO("_VP9_FBM_YAddr[%d]:0x%lx (0x%08X)", i, prFrameBuffer[i].rBuf.rBufY.ulBufVAddr, 
                        PHYSICAL(prFrameBuffer[i].rBuf.rBufY.ulBufVAddr));
            
            VP9_PRINT_INFO("_VP9_FBM_Y_LEN_Addr[%d]:0x%lx (0x%08X)", i, prFrameBuffer[i].rBuf.rUFO_LEN_Y.ulBufVAddr,
                           PHYSICAL(prFrameBuffer[i].rBuf.rUFO_LEN_Y.ulBufVAddr));
            
            VP9_PRINT_INFO("_VP9_FBM_CAddr[%d]:0x%lx (0x%08X)", i, prFrameBuffer[i].rBuf.rBufC.ulBufVAddr, 
                           PHYSICAL(prFrameBuffer[i].rBuf.rBufC.ulBufVAddr));
            
            VP9_PRINT_INFO("_VP9_FBM_C_LEN_Addr[%d]:0x%lx (0x%08X)", i, prFrameBuffer[i].rBuf.rUFO_LEN_C.ulBufVAddr, 
                           PHYSICAL(prFrameBuffer[i].rBuf.rUFO_LEN_C.ulBufVAddr));
            
            // DRAM Foot Print for UFO Mode Currently
            // ----------------------
            // |        Y           |   Y[0] (8K Alignment for 1 Y_BS)
            // ----------------------
            // |      Y_LEN         |
            // ----------------------
            // |        Y           |   Y[1]
            // ----------------------
            // |      Y_LEN         |
            // ----------------------           
            //          .
            //          .
            //
            // ----------------------
            // |        Y           |    Y[7]
            // ----------------------
            // |      Y_LEN         |
            // ----------------------
            // |        C           |    C[0]
            // ----------------------
            // |      Y_LEN         |
            // ----------------------           
            // |        C           |    C[1]
            // ----------------------
            // |      C_LEN         |
            // ----------------------            
            //          .
            //          .
            // ----------------------           
            // |        C           |    C[7]
            // ----------------------
            // |      C_LEN         |
            // ----------------------    
            // |      MV_Buffer     |    MV[0]
            // ----------------------
            // |    TILE_Buffer     |    TILE
            // ----------------------
            
        } 
        else 
        {
            prFrameBuffer[i].rBuf.rBufY.ulBufVAddr = prCommon->rDramDpb.ulBufVAddr + (i * PIC_SIZE4K_Y);
            prFrameBuffer[i].rBuf.rBufC.ulBufVAddr = prCommon->rDramDpb.ulBufVAddr + Y_OFFSET + (i * PIC_SIZE4K_C);

            VP9_PRINT_INFO("_VP9_FBM_YAddr[%d]:0x%lx (0x%08X)", i, prFrameBuffer[i].rBuf.rBufY.ulBufVAddr,
                           PHYSICAL(prFrameBuffer[i].rBuf.rBufY.ulBufVAddr));
            
            VP9_PRINT_INFO("_VP9_FBM_CAddr[%d]:0x%lx (0x%08X)", i, prFrameBuffer[i].rBuf.rBufC.ulBufVAddr,
                           PHYSICAL(prFrameBuffer[i].rBuf.rBufC.ulBufVAddr));

        #if 0
            if((Dpb_addr[i] + PIC_SIZE) >= (g2DPB[u4InstId] + PATTERN_VP9_FBM_SZ))
            {
                UTIL_Printf("Error ++++++++++++++++++++++++++++++++++\n");
                UTIL_Printf("==========DPB Size Not Enough========\n");
                UTIL_Printf("Error ++++++++++++++++++++++++++++++++++\n");
            }
        #endif
        }
    }

    // append to last c frame as mv buffer
    for( i = 0; i < MAX_VP9_MV_BUF; i++)
    {
        if(prCommon->fgUFOModeEnable)
        {
            prCommon->rMVBuffer[i].ulBufVAddr = (prFrameBuffer[MAX_FRAMES-1].rBuf.rBufC.ulBufVAddr + u4DramPic4KC_C_LENSize) + (i*u4MV_BUF_SIZE);
            VP9_PRINT_INFO("_VP9_MV_Addr[%d]:0x%lx (0x%08X)", i, prCommon->rMVBuffer[i].ulBufVAddr, PHYSICAL(prCommon->rMVBuffer[i].ulBufVAddr));
        }
        else
        {
            prCommon->rMVBuffer[i].ulBufVAddr = (prFrameBuffer[MAX_FRAMES-1].rBuf.rBufC.ulBufVAddr + PIC_SIZE4K_C) + (i*u4MV_BUF_SIZE);
            VP9_PRINT_INFO("_VP9_MV_Addr[%d]:0x%lx (0x%08X)", i, prCommon->rMVBuffer[i].ulBufVAddr, PHYSICAL(prCommon->rMVBuffer[i].ulBufVAddr));
        }
    }

    prCommon->fgFrameBufferConfiged = TRUE;
   
    VP9_PRINT_INFO("=================== End ===================");
    return;
}

static INT32 iVP9ReAllocFrameBuffer(VP9_COMMON_T * prCommon) {
    VP9_UNCOMPRESSED_HEADER_T* prUnCompressed = &prCommon->rUnCompressedHeader;
    const int aligned_width = (prUnCompressed->u4Width + 63) & ~63;
    const int aligned_height = (prUnCompressed->u4Height + 63) & ~63;
//      const int y_stride = ((aligned_width + 2 * border) + 31) & ~31;
//      const int yplane_size = (aligned_height + 2 * border) * y_stride;
    const int uv_width = aligned_width >> prUnCompressed->u4SubSampling_X;
    const int uv_height = aligned_height >> prUnCompressed->u4SubSampling_Y;
//      const int uv_stride = y_stride >> ss_x;
//      const int uv_border_w = border >> ss_x;
//      const int uv_border_h = border >> ss_y;
//      const int uvplane_size = (uv_height + 2 * uv_border_h) * uv_stride;
    
//  #define CONFIG_ALPHA 0
//  #if CONFIG_ALPHA
//      const int alpha_width = aligned_width;
//      const int alpha_height = aligned_height;
//      const int alpha_stride = y_stride;
//      const int alpha_border_w = border;
//      const int alpha_border_h = border;
//      const int alpha_plane_size = (alpha_height + 2 * alpha_border_h) *
//                                   alpha_stride;
//      const int frame_size = yplane_size + 2 * uvplane_size +
//                             alpha_plane_size;
//  #else
//      const int frame_size = yplane_size + 2 * uvplane_size;
//  #endif


    /* Only support allocating buffers that have a border that's a multiple
     * of 32. The border restriction is required to get 16-byte alignment of
     * the start of the chroma rows without introducing an arbitrary gap
     * between planes, which would break the semantics of things like
     * vpx_img_set_rect(). */
//      if (border & 0x1f)
//        return -3;
    VP9_FB_INFO_T *prFrameInfo = &(prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf);

    prFrameInfo->u4YCropWidth = prUnCompressed->u4Width;
    prFrameInfo->u4YCropHeight = prUnCompressed->u4Height;
    prFrameInfo->u4YWidth  = aligned_width;
    prFrameInfo->u4YHeight = aligned_height;
//      prFrameInfo->u4YStride = y_stride;

    prFrameInfo->u4CCropWidth = (prUnCompressed->u4Width + prUnCompressed->u4SubSampling_X) >> prUnCompressed->u4SubSampling_X;
    prFrameInfo->u4CCropHeight = (prUnCompressed->u4Height + prUnCompressed->u4SubSampling_Y) >> prUnCompressed->u4SubSampling_Y;
    prFrameInfo->u4CWidth = uv_width;
    prFrameInfo->u4CHeight = uv_height;
//      prFrameInfo->u4CStride = uv_stride;

//      prFrameInfo->border = border;
//      prFrameInfo->frame_size = frame_size;

//      ybf->y_buffer = ybf->buffer_alloc + (border * y_stride) + border;
//      ybf->u_buffer = ybf->buffer_alloc + yplane_size +
//                      (uv_border_h * uv_stride) + uv_border_w;
//      ybf->v_buffer = ybf->buffer_alloc + yplane_size + uvplane_size +
//                      (uv_border_h * uv_stride) + uv_border_w;

//  #if CONFIG_ALPHA
//      ybf->alpha_width = alpha_width;
//      ybf->alpha_height = alpha_height;
//      ybf->alpha_stride = alpha_stride;
//      ybf->alpha_buffer = ybf->buffer_alloc + yplane_size + 2 * uvplane_size +
//                          (alpha_border_h * alpha_stride) + alpha_border_w;
//  #endif
//      ybf->corrupted = 0; /* assume not corrupted by errors */
    return 0;
}

static void vVP9_Set_MB_MI(VP9_COMMON_T *prCommon, UINT32 u4Aligned_width, UINT32 u4Aligned_height) 
{
  prCommon->u4MI_cols = u4Aligned_width >> MI_SIZE_LOG2;
  prCommon->u4MI_rows = u4Aligned_height >> MI_SIZE_LOG2;
  prCommon->u4MI_stride = prCommon->u4MI_cols + MI_BLOCK_SIZE;

  prCommon->u4MB_cols = (prCommon->u4MI_cols + 1) >> 1;
  prCommon->u4MB_rows = (prCommon->u4MI_rows + 1) >> 1;
  prCommon->u4MBs = prCommon->u4MB_rows * prCommon->u4MB_cols;
}

void vp9_update_frame_size(VP9_COMMON_T *prCommon)
{
  UINT32 u4Aligned_width = ALIGN_POWER_OF_TWO(prCommon->rUnCompressedHeader.u4Width, MI_SIZE_LOG2);
  UINT32 u4Aligned_height = ALIGN_POWER_OF_TWO(prCommon->rUnCompressedHeader.u4Height, MI_SIZE_LOG2);

  vVP9_Set_MB_MI(prCommon, u4Aligned_width, u4Aligned_height);

  // leon tmp
  prCommon->rUnCompressedHeader.u4MiCols = prCommon->u4MI_cols;
  
  //SEG_ID_RESET  
  memset((void*)prCommon->rDramSegId0.ulBufVAddr, 0, prCommon->rDramSegId0.u4BufSize);//qianqian@20150215
  memset((void*)prCommon->rDramSegId1.ulBufVAddr, 0, prCommon->rDramSegId1.u4BufSize);

  #if 0
  setup_mi(cm);

  // Initialize the previous frame segment map to 0.
  if (cm->last_frame_seg_map)
    vpx_memset(cm->last_frame_seg_map, 0, cm->mi_rows * cm->mi_cols);
  #endif
}

static void vVP9ApplyFrameSize(VP9_COMMON_T * prCommon, UINT32 u4Width, UINT32 u4Height) {
    
    VP9_PRINT_INFO("apply frame size [%d x %d]", u4Width, u4Height);
    if (prCommon->rUnCompressedHeader.u4Width != u4Width || prCommon->rUnCompressedHeader.u4Height != u4Height)
    {
        // Change in frame size.
        // TODO(agrange) Don't test width/height, check overall size.
        //    if (u4CurWidth > prCommon->u4Width || u4CurHeight > prCommon->u4Height) {
        //      // Rescale frame buffers only if they're not big enough already.
        //      if (vp9_resize_frame_buffers(cm, width, height))
        //        vpx_internal_error(&cm->error, VPX_CODEC_MEM_ERROR,
        //                           "Failed to allocate frame buffers");
        //    }

        prCommon->rUnCompressedHeader.u4Width = u4Width;
        prCommon->rUnCompressedHeader.u4Height = u4Height;

        vp9_update_frame_size(prCommon);
    }

    vVP9FrameBufferConfig(prCommon);

    if (VP9_OK != iVP9ReAllocFrameBuffer(prCommon)) {
      VP9_PRINT_ERROR("Failed to allocate frame buffer %d,%d", u4Width, u4Height);
      prCommon->eErrno = VP9_ERR_MEM_ERROR;
      return; // VP9_FAIL; //qianqian @20150215
    }

}

static void vVP9SetupDisplaySize(UINT32 u4CoreId, VP9_COMMON_T * prCommon) {
  prCommon->u4DisplayWidth = prCommon->rUnCompressedHeader.u4Width;
  prCommon->u4DiaplayHeight = prCommon->rUnCompressedHeader.u4Height;
  if (u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId))
  {
      prCommon->u4DisplayWidth= u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 16) + 1;
      prCommon->u4DiaplayHeight = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 16) + 1;
  }  
  VP9_PRINT_INFO("apply display size [%d x %d]", prCommon->u4DisplayWidth, prCommon->u4DiaplayHeight);
}

static INT32 i4VP9SetupFrameSize(UINT32 u4CoreId, VP9_COMMON_T * prCommon) 
{
    UINT32 u4CurWidth = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 16) + 1;
    UINT32 u4CurHeight = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 16) + 1;
    vVP9ApplyFrameSize(prCommon, u4CurWidth, u4CurHeight);
    vVP9SetupDisplaySize(u4CoreId, prCommon);   
}

static INT32 i4VP9SetupFrameSizeWithRefs(UINT32 u4CoreId, VP9_COMMON_T * prCommon) {
  UINT32 u4Width, u4Height;
  UINT32 u4Found = 0, i;

  for (i = 0; i < REFS_PER_FRAME; ++i) {
    if (u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId)) {
      u4Width = prCommon->FRAME_REFS[i].prBuf->u4YCropWidth;
      u4Height = prCommon->FRAME_REFS[i].prBuf->u4YCropHeight;
      u4Found = 1;
      break;
    }
  }

  if (!u4Found)
  {
      u4Width= u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 16) + 1;
      u4Height = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 16) + 1;
  }

  if (u4Width <= 0 || u4Height <= 0)
  {
      VP9_PRINT_ERROR("Referenced frame with invalid size [%d x %d]", u4Width, u4Height);
      prCommon->eErrno = VP9_ERR_STREAM_CORRUPT;
      return VP9_FAIL;
  }
  
  vVP9ApplyFrameSize(prCommon, u4Width, u4Height);

  vVP9SetupDisplaySize(u4CoreId, prCommon);
  return VP9_OK;
}

static void vVP9RefCountFB(VP9_COMMON_T * prCommon, UINT32 *pu4Idx, UINT32 u4NewIdx) {
    UINT32 u4RefIdx = *pu4Idx;
    if (u4RefIdx >= 0 && prCommon->FRAME_BUFS[u4RefIdx].u4RefCount > 0)
        prCommon->FRAME_BUFS[u4RefIdx].u4RefCount--;

    *pu4Idx = u4NewIdx;
    prCommon->FRAME_BUFS[u4NewIdx].u4RefCount++;
}

/* If any buffer updating is signaled it should be done here. */
static void vVP9SwapFrameBuffers(VP9_COMMON_T* prCommon) 
{
    VP9_UNCOMPRESSED_HEADER_T* prUnCompress = &prCommon->rUnCompressedHeader;

    INT32 ref_index = 0, mask;
    for (mask = prUnCompress->u4RefreshFrameFlags; mask; mask >>= 1)
    {
        if (mask & 1) 
        {
            const int old_idx = prCommon->REF_FRAME_MAP[ref_index];
            vVP9RefCntFb(prCommon->FRAME_BUFS, &prCommon->REF_FRAME_MAP[ref_index], prCommon->u4NewFbIdx);
            
        }
        ++ref_index;
    }

    prCommon->prFrameToShow = &prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf;

    --prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].u4RefCount;

    // Invalidate these references until the next frame starts.
    for (ref_index = 0; ref_index < 3; ref_index++)
        prCommon->FRAME_REFS[ref_index].u4Idx = INT_MAX;
   #if 0
    for( ref_index = 0; ref_index < MAX_FRAMES; ref_index++)
    {        
        VP9_PRINT_INFO("FRAME_BUFS[%d].RefCount = %d", ref_index, prCommon->FRAME_BUFS[ref_index].u4RefCount);
    }
   #endif  
}

void vVP9SwapMiAndPrevMi(VP9_COMMON_T* prCommon) {
  // Swap indices.
//	    const int tmp = cm->mi_idx;
//	    cm->mi_idx = cm->prev_mi_idx;
//	    cm->prev_mi_idx = tmp;
//	
//	    // Current mip will be the prev_mip for the next frame.
//	    cm->mip = cm->mip_array[cm->mi_idx];
//	    cm->prev_mip = cm->mip_array[cm->prev_mi_idx];
//	    cm->mi_grid_base = cm->mi_grid_base_array[cm->mi_idx];
//	    cm->prev_mi_grid_base = cm->mi_grid_base_array[cm->prev_mi_idx];
//	
//	    // Update the upper left visible macroblock ptrs.
//	    cm->mi = cm->mip + cm->mi_stride + 1;
//	    cm->prev_mi = cm->prev_mip + cm->mi_stride + 1;
//	    cm->mi_grid_visible = cm->mi_grid_base + cm->mi_stride + 1;
//	    cm->prev_mi_grid_visible = cm->prev_mi_grid_base + cm->mi_stride + 1;
}


static void vVP9SetupPastIndependence(VP9_COMMON_T * prCommon)
{
    UINT32 i;    
    VP9_UNCOMPRESSED_HEADER_T* prUnCompressed = &prCommon->rUnCompressedHeader;
    VP9_PRINT_INFO("setup past independence");
    //SEG_ID_RESET
    ASSERT(prCommon->rDramSegId0.ulBufVAddr);
    memset((void*)prCommon->rDramSegId0.ulBufVAddr, 0, prCommon->rDramSegId0.u4BufSize); //qianqian@20150215      
    memset((void*)prCommon->rDramSegId1.ulBufVAddr, 0, prCommon->rDramSegId1.u4BufSize);
    
    // reset feature    
    memset(prUnCompressed->seg.feature_data, 0, sizeof(prUnCompressed->seg.feature_data));    
    memset(prUnCompressed->seg.feature_mask, 0, sizeof(prUnCompressed->seg.feature_mask));
    prUnCompressed->seg.abs_delta = SEGMENT_DELTADATA;

    #if 0
    if (prCommon->last_frame_seg_map)
        vpx_memset(cm->last_frame_seg_map, 0, (cm->mi_rows * cm->mi_cols));
    #endif
  
    // Reset the mode ref deltas for loop filter
    memset(prUnCompressed->rLoopFilter.last_ref_deltas, 0, sizeof(prUnCompressed->rLoopFilter.last_ref_deltas));    
    memset(prUnCompressed->rLoopFilter.last_mode_deltas, 0, sizeof(prUnCompressed->rLoopFilter.last_mode_deltas));

    vVP9_Set_Default_LF_Deltas(&prUnCompressed->rLoopFilter);
    
    // To force update of the sharpness
    prUnCompressed->rLoopFilter.last_sharpness_level = -1;

    // initialize coeff prob table to default
    memcpy(prCommon->fc.coef_probs[TX_4X4], default_coef_probs_4x4, sizeof(default_coef_probs_4x4));
    memcpy(prCommon->fc.coef_probs[TX_8X8], default_coef_probs_8x8, sizeof(default_coef_probs_8x8));
    memcpy(prCommon->fc.coef_probs[TX_16X16], default_coef_probs_16x16, sizeof(default_coef_probs_16x16));
    memcpy(prCommon->fc.coef_probs[TX_32X32], default_coef_probs_32x32, sizeof(default_coef_probs_32x32));

    //initialize mode probs to default
    memcpy(prCommon->fc.uv_mode_prob, default_if_uv_probs, sizeof(default_if_uv_probs));
    memcpy(prCommon->fc.y_mode_prob, default_if_y_probs, sizeof(default_if_y_probs));
    memcpy(prCommon->fc.switchable_interp_prob, default_switchable_interp_prob, sizeof(default_switchable_interp_prob));
    memcpy(prCommon->fc.partition_prob, default_partition_probs, sizeof(default_partition_probs));
    memcpy(prCommon->fc.intra_inter_prob, default_intra_inter_p, sizeof(default_intra_inter_p));
    memcpy(prCommon->fc.comp_inter_prob, default_comp_inter_p, sizeof(default_comp_inter_p));
    memcpy(prCommon->fc.comp_ref_prob, default_comp_ref_p, sizeof(default_comp_ref_p));
    memcpy(prCommon->fc.single_ref_prob, default_single_ref_p, sizeof(default_single_ref_p));
    prCommon->fc.tx_probs = default_tx_probs;
    memcpy(prCommon->fc.skip_probs, default_skip_probs, sizeof(default_skip_probs));
    memcpy(prCommon->fc.inter_mode_probs, default_inter_mode_probs, sizeof(default_inter_mode_probs));

    //initialize mv probs to default
    prCommon->fc.nmvc = default_nmv_context;


    if (prUnCompressed->u4FrameType == KEY_FRAME ||
        prUnCompressed->u4ErrResilenceMode || prUnCompressed->u4ResetFrameContext == 3) {
        // Reset all frame contexts.
        for (i = 0; i < FRAME_CONTEXTS; ++i)
            prCommon->frame_contexts[i] = prCommon->fc;
    } else if (prUnCompressed->u4ResetFrameContext == 2) {
        // Reset only the frame context specified in the frame header.
        prCommon->frame_contexts[prUnCompressed->u4FrameContextIdx] = prCommon->fc;
    }
    
    memset(prCommon->REF_FRAME_SIGN_BIAS, 0, sizeof(prCommon->REF_FRAME_SIGN_BIAS));
    prUnCompressed->u4FrameContextIdx = 0;
}

INT32 iMax_RRF_BLK_Cal(INT32 iScale_Fac)
{
  int cuch_max_rrf_blk_result;
  int scale_ratio_1000;
    
  //printf("scale_fac = %d\n", scale_fac);

  scale_ratio_1000 = (iScale_Fac*1000)/16384;
  
  //  max rrf blk restriction
  //  rest1 : QIU buffer 23x23 for each bank
  //  rest2 : no 3_page_row DRAM access

  if(scale_ratio_1000 <= 1000)
    cuch_max_rrf_blk_result = 4;  //16 pixels 
  else if((scale_ratio_1000 > 1000) && (scale_ratio_1000 < 1800))
    cuch_max_rrf_blk_result = 3;  // 8 pixels
  else if(scale_ratio_1000 >= 1800)
    
#if RRF_NEW_FORMULA
    cuch_max_rrf_blk_result = 3; //20140618 modify for perf
#else
    cuch_max_rrf_blk_result = 2; // 4 pixels
#endif

  //printf("scale_ratio = %d, cuch_max_rrf_blk_result = %d\n", scale_ratio_1000, cuch_max_rrf_blk_result);

  return cuch_max_rrf_blk_result;
}

static INT32 iVP9_Check_Scale_Factors(INT32 other_w, INT32 other_h, INT32 this_w, INT32 this_h) 
{
  return 2 * this_w >= other_w &&
         2 * this_h >= other_h &&
         this_w <= 16 * other_w &&
         this_h <= 16 * other_h;
}

static INT32 iGet_Fixed_Point_Scale_Factor(INT32 other_size, INT32 this_size) 
{
  // Calculate scaling factor once for each reference frame
  // and use fixed point scaling factors in decoding and encoding routines.
  // Hardware implementations can calculate scale factor in device driver
  // and use multiplication and shifting on hardware instead of division.
  return (other_size << REF_SCALE_SHIFT) / this_size;
}

static inline INT32 iScaled_X(INT32 val, VP9_Scale_Factors_T *sf) 
{
    return (int)(val * sf->i4X_scale_fp >> REF_SCALE_SHIFT);
}

static inline INT32 iScaled_Y(INT32 val, VP9_Scale_Factors_T *sf) 
{
    return (int)(val * sf->i4Y_scale_fp >> REF_SCALE_SHIFT);
}

static inline INT32 iVP9_Is_Scaled(VP9_Scale_Factors_T *sf)
{
    return sf->i4X_scale_fp != REF_NO_SCALE || sf->i4Y_scale_fp != REF_NO_SCALE;
}

void vVP9_Setup_Scale_Factors_For_Frame(VP9_Scale_Factors_T *rScaleFactors,
                                       INT32 other_w, INT32 other_h,
                                       INT32 this_w, INT32 this_h) 
{
    if (!iVP9_Check_Scale_Factors(other_w, other_h, this_w, this_h)) 
    {
        rScaleFactors->i4X_scale_fp = REF_INVALID_SCALE;
        rScaleFactors->i4Y_scale_fp = REF_INVALID_SCALE;
        VP9_PRINT_ERROR("@@@@ REF_INVALID_SCALE @@@@@");
        return;
    }

    #if 0
    if(cuch_mc_dec_order_frame_idx == 4)
    {
      cuch_i=cuch_i;//cuch: gotoline
    }
    #endif
  
    rScaleFactors->i4X_scale_fp = iGet_Fixed_Point_Scale_Factor(other_w, this_w);
    rScaleFactors->i4Y_scale_fp = iGet_Fixed_Point_Scale_Factor(other_h, this_h);
    rScaleFactors->i4X_step_q4 = iScaled_X(16, rScaleFactors);
    rScaleFactors->i4Y_step_q4 = iScaled_Y(16, rScaleFactors);

    if((rScaleFactors->i4X_scale_fp != (1<<14))|| (rScaleFactors->i4Y_scale_fp != (1<<14)))
    {
        VP9_PRINT_INFO("RRF!!!\n"); 
    }
    
    VP9_PRINT_INFO("enter vp9_setup_scale_factors_for_frame...now x_scale_fp= %d, y_scale_fp= %d", 
                   rScaleFactors->i4X_scale_fp, rScaleFactors->i4Y_scale_fp);

    if (iVP9_Is_Scaled(rScaleFactors)) 
    {
        rScaleFactors->u4Ref_Scaling_EN = 1;
        VP9_PRINT_INFO("it's scaled for this frame");
    } 
    else 
    {
        rScaleFactors->u4Ref_Scaling_EN = 0;
        VP9_PRINT_INFO("NOT scaled for this frame");        
    }
    
#if 0
  // TODO(agrange): Investigate the best choice of functions to use here
  // for EIGHTTAP_SMOOTH. Since it is not interpolating, need to choose what
  // to do at full-pel offsets. The current selection, where the filter is
  // applied in one direction only, and not at all for 0,0, seems to give the
  // best quality, but it may be worth trying an additional mode that does
  // do the filtering on full-pel.
  if (sf->x_step_q4 == 16) {
    if (sf->y_step_q4 == 16) {
      // No scaling in either direction.
      sf->predict[0][0][0] = vp9_convolve_copy;
      sf->predict[0][0][1] = vp9_convolve_avg;
      sf->predict[0][1][0] = vp9_convolve8_vert;
      sf->predict[0][1][1] = vp9_convolve8_avg_vert;
      sf->predict[1][0][0] = vp9_convolve8_horiz;
      sf->predict[1][0][1] = vp9_convolve8_avg_horiz;
    } else {
      // No scaling in x direction. Must always scale in the y direction.
      sf->predict[0][0][0] = vp9_convolve8_vert;
      sf->predict[0][0][1] = vp9_convolve8_avg_vert;
      sf->predict[0][1][0] = vp9_convolve8_vert;
      sf->predict[0][1][1] = vp9_convolve8_avg_vert;
      sf->predict[1][0][0] = vp9_convolve8;
      sf->predict[1][0][1] = vp9_convolve8_avg;
    }
  } else {
    if (sf->y_step_q4 == 16) {
      // No scaling in the y direction. Must always scale in the x direction.
      sf->predict[0][0][0] = vp9_convolve8_horiz;
      sf->predict[0][0][1] = vp9_convolve8_avg_horiz;
      sf->predict[0][1][0] = vp9_convolve8;
      sf->predict[0][1][1] = vp9_convolve8_avg;
      sf->predict[1][0][0] = vp9_convolve8_horiz;
      sf->predict[1][0][1] = vp9_convolve8_avg_horiz;
    } else {
      // Must always scale in both directions.
      sf->predict[0][0][0] = vp9_convolve8;
      sf->predict[0][0][1] = vp9_convolve8_avg;
      sf->predict[0][1][0] = vp9_convolve8;
      sf->predict[0][1][1] = vp9_convolve8_avg;
      sf->predict[1][0][0] = vp9_convolve8;
      sf->predict[1][0][1] = vp9_convolve8_avg;
    }
  }
  // 2D subpel motion always gets filtered in both directions
  sf->predict[1][1][0] = vp9_convolve8;
  sf->predict[1][1][1] = vp9_convolve8_avg;
#endif
}

static INT32 i4VP9UnCompressedHeaderParse(VP9_COMMON_T * prCommon, VP9_UNCOMPRESSED_HEADER_T * prUnCompressed, UINT32 u4CoreId)
{
    INT32  i4RetVal;
    UINT32 i;
    UINT32 u4QIndex;
    VP9_INPUT_CTX_T* prInputCtx = &prCommon->rInputCtx;
    
    VP9_PRINT_INFO(" [#pic %d, %d]  uncompressed header parse\n", prCommon->u4FrameNum, prCommon->u4NewFbIdx);
    if (prCommon->u4NewFbIdx >= 0 && prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].u4RefCount == 0)
       	prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.u1InUse = 0;
   
    prCommon->u4NewFbIdx = iVP9GetFreeFb(prCommon);

    prCommon->u4LastFrameType = prUnCompressed->u4FrameType;  
    prCommon->pCurrentFB = &(prCommon->FRAME_BUFS[prCommon->u4NewFbIdx]);
    
    if (u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 2) != VP9_FRAME_MARKER)
    {        
        VP9_PRINT_ERROR("Invalid frame marker");
        prCommon->eErrno = VP9_ERR_STREAM_CORRUPT;
        return VP9_FAIL;
    }
    prUnCompressed->u4Profile = (u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId) | u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId) << 1);
    if (prUnCompressed->u4Profile >= MAX_PROFILES)
    {
        VP9_PRINT_ERROR("Unsupported bitstream profile");
        prCommon->eErrno = VP9_ERR_STREAM_UNSUP;
        return VP9_FAIL;
    }
    
    prUnCompressed->u4ShowExisting = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
    if (prUnCompressed->u4ShowExisting) 
    {
        // show an existing frame directly
        UINT32 u4FrameToShow = prCommon->REF_FRAME_MAP[u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 3)];        
//       VP9_PRINT_INFO("show existing frame: %d", u4FrameToShow);

        if (prCommon->FRAME_BUFS[u4FrameToShow].u4RefCount < 1)
        {            
            VP9_PRINT_ERROR("buffer %d does not contain a decoded frame",u4FrameToShow);
            prCommon->eErrno = VP9_ERR_STREAM_CORRUPT;
            return VP9_FAIL;
        }
        
        vVP9RefCountFB(prCommon, &prCommon->u4NewFbIdx, u4FrameToShow);
   
        prUnCompressed->u4RefreshFrameFlags = 0;
        prUnCompressed->rLoopFilter.filter_level = 0;
        prUnCompressed->u4ShowFrame = 1;
        return VP9_SKIP_FRAME;
    }
    
    prUnCompressed->u4FrameType = (FRAME_TYPE) u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
    prUnCompressed->u4ShowFrame = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
    prUnCompressed->u4ErrResilenceMode = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
    if (prUnCompressed->u4FrameType == KEY_FRAME) {
        if(VP9_OK != i4VP9CheckSyncCode(u4CoreId))
        {            
            prCommon->eErrno = VP9_ERR_STREAM_CORRUPT;
            return VP9_FAIL;
        }
        if (prUnCompressed->u4Profile > PROFILE_1)
            prUnCompressed->u4BitDepth = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId) ? BITS_12 : BITS_10;

        prUnCompressed->u4ColorSpace = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 3);  // colorspace        
//        VP9_PRINT_INFO("color space: %d", prUnCompressed->u4ColorSpace);
        if (prUnCompressed->u4ColorSpace != SRGB) {
            u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);  // [16,235] (including xvycc) vs [0,255] range
            if (prUnCompressed->u4Profile >= PROFILE_1) {
                prUnCompressed->u4SubSampling_X = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
                prUnCompressed->u4SubSampling_Y = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
                u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);  // has extra plane
            } else {
                prUnCompressed->u4SubSampling_Y = prUnCompressed->u4SubSampling_X = 1;
            }
        } else {
            if (prUnCompressed->u4Profile >= PROFILE_1) {
                prUnCompressed->u4SubSampling_Y = prUnCompressed->u4SubSampling_X = 0;
                u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);  // has extra plane
            } else {
                VP9_PRINT_ERROR("RGB not supported in profile 0");
                prCommon->eErrno = VP9_ERR_STREAM_UNSUP;                
                return VP9_FAIL;
            }
            
            VP9_PRINT_ERROR("RGB not supported by MTK");
            prCommon->eErrno = VP9_ERR_STREAM_UNSUP;
            return VP9_FAIL;
        }
//        VP9_PRINT_INFO("sub sampling x:%d y:%d", prUnCompressed->u4SubSampling_X, prUnCompressed->u4SubSampling_Y);

        prUnCompressed->u4RefreshFrameFlags = (1 << REF_FRAMES) - 1;

        for (i = 0; i < REFS_PER_FRAME; ++i)
        {
            prCommon->FRAME_REFS[i].u4Idx = prCommon->u4NewFbIdx;
            prCommon->FRAME_REFS[i].prBuf = &prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf;
        }
        
        i4VP9SetupFrameSize(u4CoreId, prCommon);

    }
    else {
        prUnCompressed->u4IntraOnly = prUnCompressed->u4ShowFrame ? 0 : u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);        
        VP9_PRINT_INFO("intra only: %d", prUnCompressed->u4IntraOnly);
        prUnCompressed->u4ResetFrameContext = prUnCompressed->u4ErrResilenceMode ? 0 : u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 2);        
//        VP9_PRINT_INFO("frame context reset flag: %d", prUnCompressed->u4ResetFrameContext);
        if (prUnCompressed->u4IntraOnly) {
            if(VP9_OK != i4VP9CheckSyncCode(u4CoreId))
            {
                prCommon->eErrno = VP9_ERR_STREAM_CORRUPT;
                return VP9_FAIL; 
            }
            prUnCompressed->u4RefreshFrameFlags = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, REF_FRAMES);
            i4VP9SetupFrameSize(u4CoreId, prCommon);
	
        } else {
            prUnCompressed->u4RefreshFrameFlags = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, REF_FRAMES);
			
            for (i = 0; i < REFS_PER_FRAME; ++i) {                
                UINT32 u4Ref = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, REF_FRAMES_LOG2);
                UINT32 u4FbIdx = prCommon->REF_FRAME_MAP[u4Ref];
                prCommon->FRAME_REFS[i].u4Idx = u4FbIdx;
                prCommon->FRAME_REFS[i].prBuf = &prCommon->FRAME_BUFS[u4FbIdx].rBuf;
                prCommon->REF_FRAME_SIGN_BIAS[LAST_FRAME + i] = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);             
            }
	
            if(VP9_OK != i4VP9SetupFrameSizeWithRefs(u4CoreId, prCommon))
                return VP9_FAIL;
		
	      prUnCompressed->u4AllowHighPrecisionMv = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
            prUnCompressed->eInterpFilterType = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId)? SWITCHABLE : literal_to_filter[u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 2)];
	
            for (i = 0; i < REFS_PER_FRAME; ++i) 
            {
                 VP9_REF_BUF_T *rRefBuf = &prCommon->FRAME_REFS[i];
                 vVP9_Setup_Scale_Factors_For_Frame(&rRefBuf->rScaleFactors,
                                                     rRefBuf->prBuf->u4YCropWidth,
                                                     rRefBuf->prBuf->u4YCropHeight,
                                                     prCommon->rUnCompressedHeader.u4Width, 
                                                     prCommon->rUnCompressedHeader.u4Height);
                #if 0
                if (vp9_is_scaled(&ref_buf->sf))
                    vp9_extend_frame_borders(ref_buf->buf);
                #endif
            }
	 }
    }
  
//  VP9_PRINT_INFO("refresh frame flag [0x%x]", prUnCompressed->u4RefreshFrameFlags);
//  VP9_PRINT_INFO("frame_refs: [%d %d %d]", cm->frame_refs[0].idx, cm->frame_refs[1].idx, cm->frame_refs[2].idx);

    if (!prUnCompressed->u4ErrResilenceMode) 
    {
        prUnCompressed->fgUse_Prev_MI = TRUE;
        prUnCompressed->u4RefreshFrameContext = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
        prUnCompressed->u4FrameParallelDecodingMode = u4VDEC_HAL_VP9_Read_Bit_Raw(u4CoreId);
    } 
    else 
    {
        prUnCompressed->fgUse_Prev_MI = FALSE;
        prUnCompressed->u4RefreshFrameContext = 0;
        prUnCompressed->u4FrameParallelDecodingMode = 1;
    }

    // This flag will be overridden by the call to vp9_setup_past_independence
    // below, forcing the use of context 0 for those frame types.
    prUnCompressed->u4FrameContextIdx = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, FRAME_CONTEXTS_LOG2);
    
//    VP9_PRINT_INFO("frame context idx: %d", prUnCompressed->u4FrameContextIdx);
    if ((prUnCompressed->u4FrameType == KEY_FRAME || prUnCompressed->u4IntraOnly) || prUnCompressed->u4ErrResilenceMode)
    {
        vVP9SetupPastIndependence(prCommon);
    }

    vVP9SetupLoopFilter(u4CoreId, &(prUnCompressed->rLoopFilter));
    vVP9SetupQuantization(prCommon, prUnCompressed, &prCommon->rMBD, u4CoreId);
    vVP9SetupSegmentation(&prUnCompressed->seg, u4CoreId);
    vVP9SetupTileInfo(prUnCompressed, u4CoreId);

    if(prUnCompressed->seg.enabled)
    {
        VP9_PRINT_INFO("seg.enabled");
        for(i = 0 ; i < 8; i ++)
        {
            u4QIndex = vp9_get_qindex(&prUnCompressed->seg, i, prUnCompressed->u4BaseQIdx);
            //VP9_PRINT_INFO("@@@ u4QIndex %d", u4QIndex);
            prCommon->au4DeQuant[i][0] = prCommon->au4Y_Dequant[u4QIndex][0];        
            prCommon->au4DeQuant[i][1] = prCommon->au4Y_Dequant[u4QIndex][1];
            prCommon->au4DeQuant[i][2] = prCommon->au4UV_Dequant[u4QIndex][0];
            prCommon->au4DeQuant[i][3] = prCommon->au4UV_Dequant[u4QIndex][1];
        }
    }
    else
    {
        prCommon->au4DeQuant[0][0] = prCommon->au4Y_Dequant[prUnCompressed->u4BaseQIdx][0];        
        prCommon->au4DeQuant[0][1] = prCommon->au4Y_Dequant[prUnCompressed->u4BaseQIdx][1];
        prCommon->au4DeQuant[0][2] = prCommon->au4UV_Dequant[prUnCompressed->u4BaseQIdx][0];
        prCommon->au4DeQuant[0][3] = prCommon->au4UV_Dequant[prUnCompressed->u4BaseQIdx][1];
    }

    UINT32 u4FirstPartitionSize = u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, 16);
    if (u4FirstPartitionSize == 0)
    {
        VP9_PRINT_ERROR("Invalid header size");
        prCommon->eErrno = VP9_ERR_STREAM_CORRUPT;
        return VP9_FAIL;
    }
//    VP9_PRINT_INFO("first partition size: %d", u4FirstPartitionSize);
    prUnCompressed->u4FirstPartitionSize = u4FirstPartitionSize;
    return VP9_OK;
}

static TX_MODE eVP9ReadTxMode(UINT32 u4CoreId) {
    TX_MODE eTxMode = (TX_MODE)(u4VDEC_HAL_VP9_Read_Literal(u4CoreId, 2));

    if (eTxMode == ALLOW_32X32)
    {
        eTxMode += u4VDEC_HAL_VP9_Read_Bit(u4CoreId);
    }
    return eTxMode;
}

static BOOL bVP9IsCompoundRefAllowed(VP9_COMMON_T * prCommon) {
    UINT32 i;
    for (i = 1; i < REFS_PER_FRAME; ++i)
    {
        if (prCommon->REF_FRAME_SIGN_BIAS[i + 1] != prCommon->REF_FRAME_SIGN_BIAS[1])
          return TRUE;
    }
    return FALSE;
}

static REFERENCE_MODE eVP9ReadFrameReferenceMode(VP9_COMMON_T * prCommon, UINT32 u4CoreId) {
    if (bVP9IsCompoundRefAllowed(prCommon)) {
        REFERENCE_MODE eResult;
        eResult = u4VDEC_HAL_VP9_Read_Bit(u4CoreId) ? (u4VDEC_HAL_VP9_Read_Bit(u4CoreId) ? REFERENCE_MODE_SELECT
                                                  : COMPOUND_REFERENCE) : SINGLE_REFERENCE;
        return eResult;
    } else {
        return SINGLE_REFERENCE;
    }
}

static void vVP9ReadFrameRefModeProbs(VP9_COMMON_T * prCommon, UINT32 u4CoreId) {
    if (prCommon->eRefMode== REFERENCE_MODE_SELECT)
    {
        vVDEC_HAL_VP9_UPDATE_COMP_INTER_PROBS(u4CoreId);
    }

    if (prCommon->eRefMode != COMPOUND_REFERENCE)
    {
        vVDEC_HAL_VP9_UPDATE_SINGLE_REF_PROBS(u4CoreId);
    }

    if (prCommon->eRefMode != SINGLE_REFERENCE)
    {
        vVDEC_HAL_VP9_UPDATE_COMP_REF_PROBS(u4CoreId);
    }
}

static void vVP9ReadMvProbs(UINT32 u4AllowHp, UINT32 u4CoreId) {
    vVDEC_HAL_VP9_UPDATE_MVD_INT_PROBS(u4CoreId);
    vVDEC_HAL_VP9_UPDATE_MVD_FP_PROBS(u4CoreId);

    if (u4AllowHp) {
        vVDEC_HAL_VP9_UPDATE_MVD_HP_PROBS(u4CoreId);
    }
}

static void vVP9SetupCompoundRefMode(VP9_COMMON_T *prCommon) 
{
    if (prCommon->REF_FRAME_SIGN_BIAS[LAST_FRAME] == prCommon->REF_FRAME_SIGN_BIAS[GOLDEN_FRAME]) 
    {
        prCommon->COMP_FIXED_REF = ALTREF_FRAME;
        prCommon->COMP_VAR_REF[0] = LAST_FRAME;
        prCommon->COMP_VAR_REF[1] = GOLDEN_FRAME;
    }
    else if (prCommon->REF_FRAME_SIGN_BIAS[LAST_FRAME] == prCommon->REF_FRAME_SIGN_BIAS[ALTREF_FRAME])
    {
        prCommon->COMP_FIXED_REF = GOLDEN_FRAME;
        prCommon->COMP_VAR_REF[0] = LAST_FRAME;
        prCommon->COMP_VAR_REF[1] = ALTREF_FRAME;
    } 
    else 
    {
        prCommon->COMP_FIXED_REF = LAST_FRAME;
        prCommon->COMP_VAR_REF[0] = GOLDEN_FRAME;
        prCommon->COMP_VAR_REF[1] = ALTREF_FRAME;
    }
}

static INT32 i4VP9CompressedHeaderParse(VP9_COMMON_T * prCommon, UINT32 u4CoreId) 
{
    VP9_PRINT_INFO("[#pic %d] Compress header CoreId %d\n", prCommon->u4FrameNum, u4CoreId);
    vVDEC_HAL_VP9_InitBool(u4CoreId);


    prCommon->eTxMode = prCommon->rUnCompressedHeader.u4Lossless ? ONLY_4X4 : eVP9ReadTxMode(u4CoreId);

    //set tx mode
    vVDEC_HAL_VP9_Set_TxMode(u4CoreId, prCommon->eTxMode);
    
    if (prCommon->eTxMode == TX_MODE_SELECT)
    {
        vVDEC_HAL_VP9_UPDATE_TX_PROBS(u4CoreId);
    }
    
    vVDEC_HAL_VP9_UPDATE_COEF_PROBS(u4CoreId);
    vVDEC_HAL_VP9_UPDATE_MBSKIP_PROBS(u4CoreId);
    
    if (!(prCommon->rUnCompressedHeader.u4IntraOnly || prCommon->rUnCompressedHeader.u4FrameType == KEY_FRAME)) {
        vVDEC_HAL_VP9_UPDATE_INTER_MODE_PROBS(u4CoreId);
        if (prCommon->rUnCompressedHeader.eInterpFilterType == SWITCHABLE)
        {        
            vVDEC_HAL_VP9_UPDATE_SWITCHABLE_INTERP_PROBS(u4CoreId);
        }

        vVDEC_HAL_VP9_UPDATE_INTRA_INTER_PROBS(u4CoreId);        
    
        prCommon->eRefMode = eVP9ReadFrameReferenceMode(prCommon, u4CoreId);

        if (prCommon->eRefMode != SINGLE_REFERENCE)
        {
            vVP9SetupCompoundRefMode(prCommon);
        }
        vVP9ReadFrameRefModeProbs(prCommon, u4CoreId);

        vVDEC_HAL_VP9_UPDATE_Y_MODE_PROBS(u4CoreId);
        vVDEC_HAL_VP9_UPDATE_PARTITION_PROBS(u4CoreId);
        vVP9ReadMvProbs(prCommon->rUnCompressedHeader.u4AllowHighPrecisionMv, u4CoreId);
    }

// TODO: check if vld error
    return VP9_OK;
//      return vp9_reader_has_error(&r);
}

// Set decoder related registers here.
static void vVP9SetPicInfo(UINT32 u4InstID, UINT32 u4CoreId)
{
    UINT32 u4BufferId;
    VP9_COMMON_T* prCommon = prVP9GetCommon(u4InstID);
    VP9_UNCOMPRESSED_HEADER_T* prUnCompressed = &prCommon->rUnCompressedHeader;
    UINT32 u4CompoundVal;
    UINT32 u4Tmp1 = 0;    
    UINT32 u4Tmp2 = 0;
    UINT32 i4Idx;
    UINT32 i;
    INT32 aiMax_RRF_BLK_TmpX[MAX_REF_FRAMES];
    INT32 aiMax_RRF_BLK_TmpY[MAX_REF_FRAMES];
    INT32 i4Use_prev_in_find_mv_refs;

    
    for(i4Idx=0; i4Idx<=3; i4Idx++)
    {
        u4Tmp1 = u4Tmp1 + (prCommon->REF_FRAME_SIGN_BIAS[i4Idx]<<i4Idx);
    }

    for(i4Idx=0; i4Idx<=1; i4Idx++)
    {
        u4Tmp2 = u4Tmp2 + (prCommon->COMP_VAR_REF[i4Idx]<<(i4Idx*2));
    }
 
    VP9_PRINT_INFO("comp_var_ref 0x%x, ref_frame_sign_bias 0x%x, comp_fixed_ref 0x%x, comp_pred_mode 0x%x",
                    u4Tmp2, u4Tmp1 , prCommon->COMP_FIXED_REF, prCommon->eRefMode );
    u4CompoundVal = (u4Tmp2<<24) + (u4Tmp1<<16) + (prCommon->COMP_FIXED_REF<<8) + (prCommon->eRefMode) ;

    vVDEC_HAL_VP9_Set_Compound_Ref(u4CoreId, u4CompoundVal);

    // MV settings

    // set prev mode info, check use previous mv enable or not
    i4Use_prev_in_find_mv_refs = prUnCompressed->u4Width == prCommon->u4LastWidth &&
                                 prUnCompressed->u4Height== prCommon->u4LastHeight &&
                                !prUnCompressed->u4IntraOnly && prCommon->u4LastShowFrame;
    
    // Special case: set fgUse_Prev_MI to FALSE when the previous mode info
    // context cannot be used.    
//#if (VP9_CONFIG_CHIP_VER_CURR > VP9_CONFIG_CHIP_VER_MT5890)
    prUnCompressed->fgUse_Prev_MI = (!prUnCompressed->u4ErrResilenceMode) && (i4Use_prev_in_find_mv_refs);
//#else
//    prUnCompressed->fgUse_Prev_MI = prUnCompressed->fgUse_Prev_MI && i4Use_prev_in_find_mv_refs;
//#endif    
    
    vVDEC_HAL_VP9_Set_MVP_Enable(u4CoreId, prUnCompressed->fgUse_Prev_MI);
    
    // Core 0/1 Use the same MV buffer, LAE use another
    u4BufferId = (u4CoreId == VP9_LAE_ID) ? 1:0;
        
    vVDEC_HAL_VP9_Set_MV_Buffer_Addr(u4CoreId, prCommon->rMVBuffer[u4BufferId].ulBufVAddr);
    // ~MV Settings Done

    // MC Ref Buffer Settings
    if(u4CoreId != VP9_LAE_ID)
    {
        vVDEC_HAL_VP9_Set_MC_RefBuf_Addr(u4CoreId, prCommon->FRAME_REFS[0], prCommon->FRAME_REFS[1], prCommon->FRAME_REFS[2]);

        if (prUnCompressed->u4FrameType != KEY_FRAME)
        {
            vVDEC_HAL_VP9_Set_MC_Ref_Scaling_Enable(u4CoreId, prCommon->FRAME_REFS[0].rScaleFactors.u4Ref_Scaling_EN,
                                                    prCommon->FRAME_REFS[1].rScaleFactors.u4Ref_Scaling_EN,
                                                    prCommon->FRAME_REFS[2].rScaleFactors.u4Ref_Scaling_EN);
            for(i = 0; i < REFS_PER_FRAME; i++)
            {
                vVDEC_HAL_VP9_Set_MC_Set_UMV(u4CoreId,  prCommon->FRAME_REFS[i].prBuf->u4YCropWidth,
                                             prCommon->FRAME_REFS[i].prBuf->u4YCropHeight, i);
            
                if(prCommon->FRAME_REFS[i].rScaleFactors.u4Ref_Scaling_EN == 1)
                {
                    vVDEC_HAL_VP9_Set_MC_Ref_Scaling_Step(u4CoreId, prCommon->FRAME_REFS[i].rScaleFactors.i4X_step_q4,
                                                          prCommon->FRAME_REFS[i].rScaleFactors.i4Y_step_q4, i);
                    vVDEC_HAL_VP9_Set_MC_Set_Scaling_Factor(u4CoreId, prCommon->FRAME_REFS[i].rScaleFactors.i4X_scale_fp,
                                                            prCommon->FRAME_REFS[i].rScaleFactors.i4Y_scale_fp, i);
                
                    aiMax_RRF_BLK_TmpX[i] = iMax_RRF_BLK_Cal(prCommon->FRAME_REFS[i].rScaleFactors.i4X_scale_fp);
                    aiMax_RRF_BLK_TmpY[i] = iMax_RRF_BLK_Cal(prCommon->FRAME_REFS[i].rScaleFactors.i4Y_scale_fp);  
                }
                else
                {
                    aiMax_RRF_BLK_TmpX[i] = 7;
                    aiMax_RRF_BLK_TmpY[i] = 7;
                }
                VP9_PRINT_VERBOSE("aiMax_RRF_BLK_TmpX[%d] %d, aiMax_RRF_BLK_TmpY[%d] %d",i,aiMax_RRF_BLK_TmpX[i],i, aiMax_RRF_BLK_TmpY[i] );
                vVDEC_HAL_VP9_Set_MC_Ref_Pitch(u4CoreId, prCommon->FRAME_REFS[i].prBuf->u4YCropWidth, i);
            }
            vVDEC_HAL_VP9_Set_MC_MAX_RRF_BLK_Size(u4CoreId, aiMax_RRF_BLK_TmpX, aiMax_RRF_BLK_TmpY);
        }    
        vVDEC_HAL_VP9_Set_MC_MI_COLS_ROWS(u4CoreId, prCommon->u4MI_rows, prCommon->u4MI_cols);
    }
    //~~~ MC settings

    // PP settings
    if(u4CoreId != VP9_LAE_ID)
    {
        vVDEC_HAL_VP9_Set_PP_EN(u4CoreId, 0x1);    

        vVDEC_HAL_VP9_Set_MC_DecodeBuf_Addr(u4CoreId, prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufY.ulBufVAddr,
                                            prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufC.ulBufVAddr);

        vVDEC_HAL_VP9_Set_PP_MB_Width(u4CoreId, prCommon->pCurrentFB->rBuf.u4YCropWidth);

	vVDEC_HAL_VP9_Set_PP_DBK_EN(u4CoreId, 3);                                                       // Turn on Y, CBCR Deblocking

        vVDEC_HAL_VP9_Set_PP_WriteByPost(u4CoreId, 1);                                                  // prevents motion compensator from writing to DRAM

	vVDEC_HAL_VP9_Set_PP_MB_LeftRightMostIdx(u4CoreId, prCommon->pCurrentFB->rBuf.u4YCropWidth);   

        vVDEC_HAL_VP9_Set_PP_MB_UpDownMostIdx(u4CoreId, prCommon->pCurrentFB->rBuf.u4YCropHeight);

        vVDEC_HAL_VP9_Set_PP_Display_Range(u4CoreId, prCommon->pCurrentFB->rBuf.u4YCropWidth, prCommon->pCurrentFB->rBuf.u4YCropHeight);
    }
    // ~PP Settins 

    // SQT settings
    if(u4CoreId != VP9_LAE_ID)
    {
        vVDEC_HAL_VP9_Set_SQT_IQ_SRAM_EN(u4CoreId, 0x1);  // enable write inverse quant table   
        vVDEC_HAL_VP9_Set_SQT_Q_Table(u4CoreId, prCommon->au4DeQuant);     
        vVDEC_HAL_VP9_Set_SQT_IQ_SRAM_EN(u4CoreId, 0x0);  // disable write inverse quant table
    }
    //~~~ SQT settings

    // UFO Config
    if(prCommon->fgUFOModeEnable && u4CoreId != VP9_LAE_ID)
    {        
        UINT32 u4SizeY2LenY = prCommon->pCurrentFB->rBuf.rUFO_LEN_Y.ulBufVAddr - prCommon->pCurrentFB->rBuf.rBufY.ulBufVAddr;        
        UINT32 u4SizeC2LenC = prCommon->pCurrentFB->rBuf.rUFO_LEN_C.ulBufVAddr - prCommon->pCurrentFB->rBuf.rBufC.ulBufVAddr;
        
        // clear output buffer
        memset((void*)prCommon->pCurrentFB->rBuf.rBufY.ulBufVAddr, 0, prCommon->pCurrentFB->rBuf.rBufY.u4BufSize);
        memset((void*)prCommon->pCurrentFB->rBuf.rBufC.ulBufVAddr, 0, prCommon->pCurrentFB->rBuf.rBufC.u4BufSize);        
        memset((void*)prCommon->pCurrentFB->rBuf.rUFO_LEN_Y.ulBufVAddr, 0, prCommon->pCurrentFB->rBuf.rUFO_LEN_Y.u4BufSize);
        memset((void*)prCommon->pCurrentFB->rBuf.rUFO_LEN_C.ulBufVAddr, 0, prCommon->pCurrentFB->rBuf.rUFO_LEN_C.u4BufSize);
        
        vVDEC_HAL_VP9_UFO_Config(u4CoreId, prCommon->rUnCompressedHeader.u4Width,prCommon->rUnCompressedHeader.u4Height,
                                 u4SizeY2LenY, u4SizeC2LenC, 
                                 prCommon->pCurrentFB->rBuf.rUFO_LEN_Y.ulBufVAddr,  prCommon->pCurrentFB->rBuf.rUFO_LEN_C.ulBufVAddr);
    }


}

BOOL fgVP9FileIsIVF(ULONG ulFileStartAddr, UINT32 u4FileLength)
{
    UCHAR FileHeader[32];
    static const char *IVF_SIGNATURE = "DKIF";
    if(u4FileLength <= 32)
    {        
        VP9_PRINT_ERROR("File too Small");
        return FALSE;
    }
    
    memcpy((void*)FileHeader, (void*)ulFileStartAddr, 32);
    
    if (memcmp(IVF_SIGNATURE, FileHeader, 4) == 0) {
        UINT32 u4Version = FileHeader[5] << 8;
        u4Version |= FileHeader[4];
        if (u4Version != 0) {            
            VP9_PRINT_INFO("IVF Version unrecognized, This file may not decode properly.");
        }
        return TRUE;
    }
    
    VP9_PRINT_ERROR("Sorry, The File is not IVF");
    return FALSE;
}

extern HANDLE_T hVdecDecDone[2];
void vVerInitVP9(UINT32 u4InstID)
{
    VP9_COMMON_T* prCommon = prVP9GetCommon(u4InstID);
    VP9_INPUT_CTX_T* prInputCtx = &prCommon->rInputCtx;   
    UINT32 i;
    ULONG ulTileBufAddr; //qianqian@20150215
    UINT32 u4Align;
    memset(prCommon, 0, sizeof(VP9_COMMON_T));
    prCommon->u4InstID = u4InstID;
    //prCommon->u4NewFbIdx = -1;
    prCommon->u4NewFbIdx = 0U;

    prCommon->u4StartNum = _u4StartCompPicNum[u4InstID];    
    prCommon->u4EndNum = _u4EndCompPicNum[u4InstID];
    ASSERT(prCommon->u4StartNum <= prCommon->u4EndNum);
    prInputCtx->ulVaFifoStart = (ULONG)(_pucVFifo[u4InstID]);
	
    prInputCtx->u4FileLength = _tInFileInfo[u4InstID].u4FileLength;
	
    prInputCtx->u4FileOffset = 0;
    memset(prInputCtx->ucBitStreamName, 0, 256);
    strcpy (prInputCtx->ucBitStreamName , _bFileStr1[0][1]);
    VP9_PRINT_INFO("vVerInitVP9 bitstream length: %d, compare %d to %d", prInputCtx->u4FileLength, prCommon->u4StartNum, prCommon->u4EndNum);
    
    if(!fgVP9FileIsIVF(prInputCtx->ulVaFifoStart, prInputCtx->u4FileLength))
    {
        prCommon->eErrno = VP9_ERR_STREAM_UNSUP;
        return;
    }

    prInputCtx->u4BitstreamLoadingCnt++;
    prInputCtx->ulVaFifoEnd = (ULONG)((_pucVFifo[u4InstID] + VP9_V_FIFO_SZ));

    // vp9 changed LAE cli to UFO mode switch
    prCommon->fgUFOModeEnable = _u4LaeMode[u4InstID];
    // judge mcore enable or not by resolution
    prCommon->fgMultiCoreEnable = FALSE;
	prCommon->p_hVdecDecDone = &hVdecDecDone[u4InstID];

    #if VP9_CRC_ENABLE
    // MCore CRC
    prCommon->rDramCRCYBuf0.ulBufVAddr = (ULONG)_pucVP9CRCYBuf0[u4InstID]; //(UINT32)((_pucVFifo[u4InstID] + VP9_V_FIFO_SZ));
    prCommon->rDramCRCYBuf0.u4BufSize = VP9_CRC_BUFFER_SZ;
    prCommon->rDramCRCCBuf0.ulBufVAddr = (ULONG)_pucVP9CRCCBuf0[u4InstID]; //prCommon->rDramCRCYBuf0.u4BufVAddr + prCommon->rDramCRCYBuf0.u4BufSize;    
    prCommon->rDramCRCCBuf0.u4BufSize = VP9_CRC_BUFFER_SZ;
    prCommon->rDramCRCYBuf1.ulBufVAddr = (ULONG) _pucVP9CRCYBuf1[u4InstID]; //prCommon->rDramCRCCBuf0.u4BufVAddr + prCommon->rDramCRCCBuf0.u4BufSize;
    prCommon->rDramCRCYBuf1.u4BufSize =  VP9_CRC_BUFFER_SZ;
    prCommon->rDramCRCCBuf1.ulBufVAddr = (ULONG)_pucVP9CRCCBuf1[u4InstID] ; //prCommon->rDramCRCYBuf1.u4BufVAddr + prCommon->rDramCRCYBuf1.u4BufSize;    
    prCommon->rDramCRCCBuf1.u4BufSize =  VP9_CRC_BUFFER_SZ;

    // SCore CRC
    prCommon->rDramCRCYBuf2.ulBufVAddr = (ULONG) _pucVP9CRCYBuf2[u4InstID]; //prCommon->rDramCRCCBuf1.u4BufVAddr + prCommon->rDramCRCCBuf1.u4BufSize;
    prCommon->rDramCRCYBuf2.u4BufSize =  VP9_CRC_BUFFER_SZ;
    prCommon->rDramCRCCBuf2.ulBufVAddr = (ULONG)_pucVP9CRCCBuf2[u4InstID]; //prCommon->rDramCRCYBuf2.u4BufVAddr + prCommon->rDramCRCYBuf2.u4BufSize;    
    prCommon->rDramCRCCBuf2.u4BufSize =  VP9_CRC_BUFFER_SZ;
    #endif

    prCommon->rDramSegId0.ulBufVAddr = (ULONG)_pucSegId[u4InstID][0]; //prCommon->rDramCRCCBuf2.u4BufVAddr  + VP9_CRC_BUFFER_SZ;
    prCommon->rDramSegId0.u4BufSize = VP9_SEG_ID_SZ;
    prCommon->rDramSegId1.ulBufVAddr = (ULONG)_pucSegId[u4InstID][1]; //prCommon->rDramSegId0.u4BufVAddr + VP9_SEG_ID_SZ;
    prCommon->rDramSegId1.u4BufSize = VP9_SEG_ID_SZ;

    //u4TileBufAddr = prCommon->rDramSegId1.u4BufVAddr + prCommon->rDramSegId1.u4BufSize;
	ulTileBufAddr = (ULONG)_pucTileBuf;
	
    // MCore
#if VP9_ENABLE_MCORE   
    prCommon->rLAEBuffer.ulBufVAddr = ulTileBufAddr;
    prCommon->rLAEBuffer.u4BufSize = VP9_LAE_BUFFER_SZ;
    prCommon->rErrorBuffer.ulBufVAddr = prCommon->rLAEBuffer.ulBufVAddr + VP9_LAE_BUFFER_SZ;
    prCommon->rErrorBuffer.u4BufSize = VP9_ERR_BUFFER_SZ;        
    ulTileBufAddr = prCommon->rErrorBuffer.ulBufVAddr + VP9_ERR_BUFFER_SZ;   
#endif
    
    // tile maxmium # => 4096 >> 6 (SB size) << 2 (MAX Tile Row size)
    // however, hw only need 1 4k physically continous address for tile setting is ok..
    prCommon->rTileBuffer.ulBufVAddr = ulTileBufAddr;    
    prCommon->rTileBuffer.u4BufSize = VP9_TILE_BUFFER_SIZE;

    //prCommon->rCountTBLBuffer.u4BufVAddr = prCommon->rTileBuffer.u4BufVAddr  + prCommon->rTileBuffer.u4BufSize;    
    prCommon->rCountTBLBuffer.ulBufVAddr = (ULONG)_pucCountTBLBuffer;
    prCommon->rCountTBLBuffer.u4BufSize = VP9_COUNT_TBL_SZ;

    //prCommon->rProbTBLBuffer.u4BufVAddr = prCommon->rCountTBLBuffer.u4BufVAddr  + prCommon->rCountTBLBuffer.u4BufSize;    
    prCommon->rProbTBLBuffer.ulBufVAddr =(ULONG)_pucProbTBLBuffer;
    prCommon->rProbTBLBuffer.u4BufSize = VP9_PROB_TBL_SZ;

    //DPB Buffer for decode
    //u4Align = (((UINT32)(prCommon->rProbTBLBuffer.u4BufVAddr + prCommon->rProbTBLBuffer.u4BufSize) + 4095) >> 12 ) << 12;
    //prCommon->rDramDpb.u4BufVAddr =u4Align;
    prCommon->rDramDpb.ulBufVAddr  = (ULONG)_pucDPB[u4InstID];
    prCommon->rDramDpb.u4BufSize = VP9_DPB_SZ;
    
    memset(prCommon->ucBitstreamName, 0, 256);
    strncpy (prCommon->ucBitstreamName , _bFileStr1[0][1], (strlen(_bFileStr1[0][1]) - 21));
    
    for(i = 0; i < MAX_REF_FRAMES; i++)
    {
        prCommon->FRAME_REFS[i].rScaleFactors.i4X_scale_fp = REF_INVALID_SCALE;
        prCommon->FRAME_REFS[i].rScaleFactors.i4Y_scale_fp = REF_INVALID_SCALE;
        prCommon->FRAME_REFS[i].rScaleFactors.i4X_step_q4 = REF_INVALID_STEP;        
        prCommon->FRAME_REFS[i].rScaleFactors.i4Y_step_q4 = REF_INVALID_STEP;
        prCommon->FRAME_REFS[i].rScaleFactors.u4Ref_Scaling_EN = 0;
    }
        
    for(i = 0; i < REF_FRAMES; i ++)
    {
        prCommon->REF_FRAME_MAP[i] = -1;
    }
    
    prCommon->COMP_FIXED_REF = INTRA_FRAME;
    
    vVP9_Init_Dequantizer(prCommon);
    
    prInputCtx->u4FileOffset += VP9_IVF_FILE_HEADER_SZ;
   
    prCommon->fgFrameBufferConfiged = FALSE;
    prCommon->fgCRCOpen = FALSE;
	
    VP9_PRINT_INFO("VP9_FIFO::          0x%p (0x%08X)", _pucVFifo[u4InstID], PHYSICAL(_pucVFifo[u4InstID]));    
#if VP9_CRC_ENABLE
    VP9_PRINT_INFO("VP9CRCYBuf0:    0x%lX (0x%08X)", prCommon->rDramCRCYBuf0.ulBufVAddr, PHYSICAL(prCommon->rDramCRCYBuf0.ulBufVAddr));
    VP9_PRINT_INFO("VP9CRCCBuf0:    0x%lX (0x%08X)", prCommon->rDramCRCCBuf0.ulBufVAddr, PHYSICAL(prCommon->rDramCRCCBuf0.ulBufVAddr));
    VP9_PRINT_INFO("VP9CRCYBuf1:    0x%lX (0x%08X)", prCommon->rDramCRCYBuf1.ulBufVAddr, PHYSICAL(prCommon->rDramCRCYBuf1.ulBufVAddr));
    VP9_PRINT_INFO("VP9CRCCBuf1:    0x%lX (0x%08X)", prCommon->rDramCRCCBuf1.ulBufVAddr, PHYSICAL(prCommon->rDramCRCCBuf1.ulBufVAddr));
    VP9_PRINT_INFO("VP9CRCYBuf2:    0x%lX (0x%08X)", prCommon->rDramCRCYBuf2.ulBufVAddr, PHYSICAL(prCommon->rDramCRCYBuf2.ulBufVAddr));
    VP9_PRINT_INFO("VP9CRCCBuf2:    0x%lX (0x%08X)", prCommon->rDramCRCCBuf2.ulBufVAddr, PHYSICAL(prCommon->rDramCRCCBuf2.ulBufVAddr));

#endif

    VP9_PRINT_INFO("VP9_SEG_ID_Addr0:    0x%lX (0x%08X)", prCommon->rDramSegId0.ulBufVAddr, PHYSICAL(prCommon->rDramSegId0.ulBufVAddr));
    VP9_PRINT_INFO("VP9_SEG_ID_Addr1:    0x%lX (0x%08X)", prCommon->rDramSegId1.ulBufVAddr, PHYSICAL(prCommon->rDramSegId1.ulBufVAddr));

#if VP9_ENABLE_MCORE
    VP9_PRINT_INFO("VP9_LAE_Addr:       0x%lX (0x%08X)", prCommon->rLAEBuffer.ulBufVAddr, PHYSICAL(prCommon->rLAEBuffer.ulBufVAddr));
    VP9_PRINT_INFO("VP9_ERR_Addr:       0x%lX (0x%08X)", prCommon->rErrorBuffer.ulBufVAddr, PHYSICAL(prCommon->rErrorBuffer.ulBufVAddr));    
#endif

    VP9_PRINT_INFO("VP9_TILE_Addr:      0x%lX (0x%08X)", prCommon->rTileBuffer.ulBufVAddr, PHYSICAL(prCommon->rTileBuffer.ulBufVAddr));
    VP9_PRINT_INFO("VP9_COUNT_TBL_Addr: 0x%lX (0x%08X)", prCommon->rCountTBLBuffer.ulBufVAddr, PHYSICAL(prCommon->rCountTBLBuffer.ulBufVAddr));
    VP9_PRINT_INFO("VP9_PROB_TBL_Addr:  0x%lX (0x%08X)", prCommon->rProbTBLBuffer.ulBufVAddr, PHYSICAL(prCommon->rProbTBLBuffer.ulBufVAddr));
    VP9_PRINT_INFO("VP9_DPB_Addr:       0x%lX (0x%08X)", prCommon->rDramDpb.ulBufVAddr, PHYSICAL( prCommon->rDramDpb.ulBufVAddr));
    return;
}

// *********************************************************************
// Function : BOOL u4VerVParserVP8(UINT32 u4InstID)
// Description :
// Parameter :
// Return    :
// *********************************************************************
void vVerVParserVP9(UINT32 u4InstID, BOOL fgInquiry)
{
    INT32  i4RetVal;
    VP9_COMMON_T* prCommon = prVP9GetCommon(u4InstID);    
    VP9_INPUT_CTX_T* prInputCtx = &prCommon->rInputCtx;
    VP9_UNCOMPRESSED_HEADER_T* prUnCompressed = &prCommon->rUnCompressedHeader;
    VP9_SUPER_FRAME_INFO_T* prSuperFrame = &prInputCtx->rSuperFrame;
    UINT32 i;
#if VP9_ENABLE_MCORE
    UINT32 u4CoreId = VP9_LAE_ID;
#else
    UINT32 u4CoreId = CORE_0_ID;
#endif

    UINT32 u4UnCompressedRet = 0;
    
	VP9_PRINT_INFO("------[QQ]1. vVerVParserVP9 0x%x-------",prInputCtx->u4FileLength);
    CHECK_ERROR(prCommon->eErrno);
    
    if(!prSuperFrame->fgInSuperFrame)
    {
        VP9_PRINT_INFO("------[QQ]2. vVerVParserVP9 0x%x-------",prInputCtx->u4FileLength);
        if(VP9_OK != u4VP9IVFReadFrame(prInputCtx))
        {
            _u4VerBitCount[u4InstID] = 0xffffffff;
            prCommon->eErrno = VP9_ERR_STREAM_EOF;
            return;
        }
        vVP9SuperFrameParse(prInputCtx);
    }
    else
    {
        ASSERT(prSuperFrame->u4SuperFrmIndex < (prSuperFrame->u4SuperFrmCount - 1));
        prSuperFrame->u4SuperFrmIndex++;
        prInputCtx->u4FrameSize = prSuperFrame->u4SuperFrmSizes[prSuperFrame->u4SuperFrmIndex];        
        prInputCtx->ulVaFrameStart = prInputCtx->ulVaFrameEnd;
        prInputCtx->ulVaFrameEnd = prInputCtx->ulVaFrameStart + prInputCtx->u4FrameSize;
        if(prSuperFrame->u4SuperFrmIndex == (prSuperFrame->u4SuperFrmCount - 1))
        {
            //reach super frame end
            memset(prSuperFrame, 0, sizeof(VP9_SUPER_FRAME_INFO_T));
            VP9_PRINT_INFO("------super frame read done-------");
        }
    }

#if VP9_ENABLE_MCORE
    vVDEC_HAL_VP9_Mcore_Init(PHYSICAL(prCommon->rLAEBuffer.ulBufVAddr), PHYSICAL(prCommon->rErrorBuffer.ulBufVAddr));
#endif
    vVDEC_HAL_VP9_SW_Reset(u4CoreId);
    //i4VDEC_HAL_VP9_InitBarrelShifter(u4CoreId, PHYSICAL(prInputCtx->u4VaFrameStart), PHYSICAL(prInputCtx->u4VaFrameEnd), PHYSICAL(prInputCtx->u4VaFifoStart), PHYSICAL(prInputCtx->u4VaFifoEnd));
    i4VDEC_HAL_VP9_InitBarrelShifter(u4CoreId, prInputCtx->ulVaFrameStart, prInputCtx->ulVaFrameEnd, prInputCtx->ulVaFifoStart, prInputCtx->ulVaFifoEnd);

    u4UnCompressedRet = i4VP9UnCompressedHeaderParse(prCommon, prUnCompressed, u4CoreId);

    if(u4UnCompressedRet == VP9_SKIP_FRAME)
    {
        return;
    }
    else if(u4UnCompressedRet == VP9_FAIL);
    {        
        CHECK_ERROR(prCommon->eErrno);
    }
    
    #if 0
    if(prUnCompressed->u4ShowExisting)
    {
        ASSERT(prUnCompressed->u4FirstPartitionSize == 0);
        return;
    }
    #endif

    if (!prCommon->u4KeyFrameDecoded && (prUnCompressed->u4FrameType!= KEY_FRAME))
    {
        if(!(prUnCompressed->u4IntraOnly))
        {
            prCommon->eErrno = VP9_ERR_STREAM_CORRUPT;        
            VP9_PRINT_ERROR("First frame should be key frame or Intra-Coded Frame[%d,%d],%d, return",
				prUnCompressed->u4FrameType,prUnCompressed->u4IntraOnly,prCommon->u4FrameNum);
            return;
        }
        else
        {
            for (i = 0; i < REFS_PER_FRAME; ++i)
            {
                prCommon->FRAME_REFS[i].u4Idx = prCommon->u4NewFbIdx;
                prCommon->FRAME_REFS[i].prBuf = &prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf;
            }
        }        
    }

    //align vld rp and reset bits count
    UINT32 u4BitsRead = u4VDEC_HAL_VP9_Get_Bits_Count(u4CoreId);
    VP9_PRINT_INFO("u4BitsRead = %d", u4BitsRead);
    if(u4BitsRead & 0x7)
    {
        u4VDEC_HAL_VP9_Read_Literal_Raw(u4CoreId, (8 - (u4BitsRead & 0x7)));//shift vld to byte align
    }
    prInputCtx->u4UnCompressSize = (u4BitsRead / 8) + ((u4BitsRead&0x7) > 0);
#if VP9_ENABLE_MCORE
    // Determine MCore flow or SCore flow
    if(prCommon->rUnCompressedHeader.u4Width > 384 && prCommon->rUnCompressedHeader.u4Height > 128)
    {        
        VP9_PRINT_INFO("Multi Core Flow");
        prCommon->fgMultiCoreEnable = TRUE;
    }
    else
#endif        
    {
        VP9_PRINT_INFO("Single Core Flow");
        prCommon->fgMultiCoreEnable = FALSE;
    }

#if VP9_ENABLE_MCORE
   vVDEC_HAL_VP9_Mcore_Enable(prCommon->fgMultiCoreEnable);

   if(prCommon->fgMultiCoreEnable == FALSE)
   {
        u4CoreId = CORE_0_ID;
        vVDEC_HAL_VP9_SW_Reset(u4CoreId);
        i4VDEC_HAL_VP9_InitBarrelShifter(u4CoreId, PHYSICAL(prInputCtx->ulVaFrameStart) + prInputCtx->u4UnCompressSize, PHYSICAL(prInputCtx->ulVaFrameEnd), PHYSICAL(prInputCtx->ulVaFifoStart), PHYSICAL(prInputCtx->ulVaFifoEnd));
   }
   else
   {
        vVDEC_HAL_VP9_SW_Reset(CORE_0_ID);
        i4VDEC_HAL_VP9_InitBarrelShifter(CORE_0_ID, PHYSICAL(prInputCtx->ulVaFrameStart) + prInputCtx->u4UnCompressSize, PHYSICAL(prInputCtx->ulVaFrameEnd), PHYSICAL(prInputCtx->ulVaFifoStart), PHYSICAL(prInputCtx->ulVaFifoEnd));    
        vVDEC_HAL_VP9_SW_Reset(CORE_1_ID);
        i4VDEC_HAL_VP9_InitBarrelShifter(CORE_1_ID, PHYSICAL(prInputCtx->ulVaFrameStart) + prInputCtx->u4UnCompressSize, PHYSICAL(prInputCtx->ulVaFrameEnd), PHYSICAL(prInputCtx->ulVaFifoStart), PHYSICAL(prInputCtx->ulVaFifoEnd));    
   }
#endif

#if 0
    VP9_PRINT_INFO(" ## prInputCtx->u4UnCompressSize %d", prInputCtx->u4UnCompressSize);
    
    {
        VP9_PRINT_INFO("=========Uncompressed Header Result===========");
        VP9_PRINT_INFO("|--- profile: %d", prUnCompressed->u4Profile);
        VP9_PRINT_INFO("|--- frame type: %d", prUnCompressed->u4FrameType);
        VP9_PRINT_INFO("|--- show frame: %d", prUnCompressed->u4ShowFrame);
        VP9_PRINT_INFO("|--- show existing: %d", prUnCompressed->u4ShowExisting);
        VP9_PRINT_INFO("|--- size [%d x %d]", prUnCompressed->u4Width, prUnCompressed->u4Height);        
        VP9_PRINT_INFO("|--- ssx & ssy [%d x %d]", prUnCompressed->u4SubSampling_X, prUnCompressed->u4SubSampling_Y);
        VP9_PRINT_INFO("|--- color: %d", prUnCompressed->u4ColorSpace);        
        VP9_PRINT_INFO("|--- bit depth: %d", prUnCompressed->u4BitDepth);
        VP9_PRINT_INFO("|--- err resilence mode: %d", prUnCompressed->u4ErrResilenceMode);
        VP9_PRINT_INFO("|--- refresh flags: 0x%x", prUnCompressed->u4RefreshFrameFlags);
        VP9_PRINT_INFO("|--- intra only: %d", prUnCompressed->u4IntraOnly);
        VP9_PRINT_INFO("|--- reset frame context flag: %d", prUnCompressed->u4ResetFrameContext);
        VP9_PRINT_INFO("|--- parallel mode: %d", prUnCompressed->u4FrameParallelDecodingMode);
        VP9_PRINT_INFO("|--- refresh frame context: %d", prUnCompressed->u4RefreshFrameContext);
        VP9_PRINT_INFO("|--- frame context idx: %d", prUnCompressed->u4FrameContextIdx);
        VP9_PRINT_INFO("|--- allow hp: %d", prUnCompressed->u4AllowHighPrecisionMv);
        VP9_PRINT_INFO("|--- tile rows & cols [%d x %d]", prUnCompressed->u4Log2TileRows, prUnCompressed->u4Log2TileCols);
        VP9_PRINT_INFO("|--- first partition size: 0x%x", prUnCompressed->u4FirstPartitionSize);
        VP9_PRINT_INFO("===============================================");
    }
#endif 
    if ((prUnCompressed->u4FirstPartitionSize == 0) || (prUnCompressed->u4FirstPartitionSize > (prInputCtx->u4FrameSize - prInputCtx->u4UnCompressSize)))
    {
        prCommon->eErrno = VP9_ERR_STREAM_CORRUPT;
        VP9_PRINT_ERROR("Truncated packet or corrupt header length");
        return;
    }
    vVDEC_HAL_VP9_Set_UnCompressed(u4CoreId, prUnCompressed);
    
    if(prCommon->fgMultiCoreEnable == TRUE)
    {
        // LAE SegmentID use SEG_ID 1
        vVDEC_HAL_VP9_Set_Segmentation(u4CoreId, &prUnCompressed->seg, prCommon->rDramSegId1.ulBufVAddr);
    }
    else
     {
        // Core 0/1 Use SEG_ID 0
        vVDEC_HAL_VP9_Set_Segmentation(u4CoreId, &prUnCompressed->seg, prCommon->rDramSegId0.ulBufVAddr);
    }

    prCommon->fc = prCommon->frame_contexts[prUnCompressed->u4FrameContextIdx];
    vVDEC_HAL_VP9_Set_Probs_Table(u4CoreId, &prCommon->fc);
    msleep(10);
	
    if(prCommon->fgMultiCoreEnable == TRUE)
    {
        vVDEC_HAL_VP9_Set_UnCompressed(CORE_0_ID, prUnCompressed);    
        vVDEC_HAL_VP9_Set_Segmentation(CORE_0_ID, &prUnCompressed->seg, prCommon->rDramSegId0.ulBufVAddr);
        vVDEC_HAL_VP9_Set_Probs_Table(CORE_0_ID, &prCommon->fc);
        vVDEC_HAL_VP9_Set_UnCompressed(CORE_1_ID, prUnCompressed);    
        vVDEC_HAL_VP9_Set_Segmentation(CORE_1_ID, &prUnCompressed->seg, prCommon->rDramSegId0.ulBufVAddr);
        vVDEC_HAL_VP9_Set_Probs_Table(CORE_1_ID, &prCommon->fc);
    }
    
//    VERIFY(u4VDEC_HAL_VP9_Reset_Bits_Count(u4CoreId) == 0);
	u4VDEC_HAL_VP9_Reset_Bits_Count(u4CoreId);

    i4VP9CompressedHeaderParse(prCommon, u4CoreId);
    
    if(prCommon->fgMultiCoreEnable == TRUE)
    {
        i4VP9CompressedHeaderParse(prCommon, CORE_0_ID);
        i4VP9CompressedHeaderParse(prCommon, CORE_1_ID);
    }

    u4VDEC_HAL_VP9_Get_Bits_Count(u4CoreId);
    
//  u4BytesRead = u4BitsRead / 8 + ((u4BitsRead&0x7) > 0);
//  VERIFY(u4BytesRead == prUnCompressed->u4FirstPartitionSize);
    // set registers
    vVP9SetPicInfo(u4InstID, u4CoreId);

    vVDEC_HAL_VP9_Set_Tile_Info(u4CoreId, prCommon);
    vVDEC_HAL_VP9_Clear_Counts_Table(u4CoreId);

    if(prCommon->fgMultiCoreEnable == TRUE)
    {
        vVP9SetPicInfo(u4InstID, CORE_0_ID);
        vVDEC_HAL_VP9_Set_Tile_Info(CORE_0_ID, prCommon);
        vVDEC_HAL_VP9_Clear_Counts_Table(CORE_0_ID);
        
        vVP9SetPicInfo(u4InstID, CORE_1_ID);
        vVDEC_HAL_VP9_Set_Tile_Info(CORE_1_ID, prCommon);
        vVDEC_HAL_VP9_Clear_Counts_Table(CORE_1_ID);
    }
    
    for(i = 0; i < REFS_PER_FRAME; i++)
    {
        prCommon->FRAME_REFS[i].rScaleFactors.i4X_scale_fp = REF_INVALID_SCALE;
        prCommon->FRAME_REFS[i].rScaleFactors.i4Y_scale_fp = REF_INVALID_SCALE;
        prCommon->FRAME_REFS[i].rScaleFactors.i4X_step_q4 = REF_INVALID_STEP;        
        prCommon->FRAME_REFS[i].rScaleFactors.i4Y_step_q4 = REF_INVALID_STEP;
        prCommon->FRAME_REFS[i].rScaleFactors.u4Ref_Scaling_EN = 0;
    }
    memset(&prCommon->counts, 0, sizeof(FRAME_COUNTS));
    CHECK_ERROR(prCommon->eErrno);
    return;
}
#ifndef IRQ_DISABLE 
#include "../../UDVT/u_os.h"
typedef enum
{
    X_SEMA_TYPE_BINARY = 1,
    X_SEMA_TYPE_MUTEX,
    X_SEMA_TYPE_COUNTING
}   SEMA_TYPE_T;
#define X_SEMA_STATE_LOCK   ((UINT32) 0)

extern INT32 x_sema_create (HANDLE_T*    ph_sema_hdl,
                     SEMA_TYPE_T  e_type,
                     UINT32       ui4_init_value);
extern INT32 x_sema_unlock (HANDLE_T  h_sema_hdl);
extern INT32 x_sema_lock (HANDLE_T       h_sema_hdl,
                   SEMA_OPTION_T  e_option);

void VP9CreateSema()
{
	if(x_sema_create(&hVdecDecDone[0], X_SEMA_TYPE_BINARY, X_SEMA_STATE_LOCK) == 0)
	if(x_sema_create(&hVdecDecDone[1], X_SEMA_TYPE_BINARY, X_SEMA_STATE_LOCK) == 0)

}
void VP9UnlockSema()
{
	VERIFY(x_sema_unlock(hVdecDecDone[0]) == OSR_OK);
}
#endif

UINT32 gl_b_u4FrameNum = 0;
UINT32 gl_b_u4Ret = 0;
UINT32 gl_b_u4Ret1 = 0;
UINT32 gl_b_u4Ret2 = 0;
UINT32 gl_b_u4Ret3 = 0;
ULONG gl_b_jiffies = 0;

UINT32 gl_a_u4FrameNum = 0;
UINT32 gl_a_u4Ret = 0;
UINT32 gl_a_u4Ret1 = 0;
UINT32 gl_a_u4Ret2 = 0;
UINT32 gl_a_u4Ret3 = 0;
ULONG gl_a_jiffies = 0;

UINT64 gl_DecodeTime = 0;


void vVerDecodeVP9(UINT32 u4InstID)
{   
    UINT32 ret;
    UINT32 u4Ret = 0;
    UINT32 u4Ret1 = 0;
    UINT32 u4Ret2 = 0;
    UINT32 u4Ret3 = 0;
    UINT32 u4Ret4 = 0;
    VP9_COMMON_T* prCommon = prVP9GetCommon(u4InstID);
    CHECK_ERROR(prCommon->eErrno);
    
    VP9_PRINT_INFO("[#pic %d] Decode -------\n", prCommon->u4FrameNum);
    UINT32 u4CoreId = 0;

    if(prCommon->fgMultiCoreEnable)
    {
        u4CoreId = VP9_LAE_ID;
    }
    else
    {
        u4CoreId = CORE_0_ID;
    }

    if(prCommon->rUnCompressedHeader.u4ShowExisting)
    {
        VP9_PRINT_INFO("Skip Decode");
        return;
    }
    
    if((prCommon->rUnCompressedHeader.u4FrameType == KEY_FRAME||prCommon->rUnCompressedHeader.u4IntraOnly) && !prCommon->u4KeyFrameDecoded)
    {
        prCommon->u4KeyFrameDecoded = 1;
    }
	vVP9RISCRead_VLD_TOP(21,&ret,u4CoreId);	
	VP9_PRINT_INFO("VP9 1.  VLD_TOP_21 = 0x%x.",ret);
	ret = ret | 0x1;
	vVP9RISCWrite_VLD_TOP(21,ret,u4CoreId);
	VP9_PRINT_INFO("VP9 2. VLD_TOP_21 = 0x%x.",ret);

	vVP9RISCRead_MISC(88,&ret,u4CoreId); 
	VP9_PRINT_INFO("VP9 1.	MISC_88 = 0x%x.",ret);
	ret = ret | 0x10;
	vVP9RISCWrite_MISC(88,ret,u4CoreId);
	VP9_PRINT_INFO("VP9 2. MISC_88 = 0x%x.",ret);

	
#if VP9_CRC_ENABLE
    //vVP9RISCWrite_MCore_TOP(192, 1, 0);
    vVP9RISCWrite_MISC(1, 0x110, 0);
#endif
	vVP9RISCRead_MISC(41 , &u4Ret, u4InstID);
	vVP9RISCRead_VLD_TOP(40 , &u4Ret1, u4InstID);
	vVP9RISCRead_VP9_VLD(95, &u4Ret2, u4InstID);
	vVP9RISCRead_MC(478, &u4Ret3, u4InstID);

	//vVP9RISCRead_VLD_TOP(72 , &u4Ret4, u4InstID);
	//VP9_PRINT_ERROR("1. VP9 VLD_TOP_72 = 0x%x.",u4Ret4);
	//u4Ret4 = u4Ret4 & 0xFFFFFFFD;
	//vVP9RISCWrite_VLD_TOP(72 , u4Ret4, u4InstID);
	//VP9_PRINT_ERROR("2. VP9 VLD_TOP_72 = 0x%x.",u4Ret4);

	
	gl_b_u4FrameNum = prCommon->u4FrameNum;
	gl_b_u4Ret = u4Ret;
	gl_b_u4Ret1 = u4Ret1;
	gl_b_u4Ret2 = u4Ret2;
	gl_b_u4Ret3 = u4Ret3;
	gl_b_jiffies = jiffies;
	VP9_PRINT_INFO("\n [Before Trigger Decode][#pic %d] MISC_41=0x%x,VLD_TOP_40=0x%x,VP9_VLD_95=0x%x,MC_478=0x%x,FWtime=%ld !\n",
    prCommon->u4FrameNum,u4Ret,u4Ret1,u4Ret2,u4Ret3,jiffies);
#ifndef IRQ_DISABLE  
	VERIFY(x_sema_unlock(*(prCommon->p_hVdecDecDone)) == 0);
	x_sema_lock(*(prCommon->p_hVdecDecDone), X_SEMA_OPTION_NOWAIT);//make sure it's locked before trigger decode
#endif				
    // for MCore need refine
    if(prCommon->fgMultiCoreEnable)
    {                
        vVP9RISCWrite_VLD_TOP(73, 0x0B2003FC, 1);
        vVP9RISCWrite_VLD_TOP(73, 0x07200190, 8);
        vVP9RISCWrite_VP9_VLD(46, 0x1, VP9_LAE_ID);
        prCommon->u4DecodeResult = VP9_Wait_LAE_Decode_Done(jiffies, u4CoreId);
        
        if(!prCommon->u4DecodeResult)
        {
            // disable core0/core1 error detector
            vVP9RISCWrite_VP9_VLD(75, 0, 0);
            vVP9RISCWrite_VP9_VLD(75, 0, 1);
    
            // Trigger MCore Decode
            vVP9RISCWrite_MCore_TOP(25, 1, 0);            
        }
        else // LAE Decode Error
        {
            //hanlde error in decdoe done stage
            return;
        }
    }
    else
    {
        vVP9RISCWrite_VP9_VLD(46, 0x1, u4CoreId);
    }
	vVP9RISCRead_MISC(41 , &u4Ret, u4InstID);
	vVP9RISCRead_VLD_TOP(40 , &u4Ret1, u4InstID);
	vVP9RISCRead_VP9_VLD(95, &u4Ret2, u4InstID);
	vVP9RISCRead_MC(478, &u4Ret3, u4InstID);
	
	gl_a_u4FrameNum = prCommon->u4FrameNum;
	gl_a_u4Ret = u4Ret;
	gl_a_u4Ret1 = u4Ret1;
	gl_a_u4Ret2 = u4Ret2;
	gl_a_u4Ret3 = u4Ret3;
	gl_a_jiffies = jiffies;
	VP9_PRINT_INFO("\n [After Trigger Decode][#pic %d] MISC_41=0x%x,VLD_TOP_40=0x%x,VP9_VLD_95=0x%x,MC_478=0x%x,FWtime=%ld !\n",
    prCommon->u4FrameNum,u4Ret,u4Ret1,u4Ret2,u4Ret3,jiffies);
}

void vVP9DumpMem(UCHAR* buf, UINT32 size ,UINT32 frame_num ,UINT32 u4Type)
{
    UCHAR fpDumpFile[80] = "d:\\8590\\vDecY_";   //qianqian@20150215
    UCHAR fpDumpFileC[100] = "d:\\8590\\vDecC_";
    UCHAR fpDumpFileLAE[100] = "d:\\8590\\vDecLAE_";
    UCHAR fpDumpFileERR[100] = "d:\\8590\\vDecERR_";
    UCHAR fpDumpFileTile[100] = "d:\\8590\vDecTile_";    
    UCHAR fpDumpFileCountTBL[100] = "d:\\8590\\vDecCountTBL_";
    UCHAR fpDumpFileProbTBL[100] = "d:\\8590\\vDecProbTBL_";
    UCHAR fpDumpFileInput[100] = "d:\\8590\\vInput_";
	
    UCHAR ucBitstreamName[256];
    
    UCHAR *fpDump;
    UINT32 u4ReadSize;
    UINT32 u4Temp;
    FILE *pFile = NULL;
    UCHAR  ucCaseName[32] = {0};
    UINT32 i = 0;
    UCHAR  ucTmpStr[256] = {0};
    UCHAR  *ucCurrent = NULL;
    UCHAR* const delim = "\\";
    UCHAR* ucToken;

    if(u4Type >= COUNT_TBL_BUFFER && u4Type != INPUT_BUFFER)
    {
    		printk("@@@@@@@@@invalid dump ,return =%d\n", u4Type );
		return;
    }

    memset(ucBitstreamName, 0, 256);
    strncpy (ucBitstreamName , _bFileStr1[0][1], (strlen(_bFileStr1[0][1]) - 21));

    strcpy(ucTmpStr, ucBitstreamName);
    ucCurrent = ucTmpStr;
    
    while (ucToken = strsep((CHAR**)&ucCurrent, delim)) //qianqian@20150215
    {
        if(strlen(ucToken) > 1)
        {
            strcpy(ucCaseName, ucToken);
        }
    }

    switch(u4Type)
    {
        case PIC_Y_BUFFER:
            fpDump = fpDumpFile;
            break;
        case PIC_C_BUFFER:
            fpDump = fpDumpFileC;
            break;
        case LAE_BUFFER:
            fpDump = fpDumpFileLAE;
            break;
        case ERR_BUFFER:
            fpDump = fpDumpFileERR;
            break;
        case TILE_BUFFER:
            fpDump = fpDumpFileTile;
            break;
        case COUNT_TBL_BUFFER:
            fpDump = fpDumpFileCountTBL;
            break;    
	case PROB_TBL_BUFFER:
            fpDump = fpDumpFileProbTBL;
            break; 
	case INPUT_BUFFER:
            fpDump = fpDumpFileInput;
            break; 
			
        default:
            
            break;
    }

    u4Temp = strlen(fpDump);
    u4Temp += sprintf(fpDump + u4Temp,"stream_%s_frame_", ucCaseName);
    u4Temp += sprintf(fpDump + u4Temp,"%d",frame_num);
    u4Temp += sprintf(fpDump + u4Temp,"%s",".dram");
	printk("dump mem %s ",fpDump);

#ifdef SATA_HDD_READ_SUPPORT
    BOOL fgResult;
    fgResult = fgWrData2PC(buf, size, 7, fpDump);
    if(!fgResult)
        printk("[Write File Error!!]\n");
//      fgOverWrData2PC(buf, size, u4Mode, fpDump);
#else
    pFile = fopen(fpDump,"wb");

    if(pFile == NULL)
    {
        printk("Create file error !\n");
    }
    u4ReadSize = fwrite ((char* )(buf), 1, size, pFile);
    printk("read file len = %d @ 0x%x\n",u4ReadSize,(UINT32)buf);
    fclose(pFile);
#endif
    return;

}

int iVP9_CRC_Check(UINT32 u4InstID, UINT32 u4FrameNum, UINT32 u4CoreId)
{
    UINT32 u4FileNameLen = 0;
    UINT32 i = 0;
    UINT32 u4HW_Y_Result[4] = {0};
    UINT32 u4HW_CbCr_Result[4] = {0};
    UINT32 u4Golden = 0;
    UINT32 u4GoldenOffset = 4 ;
    char file_name[256] = {0};
    BOOL fgOpen = FALSE;
    BOOL fgCmpRet = TRUE;
    BOOL fgDumpY = FALSE;
    BOOL fgDumpC = FALSE;
    VP9_COMMON_T* prCommon = prVP9GetCommon(u4InstID);
    
    // MCore
    ULONG ucVP9CRCYBuf0 = prCommon->rDramCRCYBuf0.ulBufVAddr;
    ULONG ucVP9CRCCBuf0 = prCommon->rDramCRCCBuf0.ulBufVAddr;
    ULONG ucVP9CRCYBuf1 = prCommon->rDramCRCYBuf1.ulBufVAddr;
    ULONG ucVP9CRCCBuf1 = prCommon->rDramCRCCBuf1.ulBufVAddr;
    // SCore
    ULONG ucVP9CRCYBuf2 = prCommon->rDramCRCYBuf2.ulBufVAddr;
    ULONG ucVP9CRCCBuf2 = prCommon->rDramCRCCBuf2.ulBufVAddr;

    if (prCommon->fgCRCOpen == FALSE)
    {
        // Load Y-CRC to DRAM        
        if(prCommon->fgMultiCoreEnable)
        {
            if(prCommon->fgUFOModeEnable)
            {
                sprintf(file_name, "%scrc/crc_ufo_mcore%d_Y.dat", prCommon->ucBitstreamName, 0);
            }
            else // w/o UFO
            {
                sprintf(file_name, "%scrc/crc_ufo_bypass_mcore%d_Y.dat", prCommon->ucBitstreamName, 0);   
            }
                
            printk("// Y0 CRC file = %s\n", file_name);
            _tInCRCFileInfo[u4InstID].fgGetFileInfo = TRUE;
		            _tInCRCFileInfo[u4InstID].pucTargetAddr = (UCHAR  *)ucVP9CRCYBuf0;  //qianqian@20150215
            _tInCRCFileInfo[u4InstID].u4TargetSz = VP9_CRC_BUFFER_SZ;
            _tInCRCFileInfo[u4InstID].u4FileLength = 0;
            _tInCRCFileInfo[u4InstID].u4FileOffset = 0;
    
            fgOpen = fgOpenFile(u4InstID, file_name, "r+b", &_tInCRCFileInfo[u4InstID]);
            if (fgOpen == FALSE)
            {
                VP9_PRINT_ERROR("Y0 CRC file fail!!");
                return TRUE;
            }
            
            memset(file_name, 0, 256);
            
            if(prCommon->fgUFOModeEnable)
            {
                sprintf(file_name, "%scrc/crc_ufo_mcore%d_Y.dat", prCommon->ucBitstreamName, 1);
            }
            else
            {                
                sprintf(file_name, "%scrc/crc_ufo_bypass_mcore%d_Y.dat", prCommon->ucBitstreamName, 1);
            }

            printk("// Y1 CRC file = %s\n", file_name);
                
		            _tInCRCFileInfo[u4InstID].pucTargetAddr = (UCHAR  *)ucVP9CRCYBuf1;
            _tInCRCFileInfo[u4InstID].u4TargetSz = VP9_CRC_BUFFER_SZ;
            _tInCRCFileInfo[u4InstID].u4FileLength = 0;
            _tInCRCFileInfo[u4InstID].u4FileOffset = 0;
            
            fgOpen = fgOpenFile(u4InstID, file_name, "r+b", &_tInCRCFileInfo[u4InstID]);
    

            if (fgOpen == FALSE)
            {
                VP9_PRINT_ERROR("Y1 CRC file fail!!");
                return TRUE;
            }
        }
        else // single-core
        {
		            if(prCommon->fgUFOModeEnable)
		            {
		                sprintf(file_name, "%scrc\\crc_ufo_single_Y.dat", prCommon->ucBitstreamName);           
		            }
		            else // w/o UFO
		            {
		                sprintf(file_name, "%scrc\\crc_ufo_bypass_single_Y.dat", prCommon->ucBitstreamName);   
		            }
		           
		            printk("// Y CRC file = %s\n", file_name);
		            _tInCRCFileInfo[u4InstID].fgGetFileInfo = TRUE;
		            _tInCRCFileInfo[u4InstID].pucTargetAddr = (UCHAR  *)ucVP9CRCYBuf2;
		            _tInCRCFileInfo[u4InstID].u4TargetSz = VP9_CRC_BUFFER_SZ;
		            _tInCRCFileInfo[u4InstID].u4FileLength = 0;
		            _tInCRCFileInfo[u4InstID].u4FileOffset = 0;
		    
		            fgOpen = fgOpenFile(u4InstID, file_name, "r+b", &_tInCRCFileInfo[u4InstID]);
		    
		            if (fgOpen == FALSE)
		            {
		                VP9_PRINT_ERROR("open Y CRC file fail!!");
		                return TRUE;
		            }
        }
    
        // Load C-CRC to DRAM
        memset(file_name, 0, 256);
    
        if(prCommon->fgMultiCoreEnable)
        {
            if(prCommon->fgUFOModeEnable)
            {
                sprintf(file_name, "%scrc/crc_ufo_mcore%d_C.dat", prCommon->ucBitstreamName, 0);
            }
            else
            {
                sprintf(file_name, "%scrc/crc_ufo_bypass_mcore%d_C.dat", prCommon->ucBitstreamName, 0);                
            }
            printk("// CRC0 file = %s\n", file_name);
               
            _tInCRCFileInfo[u4InstID].pucTargetAddr = (UCHAR  *)ucVP9CRCCBuf0;
            _tInCRCFileInfo[u4InstID].u4TargetSz = VP9_CRC_BUFFER_SZ;
            _tInCRCFileInfo[u4InstID].u4FileLength = 0;
            _tInCRCFileInfo[u4InstID].u4FileOffset = 0;
                
            fgOpen = fgOpenFile(u4InstID, file_name, "r+b", &_tInCRCFileInfo[u4InstID]);
                
            if (fgOpen == FALSE)
            {
                VP9_PRINT_ERROR("open C0 CRC file fail!!");
                return TRUE;
            }
    
            memset(file_name, 0, 200);
                
            if(prCommon->fgUFOModeEnable)
            {
                sprintf(file_name, "%scrc/crc_ufo_mcore%d_C.dat", prCommon->ucBitstreamName, 1);
            }
            else
            {
                sprintf(file_name, "%scrc/crc_ufo_bypass_mcore%d_C.dat", prCommon->ucBitstreamName, 1);
            }
            printk("// CRC1 file = %s\n", file_name);
                
            _tInCRCFileInfo[u4InstID].pucTargetAddr = (UCHAR  *)ucVP9CRCCBuf1;
            _tInCRCFileInfo[u4InstID].u4TargetSz = VP9_CRC_BUFFER_SZ;
            _tInCRCFileInfo[u4InstID].u4FileLength = 0;
            _tInCRCFileInfo[u4InstID].u4FileOffset = 0;
    
            fgOpen = fgOpenFile(u4InstID, file_name, "r+b", &_tInCRCFileInfo[u4InstID]);
            
            if (fgOpen == FALSE)
            {
                VP9_PRINT_ERROR("open C1 CRC file fail!!");
                return TRUE;
            }            
        }
        else // single-core
        {
            if(prCommon->fgUFOModeEnable)
            {
                sprintf(file_name, "%scrc\\crc_ufo_single_C.dat", prCommon->ucBitstreamName);
            }
            else
            {
                sprintf(file_name, "%scrc\\crc_ufo_bypass_single_C.dat", prCommon->ucBitstreamName);                
            }
            printk("// CRC file = %s\n", file_name);
                
            _tInCRCFileInfo[u4InstID].pucTargetAddr = (UCHAR  *)ucVP9CRCCBuf2;
            _tInCRCFileInfo[u4InstID].u4TargetSz = VP9_CRC_BUFFER_SZ;
            _tInCRCFileInfo[u4InstID].u4FileLength = 0;
            _tInCRCFileInfo[u4InstID].u4FileOffset = 0;
               
            fgOpen = fgOpenFile(u4InstID, file_name, "r+b", &_tInCRCFileInfo[u4InstID]);
                
            if (fgOpen == FALSE)
            {
                VP9_PRINT_ERROR("open C CRC file fail!!");
                return TRUE;
            }       
        }

        prCommon->fgCRCOpen = TRUE;
    }
    
    /*
    Core0 (single) Y: mcore_top_193 ~ 196
    Core0 (single) C: mcore_top_197 ~ 200
    Core1 Y:          mcore_top_201 ~ 204
    Core1 C:          mcore_top_205 ~ 208
    */
    /*
    *  8137 core0: 95~102
   */
    // check Y CRC
    for (i = 0; i < 4; i++)
    {        
        if(prCommon->fgMultiCoreEnable)
        {                 
            u4HW_Y_Result[i] = u4VDEC_HAL_VP9_VDec_ReadCRC_Y0(i, u4CoreId);
            
            u4Golden = (((ULONG*)ucVP9CRCYBuf0))[u4FrameNum*u4GoldenOffset + i];            
            // core 0 compare
            if (u4HW_Y_Result[i] != u4Golden)
            {
                fgCmpRet = FALSE;
                fgDumpY = TRUE;
                VP9_PRINT_INFO("Y CRC compare fail!!");
                VP9_PRINT_INFO("[Core-%d] i:%d, HW: 0x%x, Golden: 0x%x", 0, i, u4HW_Y_Result[i], u4Golden);
            }                
            
            u4HW_Y_Result[i] = u4VDEC_HAL_VP9_VDec_ReadCRC_Y1(i, u4CoreId);
                
            u4Golden = (((ULONG*)ucVP9CRCYBuf1))[u4FrameNum*u4GoldenOffset + i];
            // core 1 compare
            if (u4HW_Y_Result[i] != u4Golden)
            {
                fgCmpRet = FALSE;
                fgDumpY = TRUE;
                VP9_PRINT_INFO("//Y CRC compare fail!!");
                VP9_PRINT_INFO("//[Core-%d] i:%d, HW: 0x%x, Golden: 0x%x", 1, i, u4HW_Y_Result[i], u4Golden);
            }                 
        }
        else // single core
        {                       
            u4HW_Y_Result[i] = u4VDEC_HAL_VP9_VDec_ReadCRC_Y0(i, u4CoreId);
            u4Golden = (((UINT32*)ucVP9CRCYBuf2))[u4FrameNum*u4GoldenOffset + i];            
            if (u4HW_Y_Result[i] != u4Golden)
            {
                fgCmpRet = FALSE;
                fgDumpY = TRUE;
                VP9_PRINT_INFO("Y CRC compare fail!!\n");
                VP9_PRINT_INFO("i:%d, HW: 0x%x, Golden: 0x%x\n", i, u4HW_Y_Result[i], u4Golden);
            }                  
        }       
    }
        
#ifdef SATA_HDD_READ_SUPPORT
        // dump dram, already dump outside if crc fail
        if( 0 )
        {            
            vVP9DumpMem((UCHAR*)prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufY.ulBufVAddr, 
                        prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufY.u4BufSize,
                        prCommon->u4FrameNum,
                        PIC_Y_BUFFER);
        }
#endif       
    
    // check C CRC
    for (i = 0; i < 4; i++)
    {        
        if(prCommon->fgMultiCoreEnable)
        {
            u4HW_CbCr_Result[i] = u4VDEC_HAL_VP9_VDec_ReadCRC_C0(i, u4CoreId);
                
            u4Golden = (((ULONG*)ucVP9CRCCBuf0))[u4FrameNum*u4GoldenOffset + i];
            if (u4HW_CbCr_Result[i] != u4Golden)
            {
                fgCmpRet = FALSE;
                fgDumpC = TRUE;
                VP9_PRINT_INFO("CbCr CRC compare fail!!");
                VP9_PRINT_INFO("[Core-%d] i:%d, HW: 0x%x, Golden: 0x%x", 0, i, u4HW_CbCr_Result[i], u4Golden);
            }
             
            u4HW_CbCr_Result[i] = u4VDEC_HAL_VP9_VDec_ReadCRC_C1(i, u4CoreId);
                
            u4Golden = ((ULONG*)ucVP9CRCCBuf1)[u4FrameNum*u4GoldenOffset + i];                      
            if (u4HW_CbCr_Result[i] != u4Golden)
            {
                fgCmpRet = FALSE;
                fgDumpC = TRUE;
                VP9_PRINT_INFO("CbCr CRC compare fail!!");
                VP9_PRINT_INFO("[Core-%d] i:%d, HW: 0x%x, Golden: 0x%x", 1, i, u4HW_CbCr_Result[i], u4Golden);
            }
        }
        else // single core
        {   
            u4HW_CbCr_Result[i] = u4VDEC_HAL_VP9_VDec_ReadCRC_C0(i, u4CoreId);

            u4Golden = (((UINT32*)ucVP9CRCCBuf2))[u4FrameNum*u4GoldenOffset + i];
            if (u4HW_CbCr_Result[i] != u4Golden)
            {
                fgCmpRet = FALSE;
                fgDumpC = TRUE;
                VP9_PRINT_INFO("CbCr CRC compare fail!!");
                VP9_PRINT_INFO("i:%d, HW: 0x%x, Golden: 0x%x", i, u4HW_CbCr_Result[i], u4Golden);
            }
        }
    }      
                 
#ifdef SATA_HDD_READ_SUPPORT       
        // dump dram, already dump outside if crc fail
        if( 0 )
        {
            vVP9DumpMem((UCHAR*)prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufC.ulBufVAddr, 
                        prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufC.u4BufSize,
                        prCommon->u4FrameNum,
                        PIC_C_BUFFER);          
        }
#endif   
        // dump dram, already dump outside if crc fail
        if (0)
        {
            if(prCommon->fgMultiCoreEnable)
            {
                vVP9DumpMem((UCHAR*)prCommon->rLAEBuffer.ulBufVAddr, 
                            prCommon->rLAEBuffer.u4BufSize,
                            prCommon->u4FrameNum,
                            LAE_BUFFER);
                
                vVP9DumpMem((UCHAR*)prCommon->rErrorBuffer.ulBufVAddr, 
                            prCommon->rErrorBuffer.u4BufSize,
                            prCommon->u4FrameNum,
                            ERR_BUFFER);
            }
            return TRUE;
        }
        else
        {
        	if(fgDumpC ||fgDumpY)
        		{
        			printk("------CRC Mismatch-------\n");
            			return TRUE;
        		}
			else
			{
				//printk("compare pass\n");
				return FALSE;
			}
        }
}


BOOL fgGoldenCmp_VP9(ULONG u4DecBuf,ULONG u4GoldenBuf,UINT32 u4Size)
{
    UINT32 i;
    UINT32 *pSrc,*pDes;
    pSrc = (UINT32 *)u4DecBuf;
    pDes = (UINT32 *)u4GoldenBuf;
    //width 64 align,height 32 align
    for(i = 0; i < u4Size/4; i++)
    {
        if(*pSrc != *pDes)
        {
            printk("Decbuf 0x%lx [0x%x] != GoldBuf 0x%lx [0x%x] offset %d\n",u4DecBuf,*pSrc,u4GoldenBuf,*pDes,i);
            return 1;
        }
        pSrc ++;
        pDes ++;
    }

    return 0;
}


int  iVP9GoldenComparison( UINT32 u4InstID)
{
    //golden result comparison
    char file_name[200] = {0};
    struct file *fd; 
    int file_num, file_len;
    UINT32 u4RetY, u4RetC, u4Ret;
    BOOL fgOpen;
    bool isDump = TRUE;

    VP9_COMMON_T* prCommon = prVP9GetCommon(u4InstID);
    VP9_FB_INFO_T *prFrameInfo = &(prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf);

    UINT32 u4PIC_SIZE_Y = prFrameInfo->u4YWidth * prFrameInfo->u4YHeight;
    UINT32 u4UFO_LEN_SIZE_Y = ((u4PIC_SIZE_Y+ 255) >> 8);
    UINT32 u4UFO_LEN_SIZE_C = (u4UFO_LEN_SIZE_Y >> 1);
	
    //UINT32 ucVP9GoldenYBuf = prCommon->rDramGOLDENYbuf.u4BufVAddr;
    //UINT32 ucVP9GoldenCBuf = prCommon->rDramGOLDENCbuf.u4BufVAddr;
    
    u4Ret = 0;
    u4RetY = 0;
    u4RetC = 0;
    file_len = 0;
    file_num = 0;
    

    printk("// [INFO] GoldenComparison PIC_SIZE_Y: 0x%08X, Width/Height: 0x%X,0x%X,u4UFO_LEN_SIZE_Y/C :0x%X,0x%X\n", u4PIC_SIZE_Y, prFrameInfo->u4YWidth,prFrameInfo->u4YHeight,u4UFO_LEN_SIZE_Y,u4UFO_LEN_SIZE_C);

    // UFO
    if (prCommon->fgUFOModeEnable)
    {
        sprintf(file_name, "%sufo_pat/ufo_%d_bits_Y.out", prCommon->ucBitstreamName, prCommon->u4FrameNum);
    } 
    else
    {
        sprintf(file_name, "%spp_pat/frame_%d_Y.dat", prCommon->ucBitstreamName, prCommon->u4FrameNum);
    }

    //dump Y golden
    _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID] ;
    _tFBufFileInfo[u4InstID].u4TargetSz = u4PIC_SIZE_Y;  
    _tFBufFileInfo[u4InstID].u4FileLength = 0;    
    _tFBufFileInfo[u4InstID].u4FileOffset = 0; 
    memset ( _pucDumpYBuf[u4InstID]  , 0 ,u4PIC_SIZE_Y );
    fgOpen = fgOpenFile(u4InstID, file_name,"r+b", &_tFBufFileInfo[u4InstID]);
    if (fgOpen == FALSE)
    {
        printk("// Open golden file error : %s\n",file_name);
    }

    // UFO
    if (prCommon->fgUFOModeEnable)
    {
        sprintf(file_name, "%sufo_pat/ufo_%d_bits_C.out", prCommon->ucBitstreamName, prCommon->u4FrameNum);     
    } 
    else
    {
        sprintf(file_name, "%spp_pat/frame_%d_C.dat", prCommon->ucBitstreamName, prCommon->u4FrameNum);
    }
       
    //dump CbCr golden
    _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
    _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpCBuf[u4InstID];
    _tFBufFileInfo[u4InstID].u4TargetSz = (u4PIC_SIZE_Y >> 1);  
    _tFBufFileInfo[u4InstID].u4FileLength = 0;    
    _tFBufFileInfo[u4InstID].u4FileOffset = 0; 
    memset ( _pucDumpCBuf[u4InstID] , 0 ,(u4PIC_SIZE_Y >> 1) );
    fgOpen = fgOpenFile(u4InstID, file_name,"r+b", &_tFBufFileInfo[u4InstID]);
    if (fgOpen == FALSE)
    {
        printk("// Open golden file error : %s\n",file_name);
    }

    //////////////Y golden comparison//////////////////// 

    u4RetY = fgGoldenCmp_VP9(prFrameInfo->rBufY.ulBufVAddr, (ULONG)_pucDumpYBuf[u4InstID] ,u4PIC_SIZE_Y);
    printk("\n// ======== Frame %d Golden Y test: %d ========\n", prCommon->u4FrameNum, u4RetY); 
 
    //////////////C golden comparison////////////////////

    u4RetC = fgGoldenCmp_VP9(prFrameInfo->rBufC.ulBufVAddr, (ULONG)_pucDumpCBuf[u4InstID], (u4PIC_SIZE_Y >> 1));
    printk("\n// ======== Frame %d Golden C test: %d ========\n", prCommon->u4FrameNum, u4RetC );

    if((u4RetY != 0) || (u4RetC != 0))
    {
        printk("// Compare mismatch here,please check!\n");
        return 1;
    }

#ifndef SATA_HDD_READ_SUPPORT
	if (u4RetY != 0 )
	{
		if (isDump)
		{
			vVP9DumpMem((UCHAR*)prFrameInfo->rBufY.u4BufVAddr,u4PIC_SIZE_Y,prCommon->u4FrameNum,PIC_Y_BUFFER);
		}
		//set_fs( oldfs );
		//return 1;
	}
#endif
	

#ifndef SATA_HDD_READ_SUPPORT
	if (u4RetC !=0 )
	{
		if (isDump)
		{
			vVP9DumpMem((UCHAR*)prFrameInfo->rBufC.u4BufVAddr,u4PIC_SIZE_Y>>1,prCommon->u4FrameNum,PIC_C_BUFFER);
		}
		//set_fs( oldfs );
		//return 1;
	}
#endif
	
    if (prCommon->fgUFOModeEnable)
    {
     
         //////////////Y LEN comparison////////////////////
        sprintf(file_name, "%sufo_pat/ufo_%d_len_Y.out", prCommon->ucBitstreamName, prCommon->u4FrameNum);

        //dump Y golden
        _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
        _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpYBuf[u4InstID] ; // use Y buf
        _tFBufFileInfo[u4InstID].u4TargetSz = u4UFO_LEN_SIZE_Y;  
        _tFBufFileInfo[u4InstID].u4FileLength = 0;        
        _tFBufFileInfo[u4InstID].u4FileOffset = 0; 
        memset(_pucDumpYBuf[u4InstID] , 0 ,u4UFO_LEN_SIZE_Y );
        fgOpen = fgOpenFile(u4InstID, file_name,"r+b", &_tFBufFileInfo[u4InstID]);
        if (fgOpen == FALSE)
        {
            printk("// Open golden file error : %s\n",file_name);
        }
        else
        {
            
        }
        u4Ret = memcmp(_pucDumpYBuf[u4InstID] , (void*)prFrameInfo->rUFO_LEN_Y.ulBufVAddr, u4UFO_LEN_SIZE_Y);
        
        if ( isDump || u4Ret ==0 )
        {
            printk("\n// ======== Frame %d UFO Y LEN test: %d ========\n", prCommon->u4FrameNum, u4Ret );
        }
        
#ifndef SATA_HDD_READ_SUPPORT
        if (u4Ret !=0 )
        {
            if (isDump)
            {
            	vVP9DumpMem((UCHAR*)prFrameInfo->rUFO_LEN_Y.u4BufVAddr,u4UFO_LEN_SIZE_Y,prCommon->u4FrameNum,UFO_Y_LEN);

            }
            //set_fs( oldfs );
            return 1;
        }
#else
        if(u4Ret != 0)
        {
            return u4Ret;
        }
#endif
        //////////////C LEN comparison////////////////////
        sprintf(file_name, "%sufo_pat/ufo_%d_len_C.out", prCommon->ucBitstreamName, prCommon->u4FrameNum);

        //dump Y golden
        _tFBufFileInfo[u4InstID].fgGetFileInfo = TRUE;  
        _tFBufFileInfo[u4InstID].pucTargetAddr = _pucDumpCBuf[u4InstID];
        _tFBufFileInfo[u4InstID].u4TargetSz = u4UFO_LEN_SIZE_C;  
        _tFBufFileInfo[u4InstID].u4FileLength = 0;        
        _tFBufFileInfo[u4InstID].u4FileOffset = 0; 
        memset ( _pucDumpCBuf[u4InstID] , 0 ,u4UFO_LEN_SIZE_C);
        fgOpen = fgOpenFile(u4InstID, file_name,"r+b", &_tFBufFileInfo[u4InstID]);
        if (fgOpen == FALSE)
        {
            printk("Open golden file error : %s\n",file_name);
        }
        else
        {
            
        }       
        u4Ret =  memcmp((void*)_pucDumpCBuf[u4InstID], (void*)prFrameInfo->rUFO_LEN_C.ulBufVAddr, u4UFO_LEN_SIZE_C);       
        if ( isDump || u4Ret == 0 )
        {
            printk("\n// ======== Frame %d UFO C LEN test: %d ========\n", prCommon->u4FrameNum, u4Ret );
        }
        
        #ifndef SATA_HDD_READ_SUPPORT
        if (u4Ret !=0 )
        {
            if (isDump)
            {
            	vVP9DumpMem((UCHAR*)prFrameInfo->rUFO_LEN_C.ulBufVAddr,u4UFO_LEN_SIZE_C,prCommon->u4FrameNum,UFO_C_LEN);
            }
            return 1;
        }
        #else
        if(u4Ret != 0)
        {
            return u4Ret;
        }
        #endif

    }
    
    printk("\n");    
    return 0;    
}

//UINT32 u4ReadReg_Ex1(UINT32 dAddr);
//UINT32 u4ReadReg_Ex2(UINT32 dAddr);
void vVerVP9DecEnd(UINT32 u4InstID)
{    
    VP9_COMMON_T* prCommon = prVP9GetCommon(u4InstID);    
    BOOL fgIsTimeOut = 0,fgDecDone = 0;    
    UINT32 u4RetryDelay = 100000;
    UINT32 u4IsFail = 0;
    unsigned long  start_time = jiffies;    
    unsigned long  timeCounter = jiffies; 
    unsigned long  timeMarker = jiffies;    
    UCHAR ucBitstreamName[256];
    UINT32 u4Ret = 0;
    UINT32 u4Ret1 = 0;
    UINT32 u4Ret2 = 0;
    UINT32 u4Ret3 = 0;
    UINT32 u4LoopCount = 0;
	
    memset(ucBitstreamName, 0, 256);
    strncpy (ucBitstreamName , _bFileStr1[0][1], (strlen(_bFileStr1[0][1]) - 21));
    
    CHECK_ERROR(prCommon->eErrno);
	vVP9RISCRead_MISC(41 , &u4Ret, u4InstID);
    VP9_PRINT_INFO("[QQ][#pic %d]  Dec End jiffies=%ld, [0x%x]-------\n", prCommon->u4FrameNum,jiffies,u4Ret);
    
    if(prCommon->rUnCompressedHeader.u4ShowExisting)
    {
	        if(prCommon->u4EndNum == prCommon->u4FrameNum)
	        {
	            _u4VerBitCount[u4InstID] = 0xffffffff;
	        }
	        
	        vVP9SwapFrameBuffers(prCommon);
	        VP9_PRINT_SHOW(" Decode Ok @%d (Skipped)", prCommon->u4FrameNum);
	        prCommon->u4FrameNum++;
	        return;
    }
#ifndef IRQ_DISABLE  
    fgIsTimeOut = vVerVP9WaitDecEnd(prCommon, u4InstID);
    if(fgIsTimeOut)
    {
       fgDecDone = 1;
    }else
    {
       fgDecDone = 0;
    }
#else
    if(prCommon->fgMultiCoreEnable == TRUE && prCommon->u4DecodeResult == 0) // Mcore wait decode done
    {
        	fgDecDone = u4VDEC_HAL_VP9_MCORE_VDec_ReadFinishFlag(u4InstID);
    }
    else if(prCommon->fgMultiCoreEnable == FALSE) // SCore wait decode done
    {
        	fgDecDone = u4VDEC_HAL_VP9_VDec_ReadFinishFlag(u4InstID);
    }
    else // LAE timeout
    {
        	fgDecDone = 1;
        	fgIsTimeOut= 1;
    }
	vVP9RISCRead_MISC(41 , &u4Ret, u4InstID);
    VP9_PRINT_INFO("[#pic %d]Wait Decode Done... 0x%lx,[0x%x]\n",prCommon->u4FrameNum,jiffies,u4Ret);
    
    while(fgDecDone != 1)
    {
		//VP9_PRINT_INFO("[QQ][#pic %d][#loop %d]1. FwTime=%ld\n",prCommon->u4FrameNum,u4LoopCount,jiffies);
	        if(prCommon->fgMultiCoreEnable == TRUE)
	        {
	            fgDecDone = u4VDEC_HAL_VP9_MCORE_VDec_ReadFinishFlag(u4InstID);
	        }
	        else
	        {
	            fgDecDone = u4VDEC_HAL_VP9_VDec_ReadFinishFlag(u4InstID);
	        }
			
			//VP9_PRINT_INFO("[QQ][#pic %d][#loop %d]2. FwTime=%ld\n",prCommon->u4FrameNum,u4LoopCount,jiffies);
			if(jiffies - timeCounter > 4)
			{
				//VP9_PRINT_INFO("[QQ][#pic %d][#loop %d]3. FwTime=%ld\n",prCommon->u4FrameNum,u4LoopCount,jiffies);
				vVP9RISCRead_MISC(41 , &u4Ret, u4InstID);
				vVP9RISCRead_VLD_TOP(40 , &u4Ret1, u4InstID);
				vVP9RISCRead_VP9_VLD(95, &u4Ret2, u4InstID);
				vVP9RISCRead_MC(478, &u4Ret3, u4InstID);
				VP9_PRINT_INFO("\n  [After 20ms][#pic %d]%d,MISC_41=0x%x,VLD_TOP_40=0x%x,VP9_VLD_95=0x%x,MC_478=0x%x,Fwtime=%ld (%ld) !!!!!!\n",
					prCommon->u4FrameNum,fgDecDone,u4Ret,u4Ret1,u4Ret2,u4Ret3,jiffies,timeCounter);
				//VP9_PRINT_INFO("\n	[#pic %d]PLL Setting 0x%x,0x%x,0x%x,0x%x!\n",
				//	prCommon->u4FrameNum,u4ReadReg_Ex1(0x50),
				//	u4ReadReg_Ex1(0x40),
				//	u4ReadReg_Ex2(0x290),
				//	u4ReadReg_Ex2(0x294));
				timeCounter = jiffies;
			}
			//VP9_PRINT_INFO("[QQ][#pic %d][#loop %d]6. FwTime=%ld\n",prCommon->u4FrameNum,u4LoopCount,jiffies);
	        if ( ( jiffies - start_time > 30) )
	        {
				//VP9_PRINT_INFO("[QQ][#pic %d][#loop %d]7. FwTime=%ld\n",prCommon->u4FrameNum,u4LoopCount,jiffies);
				vVP9RISCRead_MISC(41 , &u4Ret, u4InstID);
				vVP9RISCRead_VLD_TOP(40 , &u4Ret1, u4InstID);
				
				vVP9RISCRead_VP9_VLD(95, &u4Ret2, u4InstID);
				VP9_PRINT_ERROR("\n!!!!!! 500 Decode Polling int timeout %d,%ld,%ld[0x%x,0x%x,0x%x] !!!!!!\n",
					fgDecDone,jiffies - start_time,start_time,u4Ret,u4Ret1,u4Ret2);
				//VP9_PRINT_ERROR("\n	PLL Setting 0x%x,0x%x,0x%x,0x%x!\n",
				//	u4ReadReg_Ex1(0x50),
				//	u4ReadReg_Ex1(0x40),
				//	u4ReadReg_Ex2(0x290),
				//	u4ReadReg_Ex2(0x294));
				
				VP9_PRINT_INFO("\n [Before Trigger Decode][#pic %d] MISC_41=0x%x,VLD_TOP_40=0x%x,VP9_VLD_95=0x%x,MC_478=0x%x,FWtime=%ld !\n",
				gl_b_u4FrameNum,gl_b_u4Ret,gl_b_u4Ret1,gl_b_u4Ret2,gl_b_u4Ret3,gl_b_jiffies);
				
				VP9_PRINT_INFO("\n [After Trigger Decode][#pic %d] MISC_41=0x%x,VLD_TOP_40=0x%x,VP9_VLD_95=0x%x,MC_478=0x%x,FWtime=%ld !\n",
				gl_a_u4FrameNum,gl_a_u4Ret,gl_a_u4Ret1,gl_a_u4Ret2,gl_a_u4Ret3,gl_a_jiffies);
	            fgIsTimeOut = 1;
				if(u4Ret == 0x10000)
				{
				   fgIsTimeOut = 0;
				}
				//VP9_PRINT_INFO("[QQ][#pic %d][#loop %d]8. FwTime=%ld\n",prCommon->u4FrameNum,u4LoopCount,jiffies);
	            break;
	        }
			//VP9_PRINT_INFO("[QQ][#pic %d][#loop %d]9. FwTime=%ld\n",prCommon->u4FrameNum,u4LoopCount,jiffies);
	        while(u4RetryDelay >0)
	        {
	            u4RetryDelay--;
	        }
	        u4RetryDelay = 100000;
			//VP9_PRINT_INFO("[QQ][#pic %d][#loop %d]10. FwTime=%ld\n",prCommon->u4FrameNum,u4LoopCount,jiffies);
			u4LoopCount++;
    }
#endif
	if(fgDecDone == 1)
	{
		vVP9RISCRead_VLD_TOP(40 , &u4Ret1, u4InstID);
		vVP9RISCRead_MC(478, &u4Ret3, u4InstID);
		VP9_PRINT_INFO("\n	[Decode Done][#pic %d]VLD_TOP_40=0x%x,MC_478=0x%x, no timeout(%ld)[%ld,%ld]\n",
			prCommon->u4FrameNum,u4Ret1,u4Ret3,jiffies - start_time,start_time,jiffies);

		//gl_DecodeTime = 
	}
	else
	{
	   vVP9RISCRead_VLD_TOP(40 , &u4Ret1, u4InstID);
	   vVP9RISCRead_MC(478, &u4Ret3, u4InstID);
	   VP9_PRINT_INFO("\n  [Decode Done][#pic %d]VLD_TOP_40=0x%x,MC_478=0x%x, timeout(%ld)[%ld,%ld]\n",
		   prCommon->u4FrameNum,u4Ret1,u4Ret3,jiffies - start_time,start_time,jiffies);
	}
	
#ifndef VDEC_BREAK_EN    
    if(fgIsTimeOut)
    {
	        prCommon->eErrno = VP9_ERR_UNKNOW_ERROR;
	        vVDEC_HAL_VP9_VDec_DumpReg(prCommon->u4FrameNum, prCommon->fgMultiCoreEnable, 1);
	        
	        vVP9DumpMem((UCHAR*)prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufY.ulBufVAddr, 
	                    prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufY.u4BufSize,
	                    prCommon->u4FrameNum,
	                    PIC_Y_BUFFER);
	        
	        vVP9DumpMem((UCHAR*)prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufC.ulBufVAddr, 
	                    prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufC.u4BufSize,
	                    prCommon->u4FrameNum,
	                    PIC_C_BUFFER);
	        
	        if(prCommon->fgMultiCoreEnable)
	        {
	            vVP9DumpMem((UCHAR*)prCommon->rLAEBuffer.ulBufVAddr, 
	                        prCommon->rLAEBuffer.u4BufSize,
	                        prCommon->u4FrameNum,
	                        LAE_BUFFER);
	        
	            vVP9DumpMem((UCHAR*)prCommon->rErrorBuffer.ulBufVAddr, 
	                        prCommon->rErrorBuffer.u4BufSize,
	                        prCommon->u4FrameNum,
	                        ERR_BUFFER);
	        }       

	        VP9_PRINT_ERROR("Decode NG, Decode Timeout @%d", prCommon->u4FrameNum);
	        VP9_PRINT_ERROR("@@ VP9 Decode Failed [%s]",ucBitstreamName);
	        //_u4VerBitCount[u4InstID] = 0xFFFFFFFF;
    }
#endif   
    // Compare Prob & Count ?
    
    if(prCommon->fgMultiCoreEnable)
    {
        vVDEC_HAL_VP9_Get_Probs_Table(VP9_LAE_ID, prCommon->rProbTBLBuffer.ulBufVAddr, &prCommon->fc);
        vVDEC_HAL_VP9_Get_Counts_Table(VP9_LAE_ID, prCommon->rCountTBLBuffer.ulBufVAddr, &prCommon->counts);
    }
    else
    {
	        if (prCommon->rUnCompressedHeader.u4RefreshFrameContext)
	        {
	            vVDEC_HAL_VP9_Get_Probs_Table(u4InstID, prCommon->rProbTBLBuffer.ulBufVAddr, &prCommon->fc);              
	        }

	        
	        if (!prCommon->rUnCompressedHeader.u4ErrResilenceMode && !prCommon->rUnCompressedHeader.u4FrameParallelDecodingMode)
	        {
	            vVDEC_HAL_VP9_Get_Counts_Table(u4InstID, prCommon->rCountTBLBuffer.ulBufVAddr, &prCommon->counts);
	        }
    }       
#ifndef VDEC_BREAK_EN
    #if VP9_CRC_ENABLE        
        u4IsFail = iVP9_CRC_Check(u4InstID, prCommon->u4FrameNum, 0); 
    #else
        // Golden compare need implementation Here...
        u4IsFail = iVP9GoldenComparison(u4InstID);
    #endif
#endif   
    //if(u4IsFail && !fgIsTimeOut)
    if(u4IsFail)
    {
	        prCommon->eErrno = VP9_ERR_UNKNOW_ERROR;
	        /**vVDEC_HAL_VP9_VDec_DumpReg(prCommon->u4FrameNum, prCommon->fgMultiCoreEnable, 1);
	        
	        vVP9DumpMem((UCHAR*)prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufY.ulBufVAddr, 
	                    prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufY.u4BufSize,
	                    prCommon->u4FrameNum,
	                    PIC_Y_BUFFER);
	        
	        vVP9DumpMem((UCHAR*)prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufC.ulBufVAddr, 
	                    prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.rBufC.u4BufSize,
	                    prCommon->u4FrameNum,
	                    PIC_C_BUFFER);*/
	        
	        if(prCommon->fgMultiCoreEnable)
	        {
	            vVP9DumpMem((UCHAR*)prCommon->rLAEBuffer.ulBufVAddr, 
	                        prCommon->rLAEBuffer.u4BufSize,
	                        prCommon->u4FrameNum,
	                        LAE_BUFFER);
	        	
			vVP9DumpMem((UCHAR*)prCommon->rErrorBuffer.ulBufVAddr, 
	                        prCommon->rErrorBuffer.u4BufSize,
	                        prCommon->u4FrameNum,
	                        ERR_BUFFER);

	        }
        
	    #if VP9_CRC_ENABLE
	        VP9_PRINT_ERROR("Decode NG, CRC Mismatch @%d", prCommon->u4FrameNum);
	    #else
	        VP9_PRINT_ERROR("Decode NG, Golden Mismatch @%d", prCommon->u4FrameNum);
	    #endif
	        VP9_PRINT_ERROR("@@ VP9 Decode Failed [%s]", ucBitstreamName);
    }
    else if(!fgIsTimeOut)
    {
        VP9_UNCOMPRESSED_HEADER_T* prUnCompress = &prCommon->rUnCompressedHeader;
        if (!prUnCompress->u4ErrResilenceMode && !prUnCompress->u4FrameParallelDecodingMode) 
        {
            vVP9AdaptCoefProbs(prCommon);

            if (!fgVP9IntraOnly(prCommon))
            {
              vVP9AdaptModeProbs(prCommon);
              vVP9AdaptMvProbs(prCommon, prUnCompress->u4AllowHighPrecisionMv);
            }
        } 
        else 
        {        
        //   debug_check_frame_counts(cm);
        }
        
        if (prUnCompress->u4RefreshFrameContext)
            prCommon->frame_contexts[prUnCompress->u4FrameContextIdx] = prCommon->fc;

        vVP9SwapFrameBuffers(prCommon);

        prCommon->u4LastWidth = prUnCompress->u4Width;
        prCommon->u4LastHeight = prUnCompress->u4Height;

        if (!prUnCompress->u4ShowExisting)
            prCommon->u4LastShowFrame = prUnCompress->u4ShowFrame;
        
        if (prUnCompress->u4ShowFrame) 
        {
            if (!prUnCompress->u4ShowExisting)
            {
	                vVP9SwapMiAndPrevMi(prCommon);
	            }
	            //	      prUnCompress->current_video_frame++;
	        }
	        VP9_PRINT_SHOW("Decode Ok @%d", prCommon->u4FrameNum);
	        prCommon->FRAME_BUFS[prCommon->u4NewFbIdx].rBuf.u4FrameNum = prCommon->u4FrameNum;
    }


    if(prCommon->u4EndNum == prCommon->u4FrameNum)
    {
        _u4VerBitCount[u4InstID] = 0xffffffff;
        return;
    }
    prCommon->u4FrameNum++;
    CHECK_ERROR(prCommon->eErrno);
}

