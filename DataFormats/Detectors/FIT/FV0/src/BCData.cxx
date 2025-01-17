// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

#include "DataFormatsFV0/BCData.h"
#include "DataFormatsFV0/ChannelData.h"
#include <bitset>

using namespace o2::fv0;

void Triggers::printLog() const
{
  LOG(info) << "mTrigger: " << static_cast<uint16_t>(triggerSignals);
  LOG(info) << "nChanA: " << static_cast<uint16_t>(nChanA) /* << " | nChanC: " << static_cast<uint16_t>(nChanC)*/;
  LOG(info) << "amplA: " << amplA /* << " | amplC: " << amplC*/;
  //  LOG(info) << "timeA: " << timeA << " | timeC: " << timeC;
}

void BCData::print() const
{
  ir.print();
  printf("\n");
}

gsl::span<const ChannelData> BCData::getBunchChannelData(const gsl::span<const ChannelData> tfdata) const
{
  // extract the span of channel data for this bunch from the whole TF data
  return ref.getEntries() ? gsl::span<const ChannelData>(&tfdata[ref.getFirstEntry()], ref.getEntries()) : gsl::span<const ChannelData>();
}
void BCData::printLog() const
{
  LOG(info) << "______________DIGIT DATA____________";
  LOG(info) << "BC: " << ir.bc << "| ORBIT: " << ir.orbit;
  LOG(info) << "Ref first: " << ref.getFirstEntry() << "| Ref entries: " << ref.getEntries();
  mTriggers.printLog();
}
void TriggersExt::printLog() const
{
  LOG(info) << "______________EXTENDED TRIGGERS____________";
  LOG(info) << "BC: " << mIntRecord.bc << "| ORBIT: " << mIntRecord.orbit;
  for (int i = 0; i < 20; i++) {
    LOG(info) << "N: " << i + 1 << " | TRG: " << mTriggerWords[i];
  }
}