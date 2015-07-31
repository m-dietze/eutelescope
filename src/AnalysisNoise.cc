#ifdef USE_GEAR
#include "EUTELESCOPE.h"
#include "EUTelTrackerDataInterfacerImpl.h"
#include "EUTelGenericSparsePixel.h"
#include "AnalysisNoise.h"

#include "marlin/Global.h"

using namespace lcio;
using namespace marlin;
using namespace std;
using namespace eutelescope;
using namespace gear;

AnalysisNoise aAnalysisNoise;

AnalysisNoise::AnalysisNoise()
: Processor("AnalysisNoise"),
  _zsDataCollectionName(""),
  _fillHistos(false),
  _nEvent(0),
  _nFiredPixel(),
  _nLayer(0),
  _xPixel(),
  _yPixel(),
  _siPlanesParameters(0),
  _siPlanesLayerLayout(0),
  _energy(6.0),
  _dim4Sec(0),
  _chipID(),
  _irradiation(),
  _dutIDs(),
  _rate(""),
  _outputSettingsFolderName("./")
  {
    _description="Ananlysis of noise runs";
    registerInputCollection (LCIO::TRACKERDATA, "ZSDataCollectionName",
                           "Input of Zero Suppressed data",
                           _zsDataCollectionName, string ("zsdata") );
    registerProcessorParameter("HistogramFilling","Switch on or off the histogram filling",                     _fillHistos, static_cast< bool > ( true ) );
    registerOptionalParameter("Energy","Particle energy",
                            _energy, static_cast< double > ( 6.0 ) );
    EVENT::StringVec _stringVecExample;
   _stringVecExample.push_back(" ");
    registerOptionalParameter("ChipID","Chip IDs",
                            _chipID, _stringVecExample );
    registerOptionalParameter("Irradiation","Irradiation level",
                            _irradiation, _stringVecExample );
    registerOptionalParameter("Rate","Data taking rate",
                            _rate, static_cast< string > ( "" ) );
    registerOptionalParameter("dutIDs","DUT IDs",
                            _dutIDs, _stringVecExample );
    registerOptionalParameter("OutputSettingsFolderName","Folder name where all the settings of each run will be saved",
                            _outputSettingsFolderName, static_cast< string > ( "./" ) );
    registerOptionalParameter("Dim4Sec","Size Sector 4 (pixel)",
                            _dim4Sec, static_cast< int > (0 ) );
    _isFirstEvent = true;
  }

void AnalysisNoise::init() {

#ifndef USE_GEAR
  streamlog_out ( ERROR4 ) <<  "Marlin was not built with GEAR support." << endl <<  "You need to install GEAR and recompile Marlin with -DUSE_GEAR before continue." << endl;
  exit(-1);
#else
  if ( Global::GEAR == 0x0 ) {
    streamlog_out ( ERROR4 ) <<  "The GearMgr is not available, for an unknown reason." << endl;
    exit(-1);
  }                                                                                                  _siPlanesParameters  = const_cast<SiPlanesParameters* > (&(Global::GEAR->getSiPlanesParameters())); 
  _siPlanesLayerLayout = const_cast<SiPlanesLayerLayout*> ( &(_siPlanesParameters->getSiPlanesLayerLayout() ));
#endif
  _nLayer = _siPlanesLayerLayout->getNLayers();
  vector<int> tmp(5,0);
  for (int iLayer=0; iLayer<_nLayer; iLayer++ )
  {
    _xPixel.push_back(_siPlanesLayerLayout->getSensitiveNpixelX(iLayer));
    _yPixel.push_back(_siPlanesLayerLayout->getSensitiveNpixelY(iLayer));
    _nFiredPixel.push_back(tmp);
//    cerr << iLayer << "\t" << nFiredPixel[0][iLayer] << endl;
  }
  for (unsigned int i=0; i<_dutIDs.size(); i++)
  {
    bool newFile = false;
    string _outputSettingsFileName = _outputSettingsFolderName + "settings_DUT" + _dutIDs[i] + ".txt";
    if (!std::ifstream(_outputSettingsFileName.c_str()))
      newFile = true;
    settingsFile[i].open (_outputSettingsFileName.c_str(), ios::out | ios::app );
    if (newFile) settingsFile[i] << "Run number;Energy;Chip ID;Irradiation level(0-nonIrradiated,1-2.5e12,2-1e13,3-700krad,4-combined:1e13+700krad);Rate;BB;Ithr;Idb;Vcasn;Vaux;Vcasp;Vreset;Threshold and their RMS for all four sectors;Noise and their RMS for all four sectors;Readout delay;Trigger delay;Strobe length;StrobeB length;Data (1) or noise (0);Number of events;Efficiency,Number of tracks,Number of tracks with associated hit for all sectors" << endl;
  }
}

void AnalysisNoise::processEvent(LCEvent *evt)
{
//  cerr << evt->getEventNumber() << endl;
#if defined(USE_AIDA) || defined(MARLIN_USE_AIDA)
  if (_isFirstEvent )
  {
    for (unsigned int i=0; i< _dutIDs.size(); i++)
    {
      int dutID = atoi(_dutIDs[i].c_str());
      settingsFile[i] << evt->getRunNumber() << ";" << _energy << ";" << _chipID[dutID] << ";" << _irradiation[dutID] << ";" << _rate << ";" << evt->getParameters().getFloatVal("BackBiasVoltage") << ";" << evt->getParameters().getIntVal(Form("Ithr_%d",dutID)) << ";" << evt->getParameters().getIntVal(Form("Idb_%d",dutID)) << ";" << evt->getParameters().getIntVal(Form("Vcasn_%d",dutID)) << ";" << evt->getParameters().getIntVal(Form("Vaux_%d",dutID)) << ";" << evt->getParameters().getIntVal(Form("Vcasp_%d",dutID)) << ";" << evt->getParameters().getIntVal(Form("Vreset_%d",dutID)) << ";";
      for (int iSector=0; iSector<5; iSector++)
        settingsFile[i] << evt->getParameters().getFloatVal(Form("Thr_%d_%d",dutID,iSector)) << ";" << evt->getParameters().getFloatVal(Form("ThrRMS_%d_%d",dutID,iSector)) << ";";
      for (int iSector=0; iSector<5; iSector++)
        settingsFile[i] << evt->getParameters().getFloatVal(Form("Noise_%d_%d",dutID,iSector)) << ";" << evt->getParameters().getFloatVal(Form("NoiseRMS_%d_%d",dutID,iSector)) << ";";
      settingsFile[i] << evt->getParameters().getIntVal(Form("m_readout_delay_%d",dutID)) << ";" << evt->getParameters().getIntVal(Form("m_trigger_delay_%d",dutID)) << ";" << evt->getParameters().getIntVal(Form("m_strobe_length_%d",dutID)) << ";" << evt->getParameters().getIntVal(Form("m_strobeb_length_%d",dutID)) << ";0;";
    }
    if (_fillHistos)
      bookHistos();
    _isFirstEvent = false;
  }
  timeStampHisto->Fill(evt->getTimeStamp());
#endif
  try
  { 
    zsInputDataCollectionVec = dynamic_cast< LCCollectionVec * > ( evt->getCollection( _zsDataCollectionName ) ) ;
  } catch ( lcio::DataNotAvailableException ) 
  {
    cerr << "In event " << evt->getEventNumber() << "_zsDataCollectionName " << _zsDataCollectionName.c_str() << " not found " << endl;
    return;
  }
  _nEvent++;
  for ( unsigned int iDetector = 0 ; iDetector < zsInputDataCollectionVec->size(); iDetector++ )
  {
    TrackerDataImpl * zsData = dynamic_cast< TrackerDataImpl * > ( zsInputDataCollectionVec->getElementAt( iDetector ) );
    auto_ptr<EUTelTrackerDataInterfacerImpl<EUTelGenericSparsePixel > >  sparseData(new EUTelTrackerDataInterfacerImpl<EUTelGenericSparsePixel> ( zsData ));
    for ( unsigned int iPixel = 0; iPixel < sparseData->size(); iPixel++ )
    {
      EUTelGenericSparsePixel *sparsePixel =  new EUTelGenericSparsePixel() ; 
      sparseData->getSparsePixelAt( iPixel, sparsePixel );
      noiseMap[iDetector]->Fill(sparsePixel->getXCoord(),sparsePixel->getYCoord());
      for (int iSector=0; iSector<4; iSector++)
        if (sparsePixel->getXCoord() >= iSector*_xPixel[iDetector]/4 && sparsePixel->getXCoord() < (iSector+1)*_xPixel[iDetector]/4 )
        {
          if(iSector != 0) _nFiredPixel[iDetector][iSector]++;
          else 
          {
            if(sparsePixel->getXCoord() < _dim4Sec ) _nFiredPixel[iDetector][4]++;
            else _nFiredPixel[iDetector][0]++;
          }
        }
      delete sparsePixel;
    }
  }
}

void AnalysisNoise::bookHistos()
{
  timeStampHisto = new TH1I("timeStampHisto","Distribution of the time stamp of the events; Time stamp (in 12.5 ns units)",1000,0,50000);
  for (int iLayer=0; iLayer<_nLayer; iLayer++ )
  {
    noiseMap[iLayer] = new TH2I(Form("noiseMap_%d",iLayer),Form("Noise map of layer %d",iLayer),_xPixel[iLayer],0,_xPixel[iLayer],_yPixel[iLayer],0,_yPixel[iLayer]);
    noiseOccupancy[iLayer] = new TH1F(Form("noiseOccupancy_%d",iLayer),Form("Noise occupancy in layer %d",iLayer),5,0,5);
  }
}

void AnalysisNoise::end()
{
  cout << "Total number of events: " << _nEvent << endl;
  for (unsigned int i=0; i< _dutIDs.size(); i++)
    settingsFile[i] << _nEvent << ";0;0;0;0;0;0;0;0;0;0;0;0" << endl;
  for (int iLayer=0; iLayer<_nLayer; iLayer++ )
  {
    for (int iSector=0; iSector<5; iSector++)
    {
      if (iSector == 0)
      {
        noiseOccupancy[iLayer]->SetBinContent(iSector+1,(double)_nFiredPixel[iLayer][iSector]/_nEvent/((_xPixel[iLayer]/4-_dim4Sec)*_yPixel[iLayer]));
        noiseOccupancy[iLayer]->SetBinError(iSector+1,sqrt((double)_nFiredPixel[iLayer][iSector])/_nEvent/((_xPixel[iLayer]/4-_dim4Sec)*_yPixel[iLayer]));
      }
      else if (iSector == 4)
      {
        if(_dim4Sec != 0)
        {
          noiseOccupancy[iLayer]->SetBinContent(iSector+1,(double)_nFiredPixel[iLayer][iSector]/_nEvent/(_dim4Sec*_yPixel[iLayer]));
          noiseOccupancy[iLayer]->SetBinError(iSector+1,sqrt((double)_nFiredPixel[iLayer][iSector])/_nEvent/(_dim4Sec*_yPixel[iLayer]));
        }
        else continue;
      }
      else
      {
        noiseOccupancy[iLayer]->SetBinContent(iSector+1,(double)_nFiredPixel[iLayer][iSector]/_nEvent/(_xPixel[iLayer]/4*_yPixel[iLayer]));
        noiseOccupancy[iLayer]->SetBinError(iSector+1,sqrt((double)_nFiredPixel[iLayer][iSector])/_nEvent/(_xPixel[iLayer]/4*_yPixel[iLayer]));
      }
    }
  }
}
#endif // GEAR
