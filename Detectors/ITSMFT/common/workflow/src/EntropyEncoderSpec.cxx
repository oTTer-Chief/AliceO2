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

/// @file   EntropyEncoderSpec.cxx

#include <vector>

#include "Framework/ControlService.h"
#include "Framework/ConfigParamRegistry.h"
#include "DataFormatsITSMFT/CompCluster.h"
#include "ITSMFTWorkflow/EntropyEncoderSpec.h"
#include "DetectorsCommonDataFormats/DetID.h"

using namespace o2::framework;

namespace o2
{
namespace itsmft
{

EntropyEncoderSpec::EntropyEncoderSpec(o2::header::DataOrigin orig)
  : mOrigin(orig), mCTFCoder(orig == o2::header::gDataOriginITS ? o2::detectors::DetID::ITS : o2::detectors::DetID::MFT)
{
  assert(orig == o2::header::gDataOriginITS || orig == o2::header::gDataOriginMFT);
  mTimer.Stop();
  mTimer.Reset();
}

void EntropyEncoderSpec::init(o2::framework::InitContext& ic)
{
  std::string dictPath = ic.options().get<std::string>("ctf-dict");
  mCTFCoder.setMemMarginFactor(ic.options().get<float>("mem-factor"));
  if (!dictPath.empty() && dictPath != "none") {
    mCTFCoder.createCodersFromFile<CTF>(dictPath, o2::ctf::CTFCoderBase::OpType::Encoder);
  }
}

void EntropyEncoderSpec::run(ProcessingContext& pc)
{
  auto cput = mTimer.CpuTime();
  mTimer.Start(false);
  auto compClusters = pc.inputs().get<gsl::span<o2::itsmft::CompClusterExt>>("compClusters");
  auto pspan = pc.inputs().get<gsl::span<unsigned char>>("patterns");
  auto rofs = pc.inputs().get<gsl::span<o2::itsmft::ROFRecord>>("ROframes");

  auto& buffer = pc.outputs().make<std::vector<o2::ctf::BufferType>>(Output{mOrigin, "CTFDATA", 0, Lifetime::Timeframe});
  mCTFCoder.encode(buffer, rofs, compClusters, pspan);
  auto eeb = CTF::get(buffer.data()); // cast to container pointer
  eeb->compactify();                  // eliminate unnecessary padding
  buffer.resize(eeb->size());         // shrink buffer to strictly necessary size
  //  eeb->print();
  mTimer.Stop();
  LOG(info) << "Created encoded data of size " << eeb->size() << " for " << mOrigin.as<std::string>() << " in " << mTimer.CpuTime() - cput << " s";
}

void EntropyEncoderSpec::endOfStream(EndOfStreamContext& ec)
{
  LOGF(info, "%s Entropy Encoding total timing: Cpu: %.3e Real: %.3e s in %d slots",
       mOrigin.as<std::string>(), mTimer.CpuTime(), mTimer.RealTime(), mTimer.Counter() - 1);
}

DataProcessorSpec getEntropyEncoderSpec(o2::header::DataOrigin orig)
{
  std::vector<InputSpec> inputs;
  inputs.emplace_back("compClusters", orig, "COMPCLUSTERS", 0, Lifetime::Timeframe);
  inputs.emplace_back("patterns", orig, "PATTERNS", 0, Lifetime::Timeframe);
  inputs.emplace_back("ROframes", orig, "CLUSTERSROF", 0, Lifetime::Timeframe);

  return DataProcessorSpec{
    orig == o2::header::gDataOriginITS ? "its-entropy-encoder" : "mft-entropy-encoder",
    inputs,
    Outputs{{orig, "CTFDATA", 0, Lifetime::Timeframe}},
    AlgorithmSpec{adaptFromTask<EntropyEncoderSpec>(orig)},
    Options{{"ctf-dict", VariantType::String, o2::base::NameConf::getCTFDictFileName(), {"File of CTF encoding dictionary"}},
            {"mem-factor", VariantType::Float, 1.f, {"Memory allocation margin factor"}}}};
}

} // namespace itsmft
} // namespace o2
