# Toys Studies

### Detailed Output

When you have a `ProfileLikelihoodTestSat` called plts, activate the detailed output like this:

```
   plts->EnableDetailedOutput( true );
```

This will create additional outputs stored in the `HypoTestResult` htr and can be retrieved using `htr->GetNullDetailedOutput()` and `htr->GetAltDetailedOutput()`. This detailed output is stored in a `RooDataSet`. One can use the standard filters to extract single columns and correlation matrices.

### Multiple Test Statistics

If you have a second test statistic, say an instance of `SimpleLikelihoodTestStat` called slts, you can get its output for the exact same toy by adding it to the `ToyMCSampler` as a second test statistic. 

```
   toymcs->AddTestStatistic( slrts );
```

This is sometimes useful for debugging, as this test statistic does not do any fits, but still says something about the "signal-likeness" of this toy.

### Importance Sampling

`ToyMCImportanceSampler` is a `ToyMCSampler` that can be an in-place replacement for a ToyMCSampler. However, it needs a few more configuration steps in most use cases.

### Plotting Detailed Output

After extracting the detailed output from the `HypoTestResult` htr using for example 

```
fullResult = htr.GetNullDetailedOutput()
```
The parameter distributions can be plotted with something like this:

```
h1 = ROOT.TH1F( 
   "oneVar_%s" % p.GetName(), p.GetTitle(), 
   50, -10, 10
)
fullResult.fillHistogram( 
   h1, 
   ROOT.RooArgList(p,"tmpList")
)
h1.Draw( "HIST" )
```

Similarly, correlation and 2D plots can be extracted from a `RooDataSet`. Please have a look at `tutorials/roostats/py/plotHypoTestResult.py` for examples of making plots.