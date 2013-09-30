/***************************************************************************** 
 * Project: RooFit                                                           * 
 *                                                                           * 
 * This code was autogenerated by RooClassFactory                            * 
 *****************************************************************************/ 

// Your description goes here... 

#include "Riostream.h" 

#include "RooMomentMorph.h" 
#include "RooAbsCategory.h" 
#include "RooRealIntegral.h"
#include "RooRealConstant.h"
#include "RooRealVar.h"
#include "RooFormulaVar.h"
#include "RooCustomizer.h"
#include "RooAddPdf.h"
#include "RooAddition.h"
#include "RooMoment.h"
#include "RooLinearVar.h"
#include "RooChangeTracker.h"

#include "TMath.h"
#include "TH1.h"

using namespace std;

ClassImp(RooMomentMorph) 


//_____________________________________________________________________________
RooMomentMorph::RooMomentMorph() : _curNormSet(0), _mref(0), _M(0), _useHorizMorph(true)
{
  // coverity[UNINIT_CTOR]
  _varItr    = _varList.createIterator() ;
  _pdfItr    = _pdfList.createIterator() ; 
}



//_____________________________________________________________________________
RooMomentMorph::RooMomentMorph(const char *name, const char *title, 
                           RooAbsReal& _m,
                           const RooArgList& varList,
                           const RooArgList& pdfList,
                           const TVectorD& mrefpoints,
                           const Setting& setting) :
  RooAbsPdf(name,title), 
  _cacheMgr(this,10),
  m("m","m",this,_m),
  _varList("varList","List of variables",this),
  _pdfList("pdfList","List of pdfs",this),
  _setting(setting),
  _useHorizMorph(true)
{ 
  // CTOR

  // observables
  TIterator* varItr = varList.createIterator() ;
  RooAbsArg* var ;
  for (Int_t i=0; (var = (RooAbsArg*)varItr->Next()); ++i) {
    if (!dynamic_cast<RooAbsReal*>(var)) {
      coutE(InputArguments) << "RooMomentMorph::ctor(" << GetName() << ") ERROR: variable " << var->GetName() << " is not of type RooAbsReal" << endl ;
      throw string("RooPolyMorh::ctor() ERROR variable is not of type RooAbsReal") ;
    }
    _varList.add(*var) ;
  }
  delete varItr ;

  // reference p.d.f.s
  TIterator* pdfItr = pdfList.createIterator() ;
  RooAbsPdf* pdf ;
  for (Int_t i=0; (pdf = dynamic_cast<RooAbsPdf*>(pdfItr->Next())); ++i) {
    if (!pdf) {
      coutE(InputArguments) << "RooMomentMorph::ctor(" << GetName() << ") ERROR: pdf " << pdf->GetName() << " is not of type RooAbsPdf" << endl ;
      throw string("RooPolyMorh::ctor() ERROR pdf is not of type RooAbsPdf") ;
    }
    _pdfList.add(*pdf) ;
  }
  delete pdfItr ;

  _mref      = new TVectorD(mrefpoints);
  _varItr    = _varList.createIterator() ;
  _pdfItr    = _pdfList.createIterator() ; 

  // initialization
  initialize();
} 



//_____________________________________________________________________________
RooMomentMorph::RooMomentMorph(const char *name, const char *title, 
                           RooAbsReal& _m,
                           const RooArgList& varList,
                           const RooArgList& pdfList,
                           const RooArgList& mrefList,
                           const Setting& setting) :
  RooAbsPdf(name,title), 
  _cacheMgr(this,10),
  m("m","m",this,_m),
  _varList("varList","List of variables",this),
  _pdfList("pdfList","List of pdfs",this),
  _setting(setting),
  _useHorizMorph(true)
{ 
  // CTOR

  // observables
  TIterator* varItr = varList.createIterator() ;
  RooAbsArg* var ;
  for (Int_t i=0; (var = (RooAbsArg*)varItr->Next()); ++i) {
    if (!dynamic_cast<RooAbsReal*>(var)) {
      coutE(InputArguments) << "RooMomentMorph::ctor(" << GetName() << ") ERROR: variable " << var->GetName() << " is not of type RooAbsReal" << endl ;
      throw string("RooPolyMorh::ctor() ERROR variable is not of type RooAbsReal") ;
    }
    _varList.add(*var) ;
  }
  delete varItr ;

  // reference p.d.f.s
  TIterator* pdfItr = pdfList.createIterator() ;
  RooAbsPdf* pdf ;
  for (Int_t i=0; (pdf = dynamic_cast<RooAbsPdf*>(pdfItr->Next())); ++i) {
    if (!pdf) {
      coutE(InputArguments) << "RooMomentMorph::ctor(" << GetName() << ") ERROR: pdf " << pdf->GetName() << " is not of type RooAbsPdf" << endl ;
      throw string("RooPolyMorh::ctor() ERROR pdf is not of type RooAbsPdf") ;
    }
    _pdfList.add(*pdf) ;
  }
  delete pdfItr ;
  
  // reference points in m
  _mref      = new TVectorD(mrefList.getSize());
  TIterator* mrefItr = mrefList.createIterator() ;
  RooAbsReal* mref ;
  for (Int_t i=0; (mref = dynamic_cast<RooAbsReal*>(mrefItr->Next())); ++i) {
    if (!mref) {
      coutE(InputArguments) << "RooMomentMorph::ctor(" << GetName() << ") ERROR: mref " << mref->GetName() << " is not of type RooAbsReal" << endl ;
      throw string("RooPolyMorh::ctor() ERROR mref is not of type RooAbsReal") ;
    }
    if (!dynamic_cast<RooConstVar*>(mref)) {
      coutW(InputArguments) << "RooMomentMorph::ctor(" << GetName() << ") WARNING mref point " << i << " is not a constant, taking a snapshot of its value" << endl ;
    }
    (*_mref)[i] = mref->getVal() ;
  }
  delete mrefItr ;
  
  _varItr    = _varList.createIterator() ;
  _pdfItr    = _pdfList.createIterator() ; 

  // initialization
  initialize();
} 



//_____________________________________________________________________________
RooMomentMorph::RooMomentMorph(const RooMomentMorph& other, const char* name) :  
  RooAbsPdf(other,name), 
  _cacheMgr(other._cacheMgr,this),
  _curNormSet(0),
  m("m",this,other.m),
  _varList("varList",this,other._varList),
  _pdfList("pdfList",this,other._pdfList),
  _setting(other._setting),
  _useHorizMorph(other._useHorizMorph)
{ 
  _mref = new TVectorD(*other._mref) ;
  _varItr    = _varList.createIterator() ;
  _pdfItr    = _pdfList.createIterator() ; 

  // initialization
  initialize();
} 

//_____________________________________________________________________________
RooMomentMorph::~RooMomentMorph() 
{
  if (_mref)   delete _mref;
  if (_varItr) delete _varItr;
  if (_pdfItr) delete _pdfItr;
  if (_M)      delete _M;
}



//_____________________________________________________________________________
void RooMomentMorph::initialize() 
{

  Int_t nPdf = _pdfList.getSize();

  // other quantities needed
  if (nPdf!=_mref->GetNrows()) {
    coutE(InputArguments) << "RooMomentMorph::initialize(" << GetName() << ") ERROR: nPdf != nRefPoints" << endl ;
    assert(0) ;
  }

  TVectorD* dm = new TVectorD(nPdf);
  _M = new TMatrixD(nPdf,nPdf);

  // transformation matrix for non-linear extrapolation, needed in evaluate()
  TMatrixD M(nPdf,nPdf);
  for (Int_t i=0; i<_mref->GetNrows(); ++i) {
    (*dm)[i] = (*_mref)[i]-(*_mref)[0];
    M(i,0) = 1.;
    if (i>0) M(0,i) = 0.;
  }
  for (Int_t i=1; i<_mref->GetNrows(); ++i) {
    for (Int_t j=1; j<_mref->GetNrows(); ++j) {
      M(i,j) = TMath::Power((*dm)[i],(double)j);
    }
  }
  (*_M) = M.Invert();

  delete dm ;
}

//_____________________________________________________________________________
RooMomentMorph::CacheElem* RooMomentMorph::getCache(const RooArgSet* /*nset*/) const
{
  CacheElem* cache = (CacheElem*) _cacheMgr.getObj(0,(RooArgSet*)0) ;
  if (cache) {
    return cache ;
  }
  Int_t nVar = _varList.getSize();
  Int_t nPdf = _pdfList.getSize();

  RooAbsReal* null = 0 ;
  vector<RooAbsReal*> meanrv(nPdf*nVar,null);
  vector<RooAbsReal*> sigmarv(nPdf*nVar,null); 
  vector<RooAbsReal*> myrms(nVar,null);      
  vector<RooAbsReal*> mypos(nVar,null);      
  vector<RooAbsReal*> slope(nPdf*nVar,null); 
  vector<RooAbsReal*> offs(nPdf*nVar,null); 
  vector<RooAbsReal*> transVar(nPdf*nVar,null); 
  vector<RooAbsReal*> transPdf(nPdf,null);      

  RooArgSet ownedComps ;

  RooArgList fracl ;

  // fraction parameters
  RooArgList coefList("coefList");
  RooArgList coefList2("coefList2");
  for (Int_t i=0; i<2*nPdf; ++i) {
    std::string fracName = Form("frac_%d",i);
    fracl.add(*new RooRealVar(fracName.c_str(),fracName.c_str(),1.)); // to be set later 
    if (i<nPdf) coefList.add(*(RooRealVar*)(fracl.at(i))) ;
    else coefList2.add(*(RooRealVar*)(fracl.at(i))) ;
    ownedComps.add(*(RooRealVar*)(fracl.at(i))) ;
  }

  RooAddPdf* theSumPdf = 0;
  std::string sumpdfName = Form("%s_sumpdf",GetName());
    
  if (_useHorizMorph) {
    // mean and sigma
    RooArgList varList(_varList) ;
    for (Int_t i=0; i<nPdf; ++i) {
      for (Int_t j=0; j<nVar; ++j) {
	
	std::string meanName = Form("%s_mean_%d_%d",GetName(),i,j);
	std::string sigmaName = Form("%s_sigma_%d_%d",GetName(),i,j);      
	
	RooAbsPdf* thisPdf = (RooAbsPdf*)_pdfList.at(i);
	
	// fast calculation of mean and RMS for RooHistPdfs
	if (thisPdf->IsA()->InheritsFrom("RooHistPdf") && nVar>1) {
	  
	  RooRealVar& var_j = (RooRealVar&)*varList.at(j);
	  
	  RooArgList notJ(_varList);
	  notJ.remove(var_j);
	  
	  //Make a ROOT histogram from the RooHistPdf
	  RooCmdArg zVarArg=RooCmdArg::none();
	  if (nVar==3) zVarArg=RooFit::ZVar(*(RooRealVar*)notJ.at(1)); 
	  
	  TH1 * hist = thisPdf->createHistogram("testProjection", var_j, 
						RooFit::YVar(*(RooRealVar*)notJ.at(0)),
						zVarArg);	
	  
	  //Read out the mean and sigma (RMS is historical name)
	  double histmean  = hist->GetMean(1); // 1 gives mean along x-axis
	  double histsigma = hist->GetRMS(1);
	  
	  delete hist;
	  
	  sigmarv[ij(i,j)] = new RooRealVar(sigmaName.c_str(),sigmaName.c_str(), histsigma);
	  meanrv [ij(i,j)] = new RooRealVar(meanName.c_str(), meanName.c_str(), histmean);
	  
	}
	else {
	  
	  if (nVar>1) {
	    
	    //Create an integral over all observables except the j-th positioned one (and any variables in nset)
	    RooArgSet * notJ = new RooArgSet( varList, "notJ" );
	    RooRealVar& var_j = (RooRealVar&)*varList.at(j);
	    notJ->remove( var_j );
	    
	    RooAbsReal * intTest = thisPdf->createIntegral( *notJ );
	    
	    //Get the mean and sigma with respect to the j-th observable
	    sigmarv[ij(i,j)] = intTest->sigma( var_j );
	    meanrv [ij(i,j)] = intTest->mean ( var_j );		  
	  }
	  else {
	    RooMoment* mom = ((RooAbsPdf*)_pdfList.at(i))->sigma((RooRealVar&)*varList.at(j)) ;
	    
	    sigmarv[ij(i,j)] = mom ;
	    meanrv[ij(i,j)]  = mom->mean() ;
	  }
	}
	ownedComps.add(*sigmarv[ij(i,j)]) ;      
      }
    }
    
    // slope and offset (to be set later, depend on m)
    for (Int_t j=0; j<nVar; ++j) {
      RooArgList meanList("meanList");
      RooArgList rmsList("rmsList");
      for (Int_t i=0; i<nPdf; ++i) {
	meanList.add(*meanrv[ij(i,j)]);
	rmsList.add(*sigmarv[ij(i,j)]);
      }
      std::string myrmsName = Form("%s_rms_%d",GetName(),j);
      std::string myposName = Form("%s_pos_%d",GetName(),j);
      myrms[j] = new RooAddition(myrmsName.c_str(),myrmsName.c_str(),rmsList,coefList2);
      mypos[j] = new RooAddition(myposName.c_str(),myposName.c_str(),meanList,coefList2);
      ownedComps.add(RooArgSet(*myrms[j],*mypos[j])) ;
    }
    // construction of unit pdfs
    _pdfItr->Reset();
    RooAbsPdf* pdf;
    RooArgList transPdfList;
    
    for (Int_t i=0; i<nPdf; ++i) {
      _varItr->Reset() ;
      RooRealVar* var ;
      
      pdf = (RooAbsPdf*)_pdfItr->Next();
      std::string pdfName = Form("pdf_%d",i);
      RooCustomizer cust(*pdf,pdfName.c_str());
      
      for (Int_t j=0; j<nVar; ++j) {
	// slope and offset formulas
	std::string slopeName = Form("%s_slope_%d_%d",GetName(),i,j);
	std::string offsetName = Form("%s_offset_%d_%d",GetName(),i,j);
	slope[ij(i,j)]  = new RooFormulaVar(slopeName.c_str(),"@0/@1",RooArgList(*sigmarv[ij(i,j)],*myrms[j]));
	offs[ij(i,j)] = new RooFormulaVar(offsetName.c_str(),"@0-(@1*@2)",RooArgList(*meanrv[ij(i,j)],*mypos[j],*slope[ij(i,j)]));
	ownedComps.add(RooArgSet(*slope[ij(i,j)],*offs[ij(i,j)])) ;
	// linear transformations, so pdf can be renormalized
	var = (RooRealVar*)(_varItr->Next());
	std::string transVarName = Form("%s_transVar_%d_%d",GetName(),i,j);
	//transVar[ij(i,j)] = new RooFormulaVar(transVarName.c_str(),transVarName.c_str(),"@0*@1+@2",RooArgList(*var,*slope[ij(i,j)],*offs[ij(i,j)]));
	transVar[ij(i,j)] = new RooLinearVar(transVarName.c_str(),transVarName.c_str(),*var,*slope[ij(i,j)],*offs[ij(i,j)]);
	
	ownedComps.add(*transVar[ij(i,j)]) ;
	cust.replaceArg(*var,*transVar[ij(i,j)]);
      }
      transPdf[i] = (RooAbsPdf*) cust.build() ;
      transPdfList.add(*transPdf[i]);
      ownedComps.add(*transPdf[i]) ;
    }
    // sum pdf
    
    
    theSumPdf = new RooAddPdf(sumpdfName.c_str(),sumpdfName.c_str(),transPdfList,coefList);
  }
  else {
    theSumPdf = new RooAddPdf(sumpdfName.c_str(),sumpdfName.c_str(),_pdfList,coefList);
  }

  theSumPdf->addOwnedComponents(ownedComps) ;

  // change tracker for fraction parameters
  std::string trackerName = Form("%s_frac_tracker",GetName()) ;
  RooChangeTracker* tracker = new RooChangeTracker(trackerName.c_str(),trackerName.c_str(),m.arg(),kTRUE) ;


  // Store it in the cache
  cache = new CacheElem(*theSumPdf,*tracker,fracl) ;
  _cacheMgr.setObj(0,0,cache,0) ;

  return cache ;
}



//_____________________________________________________________________________
RooArgList RooMomentMorph::CacheElem::containedArgs(Action) 
{
  return RooArgList(*_sumPdf,*_tracker) ; 
}



//_____________________________________________________________________________
RooMomentMorph::CacheElem::~CacheElem() 
{ 
  delete _sumPdf ; 
  delete _tracker ; 
} 



//_____________________________________________________________________________
Double_t RooMomentMorph::getVal(const RooArgSet* set) const 
{
  // Special version of getVal() overrides RooAbsReal::getVal() to save value of current normalization set
  _curNormSet = set ? (RooArgSet*)set : (RooArgSet*)&_varList ;
  return RooAbsPdf::getVal(set) ;
}



//_____________________________________________________________________________
RooAbsPdf* RooMomentMorph::sumPdf(const RooArgSet* nset) 
{
  CacheElem* cache = getCache(nset ? nset : _curNormSet) ;
  
  if (cache->_tracker->hasChanged(kTRUE)) {
    cache->calculateFractions(*this,kFALSE); // verbose turned off
  } 
  
  return cache->_sumPdf ;
}


//_____________________________________________________________________________
Double_t RooMomentMorph::evaluate() const 
{ 
  CacheElem* cache = getCache(_curNormSet) ;
  
  if (cache->_tracker->hasChanged(kTRUE)) {
    cache->calculateFractions(*this,kFALSE); // verbose turned off
  } 
  
  Double_t ret = cache->_sumPdf->getVal(_pdfList.nset());
  return ret ;
} 

//_____________________________________________________________________________
RooRealVar* RooMomentMorph::CacheElem::frac(Int_t i ) 
{ 
  return (RooRealVar*)(_frac.at(i))  ; 
}



//_____________________________________________________________________________
const RooRealVar* RooMomentMorph::CacheElem::frac(Int_t i ) const 
{ 
  return (RooRealVar*)(_frac.at(i))  ; 
}


//_____________________________________________________________________________
void RooMomentMorph::CacheElem::calculateFractions(const RooMomentMorph& self, Bool_t verbose) const
{
  Int_t nPdf = self._pdfList.getSize();

  Double_t dm = self.m - (*self._mref)[0];

  // fully non-linear
  double sumposfrac=0.;
  for (Int_t i=0; i<nPdf; ++i) {
    double ffrac=0.;
    for (Int_t j=0; j<nPdf; ++j) { ffrac += (*self._M)(j,i) * (j==0?1.:TMath::Power(dm,(double)j)); }
    if (ffrac>=0) sumposfrac+=ffrac;
    // fractions for pdf
    ((RooRealVar*)frac(i))->setVal(ffrac);
    // fractions for rms and mean
    ((RooRealVar*)frac(nPdf+i))->setVal(ffrac);
    if (verbose) { cout << ffrac << endl; }
  }

  // various mode settings
  int imin = self.idxmin(self.m);
  int imax = self.idxmax(self.m);
  double mfrac = (self.m-(*self._mref)[imin])/((*self._mref)[imax]-(*self._mref)[imin]);
  switch (self._setting) {
    case NonLinear:
      // default already set above
    break;

    case SineLinear:
      mfrac = TMath::Sin( TMath::PiOver2()*mfrac ); // this gives a continuous differentiable transition between grid points. 

      // now fall through to Linear case

    case Linear: 
      for (Int_t i=0; i<2*nPdf; ++i)
        ((RooRealVar*)frac(i))->setVal(0.);      
      if (imax>imin) { // m in between mmin and mmax
        ((RooRealVar*)frac(imin))->setVal(1.-mfrac); 
        ((RooRealVar*)frac(nPdf+imin))->setVal(1.-mfrac);
        ((RooRealVar*)frac(imax))->setVal(mfrac);
        ((RooRealVar*)frac(nPdf+imax))->setVal(mfrac);
      } else if (imax==imin) { // m outside mmin and mmax
        ((RooRealVar*)frac(imin))->setVal(1.);
        ((RooRealVar*)frac(nPdf+imin))->setVal(1.);
      }
    break;
    case NonLinearLinFractions:
      for (Int_t i=0; i<nPdf; ++i)
        ((RooRealVar*)frac(i))->setVal(0.);
      if (imax>imin) { // m in between mmin and mmax
        ((RooRealVar*)frac(imin))->setVal(1.-mfrac);
        ((RooRealVar*)frac(imax))->setVal(mfrac);
      } else if (imax==imin) { // m outside mmin and mmax
        ((RooRealVar*)frac(imin))->setVal(1.);
      }
    break;
    case NonLinearPosFractions:
      for (Int_t i=0; i<nPdf; ++i) {
        if (((RooRealVar*)frac(i))->getVal()<0) ((RooRealVar*)frac(i))->setVal(0.);
        ((RooRealVar*)frac(i))->setVal(((RooRealVar*)frac(i))->getVal()/sumposfrac);
      }
    break;
  }
 
}

//_____________________________________________________________________________
int RooMomentMorph::idxmin(const double& mval) const
{
  int imin(0);
  Int_t nPdf = _pdfList.getSize();
  double mmin=-DBL_MAX;
  for (Int_t i=0; i<nPdf; ++i) 
    if ( (*_mref)[i]>mmin && (*_mref)[i]<=mval ) { mmin=(*_mref)[i]; imin=i; }
  return imin;
}


//_____________________________________________________________________________
int RooMomentMorph::idxmax(const double& mval) const
{
  int imax(0);
  Int_t nPdf = _pdfList.getSize();
  double mmax=DBL_MAX;
  for (Int_t i=0; i<nPdf; ++i) 
    if ( (*_mref)[i]<mmax && (*_mref)[i]>=mval ) { mmax=(*_mref)[i]; imax=i; }
  return imax;
}



