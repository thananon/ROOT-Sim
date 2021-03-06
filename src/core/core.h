/**
*			Copyright (C) 2008-2015 HPDCS Group
*			http://www.dis.uniroma1.it/~hpdcs
*
*
* This file is part of ROOT-Sim (ROme OpTimistic Simulator).
*
* ROOT-Sim is free software; you can redistribute it and/or modify it under the
* terms of the GNU General Public License as published by the Free Software
* Foundation; either version 3 of the License, or (at your option) any later
* version.
*
* ROOT-Sim is distributed in the hope that it will be useful, but WITHOUT ANY
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
* A PARTICULAR PURPOSE. See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License along with
* ROOT-Sim; if not, write to the Free Software Foundation, Inc.,
* 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*
* @file core.h
* @brief This header defines all the shared symbols which are needed by different subsystems
* @author Francesco Quaglia
* @author Roberto Vitali
* @author Alessandro Pellegrini
*
*/


#pragma once
#ifndef _PLATFORM_H_
#define _PLATFORM_H_

#include <ROOT-Sim.h>
#include <stdio.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include <stdint.h>

#include <lib/numerical.h>
#include <arch/thread.h>
#include <statistics/statistics.h>

/// If set, ROOT-Sim will produce statistics on the execution
#define PLATFORM_STATS

/// This macro expands to true if the local kernel is the master kernel
#define master_kernel() (kid == 0)


// XXX: This should be moved to state or queues
#define INVALID_SNAPSHOT	2000
#define FULL_SNAPSHOT		2001

/// This defines an idle process (i.e., the fake process to be executed when no events are available)
#define IDLE_PROCESS	UINT_MAX


#define OUTPUT_DIR		"outputs"
#define STAT_FILE		"execution_stats"
#define LOCAL_STAT_FILE		"local_stats"


/// Maximum number of kernels the distributed simulator can handle
#define N_KER_MAX	128

/// Maximum number of LPs the simulator will handle
#define MAX_LPs		8192		// This is 2^20

/// Maximum event size (in bytes)
#define MAX_EVENT_SIZE	1120
//#define MAX_EVENT_SIZE	(150 * sizeof(int))
//#define MAX_EVENT_SIZE	1500

// XXX: this should be moved somewhere else...
#define VERBOSE_INFO	1700
#define VERBOSE_DEBUG	1701
#define VERBOSE_NO	1702


// XXX Do we still use transient time?
/// Transient duration (in msec)
#define STARTUP_TIME	0

/// Distribute exceeding LPs according to a block policy
#define LP_DISTRIBUTION_BLOCK 0
/// Distribute exceeding LPs according to a circular policy
#define LP_DISTRIBUTION_CIRCULAR 1


// XXX should be moved to a more librarish header
/// Equality condition for floats
#define F_EQUAL(a,b) (fabs((a) - (b)) < FLT_EPSILON)
/// Equality to zero condition for floats
#define F_EQUAL_ZERO(a) (fabs(a) < FLT_EPSILON)
/// Difference condition for floats
#define F_DIFFER(a,b) (fabs((a) - (b)) >= FLT_EPSILON)
/// Difference from zero condition for floats
#define F_DIFFER_ZERO(a) (fabs(a) >= FLT_EPSILON)


/// Equality condition for doubles
#define D_EQUAL(a,b) (fabs((a) - (b)) < DBL_EPSILON)
/// Equality to zero condition for doubles
#define D_EQUAL_ZERO(a) (fabs(a) < DBL_EPSILON)
/// Difference condition for doubles
#define D_DIFFER(a,b) (fabs((a) - (b)) >= DBL_EPSILON)
/// Difference from zero condition for doubles
#define D_DIFFER_ZERO(a) (fabs(a) >= DBL_EPSILON)


/// Macro to find the maximum among two values
#ifdef max
#undef max
#endif
#define max(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

/// Macro to find the minimum among two values
#ifdef min
#undef min
#endif
#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


// to avoid a circular inclusion, but I hate this! :(
struct _state_t;

typedef enum {positive, negative, other} message_kind_t;


/// Message Type definition
typedef struct _msg_t {
	// Kernel's information
	unsigned int   		sender;
	unsigned int   		receiver;
	int   			type;
	simtime_t		timestamp;
	simtime_t		send_time;
	// TODO: risistemare questa cosa degli antimessaggi
	message_kind_t		message_kind;
	unsigned long long	mark;	/// Unique identifier of the message, used for antimessages
	unsigned long long	rendezvous_mark;	/// Unique identifier of the message, used for rendez-vous events
//	struct _state_t 	*is_first_event_of;
	// Application informations
	char event_content[MAX_EVENT_SIZE];
	int size;
} msg_t;


/// Message envelope definition. This is used to handle the output queue and stores information needed to generate antimessages
typedef struct _msg_hdr_t {
	// Kernel's information
	unsigned int   		sender;
	unsigned int   		receiver;
	simtime_t		timestamp;
	simtime_t		send_time;
	unsigned long long	mark;
} msg_hdr_t;



/// Configuration of the execution of the simulator
typedef struct _simulation_configuration {
	char *output_dir;		/// Destination Directory of output files
	int backtrace;			/// Debug mode flag
	int scheduler;			/// Which scheduler to be used
	int gvt_time_period;		/// Wall-Clock time to wait before executiong GVT operations
	int gvt_snapshot_cycles;	/// GVT operations to be executed before rebuilding the state
	int simulation_time;		/// Wall-clock-time based termination predicate
	int lps_distribution;		/// Policy for the LP to Kernel mapping
	int ckpt_mode;			/// Type of checkpointing mode (Synchronous, Semi-Asyncronous, ...)
	int checkpointing;		/// Type of checkpointing scheme (e.g., PSS, CSS, ...)
	int ckpt_period;		/// Number of events to execute before taking a snapshot in PSS (ignored otherwise)
	int snapshot;			/// Type of snapshot (e.g., full, incremental, autonomic, ...)
	int check_termination_mode;	/// Check termination strategy: standard or incremental
	bool blocking_gvt;		/// GVT protocol blocking or not
	bool deterministic_seed;	/// Does not change the seed value config file that will be read during the next runs
	int verbose;			/// Kernel verbose
	enum stat_levels stats;		/// Produce performance statistic file (default STATS_ALL)
	bool serial;			// If the simulation must be run serially
	seed_type set_seed;		/// The master seed to be used in this run
} simulation_configuration;


/// Barrier for all worker threads
extern barrier_t all_thread_barrier;

// XXX: this should be refactored someway
extern unsigned int	kid,		/* Kernel ID for the local kernel */
			n_ker,		/* Total number of kernel instances */
			n_cores,	/* Total number of cores required for simulation */
			n_prc,		/* Number of LPs hosted by the current kernel instance */
			*kernel;


extern bool mpi_is_initialized;

extern simulation_configuration rootsim_config;

extern void ProcessEvent_light(unsigned int me, simtime_t now, int event_type, void *event_content, unsigned int size, void *state);
bool OnGVT_light(int gid, void *snapshot);
extern void ProcessEvent_inc(unsigned int me, simtime_t now, int event_type, void *event_content, unsigned int size, void *state);
bool OnGVT_inc(int gid, void *snapshot);
extern bool (**OnGVT)(int gid, void *snapshot);
extern void (**ProcessEvent)(unsigned int me, simtime_t now, int event_type, void *event_content, unsigned int size, void *state);

extern void base_init(void);
extern void base_fini(void);
extern unsigned int LidToGid(unsigned int lid);
extern unsigned int GidToLid(unsigned int gid);
extern unsigned int GidToKernel(unsigned int gid);
extern void rootsim_error(bool fatal, const char *msg, ...);
extern void distribute_lps_on_kernels(void);
extern void simulation_shutdown(int code) __attribute__((noreturn));
extern inline bool simulation_error(void);
extern void initialization_complete(void);

#endif

