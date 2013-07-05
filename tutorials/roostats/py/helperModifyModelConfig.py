
import ROOT

def addOptionsToOptParse( parser ):
   # standard options for ModelConfig
   parser.add_option("-i", "--input", help="root file", type="string", dest="input", default="results/example_combined_GaussExample_model.root")
   parser.add_option("-w", "--wsName", help="Workspace name", type="string", dest="wsName", default="combined")
   parser.add_option("-m", "--mcName", help="ModelConfig name", type="string", dest="mcName", default="ModelConfig")
   parser.add_option("-d", "--dataName", help="data name", type="string", dest="dataName", default="obsData")
   
   # for example for asimov data runs
   parser.add_option(      "--loadSnapshots", help="loads this comma separated list of snapshots", dest="loadSnapshots", default=None )

   # for modifications
   parser.add_option(      "--overwritePOI", help="Force to take comma separated list of parameters with value for poi. Example: \"mu=1,mH=125\" will make these two the poi.", dest="overwritePOI", default=False )
   parser.add_option(      "--overwriteRange", help="Overwrite range. Example: \"mu=[-5:10],mH=[120:130]\".", dest="overwriteRange", default=False )
   parser.add_option(      "--overwriteBins", help="Overwrite bins. Example: \"mu=5,mH=100\".", dest="overwriteBins", default=False )



def varsDictFromString( str ):
   """ Helper function to make a dictionary from an options string. """
   
   vars = str.split(",")
   vars = dict(   (v.split("=")[0].strip(), v.split("=")[1]) for v in vars   )
   for name,valErr in vars.iteritems():
      if "+/-" in valErr:
         vars[ name ] = (   float( valErr.split("+/-")[0] ),float( valErr.split("+/-")[1] )   )
      else:
         vars[ name ] = (   float( valErr ), None    )
   return vars


def apply( options, w, mc ):
   """ Todo: use varsDictFromString() here. """

   if options.overwriteRange:
      parAndRange = options.overwriteRange.split(",")
      for pr in parAndRange:
         p,r = pr.split("=")
         r = r.split(":")
         rMin = float( r[0][1:] )
         rMax = float( r[1][:-1] )
         
         print( "Setting range for "+p+"=["+str(rMin)+","+str(rMax)+"]" )
         w.var( p ).setRange( rMin, rMax )

   if options.overwriteBins:
      parAndBins = options.overwriteBins.split(",")
      for pb in parAndBins:
         p,b = pb.split("=")
         print( "Setting number of bins for "+p+"="+str(b) )
         w.var( p ).setBins( int(b) )

   if options.overwritePOI:
      print( "" )
      print( "=== Using given set for POI ===" )
      poiAndValue = {}
      pvs = options.overwritePOI.split(",")
      for pv in pvs:
         name,value = pv.split("=")
         poiAndValue[ name ] = float( value )
      
      remove = []
      poiL = ROOT.RooArgList( mc.GetParametersOfInterest() )
      for p in range( poiL.getSize() ):
         name = poiL.at(p).GetName()
         if name not in poiAndValue.keys():
            print( "Adding "+name+"["+str(poiL.at(p).getMin())+","+str(poiL.at(p).getMax())+"] to nuisance parameters." )
            w.set( mc.GetName()+"_NuisParams" ).add( poiL.at(p) )
            remove.append( name )
#          else:
#             print( "Setting value of POI "+name+"="+str(poiAndValue[name])+"." )

      for r in remove: poiL.remove( w.var(r) )
            
      for p,v in poiAndValue.iteritems():
         if not poiL.contains( w.var(p) ):
            print( "Adding "+p+"="+str(v)+" to POI." )
            poiL.add( w.var(p) )
            w.var(p).setVal( v )
         else:
            print( "Setting value of POI "+p+"="+str(v)+"." )
            w.var(p).setVal( v )
         
         if mc.GetNuisanceParameters().contains( w.var(p) ):
            print( "Removing "+w.var(p).GetName()+" from the list of nuisance parameters." )
            mc.GetNuisanceParameters().remove( w.var(p) )
            
      print( "Setting new POI and new snapshot for ModelConfig: " )
      mc.SetParametersOfInterest( ROOT.RooArgSet(poiL) )
      mc.SetSnapshot( ROOT.RooArgSet(poiL) )
      mc.GetSnapshot().Print("V")
      print( "" )
      print( "" )
      
   if options.loadSnapshots:
      sn = options.loadSnapshots.split(",")
      for s in sn:
         print( "Loading snapshot "+s+" ..." )
         w.loadSnapshot( s )
         print( "done." )
      
      

