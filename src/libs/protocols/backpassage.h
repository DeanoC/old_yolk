/**
 @file	backpassages.h

 Declares the backpassage simple message class.
 */

#pragma once
#ifndef PROTOCOLS_BACKPASSAGE_H_
#define PROTOCOLS_BACKPASSAGE_H_


// heart beat returns encode a very simple message channel,
// so far a single byte (endian order independent)
// note must idempotic (as UDP) 
// should send an ACK (anything will do) as will repeat until ACK is recv unless NO_MESSAGE or not understood
#define BP_NO_MESSAGE	0
#define BP_RET_TCP_CHAN	1

#endif