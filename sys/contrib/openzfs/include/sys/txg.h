// SPDX-License-Identifier: CDDL-1.0
/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or https://opensource.org/licenses/CDDL-1.0.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2010 Sun Microsystems, Inc.  All rights reserved.
 * Use is subject to license terms.
 */
/*
 * Copyright (c) 2012, 2017 by Delphix. All rights reserved.
 * Copyright (c) 2025, Klara, Inc.
 */

#ifndef _SYS_TXG_H
#define	_SYS_TXG_H

#include <sys/spa.h>
#include <sys/zfs_context.h>

#ifdef	__cplusplus
extern "C" {
#endif

#define	TXG_CONCURRENT_STATES	3	/* open, quiescing, syncing	*/
#define	TXG_SIZE		4		/* next power of 2	*/
#define	TXG_MASK		(TXG_SIZE - 1)	/* mask for size	*/
#define	TXG_INITIAL		TXG_SIZE	/* initial txg 		*/
#define	TXG_IDX			(txg & TXG_MASK)
#define	TXG_UNKNOWN		0

/* Number of txgs worth of frees we defer adding to in-core spacemaps */
#define	TXG_DEFER_SIZE		2

typedef struct tx_cpu tx_cpu_t;

typedef struct txg_handle {
	tx_cpu_t	*th_cpu;
	uint64_t	th_txg;
} txg_handle_t;

typedef struct txg_node {
	struct txg_node	*tn_next[TXG_SIZE];
	uint8_t		tn_member[TXG_SIZE];
} txg_node_t;

typedef struct txg_list {
	kmutex_t	tl_lock;
	size_t		tl_offset;
	spa_t		*tl_spa;
	txg_node_t	*tl_head[TXG_SIZE];
} txg_list_t;

/*
 * Wait flags for txg_wait_synced_flags(). By default (TXG_WAIT_NONE), it will
 * wait until the wanted txg is reached, or block forever. Additional flags
 * indicate other conditions that the caller is interested in, that will cause
 * the wait to break and return an error code describing the condition.
 */
typedef enum {
	/* No special flags. Guaranteed to block forever or return 0 */
	TXG_WAIT_NONE   = 0,

	/* If a signal arrives while waiting, abort and return EINTR */
	TXG_WAIT_SIGNAL = (1 << 0),

	/* If the pool suspends while waiting, abort and return ESHUTDOWN. */
	TXG_WAIT_SUSPEND = (1 << 1),
} txg_wait_flag_t;

struct dsl_pool;

extern void txg_init(struct dsl_pool *dp, uint64_t txg);
extern void txg_fini(struct dsl_pool *dp);
extern void txg_sync_start(struct dsl_pool *dp);
extern void txg_sync_stop(struct dsl_pool *dp);
extern uint64_t txg_hold_open(struct dsl_pool *dp, txg_handle_t *txghp);
extern void txg_rele_to_quiesce(txg_handle_t *txghp);
extern void txg_rele_to_sync(txg_handle_t *txghp);
extern void txg_register_callbacks(txg_handle_t *txghp, list_t *tx_callbacks);

extern void txg_delay(struct dsl_pool *dp, uint64_t txg, hrtime_t delta,
    hrtime_t resolution);
extern void txg_kick(struct dsl_pool *dp, uint64_t txg);

/*
 * Wait until the given transaction group has finished syncing.
 * Try to make this happen as soon as possible (eg. kick off any
 * necessary syncs immediately).  If txg==0, wait for the currently open
 * txg to finish syncing.
 * See txg_wait_flag_t above for a description of how the flags affect the wait.
 */
extern int txg_wait_synced_flags(struct dsl_pool *dp, uint64_t txg,
    txg_wait_flag_t flags);

/*
 * Traditional form of txg_wait_synced_flags, waits forever.
 * Shorthand for VERIFY0(txg_wait_synced_flags(dp, TXG_WAIT_NONE))
 */
extern void txg_wait_synced(struct dsl_pool *dp, uint64_t txg);

/*
 * Wake all threads waiting in txg_wait_synced_flags() so they can reevaluate.
 */
extern void txg_wait_kick(struct dsl_pool *dp);

/*
 * Wait until the given transaction group, or one after it, is
 * the open transaction group.  Try to make this happen as soon
 * as possible (eg. kick off any necessary syncs immediately) when
 * should_quiesce is set.  If txg == 0, wait for the next open txg.
 */
extern void txg_wait_open(struct dsl_pool *dp, uint64_t txg,
    boolean_t should_quiesce);

/*
 * Returns TRUE if we are "backed up" waiting for the syncing
 * transaction to complete; otherwise returns FALSE.
 */
extern boolean_t txg_stalled(struct dsl_pool *dp);

/* returns TRUE if someone is waiting for the next txg to sync */
extern boolean_t txg_sync_waiting(struct dsl_pool *dp);

extern void txg_verify(spa_t *spa, uint64_t txg);

/*
 * Wait for pending commit callbacks of already-synced transactions to finish
 * processing.
 */
extern void txg_wait_callbacks(struct dsl_pool *dp);

/*
 * Per-txg object lists.
 */

#define	TXG_CLEAN(txg)	((txg) - 1)

extern void txg_list_create(txg_list_t *tl, spa_t *spa, size_t offset);
extern void txg_list_destroy(txg_list_t *tl);
extern boolean_t txg_list_empty(txg_list_t *tl, uint64_t txg);
extern boolean_t txg_all_lists_empty(txg_list_t *tl);
extern boolean_t txg_list_add(txg_list_t *tl, void *p, uint64_t txg);
extern boolean_t txg_list_add_tail(txg_list_t *tl, void *p, uint64_t txg);
extern void *txg_list_remove(txg_list_t *tl, uint64_t txg);
extern void *txg_list_remove_this(txg_list_t *tl, void *p, uint64_t txg);
extern boolean_t txg_list_member(txg_list_t *tl, void *p, uint64_t txg);
extern void *txg_list_head(txg_list_t *tl, uint64_t txg);
extern void *txg_list_next(txg_list_t *tl, void *p, uint64_t txg);

/* Global tuning */
extern uint_t zfs_txg_timeout;


#ifdef ZFS_DEBUG
#define	TXG_VERIFY(spa, txg)		txg_verify(spa, txg)
#else
#define	TXG_VERIFY(spa, txg)
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* _SYS_TXG_H */
