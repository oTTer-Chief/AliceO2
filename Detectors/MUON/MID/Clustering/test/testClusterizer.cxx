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

/// \file   MID/Clustering/test/testClusterizer.cxx
/// \brief  Test clustering device for MID
/// \author Diego Stocco <Diego.Stocco at cern.ch>
/// \date   08 March 2018

#define BOOST_TEST_MODULE midClustering
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
// Keep this separate or clang format will sort the include
// thus breaking compilation
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/monomorphic/generators/xrange.hpp>
#include <boost/test/data/test_case.hpp>
#include <cstdint>
#include <iostream>
#include <vector>
#include <random>
#include <gsl/gsl>
#include "DataFormatsMID/ColumnData.h"
#include "DataFormatsMID/Cluster.h"
#include "MIDClustering/PreClusterizer.h"
#include "MIDClustering/Clusterizer.h"

namespace o2
{
namespace mid
{

std::vector<ColumnData> getColumnsFixed(int event)
{
  std::vector<ColumnData> columns;
  switch (event) {
    case 0:
      columns.emplace_back(ColumnData());
      columns.back().deId = 31;
      columns.back().columnId = 1;
      columns.back().setNonBendPattern(1 << 4 | 1 << 5 | 1 << 7);
      columns.back().setBendPattern(1 << 14 | 1 << 15, 0);
      columns.back().setBendPattern(1 << 0 | 1 << 1, 1);
      columns.emplace_back(ColumnData());
      columns.back().deId = 31;
      columns.back().columnId = 2;
      columns.back().setNonBendPattern(1 << 0);
      columns.back().setBendPattern(1 << 0, 1);
      break;
    case 1:
      columns.emplace_back(ColumnData());
      columns.back().deId = 68;
      columns.back().columnId = 0;
      columns.back().setNonBendPattern(1 << 2 | 1 << 3 | 1 << 15);
      columns.back().setBendPattern(1 << 14 | 1 << 15, 2);
      columns.back().setBendPattern(1 << 0 | 1 << 1, 3);
      columns.emplace_back(ColumnData());
      columns.back().deId = 68;
      columns.back().columnId = 1;
      columns.back().setNonBendPattern(1 << 0);
      columns.back().setBendPattern(1 << 1, 3);
      break;
    case 2:
      columns.emplace_back(ColumnData());
      columns.back().deId = 31;
      columns.back().columnId = 1;
      columns.back().setNonBendPattern(1 << 7);
      columns.back().setBendPattern(1 << 14 | 1 << 15, 0);
      columns.back().setBendPattern(1 << 0 | 1 << 1, 1);
      columns.emplace_back(ColumnData());
      columns.back().deId = 31;
      columns.back().columnId = 2;
      columns.back().setNonBendPattern(1 << 0);
      columns.back().setBendPattern(1 << 14 | 1 << 15, 0);
      columns.back().setBendPattern(1 << 0 | 1 << 1, 1);
      break;
    default:
      std::cerr << "Event " << event << "not defined" << std::endl;
  }

  return columns;
}

std::vector<Cluster> getClusters(int event)
{
  std::vector<Cluster> clusters;
  Cluster clus;
  switch (event) {
    case 0:
      clus.deId = 31; // LegacyUtility::convertFromLegacyDeId(1400);
      clus.xCoor = -98.0417;
      clus.yCoor = -18.2403;
      clus.xErr = 1.3163814;
      clus.yErr = 1.3163814;
      clusters.push_back(clus);
      clus.deId = 31; // LegacyUtility::convertFromLegacyDeId(1400);
      clus.xCoor = -91.8856;
      clus.yCoor = -18.1263;
      clus.xErr = 1.1247177;
      clus.yErr = 1.2082798;
      clusters.push_back(clus);
      break;
    case 1:
      clus.deId = 68; // LegacyUtility::convertFromLegacyDeId(1408);
      clus.xCoor = -129.962;
      clus.yCoor = 18.2403;
      clus.xErr = 1.3163814;
      clus.yErr = 1.3163814;
      clusters.push_back(clus);
      clus.deId = 68; // LegacyUtility::convertFromLegacyDeId(1408);
      clus.xCoor = -101.006;
      clus.yCoor = 18.5823;
      clus.xErr = 1.1247177;
      clus.yErr = 1.3695948;
      clusters.push_back(clus);
      break;
    case 2:
      clus.deId = 31; // LegacyUtility::convertFromLegacyDeId(1400);
      clus.xCoor = -91.2016;
      clus.yCoor = -18.2403;
      clus.xErr = 1.3163814;
      clus.yErr = 1.3163814;
      clusters.push_back(clus);
      break;
    default:
      std::cerr << "Event " << event << "not defined" << std::endl;
  }

  return clusters;
}

bool areClustersEqual(const Cluster& cl1, const Cluster& cl2)
{
  int nBad = 0;
  float precision = 1.e-3;
  if (cl1.deId != cl2.deId) {
    std::cerr << "Id: " << static_cast<int>(cl1.deId) << " != " << static_cast<int>(cl2.deId) << std::endl;
    ++nBad;
  }

  if (std::abs(cl1.xCoor - cl2.xCoor) > precision) {
    std::cerr << "xCoor: " << cl1.xCoor << " != " << cl2.xCoor << std::endl;
    ++nBad;
  }

  if (std::abs(cl1.yCoor - cl2.yCoor) > precision) {
    std::cerr << "yCoor: " << cl1.yCoor << " != " << cl2.yCoor << std::endl;
    ++nBad;
  }

  if (std::abs(cl1.xErr - cl2.xErr) > precision) {
    std::cerr << "xErr: " << cl1.xErr << " != " << cl2.xErr << std::endl;
    ++nBad;
  }

  if (std::abs(cl1.yErr - cl2.yErr) > precision) {
    std::cerr << "yErr: " << cl1.yErr << " != " << cl2.yErr << std::endl;
    ++nBad;
  }

  return (nBad == 0);
}

class MyFixture
{
 public:
  MyFixture() : preClusterizer(), clusterizer(), mapping()
  {
    preClusterizer.init();
    clusterizer.init();
  }
  PreClusterizer preClusterizer;
  Clusterizer clusterizer;
  Mapping mapping;
};

BOOST_DATA_TEST_CASE_F(MyFixture, MID_Clustering_Fixed, boost::unit_test::data::xrange(3))
{
  preClusterizer.process(getColumnsFixed(sample));
  gsl::span<const PreCluster> preClusters(preClusterizer.getPreClusters().data(), preClusterizer.getPreClusters().size());

  clusterizer.process(preClusters);
  std::vector<Cluster> clusters = getClusters(sample);
  BOOST_TEST(clusters.size() == clusterizer.getClusters().size());
  size_t minNcl = clusters.size();
  if (clusterizer.getClusters().size() < minNcl) {
    minNcl = clusterizer.getClusters().size();
  }
  for (size_t icl = 0; icl < minNcl; ++icl) {
    BOOST_TEST(areClustersEqual(clusters[icl], clusterizer.getClusters()[icl]));
  }
}

bool isWithinUncertainties(float xPos, float yPos, const Cluster& cl)
{
  std::string str[2] = {"x", "y"};
  float inputPos[2] = {xPos, yPos};
  float recoPos[2] = {cl.xCoor, cl.yCoor};
  float err[2] = {cl.xErr, cl.yErr};
  bool isOk = true;
  for (int icoor = 0; icoor < 2; ++icoor) {
    if (err[icoor] > 5. || std::abs(recoPos[icoor] - inputPos[icoor]) > 5 * err[icoor]) {
      std::cerr << str[icoor] << " position input " << inputPos[icoor] << "  reco " << recoPos[icoor] << "  sigma "
                << err[icoor] << std::endl;
      isOk = false;
    }
  }
  return isOk;
}

std::vector<ColumnData> getFiredStrips(float xPos, float yPos, int deId, const Mapping& mapping)
{
  // This is a quite simple case just for testing purposes.
  // The fired strips are simply the fired strip itself + its neighbours.
  // However, in the bending plane, this also consists of strips with no overlap
  // with the non-bending plane
  std::vector<ColumnData> columns;
  for (int cathode = 1; cathode >= 0; --cathode) {
    Mapping::MpStripIndex stripIndex = mapping.stripByPosition(xPos, yPos, cathode, deId, false);
    if (!stripIndex.isValid()) {
      continue;
    }
    std::vector<Mapping::MpStripIndex> neighbours = mapping.getNeighbours(stripIndex, cathode, deId);
    neighbours.push_back(stripIndex);
    for (auto& neigh : neighbours) {
      ColumnData* columnStruct = nullptr;
      for (auto& currCol : columns) {
        if (currCol.columnId == neigh.column) {
          columnStruct = &currCol;
          break;
        }
      }
      if (!columnStruct) {
        if (cathode == 0) {
          // For the sake of simplicity we reject the neighbour bending strips
          // that do not overlap to the non-bending plane
          continue;
        }
        columns.emplace_back(ColumnData());
        columnStruct = &columns.back();
        columnStruct->deId = deId;
        columnStruct->columnId = neigh.column;
      }
      columnStruct->addStrip(neigh.strip, cathode, neigh.line);
    }
  }
  return columns;
}

BOOST_DATA_TEST_CASE_F(MyFixture, MID_Clustering_Random, boost::unit_test::data::xrange(72), deId)
{
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<float> distX(-127.5, 127.5);
  std::uniform_real_distribution<float> distY(-68., 68.);

  for (int ievt = 0; ievt < 1000; ++ievt) {
    float xPos = distX(mt);
    float yPos = distY(mt);
    std::vector<ColumnData> columns = getFiredStrips(xPos, yPos, deId, mapping);
    if (columns.size() == 0) {
      // Position was outside detection element
      continue;
    }
    preClusterizer.process(columns);
    gsl::span<const PreCluster> preClusters(preClusterizer.getPreClusters().data(), preClusterizer.getPreClusters().size());
    clusterizer.process(preClusters);
    BOOST_TEST(clusterizer.getClusters().size() == 1);

    for (const auto& clus : clusterizer.getClusters()) {
      BOOST_TEST(isWithinUncertainties(xPos, yPos, clus));
    }
  }
}

} // namespace mid
} // namespace o2
