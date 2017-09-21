//////////////////////////////////////////////////////////////////////////
//
// Bigbite lib replay script definition header
//
// https://hallaweb.jlab.org/wiki/index.php/ReplayCore
//
// It is suggested to use this file set replay environmental variable for 
// different machine. So, they can use same replay script
//	
//////////////////////////////////////////////////////////////////////////
//	
//	Author : Jin Huang (jinhuang@jlab.org)    Aug 2007
//	Modify History:
//
//      27 Mar 08 ole  Change ROOT file names for DetL and PhysL replays
//                     from _det_nnnn to _det_L_nnnn etc.
//
//////////////////////////////////////////////////////////////////////////
//
//  To Do:
//  - Raw File Format - change to tritium
//  - File path names
//
//////////////////////////////////////////////////////////////////////////

#ifndef ROOT_REPLAY_DEF
#define ROOT_REPLAY_DEF


//////////////////////////////////////////////////////////////////
// extra structures
//////////////////////////////////////////////////////////////////

#include "TObject.h" //trick to avoid error: `Int_t' does not name a type
#include "TString.h"

//Used for setting paths. %s is necessary so that it can be used to Form the paths.
//This allows changing the directory in fewer places for ease of portability.
const char* REPLAY_DIR_PREFIX = "/home/jbane/tritium/triton/HallA-Online-Tritium/replay/%s";

typedef struct _sReplaySetUp
{
    Int_t DefReplayNum; //defaut replay event num
    char* OutFileFormat;//output file format
    char* OutDefineFile;//out define
    char* CutDefineFile;//cut define
}sReplaySetUp;


//////////////////////////////////////////////////////////////////
// common defines
//////////////////////////////////////////////////////////////////

static const char* PATHS[] = {
  "/home/jbane/raw",
  "/adaq1/data1",
  "/adaql2/data1",
  "/adaql2/data2",
  "/adaql2/data3",
  "/adaql2/data4",
  "/adaql1/data1",
  "/adaql1/data2",
  "/adaql1/data3",
  "/adaql1/data4",
  "/chafs2/work1/Ar40/raw",
   0
};

static const char* RAW_DATA_FORMAT="%s/gmp_%d.dat.%d";
//static const char* RAW_DATA_FORMAT="%s/dvcs14_%d.dat.%d";

static const char* STD_REPLAY_OUTPUT_DIR=Form(REPLAY_DIR_PREFIX,"Rootfiles");
static const char* CUSTOM_REPLAY_OUTPUT_DIR=Form(REPLAY_DIR_PREFIX,"ScratchROOTfiles");

static const char* SUMMARY_PHYSICS_FORMAT=Form(REPLAY_DIR_PREFIX,"summaryfiles/summaryphy_%d.log");

//not used since Nov 15, 2008
static const Int_t ANA_MARK_INTERVAL=1000;

// #include "ReplayCore.C"  

// DO NOT DELETE!
#endif//#define ROOT_REPLAY_DEF
