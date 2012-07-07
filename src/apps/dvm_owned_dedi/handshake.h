#pragma once
#ifndef DWM_OWNED_DEDI_HANDSHAKE_H_
#define DWM_OWNED_DEDI_HANDSHAKE_H_

bool Handshake( boost::asio::io_service& io_service, const std::string& addr, int port );


#endif