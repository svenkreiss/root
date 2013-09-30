/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitModels                                                     *
 * @(#)root/roofit:$Id$
 * Authors:                                                                  *
 *   SK, Sven Kreiss,     New York University, sk@svenkreiss.com             *
 *   DK, David Kirkby,    UC Irvine,         dkirkby@uci.edu                 *
 *   WV, Wouter Verkerke, UC Santa Barbara, verkerke@slac.stanford.edu       *
 *                                                                           *
 * Copyright (c) 2000-2005, Regents of the University of California          *
 *                          and Stanford University. All rights reserved.    *
 *                                                                           *
 * Redistribution and use in source and binary forms,                        *
 * with or without modification, are permitted according to the terms        *
 * listed in LICENSE (http://roofit.sourceforge.net/license.txt)             *
 *****************************************************************************/
#include "RooFit.h"

#include "RooKdTree.h"
#include "RooAbsReal.h"
#include "RooRealVar.h"
#include "RooRandom.h"
#include "RooDataSet.h"

using namespace std;

ClassImp(RooKdTree)


//////////////////////////////////////////////////////////////////////////////
//
// BEGIN_HTML
// Class RooKdTree implements a kd tree.
// END_HTML
//








class ComparePoints {
public:
  ComparePoints( int axis ) : _axis(axis) {}
  bool operator()( vector<double> a, vector<double> b ) { return a[_axis] < b[_axis]; }
private:
  int _axis;
};

void RooSubKdTree::createTree(
  vector< vector<double> >::iterator data_begin, 
  vector< vector<double> >::iterator data_end, 
  int axis
) {
  _axis = axis;
  std::sort( data_begin, data_end, ComparePoints(_axis) );

  // count total number of points
  int total = 0;
  for( vector< vector<double> >::iterator i = data_begin; i != data_end; ++i ) { ++total; }

  // find median point
  vector< vector<double> >::iterator median = data_begin;
  for( int i = 0; i < total/2; ++i ) { ++median; }
  _point = *median;
  //for(unsigned long i=0; i<_point.size();i++) cout << "p["<<i<<"] = " << _point[i] << endl;

  // create trees left and right of median
  if( data_begin != median ) _left = new RooSubKdTree( data_begin, median, (axis+1)%_point.size() );
  if( median+1 != data_end ) _right = new RooSubKdTree( median+1, data_end, (axis+1)%_point.size() );
}

vector< vector<double> > RooSubKdTree::toStdVectorData(RooDataSet& roodataset) {
  vector< vector<double> > out;

  for( int i=0; i<roodataset.numEntries(); ++i ) {
    RooArgList* entry = (RooArgList*)roodataset.get(i);

    vector<double> point;
    for( int j=0; j<entry->getSize(); ++j ) point.push_back( ((RooRealVar*)entry->at(j))->getVal() );

    out.push_back( point );
  }

  return out;
}





















RooKdTree::RooKdTree() : _tree(0) { 
  // coverity[UNINIT_CTOR]
}



RooKdTree::RooKdTree(const char *name, const char *title, RooDataSet& data) :
  RooAbsReal(name,title),
  _tree(0)
{
  // form the lookup table
  LoadDataSet(data);
}




RooKdTree::RooKdTree(const RooKdTree& other, const char* name):
  RooAbsReal(other,name),
  _tree(0)
{
  // TODO copy over tree
}



RooKdTree::~RooKdTree() {
  if( _tree ) delete _tree;
}



void RooKdTree::LoadDataSet( RooDataSet& data) {
  if( _tree ) delete _tree;
  _tree = new RooSubKdTree( data );
}




Double_t RooKdTree::evaluate() const {
  /*
  Int_t i = (Int_t)floor((Double_t(_x)-_lo)/_binWidth);
  if (i<0) {
    cerr << "got point below lower bound:"
	 << Double_t(_x) << " < " << _lo
	 << " -- performing linear extrapolation..." << endl;
    i=0;
  }
  if (i>_nPoints-1) {
    cerr << "got point above upper bound:"
	 << Double_t(_x) << " > " << _hi
	 << " -- performing linear extrapolation..." << endl;
    i=_nPoints-1;
  }
  Double_t dx = (Double_t(_x)-(_lo+i*_binWidth))/_binWidth;
  
  // for now do simple linear interpolation.
  // one day replace by splines...
  return (_lookupTable[i]+dx*(_lookupTable[i+1]-_lookupTable[i]));
  */
  return 0.0;
}




Double_t RooKdTree::evaluateFull( Double_t x ) const {
  /*
  Double_t y=0;

  for (Int_t i=0;i<_nEvents;++i) {
    Double_t chi=(x-_dataPts[i])/_weights[i];
    y+=_dataWgts[i]*exp(-0.5*chi*chi)/_weights[i];

    // if mirroring the distribution across either edge of
    // the range ("Boundary Kernels"), pick up the additional
    // contributions
//      if (_mirrorLeft) {
//        chi=(x-(2*_lo-_dataPts[i]))/_weights[i];
//        y+=exp(-0.5*chi*chi)/_weights[i];
//      }
    if (_asymLeft) {
      chi=(x-(2*_lo-_dataPts[i]))/_weights[i];
      y-=_dataWgts[i]*exp(-0.5*chi*chi)/_weights[i];
    }
//      if (_mirrorRight) {
//        chi=(x-(2*_hi-_dataPts[i]))/_weights[i];
//        y+=exp(-0.5*chi*chi)/_weights[i];
//      }
    if (_asymRight) {
      chi=(x-(2*_hi-_dataPts[i]))/_weights[i];
      y-=_dataWgts[i]*exp(-0.5*chi*chi)/_weights[i];
    }
  }
  
  static const Double_t sqrt2pi(sqrt(2*TMath::Pi()));  
  return y/(sqrt2pi*_sumWgt);
  */
  return 0.0;
}
