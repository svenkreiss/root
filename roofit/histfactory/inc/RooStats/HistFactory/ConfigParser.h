// @(#)root/roostats:$Id$
// Author: Kyle Cranmer, Akira Shibata
/*************************************************************************
 * Copyright (C) 1995-2008, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/


#ifndef ROOSTATS_CONFIGPARSER_h
#define ROOSTATS_CONFIGPARSER_h


#include <cstdlib>
#include <string>
#include <TXMLNode.h>

#include "TList.h"
#include "TFile.h"
#include "TXMLAttr.h"

#include "RooStats/HistFactory/Channel.h"
#include "RooStats/HistFactory/Measurement.h"
#include "RooStats/HistFactory/Sample.h"


namespace RooStats{
   namespace HistFactory {

     class ConfigParser {

     public:

       // The "main" method
       virtual std::vector< RooStats::HistFactory::Measurement > GetMeasurementsFromXML(std::string input); 
       virtual RooStats::HistFactory::Measurement CreateMeasurementFromDriverNode( TXMLNode* node );
       virtual RooStats::HistFactory::Channel ParseChannelXMLFile( std::string filen );

       // Helpers used to process a channel
       HistFactory::Data CreateDataElement( TXMLNode* node );
       HistFactory::Sample CreateSampleElement( TXMLNode* node );
       HistFactory::StatErrorConfig CreateStatErrorConfigElement( TXMLNode* node );

       // Helpers used when processing a Sample
       HistFactory::NormFactor  MakeNormFactor( TXMLNode* node );
       HistFactory::HistoSys    MakeHistoSys( TXMLNode* node );
       HistFactory::HistoFactor MakeHistoFactor( TXMLNode* node );
       HistFactory::OverallSys  MakeOverallSys( TXMLNode* node );
       HistFactory::ShapeFactor MakeShapeFactor( TXMLNode* node );
       HistFactory::ShapeSys    MakeShapeSys( TXMLNode* node );
       HistFactory::StatError   ActivateStatError( TXMLNode* node );
       HistFactory::PreprocessFunction ParseFunctionConfig( TXMLNode* functionNode );

     protected:

       bool CheckTrueFalse( std::string val, std::string Name );
       bool IsAcceptableNode( TXMLNode* functionNode );

       // To facilitate writing xml, when not
       // specified, files and paths default
       // to these cached values
       std::string m_currentInputFile;
       std::string m_currentChannel;
       std::string m_currentHistoPath;

     };
   }
}

#endif
