#include "bidder.hpp"

#include <ndn-cxx/util/logger.hpp>

namespace kua {

NDN_LOG_INIT(kua.bidder);

Bidder::Bidder(ConfigBundle& configBundle, NodeWatcher& nodeWatcher)
  : m_configBundle(configBundle)
  , m_syncPrefix(ndn::Name(configBundle.kuaPrefix).append("sync").append("auction"))
  , m_nodePrefix(configBundle.nodePrefix)
  , m_face(configBundle.face)
  , m_scheduler(m_face.getIoService())
  , m_keyChain(configBundle.keyChain)
  , m_nodeWatcher(nodeWatcher)
  , m_rng(ndn::random::getRandomNumberEngine())
{
  NDN_LOG_INFO("Constructing Bidder");

  // Master's bidder does nothing
  if (m_configBundle.isMaster)
  {
    return;
  }

  // Initialize SVS
  m_svs = std::make_unique<ndn::svs::SVSync>(
    m_syncPrefix, m_nodePrefix, m_face, std::bind(&Bidder::updateCallback, this, _1));

  initialize();
}

void
Bidder::initialize()
{
  NDN_LOG_DEBUG("Initializing Bidder");
}

void
Bidder::updateCallback(const std::vector<ndn::svs::MissingDataInfo>& missingInfo)
{
  for (const auto m : missingInfo) {
    if (m.nodeId != ndn::Name(MASTER_PREFIX)) {
      continue;
    }

    for (ndn::svs::SeqNo i = m.low; i <= m.high; i++) {
      m_svs->fetchData(m.nodeId, i, std::bind(&Bidder::processMasterMessage, this, _1));
    }
  }
}

void
Bidder::processMasterMessage(const ndn::Data& data)
{
  ndn::Name msg(data.getContent().blockFromValue());
  NDN_LOG_TRACE("RECV_MASTER_MSG=" << msg);
  auto type = msg.get(0).toUri();

  if (type == "AUCTION") {
    unsigned int bucketId = msg.get(1).toNumber();
    unsigned int auctionId = msg.get(2).toNumber();
    NDN_LOG_DEBUG("RECV_AUCTION for #" << bucketId << " AID " << auctionId);
    placeBid(bucketId, auctionId);
  }
}

void
Bidder::placeBid(unsigned int bucketId, unsigned int auctionId)
{
  auto bidAmount = m_rng();

  ndn::Name bidInfo;
  bidInfo.append("BID");
  bidInfo.appendNumber(bucketId);
  bidInfo.appendNumber(auctionId);
  bidInfo.appendNumber(bidAmount);

  NDN_LOG_DEBUG("PLACE_BID for #" << bucketId << " AID " << auctionId << " $" << bidAmount);
  m_svs->publishData(bidInfo.wireEncode(), ndn::time::milliseconds(1000));
}

} // namespace kua