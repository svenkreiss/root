#!/usr/bin/env python

#  Created on: February 12, 2013

__author__ = "Sven Kreiss, Kyle Cranmer"
__version__ = "0.1"




import optparse

parser = optparse.OptionParser(version="0.1")
parser.add_option("-i", "--inputFiles", help="glob expression for log files from BatchProfileLikelihood.py", type="string", dest="inputFiles", default="batchProfile.log")
parser.add_option("-o", "--outputFile", help="output root file", type="string", dest="outputFile", default="PL_data.root")
parser.add_option(      "--subtractMinNLL", help="subtracts the minNLL", dest="subtractMinNLL", default=False, action="store_true")
#parser.add_option(      "--
parser.add_option("-q", "--quiet", dest="verbose", action="store_false", default=True, help="Quiet output.")
(options, args) = parser.parse_args()

import ROOT
import PyROOTUtils

import os, math
import glob, re
from array import array





def getInputFromLogs( files ):
   files = glob.glob( options.inputFiles )
   print( "Files: "+str(files) )
   
   bestFit = {}
   NLL = {'nll':[]}
   POIs = []
   NUISs = []
   
   regexParValue = re.compile( "^(?P<par>[-\b\w\d_\.]+)=(?P<value>[-\d\.einf]+)$" )
   
   for fName in files:
      print( "Opening "+fName )
      f = open( fName )
      for l in f:
         if l[:6] == "* POI ":
            poiName = l[6:l.find("=")]
            poiConfig = l[l.find("=")+2:-2]
            poiConfig = [ float(p) for p in poiConfig.split(",") ]
            if poiName not in [p[0] for p in POIs]:
               POIs.append( (poiName, poiConfig) )
               NLL[ poiName ] = []
         if l[:7] == "* NUIS ":
            nuisName = l[7:l.find("=")]
            nuisConfig = l[l.find("=")+2:-2]
            nuisConfig = [ float(p) for p in nuisConfig.split(",") ]
            if nuisName not in [p[0] for p in NUISs]:
               NUISs.append( (nuisName, nuisConfig) )
               NLL[ nuisName ] = []
               
         if l[:4] == "nll=":
#             parAndValue = [(r.split("=")[0],float(r.split("=")[1])) for r in l.split(", ")]
#             for p,v in parAndValue: NLL[p].append(v)
            pars = {}
            parAndValues = l.split(", ")
            for pv in parAndValues:
               regm = regexParValue.match( pv )
               if regm:
                  try:
                     pars[ regm.groupdict()['par'] ] = float(regm.groupdict()['value'])
                  except ValueError:
                     print( "WARNING could not convert value to float." )
            
            if len( pars.keys() ) == len( NLL.keys() ):
               for p,v in pars.iteritems():
                  NLL[ p ].append( v )
            else:
               print( "WARNING: Did not find all parameters. Not adding values. line: "+l )
         if l[:14] == "ucmles -- nll=":
            parAndValue = [(r.split("=")[0],float(r.split("=")[1])) for r in l.split(", ")]
            for p,v in parAndValue:
               if p == "ucmles -- nll": p = "nll"
               bestFit[p] = v
#             parAndValues = l.split(", ")
#             for pv in parAndValues:
#                regm = regexParValue.match( pv )
#                if regm: 
#                   par = regm.groupdict()['par']
#                   if par == "ucmles -- nll": par = "nll"
#                   NLL[ par ].append(  float(regm.groupdict()['value'])   )
      f.close()
      
   return (POIs,NUISs,NLL,bestFit)


def main():
   POIs,NUISs,NLL,bestFit = getInputFromLogs( options.inputFiles )

   print( "\n--- POIs ---" )
   print( POIs )

   print( "\n--- Best fit ---" )
   print( bestFit )

   print( "\n--- NLL ---" )
   maxNLL = max( [n for n in NLL['nll'] if n < 1e10] )
   if "nll" in bestFit:
      minNLL = bestFit["nll"]
   else:
      minNLL = min( [n for n in NLL['nll']] )
   for i in range( len(NLL['nll']) ):
      if NLL['nll'][i] < minNLL: NLL['nll'][i] = minNLL
      if NLL['nll'][i] > maxNLL: NLL['nll'][i] = maxNLL
   print( "(minNLL,maxNLL) = (%f,%f)" % (minNLL,maxNLL) )

   nllHist = None
   maxHist = maxNLL
   if options.subtractMinNLL: maxHist -= minNLL
   if len( POIs ) == 1:
      poi = POIs[0]
      nllHist = ROOT.TH1D( "profiledNLL", "profiled NLL;"+poi[0]+";NLL", int(poi[1][0]), poi[1][1], poi[1][2] )
      
      # initialize to maxNLL
      for i in range( nllHist.GetNbinsX()+2 ): nllHist.SetBinContent( i, maxHist )

      for nll,p in zip(NLL['nll'],NLL[poi[0]]):
         bin,val = (None,None)
         bin = nllHist.FindBin( p )
         val = nll
         if options.subtractMinNLL: val -= minNLL
         if nllHist.GetBinContent( bin ) > val: nllHist.SetBinContent( bin, val )
   if len( POIs ) == 2:
      poi1 = POIs[0]
      poi2 = POIs[1]
      nllHist = ROOT.TH2D( 
         "profiledNLL", "profiled NLL;"+poi1[0]+";"+poi2[0]+";NLL",
         int(poi1[1][0]), poi1[1][1], poi1[1][2],
         int(poi2[1][0]), poi2[1][1], poi2[1][2],
      )
      
      # initialize to maxNLL
      for i in range( (nllHist.GetNbinsX()+2)*(nllHist.GetNbinsY()+2) ): nllHist.SetBinContent( i, maxHist )

      for nll,p1,p2 in zip(NLL['nll'],NLL[poi1[0]],NLL[poi2[0]]):
         bin,val = (None,None)
         bin = nllHist.FindBin( p1,p2 )
         val = nll
         if options.subtractMinNLL: val -= minNLL
         if nllHist.GetBinContent( bin ) > val: nllHist.SetBinContent( bin, val )
      
   if not nllHist:
      print( "ERROR: Couldn't create nll histogram." )
      return
   
      
   # 2d debug histos
   histos2d = {}
   # change the names below for your model
   params2d = [(POIs[0][0],"nuis1","nuis2"),(POIs[0][0],"nuis1","nuis3")]
   for poi,nuis1,nuis2 in params2d:
      nu1 = [ n for n in NUISs if nuis1==n[0] ]
      nu2 = [ n for n in NUISs if nuis2==n[0] ]
      if len( nu1 ) != 1   or   len( nu2 ) != 1: continue
      nu1 = nu1[0]
      nu2 = nu2[0]
      h = ROOT.TH2D( 
         poi+"_"+nuis1+"_"+nuis2, poi+"_"+nuis1+"_"+nuis2, 
         int(nu1[1][0]), nu1[1][1], nu1[1][2],
         int(nu2[1][0]), nu2[1][1], nu2[1][2],
      )
      for poiVal,n1,n2 in zip( NLL[poi], NLL[nuis1], NLL[nuis2] ):
         h.SetBinContent( h.FindBin( n1,n2 ), poiVal )
      histos2d[ h.GetName() ] = h

   # create tgraphs
   nllTGraphs = {}
   nuisParGraphs = {}
   for poi in POIs:
      pn = [ (p,n) for p,n in zip(NLL[poi[0]], NLL['nll']) if n < minNLL+100.0 ]
      nllTGraph = PyROOTUtils.Graph( pn )
      if options.subtractMinNLL: nllTGraph.add( -minNLL )
      nllTGraphs[poi[0]] = nllTGraph

      for nuis in NUISs:
         g = PyROOTUtils.Graph( NLL[poi[0]], NLL[nuis[0]] )
         nuisParGraphs[poi[0]+"_vs_"+nuis[0]] = g
   
   
   bestFitMarker = None
   if len( POIs ) == 1  and  POIs[0][0] in bestFit:
      bestFitMarker = ROOT.TMarker( bestFit[ POIs[0][0] ], 0.0, 2 )
   elif len( POIs ) >= 2  and  POIs[0][0] in bestFit  and  POIs[1][0] in bestFit:
      bestFitMarker = ROOT.TMarker( bestFit[ POIs[0][0] ], bestFit[ POIs[1][0] ], 2 )
      
      
   f = ROOT.TFile( options.outputFile, "RECREATE" )
   nllHist.Write()
   for p,g in nllTGraphs.iteritems():
      if g: g.Write( "nll_"+p )
   for p,g in nuisParGraphs.iteritems():
      if g: g.Write( "nuisParGraph_"+p )
   for h in histos2d.values():
      h.Write()
   if bestFitMarker: bestFitMarker.Write("bestFit")
   f.Close()
   
   if options.verbose:
      import helperStyle
      canvas = ROOT.TCanvas( "verboseOutput", "verbose output", 600,300 )
      canvas.Divide( 2 )
      canvas.cd(1)
      nllHist.SetLineWidth( 2 )
      nllHist.SetLineColor( ROOT.kBlue )
      nllHist.Draw("HIST")
      if nllTGraph: 
         nllTGraph.SetLineWidth( 2 )
         nllTGraph.SetLineColor( ROOT.kRed )
         nllTGraph.Draw("SAME")
      canvas.cd(2)
      canvas.SaveAs( "docImages/batchProfileLikelihood1D.png" )
      canvas.Update()
      raw_input( "Press enter to continue ..." )
   

   
   
if __name__ == "__main__":
   main()