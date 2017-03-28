#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "modes.h"
#include "e_types.h"
#include "e_address.h"

#define NUM_ECORES 16

// TODO: replace these with dynamic e_malloc() type of evaluations
#define MAX_NODES_ECORE 512
#define MAX_INEDGES_ECORE 512
#define MAX_OUTEDGES_ECORE 512

// return types from each eCore
#define E_CORE_INIT 0
#define E_CORE_RUNNING 1
#define E_CORE_COMPLETE 2

#endif