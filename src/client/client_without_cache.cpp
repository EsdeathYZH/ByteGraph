#include "client_without_cache.h"

#include <glog/logging.h>

#include <unordered_map>

#include "rpc_client.h"

using std::string;

using namespace ::ByteGraph;
using namespace ::ByteCamp;

ClientWithoutCache::ClientWithoutCache(const std::vector<std::pair<std::string, int>> &serverAddresses) {
    rpc_clients_.reserve(serverAddresses.size());
    for (const auto &server : serverAddresses) {
        auto rpc_client = std::make_shared<RpcClient>(server.first, server.second);
        rpc_clients_.emplace_back(rpc_client);
    }
    assert(rpc_clients_.size() == serverAddresses.size());
}

void ClientWithoutCache::GetFullGraphInfo(ByteGraph::GraphInfo &graphInfo) {
    rpc_clients_[0]->GetFullGraphInfo(graphInfo);
}

void ClientWithoutCache::SampleBatchNodes(const ByteGraph::NodeType &type, const int32_t &batchSize,
                                          const ByteGraph::SampleStrategy::type &sampleStrategy,
                                          ByteGraph::BatchNodes &batchNodes) {
    batchNodes.node_ids.reserve(batchSize);
    if (sampleStrategy == SampleStrategy::RANDOM) {
        int32_t size = rpc_clients_.size();
        int32_t avg = batchSize / size, last = batchSize - (avg * (size - 1));
        for (size_t i = 0; i < size; ++i) {
            ByteGraph::BatchNodes tmpBatchNodes;
            if (i == size - 1) {
                rpc_clients_[i]->SampleBatchNodes(type, last, sampleStrategy, tmpBatchNodes);
            } else {
                rpc_clients_[i]->SampleBatchNodes(type, avg, sampleStrategy, tmpBatchNodes);
            }
            batchNodes.node_ids.insert(batchNodes.node_ids.end(), tmpBatchNodes.node_ids.begin(),
                                       tmpBatchNodes.node_ids.end());
        }
    } else {
    }
}

void ClientWithoutCache::GetNodeFeature(const std::vector<ByteGraph::NodeId> &nodes,
                                        const ByteGraph::FeatureType &featureType,
                                        ByteGraph::NodesFeature &nodeFeature) {
    nodeFeature.resize(nodes.size());
    const auto rpc_client_size = rpc_clients_.size();
    std::vector<std::vector<NodeId>> rpc_clients_nodes(rpc_client_size);
    size_t index = 0;
    std::unordered_map<NodeId, size_t> nodeId2Index;
    for (const auto &node : nodes) {
        // nodeId map to nodeFeature index
        nodeId2Index[node] = index++;
        rpc_clients_nodes[node % rpc_client_size].push_back(node);
    }
# ifdef DEBUG
    size_t sum = 0;
    for (const auto& rpc_clients_node : rpc_clients_nodes) {
        sum += rpc_clients_node.size();
    }
    assert(nodes.size() == sum);
#endif
    assert(index == nodes.size());
    for (size_t i = 0; i < rpc_client_size; ++i) {
        NodesFeature tmpNodesFeature;
        rpc_clients_[i]->GetBatchNodeFeature(rpc_clients_nodes[i], featureType, tmpNodesFeature);
        assert(tmpNodesFeature.size() == rpc_clients_nodes[i].size());
        for (size_t j = 0; j < tmpNodesFeature.size(); ++j) {
            nodeFeature[nodeId2Index[rpc_clients_nodes[i][j]]] = std::move(tmpNodesFeature[j]);
        }
    }
}

void ClientWithoutCache::GetNeighborsWithFeature(const ByteGraph::NodeId &nodeId,
                                                 const ByteGraph::EdgeType &neighborType,
                                                 const ByteGraph::FeatureType &featureType,
                                                 std::vector<ByteGraph::IDFeaturePair> &neighbors) {
    rpc_clients_[nodeId % (rpc_clients_.size())]->GetNeighborsWithFeature(nodeId, neighborType, featureType, neighbors);
}

void ClientWithoutCache::SampleNeighbor(const int32_t &batchSize, const ByteGraph::NodeType &nodeType,
                                        const ByteGraph::NodeType &neighborType, const int32_t &sampleNum,
                                        std::vector<ByteGraph::IDNeighborPair> &neighbors) {
    //    rpc_client_->SampleNeighbor(batchSize, nodeType, neighborType, sampleNum, neighbors);
}

void ClientWithoutCache::RandomWalk(const int32_t &batchSize, const int32_t &walkLen,
                                    std::vector<ByteGraph::NodeId> &nodes) {
    //    rpc_client_->RandomWalk(batchSize, walkLen, nodes);
}
