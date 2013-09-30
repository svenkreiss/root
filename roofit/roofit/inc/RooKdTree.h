/*****************************************************************************
 * Project: RooFit                                                           *
 * Package: RooFitModels                                                     *
 *    File: $Id: RooKdTree.h,v 1.10 2007/05/11 09:13:07 verkerke Exp $
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
#ifndef ROO_KDTREE
#define ROO_KDTREE


#include "RooDataSet.h"
#include "RooArgList.h"
#include "RooRealVar.h"

#include <algorithm>
#include <vector>

using namespace std;


/*
 * Builds a kd tree from vector<vector<double> > data. This data is never copied but
 * sorted and sliced in place. The slicing is done by using iterators to the beginning
 * and end of the outer vector.
 */
class RooSubKdTree {
public:
  RooSubKdTree(RooDataSet& roodataset) :
    _left(0), 
    _right(0)
  {
    vector< vector<double> > vecData = toStdVectorData(roodataset);
    createTree( vecData.begin(), vecData.end() );
  }

  RooSubKdTree(vector< vector<double> >::iterator data_begin, 
               vector< vector<double> >::iterator data_end, 
               int axis) :
    _left(0),
    _right(0)
  {
    createTree( data_begin, data_end, axis );
  }

  virtual ~RooSubKdTree() {
    if( _left ) delete _left;
    if( _right ) delete _right;
  }


  void createTree(vector< vector<double> >::iterator data_begin, 
                  vector< vector<double> >::iterator data_end, 
                  int axis = 0);

  vector<double> expectations(void);
  vector< vector<double> > covariance(vector<double>& centers);

  static vector< vector<double> > toStdVectorData(RooDataSet& roodataset);

protected:
  vector<double> _point;
  Int_t _axis;

  RooSubKdTree* _left;
  RooSubKdTree* _right;  
};




class RooKdTree : public RooAbsReal {
public:

  RooKdTree() ;
  RooKdTree(const char *name, const char *title, RooDataSet& data);
  RooKdTree(const RooKdTree& other, const char* name=0);

  void LoadDataSet( RooDataSet& data);

  virtual TObject* clone(const char* newname) const {return new RooKdTree(*this,newname); }
  virtual ~RooKdTree();


protected:
  
  virtual Double_t evaluate() const;

  RooSubKdTree* _tree;


private:
  
  Double_t evaluateFull(Double_t x) const;

  
  ClassDef(RooKdTree,1) // Kd tree
};

#endif
