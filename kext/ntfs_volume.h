/*
 * ntfs_volume.h - Defines for volume structures in the NTFS kernel driver.
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

#ifndef _OSX_NTFS_VOLUME_H
#define _OSX_NTFS_VOLUME_H

#include <sys/mount.h>
#include <sys/types.h>

#include <libkern/OSAtomic.h>

#include <kern/locks.h>

/* Forward declaration. */
typedef struct _ntfs_volume ntfs_volume;

#include "ntfs_inode.h"
#include "ntfs_layout.h"
#include "ntfs_types.h"

/*
 * The NTFS in-memory mount point structure.
 */
struct _ntfs_volume {
	/* Device specifics. */
	mount_t mp;			/* Pointer back to the vfs mount. */
	dev_t dev;			/* The device number of the volume. */
	vnode_t dev_vn;			/* The device vnode of the volume. */
	LCN nr_blocks;			/* Number of NTFS_BLOCK_SIZE bytes
					   sized blocks on the device. */
	/* Configuration provided by user at mount time. */
	u32 flags;			/* Miscellaneous flags, see below. */
	uid_t uid;			/* uid that files will be mounted as. */
	gid_t gid;			/* gid that files will be mounted as. */
	mode_t fmask;			/* The mask for file permissions. */
	mode_t dmask;			/* The mask for directory
					   permissions. */
	u8 mft_zone_multiplier;		/* Initial mft zone multiplier. */
	u8 on_errors;			/* What to do on filesystem errors. */
	/* NTFS bootsector provided information. */
	u32 sector_size;		/* in bytes */
	u32 sector_size_mask;		/* sector_size - 1 */
	u8 sector_size_shift;		/* log2(sector_size) */
	u32 cluster_size;		/* in bytes */
	u32 cluster_size_mask;		/* cluster_size - 1 */
	u8 cluster_size_shift;		/* log2(cluster_size) */
	u32 mft_record_size;		/* in bytes */
	u32 mft_record_size_mask;	/* mft_record_size - 1 */
	u8 mft_record_size_shift;	/* log2(mft_record_size) */
	u32 index_record_size;		/* in bytes */
	u32 index_record_size_mask;	/* index_record_size - 1 */
	u8 index_record_size_shift;	/* log2(index_record_size) */
	LCN mft_lcn;			/* Cluster location of mft data. */
	LCN mftmirr_lcn;		/* Cluster location of copy of mft. */
	u64 serial_no;			/* The volume serial number. */
	/* Mount specific NTFS information. */
	u32 upcase_len;			/* Number of entries in upcase[]. */
	ntfschar *upcase;		/* The upcase table. */

	s32 attrdef_size;		/* Size of the attribute definition
					   table in bytes. */
	ATTR_DEF *attrdef;		/* Table of attribute definitions.
					   Obtained from FILE_AttrDef. */

	char *name;			/* Volume name (decomposed, UTF-8). */
	unsigned name_size;		/* Size in bytes of volume name buffer,
					   i.e. size of string in bytes plus
					   the NUL terminator. */

	/* Variables used by the cluster and mft allocators. */
	s64 mft_data_pos;		/* Mft record number at which to
					   allocate the next mft record. */
	LCN mft_zone_start;		/* First cluster of the mft zone. */
	LCN mft_zone_end;		/* First cluster beyond the mft zone. */
	LCN mft_zone_pos;		/* Current position in the mft zone. */
	LCN data1_zone_pos;		/* Current position in the first data
					   zone. */
	LCN data2_zone_pos;		/* Current position in the second data
					   zone. */

	/* Variables relating to the system files. */
	ntfs_inode *mft_ni;		/* The ntfs inode of $MFT. */

	ntfs_inode *mftbmp_ni;		/* Attribute ntfs inode for
					   $MFT/$BITMAP. */
	lck_rw_t mftbmp_lock;		/* Lock for serializing accesses to the
					   mft record bitmap ($MFT/$BITMAP) as
					   well as to @nr_mft_records and
					   @nr_free_mft_records. */
	s64 nr_mft_records;		/* Number of mft records on volume ==
					   number of bits in mft bitmap. */
	s64 nr_free_mft_records;	/* Number of free mft records on volume
					   == number of zero bits in mft
					   bitmap. */

	ntfs_inode *mftmirr_ni;		/* The ntfs inode of $MFTMirr. */
	int mftmirr_size;		/* Size of mft mirror in mft records. */

	ntfs_inode *logfile_ni;		/* The ntfs inode of $LogFile. */

	ntfs_inode *lcnbmp_ni;		/* The ntfs inode of $Bitmap. */
	lck_rw_t lcnbmp_lock;		/* Lock for serializing accesses to the
					   cluster bitmap ($Bitmap/$DATA) as
					   well as to @nr_clusters and
					   @nr_free_clusters. */
	LCN nr_clusters;		/* Volume size in clusters == number of
					   bits in lcn bitmap. */
	LCN nr_free_clusters;		/* Number of free clusters on volume ==
					   number of zero bits in lcn bitmap. */

	ntfs_inode *vol_ni;		/* The ntfs inode of $Volume. */
	VOLUME_FLAGS vol_flags;		/* Volume flags. */
	u8 major_ver;			/* Ntfs major version of volume. */
	u8 minor_ver;			/* Ntfs minor version of volume. */

	ntfs_inode *root_ni;		/* The ntfs inode of the root
					   directory. */

	ntfs_inode *secure_ni;		/* The ntfs inode of $Secure
					   (NTFS3.0+ only, otherwise NULL). */
	ntfs_inode *extend_ni;		/* The ntfs inode of $Extend
					   (NTFS3.0+ only, otherwise NULL). */
	/* $Quota stuff is NTFS3.0+ specific.  Unused/NULL otherwise. */
	ntfs_inode *quota_ni;		/* The ntfs inode of $Quota. */
	ntfs_inode *quota_q_ni;		/* Attribute ntfs inode for
					   $Quota/$Q. */
	/* $UsnJrnl stuff is NTFS3.0+ specific.  Unused/NULL otherwise. */
	ntfs_inode *usnjrnl_ni;		/* The ntfs inode of $UsnJrnl. */
	ntfs_inode *usnjrnl_max_ni;	/* Attribute ntfs inode for
					   $UsnJrnl/$Max. */
	ntfs_inode *usnjrnl_j_ni;	/* Attribute ntfs inode for
					   $UsnJrnl/$J. */
};

/*
 * Defined bits for the flags field in the ntfs_volume structure.
 */
enum {
	NV_Errors,		/* 1: Volume has errors, prevent remount rw. */
	NV_ShowSystemFiles,	/* 1: Return system files in ntfs_readdir(). */
	NV_CaseSensitive,	/* 1: Treat filenames as case sensitive and
				      create filenames in the POSIX namespace.
				      Otherwise be case insensitive (but still
				      create filenames in POSIX namespace). */
	NV_LogFileEmpty,	/* 1: $LogFile journal is empty. */
	NV_QuotaOutOfDate,	/* 1: $Quota is out of date. */
	NV_UsnJrnlStamped,	/* 1: $UsnJrnl has been stamped. */
	NV_SparseEnabled,	/* 1: May create sparse files. */
	NV_CompressionEnabled,	/* 1: Compression is enabled on the volume. */
	NV_ReadOnly,		/* 1: Volume is mounted read-only. */
};

/*
 * Macro to expand the NVolFoo(), NVolSetFoo(), and NVolClearFoo() functions.
 * Note, these functions are atomic but do not necessarily provide any ordering
 * guarantees.  This should be fine as they are always used with a lock held
 * and/or in places where the ordering does not matter.
 */
#define DEFINE_NVOL_BIT_OPS(flag)					\
static inline u32 NVol##flag(ntfs_volume *vol)				\
{									\
	return (vol->flags >> NV_##flag) & 1;				\
}									\
static inline void NVolSet##flag(ntfs_volume *vol)			\
{									\
	(void)OSBitOrAtomic((u32)1 << NV_##flag, (UInt32*)&vol->flags);	\
}									\
static inline void NVolClear##flag(ntfs_volume *vol)			\
{									\
	(void)OSBitAndAtomic(~((u32)1 << NV_##flag), (UInt32*)&vol->flags); \
}

/* Define the ntfs volume bitops functions. */
DEFINE_NVOL_BIT_OPS(Errors)
DEFINE_NVOL_BIT_OPS(ShowSystemFiles)
DEFINE_NVOL_BIT_OPS(CaseSensitive)
DEFINE_NVOL_BIT_OPS(LogFileEmpty)
DEFINE_NVOL_BIT_OPS(QuotaOutOfDate)
DEFINE_NVOL_BIT_OPS(UsnJrnlStamped)
DEFINE_NVOL_BIT_OPS(SparseEnabled)
DEFINE_NVOL_BIT_OPS(CompressionEnabled)
DEFINE_NVOL_BIT_OPS(ReadOnly)

#endif /* !_OSX_NTFS_VOLUME_H */
