// CSVRequestHandler.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004-2009 University Corporation for Atmospheric Research
// Author: Stephan Zednik <zednik@ucar.edu> and Patrick West <pwest@ucar.edu>
// and Jose Garcia <jgarcia@ucar.edu>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact University Corporation for Atmospheric Research at
// 3080 Center Green Drive, Boulder, CO 80301

// (c) COPYRIGHT University Corporation for Atmospheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//	zednik      Stephan Zednik <zednik@ucar.edu>
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#include "config.h"

#include <DDS.h>
#include <DAS.h>
#include <BaseTypeFactory.h>
#include <Ancillary.h>

#include <DMR.h>
#include <D4BaseTypeFactory.h>
#include <mime_util.h>
#include <InternalErr.h>

#include <BESDASResponse.h>
#include <BESDDSResponse.h>
#include <BESDataDDSResponse.h>
#include <BESDMRResponse.h>

#include <BESInfo.h>
#include <BESContainer.h>
#include <BESVersionInfo.h>
#include <BESDataNames.h>
#include <BESDapNames.h>
#include <BESResponseHandler.h>
#include <BESResponseNames.h>
#include <BESVersionInfo.h>
#include <BESTextInfo.h>
#include <BESConstraintFuncs.h>
#include <BESDapError.h>
#include <BESDebug.h>

#include "CSVDDS.h"
#include "CSVDAS.h"
#include "CSVRequestHandler.h"
// #include "CSVResponseNames.h"


CSVRequestHandler::CSVRequestHandler(string name) :
		BESRequestHandler(name)
{
	add_handler(DAS_RESPONSE, CSVRequestHandler::csv_build_das);
	add_handler(DDS_RESPONSE, CSVRequestHandler::csv_build_dds);
	add_handler(DATA_RESPONSE, CSVRequestHandler::csv_build_data);

	add_handler(DMR_RESPONSE, CSVRequestHandler::csv_build_dmr);

	add_handler(VERS_RESPONSE, CSVRequestHandler::csv_build_vers);
	add_handler(HELP_RESPONSE, CSVRequestHandler::csv_build_help);
}

CSVRequestHandler::~CSVRequestHandler()
{
}

bool CSVRequestHandler::csv_build_das(BESDataHandlerInterface &dhi)
{
	string error;
	bool ret = true;
	BESResponseObject *response = dhi.response_handler->get_response_object();
	BESDASResponse *bdas = dynamic_cast<BESDASResponse *>(response);
	DAS *das = 0;
	if (bdas)
		das = bdas->get_das();
	else
		throw BESInternalError("cast error", __FILE__, __LINE__);

	try {
		string accessed = dhi.container->access();
		csv_read_attributes(*das, accessed);
		Ancillary::read_ancillary_das(*das, accessed);
		return ret;
	}
	catch (InternalErr &e) {
		throw BESDapError(e.get_error_message(), true, e.get_error_code(), __FILE__, __LINE__);
	}
	catch (Error &e) {
		throw BESDapError(e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
	}
	catch (...) {
		throw BESDapError("Caught unknown error build CSV DAS response", true, unknown_error, __FILE__, __LINE__);
	}
}

bool CSVRequestHandler::csv_build_dds(BESDataHandlerInterface &dhi)
{
	bool ret = true;
	BESResponseObject *response = dhi.response_handler->get_response_object();
	BESDDSResponse *bdds = dynamic_cast<BESDDSResponse *>(response);
	DDS *dds = 0;
	if (bdds)
		dds = bdds->get_dds();
	else
		throw BESInternalError("cast error", __FILE__, __LINE__);

	BaseTypeFactory *factory = new BaseTypeFactory;
	dds->set_factory(factory);

	try {
		string accessed = dhi.container->access();
		dds->filename(accessed);
		csv_read_descriptors(*dds, accessed);
		Ancillary::read_ancillary_dds(*dds, accessed);

		DAS das;
		csv_read_attributes(das, accessed);
		Ancillary::read_ancillary_das(das, accessed);
		dds->transfer_attributes(&das);

		BESDEBUG( "csv", "dds = " << endl << *dds << endl );
		dhi.data[POST_CONSTRAINT] = dhi.container->get_constraint();

		return ret;
	}
	catch (InternalErr &e) {
		throw BESDapError(e.get_error_message(), true, e.get_error_code(), __FILE__, __LINE__);
	}
	catch (Error &e) {
		throw BESDapError(e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
	}
	catch (...) {
		throw BESDapError("Caught unknown error build CSV DDS response", true, unknown_error, __FILE__, __LINE__);
	}
}

bool CSVRequestHandler::csv_build_data(BESDataHandlerInterface &dhi)
{
	bool ret = true;
	BESResponseObject *response = dhi.response_handler->get_response_object();
	BESDataDDSResponse *bdds = dynamic_cast<BESDataDDSResponse *>(response);
	DataDDS *dds = 0;
	if (bdds)
		dds = bdds->get_dds();
	else
		throw BESInternalError("cast error", __FILE__, __LINE__);

	BaseTypeFactory *factory = new BaseTypeFactory;
	dds->set_factory(factory);

	try {
		string accessed = dhi.container->access();
		dds->filename(accessed);
		csv_read_descriptors(*dds, accessed);
		Ancillary::read_ancillary_dds(*dds, accessed);

		DAS das;
		csv_read_attributes(das, accessed);
		Ancillary::read_ancillary_das(das, accessed);
		dds->transfer_attributes(&das);

		BESDEBUG( "csv", "dds = " << endl << *dds << endl );
		dhi.data[POST_CONSTRAINT] = dhi.container->get_constraint();
		return ret;
	}
	catch (InternalErr &e) {
		throw BESDapError(e.get_error_message(), true, e.get_error_code(), __FILE__, __LINE__);
	}
	catch (Error &e) {
		throw BESDapError(e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
	}
	catch (...) {
		throw BESDapError("Caught unknown error build CSV DataDDS response", true, unknown_error, __FILE__, __LINE__);
	}
}

bool CSVRequestHandler::csv_build_dmr(BESDataHandlerInterface &dhi)
{
	// Because this code does not yet know how to build a DMR directly, use
	// the DMR ctor that builds a DMR using a 'full DDS' (a DDS with attributes).
	// First step, build the 'full DDS'
	string data_path = dhi.container->access();
	BaseTypeFactory factory;
	DDS dds(&factory, name_path(data_path), "3.2");
	dds.filename(data_path);

	try {
		csv_read_descriptors(dds, data_path);
		// ancillary DDS objects never took off - this does nothing. jhrg 8/12/14
		// Ancillary::read_ancillary_dds(*dds, data_path);

		DAS das;
		csv_read_attributes(das, data_path);
		Ancillary::read_ancillary_das(das, data_path);
		dds.transfer_attributes(&das);

		dhi.data[POST_CONSTRAINT] = dhi.container->get_constraint();
	}
	catch (InternalErr &e) {
		throw BESDapError(e.get_error_message(), true, e.get_error_code(), __FILE__, __LINE__);
	}
	catch (Error &e) {
		throw BESDapError(e.get_error_message(), false, e.get_error_code(), __FILE__, __LINE__);
	}
	catch (...) {
		throw BESDapError("Caught unknown error build CSV DataDDS response", true, unknown_error, __FILE__, __LINE__);
	}

	// Second step, make a DMR using the DDS
	DMR *built_dmr = new DMR(new D4BaseTypeFactory, dds);

	// Extract the DMR Response object - this holds the DMR used by the
	// other parts of the framework.
	BESResponseObject *response = dhi.response_handler->get_response_object();
	BESDMRResponse &bdmr = dynamic_cast<BESDMRResponse &>(*response);

	// Remove the existing DMR and set the newly made one
	DMR *dmr = bdmr.get_dmr();
	delete dmr->factory();
	delete dmr;
	bdmr.set_dmr(built_dmr);	// BESDMRResponse will delete this object

	return true;
}

bool CSVRequestHandler::csv_build_vers(BESDataHandlerInterface &dhi)
{
	bool ret = true;

	BESResponseObject *response = dhi.response_handler->get_response_object();
	BESVersionInfo *info = dynamic_cast<BESVersionInfo *>(response);
	if (!info) throw BESInternalError("cast error", __FILE__, __LINE__);

	info->add_module(PACKAGE_NAME, PACKAGE_VERSION);
	return ret;
}

bool CSVRequestHandler::csv_build_help(BESDataHandlerInterface &dhi)
{
	bool ret = true;
	BESInfo *info = dynamic_cast<BESInfo *>(dhi.response_handler->get_response_object());
	if (!info) throw BESInternalError("cast error", __FILE__, __LINE__);

	map<string, string> attrs;
	attrs["name"] = PACKAGE_NAME;
	attrs["version"] = PACKAGE_VERSION;
	string handles = (string) DAS_RESPONSE + "," + DDS_RESPONSE + "," + DATA_RESPONSE + "," + HELP_RESPONSE + ","
			+ VERS_RESPONSE;
	attrs["handles"] = handles;
	info->begin_tag("module", &attrs);
	info->end_tag("module");

	return ret;
}

void CSVRequestHandler::dump(ostream &strm) const
{
	strm << BESIndent::LMarg << "CSVRequestHandler::dump - (" << (void *) this << ")" << endl;
	BESIndent::Indent();
	BESRequestHandler::dump(strm);
	BESIndent::UnIndent();
}

