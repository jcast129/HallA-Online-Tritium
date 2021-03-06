/*
* 
 * Tritium Replay Script
 * Works for non-coincidence experiments using 2 daqs
 */

///////////////////////////////////// To Do ////////////////////////////////////
/*
 * - Correctly set run number ranges for each arm
 * - Automatically open Online Plots GUI for shift takers?
 * - Make work for coincidence as well? Or use separate script?
 * - Do we use GMP S0 Class or stock? GMP class needs to be updated to be 1.6 compatible
 * - Are we using energy loss classes? Need to be made 1.6 compatible
 * - Scaler code is not 1.6 correct. Needs a new DB file before correct code would work.
 */

#include "/home/jbane/tritium/triton/HallA-Online-Tritium/replay/def_tritium.h"
using namespace std;

#define RIGHT_ARM_CONDITION nrun>=20000
#define LEFT_ARM_CONDITION nrun<20000

void tri_replay_right(Int_t runnumber=0,Int_t all=50000,Int_t fstEvt=0,Bool_t QuietRun = kFALSE){

  char buf[300];
  Int_t nrun=0;
  if (runnumber<=0)
  {
	    cout << "\nreplay: Please enter a Run Number (-1 to exit):";
	    cin >> nrun;
	    fgets(buf,300,stdin);//get the extra '\n' from stdin
	    if( nrun<=0 ){ return;}
	    runnumber = nrun;
  }

  
  //Enable modules
  Bool_t bScaler=kTRUE;
  Bool_t bHelicity=kFALSE;
  Bool_t bPhysics=kTRUE;
  Bool_t bPlots=kFALSE; //not open GUI automatically
  Bool_t bEloss=kFALSE;
 
 // char* def_dir="/home/jbane/tritium/triton/HallA-Online-Tritium/replay/";
  char* RNAME="%s/tritium_%d.root";

	char* ODEF="/home/jbane/tritium/triton/HallA-Online-Tritium/replay/RHRS.odef";
	char* CUTS="/home/jbane/tritium/triton/HallA-Online-Tritium/replay/RHRS.cuts";
//    ODEF=Form("%sLHRS.odef",def_dir);
 //   CUTS=Form("%sLHRS.cuts",def_dir);
    //==================================
    //  Detectors
    //==================================
    THaHRS* HRSR = new THaHRS("R","Right arm HRS");
    //HRSR->SetDebug(1);
    gHaApps->Add( HRSR );
	HRSR->AddDetector( new THaVDC("vdc","Drift Chamber" ));
    HRSR->AddDetector( new THaScintillator("s0","s0 scintillator") );
	HRSR->AddDetector( new THaScintillator("s2","s2 scintillator") );
    HRSR->AddDetector( new THaCherenkov("cer", "Gas Cherenkov counter" ));
    HRSR->AddDetector( new THaShower("ps", "Pre-shower pion rej." ));
    HRSR->AddDetector( new THaShower("sh", "Show pion rej." ));


        //==================================
    //  Scalers
    //==================================
    if(bScaler){

	gHaEvtHandlers->Add (new THaScalerEvtHandler("Right","HA scaler event type 140"));
      /*THaScalerGroup* RightScalers = new THaScalerGroup("Right");
      RightScalers->GetScalerObj()->SetClockRate(103700);
      gHaScalers->Add(RightScalers); 
      THaScalerGroup* EvRightScalers = new THaScalerGroup("evright");
      EvRightScalers->GetScalerObj()->SetClockRate(103700);
      gHaScalers->Add(EvRightScalers);*/		
    }

    //==================================
    //  Decoder Data
    //==================================
    THaApparatus* decR = new THaDecData("DR","Misc. Decoder Data");
    gHaApps->Add( decR );

    //==================================
    //  Beam
    //==================================
    THaIdealBeam* ib = new THaIdealBeam("ib","Ideal beam");
    gHaApps->Add(ib);

    THaRasteredBeam* Rrb = new THaRasteredBeam("Rrb", "Rastered beam to R-HRS");
    Rrb->AddDetector(new THaRaster("Raster2", "Downstream Raster"));
    Rrb->AddDetector(new THaBPM("BPMA", "First BPM"));
    Rrb->AddDetector(new THaBPM("BPMB", "Second BPM"));
    gHaApps->Add(Rrb);
    
    //==================================
    //  Physics
    //==================================
    if(bPhysics){
      Double_t mass_el  = 0.000511; // electron mass
      Double_t amu = 931.494*1.e-3; // amu to GeV
      Double_t mass_He3 = 3.0160293*amu;
      Double_t mass_H2 = 2.01410178*amu;
      Double_t mass_H3 = 3.0160492*amu;
      Double_t mass_tg = mass_H3; //default target 
  
      THaPhysicsModule *Rgold = new THaGoldenTrack( "R.gold", "HRS-R Golden Track", "R" );
      gHaPhysics->Add(Rgold);

      THaPhysicsModule *EKR = new THaPrimaryKine("EKR","Electron kinematics in HRS-R","R","ib",mass_tg); //Should be same if no beam included in constructor
      gHaPhysics->Add(EKR);

      THaPhysicsModule *EKRc = new THaPrimaryKine("EKRc","Corrected Electron kinematics in HRS-R","R","Rrb",mass_tg);
      gHaPhysics->Add(EKRc);

      THaReactionPoint *rpr = new THaReactionPoint("rpr","Reaction vertex for HRS-R","R","Rrb");
      gHaPhysics->Add(rpr);

      THaExtTarCor *exR =  new THaExtTarCor("exR","Corrected for extended target, HRS-R","R","rpr");
      gHaPhysics->Add(exR);

      THaPhysicsModule *EKRx = new THaPrimaryKine("EKRx","Better Corrected Electron kinematics in HRS-R","exR","Rrb",mass_tg);
      gHaPhysics->Add(EKRx);

      /*if(bEloss){
        // Beam Energy Loss
        Double_t zbeam_off = -0.075 ; //For a target centered at z=0, this should equal to the targetlength/2. (in m)
       
        Gmp_Beam_Eloss *ElbR = new Gmp_Beam_Eloss("ElbR","Beam Corrected for Energy Loss",beamchoice,"rpr",zbeam_off);
        ElbR->SetDebug(1);
        ElbR->SetMedium(1.,1.00727,0.0723); // Set medium assuming LH2 Target. According to the Cryotarget Training Slides,
                                           // the density should be 0.0723 g/cc (agrees more or less w/ NIST table).
        gHaPhysics->Add(ElbR);
        
        //Track Energy Loss
        Double_t targ_length = 0.15 ; // In meters. Set to 15 cm for GMp LH2 target
        Double_t ztrack_off = 0. ; //For a target centered at z=0, this should equal to 0. (in m)
        Double_t air_length = 0.3757; // In meters. Set to 0.3543 m for RHRS and 0.2697 m for LHRS for Spring 16.
                                     //            Set to 0.3757 m for RHRS and 0.3868 m for LHRS for Fall 16.
       
        Gmp_Track_Eloss *EltR = new Gmp_Track_Eloss("EltR","Track Corrected for Energy Loss","exR","rpr",targ_length,ztrack_off,air_length);
        EltR->SetDebug(1);
        EltR->SetMedium(1.,1.00727,0.0723); // See above for explanation.
        gHaPhysics->Add(EltR);

        THaPhysicsModule *EKRxe = new THaElectronKine("EKRxe","Best Corrected Electron kinematics in HRS-R","EltR","ElbR",mass_tg);
        gHaPhysics->Add(EKRxe);
      }*/
    }
  
  
  //=====================================
  //  Set up Analyzer and replay data
  //=====================================
  ReplayCore(
	     runnumber,        //run #
	     0,                //-1=replay all;0=ask for a number
	     all,              //default replay event num
	     RNAME,            //output file format
	     ODEF,	       //out define
	     CUTS, 	       //empty cut define
	     bScaler,          //replay scalar?
	     bHelicity,        //repaly helicity
	     fstEvt,	       //First Event To Replay
	     QuietRun	       //whether ask user for inputs
	     );

/*
  //=====================================
  //Generate online plots
  //=====================================
  if(bPlots){
    if(RIGHT_ARM_CONDITION){
      char* CONFIGFILE="/adaqfs/home/a-onl/tritium/replay/onlineGUI64/RHRS.cfg";
      char* CONFIGFILEPHYS="/adaqfs/home/a-onl/tritium/replay/onlineGUI64/RHRS_phy.cfg";
      

      gSystem->Exec(Form("/adaqfs/home/a-onl/tritium/replay/onlineGUI64/online -P -f %s -r %d", CONFIGFILE,runnumber));
      gSystem->Exec(Form("mv /adaqfs/home/a-onl/tritium/replay/summaryfiles/temp_%d.pdf /adaqfs/home/a-onl/tritium/replay/summaryfiles/right_detectors_%d.pdf",runnumber,runnumber));
      gSystem->Exec("unlink /adaqfs/home/a-onl/tritium/replay/summaryfiles/right_detectors_latest.pdf");
      gSystem->Exec(Form("ln -s right_detectors_%d.pdf /adaqfs/home/a-onl/tritium/replay/summaryfiles/right_detectors_latest.pdf",runnumber));
      

      gSystem->Exec(Form("/adaqfs/home/a-onl/tritium/replay/onlineGUI64/online -P -f %s -r %d", CONFIGFILEPHYS,runnumber));
      gSystem->Exec(Form("mv /adaqfs/home/a-onl/tritium/replay/summaryfiles/temp_%d.pdf /adaqfs/home/a-onl/tritium/replay/summaryfiles/right_physics_%d.pdf",runnumber,runnumber));
      gSystem->Exec("unlink /adaqfs/home/a-onl/tritium/replay/summaryfiles/right_physics_latest.pdf");
      gSystem->Exec(Form("ln -s right_physics_%d.pdf /adaqfs/home/a-onl/tritium/replay/summaryfiles/right_physics_latest.pdf",runnumber));
    }
    else if(LEFT_ARM_CONDITION){ 
      char* CONFIGFILE_L="/adaqfs/home/a-onl/tritium/replay/onlineGUI64/LHRS.cfg";
      char* CONFIGFILEPHYS_L="/adaqfs/home/a-onl/tritium/replay/onlineGUI64/LHRS_phy.cfg";

      gSystem->Exec(Form("/adaqfs/home/a-onl/tritium/replay/onlineGUI64/online -P -f %s -r %d", CONFIGFILE_L,runnumber));
      gSystem->Exec(Form("mv /adaqfs/home/a-onl/tritium/replay/summaryfiles/temp_%d.pdf /adaqfs/home/a-onl/tritium/replay/summaryfiles/left_detectors_%d.pdf",runnumber,runnumber));
      gSystem->Exec("unlink /adaqfs/home/a-onl/tritium/replay/summaryfiles/left_detectors_latest.pdf");
      gSystem->Exec(Form("ln -s left_detectors_%d.pdf /adaqfs/home/a-onl/tritium/replay/summaryfiles/left_detectors_latest.pdf",runnumber));

      gSystem->Exec(Form("/adaqfs/home/a-onl/tritium/replay/onlineGUI64/online -P -f %s -r %d", CONFIGFILEPHYS_L,runnumber));
      gSystem->Exec(Form("mv /adaqfs/home/a-onl/tritium/replay/summaryfiles/temp_%d.pdf /adaqfs/home/a-onl/tritium/replay/summaryfiles/left_physics_%d.pdf",runnumber,runnumber));
      gSystem->Exec("unlink /adaqfs/home/a-onl/tritium/replay/summaryfiles/left_physics_latest.pdf");
      gSystem->Exec(Form("ln -s left_physics_%d.pdf /adaqfs/home/a-onl/tritium/replay/summaryfiles/left_physics_latest.pdf",runnumber));
    }
  }*/
  exit(1);
}
 
