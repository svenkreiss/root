#  Created on: Jan 27, 2012

__author__ = "Sven Kreiss <sk@svenkreiss.com>"
__version__ = "0.1"



import os,re,glob,optparse


parser = optparse.OptionParser(usage="%prog", version="%prog 0.1")
parser.add_option("-i", "--input", help="root file with impsampl", dest="input", default="ToysOutput.root" )
parser.add_option("-w", "--workspace", help="workspace name", dest="workspace", default="ToysOutput" )
parser.add_option("-r", "--hypoTestResult", help="hypoTestResult name", dest="hypoTestResult", default="HypoTestCalculator_result" )
parser.add_option("-o", "--output", help="pdf file with impsampl output", dest="output", default="ToysOutput/" )

parser.add_option(      "--fits", help="an output for minos.py", dest="fits", default="minos.root" )

parser.add_option(      "--ymin", help="p value low y range", type="float", dest="ymin", default=5e-8)
parser.add_option(      "--ymax", help="p value max y range", type="float", dest="ymax", default=50)
parser.add_option(      "--xmin", help="xmin", type="float", dest="xmin", default=-1.5)
parser.add_option(      "--xmax", help="xmax", type="float", dest="xmax", default=13.5)
parser.add_option(      "--bins", help="bins for sampling distribution plots", type="int", dest="bins", default=60)
parser.add_option(      "--dof", help="Specify degrees-of-freedom for asym distributions.", type="int", dest="dof", default=1)
parser.add_option(      "--twoSided", help="If the test is two sided. Default is one sided.", dest="twoSided", default=False, action="store_true")
parser.add_option("-q", "--quiet", action="store_false", dest="verbose", default=True,
                  help="don't print status messages to stdout")
options, args = parser.parse_args()


os.system( "mkdir -p "+options.output )
os.system( "rm "+options.output+"*.eps" )



import ROOT
import AtlasStyle
import AtlasUtil

ROOT.gROOT.SetBatch( True )
ROOT.gStyle.SetPalette(1)




class HtrPlotMaker:
   def __init__( self, filename, wName, htrName ):
      files = glob.glob( filename )
      self.htr = None
      for fName in files:
         print( "Opening "+fName )
         f = ROOT.TFile.Open( fName )
         w = f.Get( wName )
         if self.htr: self.htr.Append( w.obj(htrName) )
         else:
            self.htr = ROOT.RooStats.HypoTestResult( w.obj(htrName) )
            self.htr.SetName( "HypoTestResult" )
         f.Close()
      
   def drawHtr( self ):
      htr = self.htr #self.ws.obj( htrName )
      #print( "drawing HypoTestResult %s" % htrName )
      htr.Print()
      nullWeights = htr.GetNullDistribution().GetSampleWeights()
      
      print( "detailedOutput for null: "+str( htr.GetNullDetailedOutput() ) )
      print( "detailedOutput for alt: "+str( htr.GetAltDetailedOutput() ) )
      
      c = ROOT.TCanvas( "c1","c1", 1*800, 1*600 )
      c2D = ROOT.TCanvas( "c2","c2", 1*800, 1*600 )
      c.cd()

      # look at full output
      fullResult = htr.GetNullDetailedOutput()
      fullResult.Print("v")
   
      l = ROOT.RooArgList( fullResult.get() )
      print( "variables in detailed data set: " )
      l.Print("v")

      
      # sampling distributions first      
      plot = ROOT.RooStats.HypoTestPlot( htr, options.bins, options.xmin+1e-3, options.xmax+1e-3 )
      plot.SetLogYaxis( True )
      plot.SetYRange( 1e-5,5 )
      plot.SetAxisTitle( "q_{0}/2" )
      plot.SetLineColor( ROOT.kGray )
      plot.SetLineWidth( 5 )
      # add overflows
      h = plot.GetTH1F()
      h.Fill( h.GetBinCenter(1), h.GetBinContent(0) )
      h.Fill( h.GetBinCenter(options.bins), h.GetBinContent(options.bins+1) )
      
      
      hAll = ROOT.TH1F( "hAll","hAll", options.bins, options.xmin+1e-3, options.xmax+1e-3 )
      fullResult.fillHistogram( hAll, ROOT.RooArgList(l.at(0),"tmpList") )
      normalization = hAll.Integral( "width" )

      if "densityLabel" in fullResult.get():
         densityLabels = fullResult.get()["densityLabel"]
         for i in range( -1, 100 ):
            if densityLabels.setIndex( i ): break
            densName = densityLabels.getLabel()
            
            hNull = ROOT.TH1F( densName,densName, options.bins, options.xmin+1e-3, options.xmax+1e-3 )
            fullResult.fillHistogram( hNull, ROOT.RooArgList(l.at(0),"tmpList"), "densityLabel == densityLabel::%s" % densName )
            hNull.Scale( 1./normalization )
      
            hNull.SetLineColor( i+3 )
            plot.AddTH1( hNull, "HIST SAME" )
      
      nPOI = options.dof
      if not options.twoSided:
         f = ROOT.TF1("f", "1*ROOT::Math::chisquared_pdf(2*x,%d,0)" % nPOI,0,20)
         f.SetLineColor( ROOT.kBlack )
         f.SetLineStyle( 7 )
         plot.AddTF1( f, "#chi^{2}(2x,%d): \"half-chisquared\"" % nPOI )
      else:
         f2 = ROOT.TF1("f2", "2*ROOT::Math::chisquared_pdf(2*x,%d,0)" % nPOI,0,20)
         f2.SetLineColor( ROOT.kBlack )
         f2.SetLineStyle( 2 )
         plot.AddTF1( f2, "2#chi^{2}(2x,%d): \"chisquared\"" % nPOI )

      plot.Draw()
      c.SaveAs( options.output+"overview.eps" )

      # same plot, but much cleaner      
      plotClean = ROOT.RooStats.HypoTestPlot( htr, options.bins, options.xmin+1e-3, options.xmax+1e-3 )
      plotClean.SetLogYaxis( True )
      plotClean.SetYRange( 1e-5,5 )
      plotClean.SetAxisTitle( "q_{0}/2" )
      #plotClean.SetLineColor( ROOT.kGray )
      #plotClean.SetLineWidth( 5 )
      if not options.twoSided: plotClean.AddTF1( f, "#chi^{2}(2x,%d): \"half-chisquared\"" % nPOI )
      else:                    plotClean.AddTF1( f2, "2#chi^{2}(2x,%d): \"chisquared\"" % nPOI )
      plotClean.Draw()
      c.SaveAs( options.output+"overview_clean.eps" )

      
      
      # correlation
      cCorr = ROOT.TCanvas( "cCorr","cCorr", 1*800, 1*600 )
      cCorr.cd().SetLogy( False )
      ROOT.gPad.SetRightMargin( 0.15 )
      ROOT.gPad.SetLeftMargin( 0.20 )
      ROOT.gPad.SetBottomMargin( 0.25 )
      # determine variables
      allCorrVars = {}
      for v in range( l.getSize() ):
         name = l.at(v).GetName()
         if name.count('_') > 1:
            ts = name[:name.find('_')+4]
            if ts in allCorrVars.keys(): allCorrVars[ts].append( name )
            else: allCorrVars[ts] = [ name ]
      print( "allCorrVars: " )
      print( allCorrVars )
      
      for k,v in allCorrVars.iteritems():
         corrVars = ROOT.RooArgList( "corrVars" )
         for i in range( l.getSize() ):
            if l.at(i).GetName() in v:
               print( "adding to correlation list: %s" % l.at(i).GetName() ) 
               corrVars.add( l.at(i) )
         corrMatrix = fullResult.correlationMatrix( corrVars )
         corrMatrix.Print()
         corrHist = ROOT.TH2D( corrMatrix )
         corrHist.GetZaxis().SetRangeUser( -1,1 )
         corrHist.SetLabelSize( 0.02, "x" )
         corrHist.SetLabelSize( 0.02, "y" )
         corrHist.SetLabelSize( 0.02, "z" )
         for i in range( corrVars.getSize() ):
            corrHist.GetXaxis().SetBinLabel( i+1, corrVars.at(i).GetTitle() )
            corrHist.GetYaxis().SetBinLabel( i+1, corrVars.at(i).GetTitle() )
         corrHist.Draw( "COLZ" )
         corrHist.LabelsOption( "v", "x" )
         corrHist.LabelsOption( "h", "y" )
         corrHist.Draw( "COLZ" )
         cCorr.SaveAs( options.output+"corrWithTS_"+corrVars.at(i).GetName()+".eps" )
      
      
      # first the 2D comparisons of the TS values
      for i in range( l.getSize() ):
         # Check whether this is a test statistic value.
         # If so, compare with the first test statistic.
         if l.at(i).GetName().count('_') == 1:
            if i == 0: continue

            lowest1, highest1 = (  ROOT.Double(-1), ROOT.Double(1)  )
            fullResult.getRange( l.at(0), lowest1, highest1 )
            lowest2, highest2 = (  ROOT.Double(-1), ROOT.Double(1)  )
            fullResult.getRange( l.at(i), lowest2, highest2 )
            
            if highest1 <= lowest1+1:
               highest1 += 1
               lowest1 -= 1
            if highest2 <= lowest2+1:
               highest2 += 1
               lowest2 -= 1
                        
            l.at(0).setRange( lowest1, highest1 )
            l.at(i).setRange( lowest2, highest2 )
            
            #l.at(0).setRange( options.xmin+1e-3, options.xmax+1e-3 )
            #l.at(i).setRange( -10, 20 )


            # first 1D
            c.cd()
            print( "(%f, %f)" % (lowest2,highest2) )
            plot = ROOT.RooStats.SamplingDistPlot( options.bins, lowest2, highest2 )
            plot.AddSamplingDistribution( ROOT.RooStats.SamplingDistribution( l.at(i).GetName(),l.at(i).GetName(), fullResult, l.at(i).GetName() ) )
            plot.SetLineColor( ROOT.kRed )
            plot.SetLogYaxis( True )
            plot.SetYRange( 1e-5,5 )
            plot.SetAxisTitle( l.at(i).GetTitle() )
            # add overflows
            h = plot.GetTH1F()
            h.Fill( h.GetBinCenter(1), h.GetBinContent(0) )
            h.Fill( h.GetBinCenter(options.bins), h.GetBinContent(options.bins+1) )
            plot.Draw()
            c.SaveAs( options.output+"tsDistribution_"+l.at(i).GetName()+".eps" )

            # now 2D comparison
            c2D.cd().SetLogz( True )
            ROOT.gPad.SetRightMargin( 0.15 )
            
            h1 = fullResult.createHistogram( l.at(0), l.at(i), options.bins, options.bins )
            h1.GetXaxis().SetTitle( l.at(0).GetTitle() )
            h1.GetYaxis().SetTitle( l.at(i).GetTitle() )
            h1.GetZaxis().SetRangeUser( 5e-8, 15 )
            # add overflows
            for x in range( 1, options.bins+1 ):
               h1.SetBinContent( x, 1, h1.GetBinContent(x,1)+h1.GetBinContent(x,0) )
               h1.SetBinContent( x, options.bins, h1.GetBinContent(x,options.bins)+h1.GetBinContent(x,options.bins+1) )
            for y in range( 1, options.bins+1 ):
               h1.SetBinContent( 1, y, h1.GetBinContent(1,y)+h1.GetBinContent(0,y) )
               h1.SetBinContent( options.bins, y, h1.GetBinContent(options.bins,y)+h1.GetBinContent(options.bins+1,y) )
            h1.Draw( "COLZ" )
            c2D.SaveAs( options.output+"comparison_"+l.at(0).GetName()+"_"+l.at(i).GetName()+".eps" )


      # now get the parameter distributions         
      for i in range( l.getSize() ):
         if l.at(i).GetName().count('_') > 1:
            c.cd()
            ROOT.gPad.SetRightMargin( 0.15 ) # in case there are powers of the x-axis values (as for NLL)
            lowest, highest = (  ROOT.Double(-1), ROOT.Double(1)  )
            fullResult.getRange( l.at(i), lowest, highest )
            # normal
            h1 = ROOT.TH1F( "oneVar_%s" % l.at(i).GetName(), l.at(i).GetTitle(), 50, lowest, highest )
            fullResult.fillHistogram( h1, ROOT.RooArgList(l.at(i),"tmpList") )
            h1.GetXaxis().SetTitle( l.at(i).GetTitle() )
            h1.SetMinimum( 0.7 )
            h1.Draw( "HIST" )
            # weird SLRTS
#             h2 = ROOT.TH1F( "oneVar_%s" % l.at(i).GetName(), l.at(i).GetTitle(), 50, lowest, highest )
#             fullResult.fillHistogram( h2, ROOT.RooArgList(l.at(i),"tmpList"), "ModelConfigNull_TS1 < 0.0" )
#             h2.SetLineColor( ROOT.kRed )
#             h2.Draw( "HIST SAME" )
            c.SaveAs( options.output+"param_"+l.at(i).GetName()+".eps" )
         
      c.Clear()
      #c.SaveAs( "%s)" % options.output )

      print( "Merging eps files" )
      fileList = "`\ls "+options.output+"*.eps`"
      os.system( "gs -q -dNOPAUSE -dBATCH -sEPSCrop -sDEVICE=pdfwrite -sOutputFile="+options.output+"merged.pdf "+fileList )
      print( "Creating zip file" )
      os.system( "rm "+options.output+"merged.zip" )
      os.system( "zip -r "+options.output+"merged.zip "+fileList )
      

if __name__ == "__main__":
   p = HtrPlotMaker( options.input, options.workspace, options.hypoTestResult )
   p.drawHtr()