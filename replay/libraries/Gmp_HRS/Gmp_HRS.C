//*-- Author :    Ole Hansen   2-Oct-01
//////////////////////////////////////////////////////////////////////////
//
// Gmp_HRS
// Adapted for the GMp experiment
// Barak Schmookler May-2016
//
//////////////////////////////////////////////////////////////////////////
// Some of the following is deprecated for the Gmp_HRS class...
//
// THaHRS
//
// The standard Hall A High Resolution Spectrometers (HRS).
// Contains three standard detectors,
//    VDC
//    Scintillator S1
//    Scintillator S2
//
// The usual name of this object is either "R" or "L", for Left 
// and Right HRS, respectively.
//
// Defines the functions FindVertices() and TrackCalc(), which are common
// to both the LeftHRS and the RightHRS.
//
// Special configurations of the HRS (e.g. more detectors, different 
// detectors) can be supported in on e of three ways:
//
//   1. Use the AddDetector() method to include a new detector
//      in this apparatus.  The detector will be decoded properly,
//      and its variables will be available for cuts and histograms.
//      Its processing methods will also be called by the generic Reconstruct()
//      algorithm implemented in THaSpectrometer::Reconstruct() and should
//      be correctly handled if the detector class follows the standard 
//      interface design.
//
//   2. Write a derived class that creates the detector in the
//      constructor.  Write a new Reconstruct() method or extend the existing
//      one if necessary.
//
//   3. Write a new class inheriting from THaSpectrometer, using this
//      class as an example.  This is appropriate if your HRS 
//      configuration has fewer or different detectors than the 
//      standard HRS. (It might not be sensible to provide a RemoveDetector() 
//      method since Reconstruct() relies on the presence of the 
//      standard detectors to some extent.)
//
//  For timing calculations, S1 is treated as the scintillator at the
//  'reference distance', corresponding to the pathlength correction
//  matrix.
//
//////////////////////////////////////////////////////////////////////////

#include "Gmp_HRS.h"
#include "THaTrackingDetector.h"
#include "THaTrack.h"
#include "THaScintillator.h"
#include "Gmp_Xscin.h"
#include "THaVDC.h"
#include "THaTrackProj.h"
#include "THaTriggerTime.h"
#include "TMath.h"
#include "TList.h"

#include "TList.h"
#include "TMath.h"

#ifdef WITH_DEBUG
#include <iostream>
#endif

using namespace std;

//_____________________________________________________________________________
Gmp_HRS::Gmp_HRS( const char* name, const char* description ) :
  THaSpectrometer( name, description )
{
  // Constructor. Defines the standard detectors for the HRS.
  AddDetector( new THaTriggerTime("trg","Trigger-based time offset"));
  AddDetector( new THaVDC("vdc", "Vertical Drift Chamber"));
  AddDetector( new THaScintillator("s2", "S2 scintillator"));

  s2_scin = static_cast<THaScintillator*>(GetDetector("s2"));

  SetTrSorting(kFALSE);
}

//_____________________________________________________________________________
Gmp_HRS::~Gmp_HRS()
{
  // Destructor
}

//_____________________________________________________________________________
Bool_t Gmp_HRS::SetTrSorting( Bool_t set )
{
  if( set )
    fProperties |= kSortTracks;
  else
    fProperties &= ~kSortTracks;

  return set;
}

//_____________________________________________________________________________
Bool_t Gmp_HRS::GetTrSorting() const
{
  return ((fProperties & kSortTracks) != 0);
}
 
//_____________________________________________________________________________
Int_t Gmp_HRS::FindVertices( TClonesArray& tracks )
{
  // Reconstruct target coordinates for all tracks found in the focal plane.

  TIter nextTrack( fTrackingDetectors );

  nextTrack.Reset();
  while( THaTrackingDetector* theTrackDetector =
	 static_cast<THaTrackingDetector*>( nextTrack() )) {
#ifdef WITH_DEBUG
    if( fDebug>1 ) cout << "Call FineTrack() for " 
			<< theTrackDetector->GetName() << "... ";
#endif
    theTrackDetector->FindVertices( tracks );
#ifdef WITH_DEBUG
    if( fDebug>1 ) cout << "done.\n";
#endif
  }

  // If enabled, sort the tracks by chi2/ndof
  if( GetTrSorting() )
    fTracks->Sort();
  
  // Find the "Golden Track". 
  if( GetNTracks() > 0 ) {
    // Select first track in the array. If there is more than one track
    // and track sorting is enabled, then this is the best fit track
    // (smallest chi2/ndof).  Otherwise, it is the track with the best
    // geometrical match (smallest residuals) between the U/V clusters
    // in the upper and lower VDCs (old behavior).
    // 
    // Chi2/dof is a well-defined quantity, and the track selected in this
    // way is immediately physically meaningful. The geometrical match
    // criterion is mathematically less well defined and not usually used
    // in track reconstruction. Hence, chi2 sortiing is preferable, albeit
    // obviously slower.

    fGoldenTrack = static_cast<THaTrack*>( fTracks->At(0) );
    fTrkIfo      = *fGoldenTrack;
    fTrk         = fGoldenTrack;
  } else
    fGoldenTrack = NULL;

  return 0;
}

//_____________________________________________________________________________
Int_t Gmp_HRS::TrackCalc()
{
  // Additioal track calculations. At present, we only calculate beta here.

  return TrackTimes( fTracks );
}

//_____________________________________________________________________________
Int_t Gmp_HRS::TrackTimes( TClonesArray* Tracks ) {
  // Do the actual track-timing (beta) calculation.

  if ( !Tracks ) return -1;

  Gmp_Xscin *s0_scin = static_cast<Gmp_Xscin*>(GetDetector("s0"));
  
  THaTrack *track=0;
  Int_t ntrack = GetNTracks();

  //Loop over tracks
  for ( Int_t i=0; i < ntrack; i++ ) {
    track = static_cast<THaTrack*>(Tracks->At(i));
    Double_t beta = kBig;
    Double_t dbeta = kBig;
    Double_t time = kBig;
    Double_t dt = kBig;
    
    //S2 Stuff
    THaTrackProj* tr_s2 = static_cast<THaTrackProj*>(s2_scin->GetTrackHits()->At(i));
    Double_t s2pathl = tr_s2->GetPathLen();
    Int_t s2pad = tr_s2->GetChannel();
    if (s2pad<0) {
      track->SetBeta(beta);
      track->SetdBeta(dbeta);
      track->SetTime(time);
      track->SetdTime(dt);
      continue;
    }
    Double_t s2time = (s2_scin->GetTimes())[s2pad];
    
    //S0 Stuff
    THaTrackProj* tr_s0 = static_cast<THaTrackProj*>(s0_scin->GetTrackHits()->At(i));
    Double_t s0pathl = tr_s0->GetPathLen();
    Int_t s0pad = tr_s0->GetChannel();
    if (s0pad<0){
      track->SetBeta(beta);
      track->SetdBeta(dbeta);
      track->SetTime(time);
      track->SetdTime(dt);
      continue;
    }
    Double_t s0time = (s0_scin->GetTimes())[s0pad];

    //The main event...
    time = s2time - s0time; //Time of Flight (this is what I put into track for "time")
    Double_t pathl = s2pathl - s0pathl;
    Double_t speed = pathl/time;

#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
    Double_t c = TMath::C();
#else
    Double_t c = 2.99792458e8;
#endif
    beta = speed/c;
    
    track->SetBeta(beta);
    track->SetdBeta(dbeta); //Not used right now...
    track->SetTime(time); 
    track->SetdTime(dt); //Not used right now...
  }

  return 0;
}


/*
//Avoiding doing it this way for now...
//_____________________________________________________________________________
Int_t THaHRS::TrackTimes( TClonesArray* Tracks ) {
  // Do the actual track-timing (beta) calculation.
  // Use multiple scintillators to average together and get "best" time at S1.
  //
  // To be useful, a meaningful timing resolution should be assigned
  // to each Scintillator object (part of the database).
  
  if ( !Tracks ) return -1;
  
  THaTrack *track=0;
  Int_t ntrack = GetNTracks();

  // linear regression to:  t = t0 + pathl/(beta*c)
  //   where t0 is the time of the track at the reference plane (sc_ref).
  //   t0 and beta are solved for.
  //
  for ( Int_t i=0; i < ntrack; i++ ) {
    track = static_cast<THaTrack*>(Tracks->At(i));
    THaTrackProj* tr_ref = static_cast<THaTrackProj*>
      (sc_ref->GetTrackHits()->At(i));
    
    Double_t pathlref = tr_ref->GetPathLen();
    
    Double_t wgt_sum=0.,wx2=0.,wx=0.,wxy=0.,wy=0.;
    Int_t ncnt=0;
    
    // linear regression to get beta and time at ref.
    TIter nextSc( fNonTrackingDetectors );
    THaNonTrackingDetector *det;
    while ( ( det = static_cast<THaNonTrackingDetector*>(nextSc()) ) ) {
      THaScintillator *sc = dynamic_cast<THaScintillator*>(det);
      if ( !sc ) continue;

      const THaTrackProj *trh = static_cast<THaTrackProj*>(sc->GetTrackHits()->At(i));
      
      Int_t pad = trh->GetChannel();
      if (pad<0) continue;
      Double_t pathl = (trh->GetPathLen()-pathlref);
      Double_t time = (sc->GetTimes())[pad];
      Double_t wgt = (sc->GetTuncer())[pad];
      
      if (pathl>.5*kBig || time>.5*kBig) continue;
      if (wgt>0) wgt = 1./(wgt*wgt);
      else continue;
      
      wgt_sum += wgt;
      wx2 += wgt*pathl*pathl;
      wx  += wgt*pathl;
      wxy += wgt*pathl*time;
      wy  += wgt*time;
      ncnt++;
    }

    Double_t beta = kBig;
    Double_t dbeta = kBig;
    Double_t time = kBig;
    Double_t dt = kBig;
    
    Double_t delta = wgt_sum*wx2-wx*wx;
    
    if (delta != 0.) {
      time = (wx2*wy-wx*wxy)/delta;
      dt = TMath::Sqrt(wx2/delta);
      Double_t invbeta = (wgt_sum*wxy-wx*wy)/delta;
      if (invbeta != 0.) {
#if ROOT_VERSION_CODE >= ROOT_VERSION(3,4,0)
	Double_t c = TMath::C();
#else
	Double_t c = 2.99792458e8;
#endif
	beta = 1./(c*invbeta);
	dbeta = TMath::Sqrt(wgt_sum/delta)/(c*invbeta*invbeta);
      }
    } 

    track->SetBeta(beta);
    track->SetdBeta(dbeta);
    track->SetTime(time);
    track->SetdTime(dt);
  }
  return 0;
}
*/

//_____________________________________________________________________________
ClassImp(Gmp_HRS)
