/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2016,  Regents of the University of California,
 *                      Colorado State University,
 *                      University Pierre & Marie Curie, Sorbonne University.
 *
 * This file is part of ndn-tools (Named Data Networking Essential Tools).
 * See AUTHORS.md for complete list of ndn-tools authors and contributors.
 *
 * ndn-tools is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndn-tools is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndn-tools, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 *
 * See AUTHORS.md for complete list of ndn-cxx authors and contributors.
 *
 * @author Wentao Shang
 * @author Steve DiBenedetto
 * @author Andrea Tosatto
 * @author Davide Pesavento
 * @author Weiwei Liu
 */

#include "ndncatchunks.hpp"
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include "../logging.h"
namespace ndn {
namespace chunks {

 int
 ndnChunks::startChunk(std::string ndnName, std::string pathName)
{

	Options options;
	std::string discoverType("fixed");
	std::string pipelineType("fixed");
	size_t maxPipelineSize(1);
	int maxRetriesAfterVersionFound(1);
	std::string uri = ndnName;


	std::string fileNameCOM = pathName + ndnName + "COM";
	std::string fileName = pathName + ndnName;
	uri = "/ndn/"+uri;
	Name prefix(uri);

	if (maxPipelineSize < 1 || maxPipelineSize > 1024) {
		std::cerr << "ERROR: pipeline size must be between 1 and 1024" << std::endl;
		return 2;
	}

	//options.interestLifetime = time::milliseconds(vm["lifetime"].as<uint64_t>());

	try {
		Face face;

		unique_ptr<DiscoverVersion> discover;
		//   if (discoverType == "fixed") {
		     discover = make_unique<DiscoverVersionFixed>(prefix, face, options);
		//   }
		//   else if (discoverType == "iterative") {
//		DiscoverVersionIterative::Options optionsIterative(options);
//		optionsIterative.maxRetriesAfterVersionFound = maxRetriesAfterVersionFound;
//		discover = make_unique<DiscoverVersionIterative>(prefix, face, optionsIterative);
		//  }
		//   else {
		//     std::cerr << "ERROR: discover version type not valid" << std::endl;
		//    return 2;
		// }

		     unique_ptr<PipelineInterests> pipeline;
		     if (pipelineType == "fixed") {
			     PipelineInterestsFixedWindow::Options optionsPipeline(options);
			     optionsPipeline.maxPipelineSize = maxPipelineSize;
			     pipeline = make_unique<PipelineInterestsFixedWindow>(face, optionsPipeline);
		     }
		     else {
			     std::cerr << "ERROR: Interest pipeline type not valid" << std::endl;
			     return 2;
		     }

		     std::ofstream m_outputStream;
		     //		//std::cerr << "file name is " + fileName << std::endl;
		     //		if(!m_outputStream.is_open())
		     //			m_outputStream.open (fileName, std::ios::out | std::ios::app );

		     ValidatorNull validator;
		     //Consumer consumer(validator, options.isVerbose, m_outputStream);
		     Consumer consumer(validator, options.isVerbose, fileNameCOM, m_outputStream);

		     BOOST_ASSERT(discover != nullptr);
		     BOOST_ASSERT(pipeline != nullptr);


 double mtime, seconds, useconds;
 struct   timeval a;
 struct   timeval b;
    gettimeofday(&a, 0);

//		     clock_t start = clock();    
		     consumer.run(std::move(discover), std::move(pipeline));
		     face.processEvents();
//		     clock_t stop = clock();    
//		     double elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;    

 gettimeofday(&b, 0);
    seconds  = (double)b.tv_sec  - (double)a.tv_sec;
    useconds = (double)b.tv_usec - (double)a.tv_usec;
    mtime = ((seconds) * 1000.0 + useconds/1000.0);

LogCvmfs(kLogCache, kLogDebug, "Elapsed time in ms:  %f", mtime);
		     printf("Time elapsed in ms: %f \n", mtime);

		     zlib::DecompressPath2Path(fileNameCOM,fileName);
		     printf(" .. done decompressing on consumer .. \n");    
		     
		     remove(fileNameCOM.c_str());
		     //m_outputStream.close();
	}
	catch (const Consumer::ApplicationNackError& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 3;
	}
	catch (const std::exception& e) {
		std::cerr << "ERROR: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}

} // namespace chunks
} // namespace ndn
