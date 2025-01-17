#pragma once

#include <map>
#include <chrono>

#include <ndn-svs/svsync.hpp>

#include "config-bundle.hpp"

#define EXCLUDE_TIME_MS 10000

namespace kua {

class NodeWatcher
{
public:
  /** Initialize the node watcher with the sync prefix */
  NodeWatcher(ConfigBundle& configBundle);

  /** Get list of nodes with their base prefixes */
  std::vector<ndn::Name> getNodeList();

private:
  /** On SVS update */
  void updateCallback(const std::vector<ndn::svs::MissingDataInfo>& missingInfo);

  /** Transmit and schedule the next heartbeat message */
  void retxHeartbeat();

private:
  ndn::Name m_syncPrefix;
  ndn::Name m_nodePrefix;
  ndn::Face& m_face;
  ndn::Scheduler m_scheduler;
  ndn::KeyChain& m_keyChain;

  ndn::random::RandomNumberEngine& m_rng;
  std::uniform_int_distribution<> m_retxDist;

  std::map<ndn::Name, std::chrono::system_clock::time_point> m_nodeMap;

  std::unique_ptr<ndn::svs::SVSync> m_svs;
};

} // namespace kua
