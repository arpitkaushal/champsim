// Jimenez's indirect predictor
// Registered paper #4 "SNIP: Scaled Neural Indirect Predictor"
// Authors: Daniel A. Jimenez (The University of Texas at San Antonio)

#include <stdio.h>
#include <cassert>
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

using namespace std;
#include "cbp3_def.h"
#include "cbp3_framework.h"

#define MAX_ROWS       	98
#define MAX_TARGETS     26
#define MAX_PERCS       1664
#define MAX_BIAS	19008
#define MAX_HL		42

// we keep a queue with in-flight branch state information in it.  the only
// state is the index in the path/pattern history arrays for this branch

class snip_update {
public:
	unsigned int ring_idx;
};

int queue_head = 0, queue_tail = 0;

// up to 128 branches might be in flight; we need to remember state for
// all of them; we keep one more to simplify maintenance of the queue

#define MAX_INFLIGHT_BRANCHES   129

snip_update
	inflight_branch_queue[MAX_INFLIGHT_BRANCHES];    // queue of update state entries

// insert a branch into the inflight branch state queue

snip_update *insert_branch (void) {
	snip_update *state = &inflight_branch_queue[queue_tail];
	queue_tail = (queue_tail + 1) % MAX_INFLIGHT_BRANCHES;
	return state;
}

// dequeue a branch

snip_update *remove_branch (void) {
	snip_update *r = &inflight_branch_queue[queue_head];
	queue_head = (queue_head + 1) % MAX_INFLIGHT_BRANCHES;
	return r;   
}

// a BTB entry, just the target, no tag

struct btb_entry {
	unsigned int target;
};

// class for indirect branch predictor

class snip_predictor {
private:

	// two hash functions by Thomas Wang, http://www.concentric.net/~ttwang/tech/inthash.htm
	// slightly modified by djimenez

	unsigned int hash32shift(unsigned int key) {
		key = ~key + (key << 15); 
		key = key ^ (key >> 12);
		key = key + (key << 2);
		key = key ^ (key >> 4);
		key = key * 34129;
		key = key ^ (key >> 16);
		return key;
	}

	unsigned int hash32shiftmult(unsigned int key) {
		key = (key ^ 61) ^ (key >> 16);
		key = key + (key << 3);
		key = key ^ (key >> 4);
		key = key * 1204868099;
		key = key ^ (key >> 15);
		return key;
	}

	unsigned int f1 (unsigned int x) { return hash32shiftmult (x); }

	unsigned int f2 (unsigned int x) { return hash32shift (x); }

	// general hash function that combines the two hash functions,
	// yielding a different hash function for each value of i

	unsigned int hash (unsigned int x, unsigned int i) {
		return (f1 (x) + i * f2 (x));
	}

	int 
		history_length,		// global history length
		theta,			// perceptron training threshold
		npercs,			// number of weights vectors
		nbias,			// number of bias weights
		nrows,			// number of rows in BTB
		assoc,			// associativity of BTB
		skip,			// bit pattern identifying bits in target to predict
		tc,			// counter for adaptive threshold training
		bpw,			// bits per weight
		max_weight, 		// maximum weight
		min_weight,		// minimum weight
		history_ring_size;	// size of circular history buffer

	// circular buffer of addresses and conditional branch history

	unsigned short int 
		address_ring[MAX_HL + MAX_INFLIGHT_BRANCHES];
	bool 
		history_ring[MAX_HL + MAX_INFLIGHT_BRANCHES];

	// branch target buffer

	btb_entry 
		targets[MAX_ROWS][MAX_TARGETS];

	// perceptron weights

	int bias_weights[MAX_BIAS];
	int weights[MAX_PERCS][MAX_HL+1];

	// index into circular buffer

	int spec_history_ring_idx;

	// coefficients vector

	double *coefficients;

	// factor to increase/decrease coefficients

	double factor;

public:

	// compute the size of the storage for a predictor

	static int size (int be, int np, int nr, int h, int a, int bpw) {
		int bits = 
			be * bpw		// size of bias weights
			+ np * bpw * h		// size of the table
			+ nr * a * 32  		// size of the BTB
			+ (h + MAX_INFLIGHT_BRANCHES) * (1 + 11) 	// size of pattern and path histories
			+ 12			// size of theta
			;
		bits += (h + MAX_INFLIGHT_BRANCHES) * 9;		// ring idx
		bits += (h + 1) * sizeof (double) * 8;
		return bits;
	}

	int size (void) {
		return size (nbias, npercs, nrows, history_length, assoc, bpw);
	}

	// put conditional branch outcome and 11 bits of address into
	// circular history buffer

	void shift_history_ring (int *ring_idx, bool t, unsigned int address) {
		if (*ring_idx > 0)
			(*ring_idx)--;
		else
			*ring_idx = history_ring_size - 1;
		history_ring[*ring_idx] = t ^ !!(address & 16);
		address_ring[*ring_idx] = (unsigned short int) address & 2047;
	}

	// return i'th branch outcome in history

	bool index_history (int ring_idx, int i) {
		int idx = ring_idx + i;
		bool bit = history_ring[idx % history_ring_size];
		return bit;
	}

	// return i'th branch address lower bits in history

	unsigned int index_address (int ring_idx, int i) {
		int idx = ring_idx + i;
		return address_ring[idx % history_ring_size];
	}

	// constructor

	snip_predictor (
		int be = 19008,		// number of bias weights
		int np = 1664, 		// number of weights vectors
		int nr = 98,		// number of rows in BTB
		int h = 42,		// history length
		int a = 26, 		// associativity of BTB
		int t = 240, 		// initial value of theta
		unsigned int sk = 0xb5ffa, // bits of target to predict
		int bp = 5,		// bits per weight
		double fa = 1.0000045500) { // factor to increase/decrease coefficients

		// make initial coefficients array

		double m = 4.3;
		double A = 0.059; 
		double B = 0.006;
		double *sp = new double[h+1];
		for (int i=0; i<=h; i++) {
			sp[i] = 1.0 / (A + B * i);
			if (sp[i] <= m) sp[i] = m;
		}
		nbias = be;
		factor = fa;
		coefficients = sp;
		bpw = bp;
		max_weight = (1<<bpw)/2-1;
		min_weight = - max_weight - 1;
		skip = sk;
		assoc = a;
		npercs = np;
		nrows = nr;
		theta = t;
		tc = 0;
		history_length = h;
		memset (bias_weights, 0, sizeof (bias_weights));
		memset (weights, 0, sizeof (weights));

		// initializing targets to 0xffffffff helps because initially the
		// perceptrons are biased to predict all-bits-0; by making invalid
		// target not resemble all-bits-0 we get better accuracy without having
		// to store valid bits in the BTB

		memset (targets, 0xff, sizeof (targets));
		memset (address_ring, 0, sizeof (address_ring));
		memset (history_ring, 0, sizeof (history_ring));
		spec_history_ring_idx = 0;
		history_ring_size = history_length + MAX_INFLIGHT_BRANCHES;
	}

	// compute perceptron output for bit w of a target prediction

	double compute_output (unsigned int pc, unsigned int ring_idx, int w) {

		// get the w'th hash of the pc

		unsigned int ha = hash (pc, w);

		// multiply bias weight by first coefficient

		double sum = bias_weights[ha % nbias] * coefficients[0];

		// find sum for rest of history weights

		for (int i=0; i<history_length; i++) {

			// find i'th bit in the history of conditional branch outcomes

			int bit = index_history (ring_idx, i);

			// compute index into weights array

			int idx = (ha ^ index_address (ring_idx, i)) % npercs;

			// convert history to bimodal

			int h = bit ? 1 : -1;

			// i'th partial sum is product of coeffient, history bit, and weight

			sum += coefficients[i+1] * h * weights[idx][i];
		}
		return sum;
	}

	int absolute_value (int x) {
		if (x < 0) return -x;
		return x;
	}

	// train perceptron to predict address_bit for bit w of a target prediction

	void train (int pc, unsigned int ring_idx, bool address_bit, int w) {

		// compute perceptron output

		double yout = compute_output (pc, ring_idx, w);
		unsigned int h = hash (pc, w);
		int a = absolute_value((int)yout);

		// figure out if prediction of this bit was correct

		bool correct = (yout >= 0) == address_bit;

		// adaptive training of threshold a la Seznec's O-GEHL

		if (!correct) {
			tc++;
			if (tc >= 1) {
				if (theta < 4095) theta++;
				tc = 0;
			}
		}
		if (correct && a < theta) {
			tc--;
			if (tc <= -1) {
				if (theta > 0) theta--;
				tc = 0;
			}
		}

		// if the branch is correct and the magnitude of the output
		// doesn't exceed theta, don't need to train

		if (correct && a >= theta) return;

		// train bias weight and bias coefficient

		int *c;
		c = &bias_weights[h%nbias];
		if ((*c >= 0) == address_bit) {
			coefficients[0] *= factor;
		} else {
			coefficients[0] /= factor;
		}
		if (address_bit) {
			if (*c < max_weight) (*c)++;
		} else {
			if (*c > min_weight) (*c)--;
		}

		// train history correlating weights and coefficients

		for (int i=0; i<history_length; i++) {
			int idx = (h ^ index_address (ring_idx, i)) % npercs;
			c = &weights[idx][i];
			int bit = index_history (ring_idx, i);
			if (bit == address_bit) {
				if (*c < max_weight) (*c)++;
			} else {
				if (*c > min_weight) (*c)--;
			}
			int h = bit ? 1 : -1;
			int sum = h * weights[idx][i];
			if ((sum >= 0) == address_bit) {
				coefficients[i+1] *= factor;
			} else {
				coefficients[i+1] /= factor;
			}
		}
	}

	// compute hamming distance between two unsigned ints

	int hamming_distance (unsigned int x, unsigned int y) {
		int d = 0;
		for (int i=0; i<32; i++) if ( (1<<i) & skip) {
			unsigned int mask = 1<<i;
			if ((x & mask) != (y & mask)) d++;
		}
		return d;
	}

	// predict a target

	unsigned int predict (unsigned int pc, snip_update *u) {
		unsigned int x = 0;
		int i;

		// remember this position in the history array for update

		u->ring_idx = spec_history_ring_idx;

		// predict several bits of the target address 

		for (i=0; i<32; i++) if ( (1<<i) & skip) {
			double yout = compute_output (pc, u->ring_idx, i);
			if (yout >= 0) x |= (1<<i);
		}

		// look for the matching btb entry with lowest hamming
		// distance from the predicted bits

		unsigned int idx = pc % nrows;
		int mini = 0;
		for (i=1; i<assoc; i++) {
			int d0 = hamming_distance (x, targets[idx][i].target);
			int d1 = hamming_distance (x, targets[idx][mini].target);
			if (d0 < d1) mini = i;
		}

		// return that target

		return targets[idx][mini].target;
	}

	// udpate predictor

	void update (unsigned int pc, snip_update *u, unsigned int target) {

		// look for the target 

		unsigned int idx = pc % nrows;
		int i;
		for (i=0; i<assoc; i++) if (targets[idx][i].target == target) break;

		// if not found, choose the lru slot

		if (i == assoc) {
			i = assoc-1;
			targets[idx][i].target = target;
		}

		// move the target to the MRU position

		btb_entry t = targets[idx][i];
		for (int w=i-1; w>=0; w--) targets[idx][w+1] = targets[idx][w];
		targets[idx][0] = t;

		// train the predictor on several bits of the target address

		for (int i=0; i<32; i++) if ( (1<<i) & skip) {
			bool target_bit = !!((1<<i) & (target));
			train (pc, u->ring_idx, target_bit, i);
		}
	}

	// record a conditional branch address and outcome in the circular
	// history buffer

	void update_conditional (unsigned int pc, bool taken) {
		shift_history_ring (&spec_history_ring_idx, taken, pc);
	}

};

// pointer for predictor object

snip_predictor *pred;

// identify branch instructions

bool is_branch (uint16_t type) {
	if (type & IS_BR_CONDITIONAL) return true;
	if (type & IS_BR_INDIRECT) return true;
	if (type & IS_BR_CALL) return true;
	if (type & IS_BR_RETURN) return true;
	if (type & IS_BR_OTHER) return true;
	return false;
}

void PredictorInit() {
	// initialize inflight branch queue

	memset (inflight_branch_queue, 0, sizeof (inflight_branch_queue));

	// make a new indirect branch predictor

	pred = new snip_predictor ();

	// print out the size

	printf ("size is %d\n", pred->size() / 8);
}

void PredictorReset() { }

void PredictorRunACycle() {
	// get info about what uops are processed at each pipeline stage
	const cbp3_cycle_activity_t *cycle_info = get_cycle_info();

	// make prediction at fetch stage

	for (int i = 0; i < cycle_info->num_fetch; i++) {
		uint32_t fe_ptr = cycle_info->fetch_q[i];
		const cbp3_uop_dynamic_t *uop = &fetch_entry(fe_ptr)->uop;
		if (uop->type & IS_BR_INDIRECT) {
			// make a prediction with no knowledge of the actual outcome...

			snip_update *state = insert_branch();
			unsigned int predicted_target = pred->predict (uop->pc, state);

			// report the prediction

			assert(report_pred(fe_ptr, false, predicted_target));
		} else if (is_branch (uop->type)) {

			// "speculatively" update the branch history

			pred->update_conditional (uop->pc, uop->br_taken);
		}
	}

	// update at retire stage

	for (int i = 0; i < cycle_info->num_retire; i++) {
		uint32_t rob_ptr = cycle_info->retire_q[i];
		const cbp3_uop_dynamic_t *uop = &rob_entry(rob_ptr)->uop;
		if (uop->type & IS_BR_INDIRECT) {
			snip_update *state = remove_branch();
			pred->update (uop->pc, state, uop->br_target);
		}
	}
}

void PredictorRunEnd() {
	rewind_marked = false;
}

void PredictorExit() { }
