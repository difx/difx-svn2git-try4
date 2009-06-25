/***************************************************************************
 *   Copyright (C) 2006 by Adam Deller                                     *
 *                                                                         *
 *   This program is free for non-commercial use: see the license file     *
 *   at http://astronomy.swin.edu.au:~adeller/software/difx/ for more      *
 *   details.                                                              *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 ***************************************************************************/
//===========================================================================
// SVN properties (DO NOT CHANGE)
//
// $Id$
// $HeadURL$
// $LastChangedRevision$
// $Author$
// $LastChangedDate$
//
//============================================================================
#ifndef POLYCO_H
#define POLYCO_H

#include <string>
#include <iostream>
#include <fstream>
#include "architecture.h"

using namespace std;

/** 
 @class Polyco
 @brief Provides pulse phase prediction for a given pulsar at a given time

 Uses a polynomial read from the provided polyco file to calculate pulse phase at given frequency and time.
 @author Adam Deller
 */
class Polyco{
public:
 /**
  * Constructor: Loads the provided file and allocates the weighting and phase arrays
  * @param filename The polyco file to load from
  * @param subcount The index of the polyco in the file to load
  * @param confindex The configuration index which this polyco relates to (ie which pulsar)
  * @param nbins The number of pulsar bins to use
  * @param nchans The number of spectral points per band
  * @param bphases The phases of the endpoints of each pulsar bin
  * @param bweights The weights associated with each pulsar bin
  * @param calcmins The period of time over which a given set of phase offsets is expected to be accurate
  */
  Polyco(string filename, int subcount, int confindex, int nbins, int nchans, double * bphases, double * bweights, double calcmins);

 /**
  * Copy constructor - performs deep copy of provided Polyco
  * @param tocopy The polyco object to copy
  */
  Polyco(const Polyco & tocopy);

  ~Polyco();

 /**
  * Loads a polyco file and stores the information in memory
  * @param filename The file to load from
  * @param subcount The index of the polyco in the file to load
  * @return True if polyco file loaded ok
  */
  bool loadPolycoFile(string filename, int subcount);

 /**
  * Checks to see if this polyco's valid range includes the specified time
  * @param incmjd The Modified Julian Date of the time to check
  * @param incmjdfraction The offset in days (as a fraction) from the specified integer MJD
  * @return True if the specified time falls in this Polyco's time range
  */
  bool includesTime(int incmjd, double incmjdfraction);

 /**
  * Sets the frequency band information and allocates the necessary arrays for bin calculation
  * @param nfreqs The number of frequencies to calculate for
  * @param freqs Array of frequency values, in MHz
  * @param bw The bandwidth of the bands, in MHz
  * @return Whether bin values are legal or not (should abort if not legal)
  */
  bool setFrequencyValues(int nfreqs, double * freqs, double bw);

 /**
  * Sets the active time (from which subsequent offsets will refer to) to the given values
  * @param startmjd The reference integer Modified Julian Date
  * @param startmjdfraction The reference Modified Julian Date fractional component
  */
  void setTime(int startmjd, double startmjdfraction);

 /**
  * Calculates the bins for each spectral point of each frequency band at the given time
  * @param offsetmins The offset from the previously set reference time, in minutes
  * @param bins The array of bin values, set by this function
  */
  void getBins(double offsetmins, int ** bins);

 /**
  * Loops through the provided Polyco array, and returns the one that matches the specified time, or null if none match
  * @param requiredconfig The configuration required of the Polyco (ensure you don't get the wrong pulsar!!!)
  * @param mjd The Modified Julian Date of the time to get
  * @param mjdfraction The offset in days (as a fraction) from the specified integer MJD
  * @param polycos The array of Polycos to check through
  * @param npolycos The number of polycos in the array
  * @param printtimes Whether to print the comparison values to cinfo or not
  * @return The matching Polyco, or null if none match
  */
  static Polyco * getCurrentPolyco(int requiredconfig, int mjd, double mjdfraction, Polyco ** polycos, int npolycos, bool printtimes);

 /**
  * Returns whether the initial file was read ok
  * @return Whether the initial file was read ok
  */
  inline bool initialisedOK() { return readok; }
  
 /**
  * Returns the bin counts
  * @return The bin counts (number of times each bin has been calculated since last cleared) for this Polyco
  */
  inline s32 *** getBinCounts() { return currentbincounts; }
 /**
  * Returns the configuration index
  * @return The configuration index this Polyco corresponds to
  */
  inline int getConfig() { return configindex; }
 /**
  * Returns the bin weights
  * @return The bin weights for this Polyco
  */
  inline f64* getBinWeights() { return binweights; }

 /**
  * Returns the weight*width product for the specified bin
  * @return The weight*width product for the specified bin
  */
  inline f64 getBinWeightTimesWidth(int bin) { f64 w = binphases[bin]-binphases[(bin+numbins-1)%numbins]; return (w<0.0)?binweights[bin]*(1.0+w):binweights[bin]*w; }

 /**
  * Returns the width of the specified bin
  * @return The width of the specified bin
  */
  inline f64 getBinWidth(int bin) { f64 w = binphases[bin]-binphases[(bin+numbins-1)%numbins]; return (w<0.0)?(1.0+w):w; }

 /**
  * Clears the bin counts
  */
  void incrementBinCount();

  inline int getMJD() { return mjd; }
  inline double getMJDfraction() { return mjdfraction; }
  inline double getSpanMinutes() { return timespan; }

  ///defines how much error will be tolerated due to frequency drift since the reference time
  static const double BIN_TOLERANCE;

  ///The constant used in converting DM to delay
  static const double DM_CONSTANT_SECS;
  
protected:
 /**
  * Calculates the phase offsets for each bin of each frequency at the given time
  * @param offsetmins The offset in minutes from the reference time
  */
  void calculateDMPhaseOffsets(double offsetmins);

  string pulsarname;
  int configindex, numbins, numchannels, numfreqs, observatory, timespan, numcoefficients, mjd;
  double mjdfraction, dt0, dm, dopplershift, logresidual, refphase, f0, obsfrequency, binaryphase, minbinwidth, bandwidth, calclengthmins;
  bool readok;
  double * coefficients;
  f64 * binphases;
  f64 * binweights;
  f32 * lofrequencies;
  f64 * freqcoefficientarray;
  f64 * phasecoefficientarray;
  f64 * timepowerarray;
  f64 * absolutephases;
  f64 ** dmPhaseOffsets;
  f64 ** channeldmdelays;
  int *** currentbincounts;
};

#endif
