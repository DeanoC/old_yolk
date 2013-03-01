/*
 * dwm.h
 *
 *  Created on: 15 Apr 2012
 *      Author: deanoc
 */
#pragma once
#ifndef YOLK_DWM_DWM_H_
#define YOLK_DWM_DWM_H_

#include "core/core.h"
#include "core/file_path.h"
#include "riak/core_types.hxx"
#include "riak/transport.hxx"
#include "dwm/world.h"
#include <boost/asio.hpp>

class VMThreads;

// todo create the two seperate libs, for now trust everyone
#define DWM_TRUSTED

#if defined( DWM_TRUSTED )
typedef std::shared_ptr<riak::object> RiakObjPtr;
#endif

class Dwm {
public:
	friend class VMThreads;
	
	Dwm();
	Dwm( WorldPtr _world );
	~Dwm();

	bool openCommChans( std::shared_ptr<boost::asio::io_service> _io, const std::string& hostname );

	void bootstrapLocal();

	WorldPtr getWorld() const { return world; }

private:
	void checkSysInfoVersion( const std::string& str );

#if defined( DWM_TRUSTED )
	std::string												riakAddr;
	int														riakPort;
	riak::transport::delivery_provider						riakConn;
#endif
	std::string												dwmChanAddr;
	int														dwmChanPort;
	std::shared_ptr<boost::asio::ip::tcp::socket>			dwmChanSock;
	std::shared_ptr<boost::asio::io_service>				io;
	std::vector<std::shared_ptr<VMThreads>>					vmThreads;

	std::string												switcherElf;
	std::string												libcElf;
	WorldPtr												world;
};


#endif /* DWM_H_ */
