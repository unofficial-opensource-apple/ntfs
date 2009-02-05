/*
 * ntfs_upcase.c - Generate the full, little endian NTFS Unicode upcase table.
 *
 * Copyright (c) 2006, 2007 Anton Altaparmakov.  All Rights Reserved.
 * Portions Copyright (c) 2006, 2007 Apple Inc.  All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution. 
 * 3. Neither the name of Apple Inc. ("Apple") nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ALTERNATIVELY, provided that this notice and licensing terms are retained in
 * full, this file may be redistributed and/or modified under the terms of the
 * GNU General Public License (GPL) Version 2, in which case the provisions of
 * that version of the GPL will apply to you instead of the license terms
 * above.  You can obtain a copy of the GPL Version 2 at
 * http://developer.apple.com/opensource/licenses/gpl-2.txt.
 */

#include <string.h>

#include "ntfs_endian.h"
#include "ntfs_types.h"
#include "ntfs_upcase.h"

/**
 * ntfs_default_upcase_generate - generate the NTFS upcase table
 * @uc:		destination buffer in which to generate the upcase table
 * @uc_size:	size of the destination buffer in bytes
 *
 * Generate the full, 16-bit, little endian NTFS Unicode upcase table.
 *
 * @uc_size must be able to at least hold the full, 16-bit Unicode upcase
 * table, i.e. 2^16 * sizeof(ntfschar) = 64ki * 2 bytes = 128kiB.
 */
void ntfs_default_upcase_generate(ntfschar *uc, int uc_size)
{
	int i, r;
	static const int uc_run_table[][3] = { /* Start, End, Add */
	{0x0061, 0x007B,  -32}, {0x0451, 0x045D, -80}, {0x1F70, 0x1F72,  74},
	{0x00E0, 0x00F7,  -32}, {0x045E, 0x0460, -80}, {0x1F72, 0x1F76,  86},
	{0x00F8, 0x00FF,  -32}, {0x0561, 0x0587, -48}, {0x1F76, 0x1F78, 100},
	{0x0256, 0x0258, -205}, {0x1F00, 0x1F08,   8}, {0x1F78, 0x1F7A, 128},
	{0x028A, 0x028C, -217}, {0x1F10, 0x1F16,   8}, {0x1F7A, 0x1F7C, 112},
	{0x03AC, 0x03AD,  -38}, {0x1F20, 0x1F28,   8}, {0x1F7C, 0x1F7E, 126},
	{0x03AD, 0x03B0,  -37}, {0x1F30, 0x1F38,   8}, {0x1FB0, 0x1FB2,   8},
	{0x03B1, 0x03C2,  -32}, {0x1F40, 0x1F46,   8}, {0x1FD0, 0x1FD2,   8},
	{0x03C2, 0x03C3,  -31}, {0x1F51, 0x1F52,   8}, {0x1FE0, 0x1FE2,   8},
	{0x03C3, 0x03CC,  -32}, {0x1F53, 0x1F54,   8}, {0x1FE5, 0x1FE6,   7},
	{0x03CC, 0x03CD,  -64}, {0x1F55, 0x1F56,   8}, {0x2170, 0x2180, -16},
	{0x03CD, 0x03CF,  -63}, {0x1F57, 0x1F58,   8}, {0x24D0, 0x24EA, -26},
	{0x0430, 0x0450,  -32}, {0x1F60, 0x1F68,   8}, {0xFF41, 0xFF5B, -32},
	{0}
	};
	static const int uc_dup_table[][2] = { /* Start, End */
	{0x0100, 0x012F}, {0x01A0, 0x01A6}, {0x03E2, 0x03EF}, {0x04CB, 0x04CC},
	{0x0132, 0x0137}, {0x01B3, 0x01B7}, {0x0460, 0x0481}, {0x04D0, 0x04EB},
	{0x0139, 0x0149}, {0x01CD, 0x01DD}, {0x0490, 0x04BF}, {0x04EE, 0x04F5},
	{0x014A, 0x0178}, {0x01DE, 0x01EF}, {0x04BF, 0x04BF}, {0x04F8, 0x04F9},
	{0x0179, 0x017E}, {0x01F4, 0x01F5}, {0x04C1, 0x04C4}, {0x1E00, 0x1E95},
	{0x018B, 0x018B}, {0x01FA, 0x0218}, {0x04C7, 0x04C8}, {0x1EA0, 0x1EF9},
	{0}
	};
	static const int uc_word_table[][2] = { /* Offset, Value */
	{0x00FF, 0x0178}, {0x01AD, 0x01AC}, {0x01F3, 0x01F1}, {0x0269, 0x0196},
	{0x0183, 0x0182}, {0x01B0, 0x01AF}, {0x0253, 0x0181}, {0x026F, 0x019C},
	{0x0185, 0x0184}, {0x01B9, 0x01B8}, {0x0254, 0x0186}, {0x0272, 0x019D},
	{0x0188, 0x0187}, {0x01BD, 0x01BC}, {0x0259, 0x018F}, {0x0275, 0x019F},
	{0x018C, 0x018B}, {0x01C6, 0x01C4}, {0x025B, 0x0190}, {0x0283, 0x01A9},
	{0x0192, 0x0191}, {0x01C9, 0x01C7}, {0x0260, 0x0193}, {0x0288, 0x01AE},
	{0x0199, 0x0198}, {0x01CC, 0x01CA}, {0x0263, 0x0194}, {0x0292, 0x01B7},
	{0x01A8, 0x01A7}, {0x01DD, 0x018E}, {0x0268, 0x0197},
	{0}
	};

	bzero(uc, uc_size);
	uc_size /= sizeof(ntfschar);
	for (i = 0; i < uc_size; i++)
		uc[i] = cpu_to_le16(i);
	for (r = 0; uc_run_table[r][0]; r++)
		for (i = uc_run_table[r][0]; i < uc_run_table[r][1]; i++)
			uc[i] = cpu_to_le16((le16_to_cpu(uc[i]) +
					uc_run_table[r][2]));
	for (r = 0; uc_dup_table[r][0]; r++)
		for (i = uc_dup_table[r][0]; i < uc_dup_table[r][1]; i += 2)
			uc[i + 1] = cpu_to_le16(le16_to_cpu(uc[i + 1]) - 1);
	for (r = 0; uc_word_table[r][0]; r++)
		uc[uc_word_table[r][0]] = cpu_to_le16(uc_word_table[r][1]);
}
