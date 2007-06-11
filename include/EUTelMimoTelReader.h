// -*- mode: c++; mode: auto-fill; mode: flyspell-prog; -*-
/*
 *   This source code is part of the Eutelescope package of Marlin.
 *   You are free to use this source files for your own development as
 *   long as it stays in a public research context. You are not
 *   allowed to use it for commercial purpose. You must put this
 *   header with author names in all development based on this file.
 *
 */

#ifdef USE_EUDAQ
#ifndef EUTELMIMOTELREADER_H
#define EUTELMIMOTELREADER_H

// personal includes ".h"

// marlin includes ".h"
#include "marlin/DataSourceProcessor.h"

// eudaq includes <.h>

// lcio includes <.h>

// system includes <>
#include <string>

namespace eutelescope {

  //!  Reads the data produced by the EUDRB board with a MIMOTEL sensor
  /*!  This Marlin reader is taking as an input the output file of
   *   the eudaq software and converting it to a LCIO event. This is
   *   linking against libeudaq and using directly the data structure
   *   used in the DAQ software. This has to be thought as a sort of
   *   link between the native DAQ raw format and the LCIO data model
   *   used for the telescope data description.
   *
   *   The main goal of this data reader is to test the quality of
   *   the data output from the hardware but it is going to disappear
   *   soon being the LCIO output already produced by the online
   *   system.
   *
   *   To link against the libeudaq libraries you have to download
   *   and install the library on your computer and then add the
   *   following few lines to the @c userlib.gmk in the Marlin top
   *   folder.
   *
   *   @code
   *   #--------------------------------------------------------------------------------
   *   #     EUDAQ
   *   #--------------------------------------------------------------------------------
   *   USERLIBS     += -L/path/to/eudaq/main -leudaq
   *   USERINCLUDES += -DUSE_EUDAQ -DEUDAQ_FUNC=__PRETTY_FUNCTION__ 
   *   USERINCLUDES += -DEUDAQ_PLATFORM=PF_LINUX -I/path/to/eudaq/main/include
   *   @endcode
   *   
   *   To execute Marlin then you need to add the libeudaq.so to the
   *   shared libraries path, for example, using the LD_LIBRARY_PATH
   *   environmental variable.
   *
   *   The data decoding is done using the decoder and classed
   *   provided by the DAQ software, so please refer to that
   *   documentation for more info.
   *
   *   @author  Antonio Bulgheroni, INFN <mailto:antonio.bulgheroni@gmail.com>
   *   @version $Id: EUTelMimoTelReader.h,v 1.1 2007-06-11 22:19:32 bulgheroni Exp $
   *
   */
  
  class EUTelMimoTelReader : public marlin::DataSourceProcessor    {
    
  public:
     
    //! Default constructor
    EUTelMimoTelReader ();
     
    //! New processor
    /*! Return a new instance of a EUTelMimoTelReader. It is
     *  called by the Marlin execution framework and shouldn't be used
     *  by the final user.
     */
    virtual EUTelMimoTelReader * newProcessor ();
     
    //! Creates events from the eudaq software
    /*! This method reads a certain number of events from the input
     *  raw data file and generates LCIO events with at least three
     *  collections. This processor is very specific for the MimoTel
     *  setup, so it cannot be used in general to read the output of
     *  the EUDRB board. This limitation is due to the fact that the
     *  main goal of this data reader is to check the quality of the
     *  data producer and soon the LCIO output will be provided
     *  directly from the DAQ software.
     *
     *  @param numEvents The number of events to be read out.
     */
    virtual void readDataSource (int numEvents);

    //! Init method
    /*! It is called at the beginning of the cycle and it prints out
     *  the parameters.
     */
    virtual void init ();

    //! End method
    /*! It prints out a good bye message 
     */
    virtual void end ();

  protected:

    //! The input file name
    /*! It is set as a Marlin parameter in the constructor
     */ 
    std::string _fileName;

    //! The first frame collection name
    std::string _firstFrameCollectionName;

    //! The second frame collection name
    std::string _secondFrameCollectionName;

    //! The third frame collection name
    std::string _thirdFrameCollectionName;

    //! The CDS collection name
    std::string _cdsCollectionName;

    //! The CDS enable flag
    /*! This flag is one if the converter should perform also online
     *  CDS calculation. It is 0 otherwise.
     */
    int _cdsCalculation;

  };

  //! A global instance of the processor
  EUTelMimoTelReader gEUTelMimoTelReader;

}                               // end namespace eutelescope
#endif
#endif
